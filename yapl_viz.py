#!/usr/bin/env python3
"""
yapl-viz — YAPL .yaplcache bytecode visualizer
Usage: python3 yapl_viz.py <file.yaplcache> [output.html]
       (output defaults to <file>.html, opened in browser automatically)
"""

import struct, sys, json, os, webbrowser
from dataclasses import dataclass, field
from typing import Optional, List

# ─── Enums ─────────────────────────────────────────────────────────────────────

VALUE_TYPE_NAMES = {
    0: "NONE", 1: "INTEGER", 2: "FLOAT", 3: "BOOL", 4: "STRING",
    5: "ARRAY", 6: "TYPE", 7: "DICT", 8: "USER_DEFINED", 9: "FUNCTION",
    10: "BUILTIN_FUNCTION", 11: "ITERATOR", 12: "UNDEFINED", 13: "CODE_OBJECT",
}

BINARY_OPS  = {0:"ADD",1:"SUB",2:"MUL",3:"MOD",4:"DIV",5:"EQ",6:"NEQ",7:"LT",8:"LQ",9:"GT",10:"GQ",11:"OR",12:"AND"}
UNARY_OPS   = {0:"POS", 1:"NEG", 2:"NOT"}
PARAM_KINDS = {0: "positional_or_keyword", 1: "keyword_only"}

# opcode value → (mnemonic, [operand_kinds])
# kinds: "const" "local" "name" "binary_op" "unary_op" "jmp" "count"
OPCODES = {
    0:  ("NOP",            []),
    1:  ("RETURN",         []),
    2:  ("HALT",           []),
    3:  ("LOAD_CONST",     ["const"]),
    4:  ("LOAD_UNDEF",     []),
    5:  ("MAKE_ARR",       ["count"]),
    6:  ("MAKE_TYPE",      ["name", "count"]),
    7:  ("MAKE_FUNC",      []),
    8:  ("KW_CALL",        ["count", "count"]),
    9:  ("CALL",           ["count"]),
    10: ("CALL_METHOD",    ["count"]),
    11: ("KW_CALL_METHOD", ["count", "count"]),
    12: ("GET_PROPERTY",   ["name"]),
    13: ("SET_PROPERTY",   ["name"]),
    14: ("INIT_VAR",       ["local"]),
    15: ("DEINIT_VAR",     ["local"]),
    16: ("LOAD_LOCAL",     ["local"]),
    17: ("STORE_LOCAL",    ["local"]),
    18: ("LOAD_NAME",      ["name"]),
    19: ("STORE_NAME",     ["name"]),
    20: ("UNARY_OP",       ["unary_op"]),
    21: ("BINARY_OP",      ["binary_op"]),
    22: ("JMP",            ["jmp"]),
    23: ("JMP_IF_FALSE",   ["jmp"]),
}

# ─── Data classes ──────────────────────────────────────────────────────────────

@dataclass
class Parameter:
    name: str
    kind: int
    has_default: bool
    default_const_index: int
    local_index: int

@dataclass
class Constant:
    index: int
    type_id: int
    type_name: str
    module: str
    display: str
    code_object_id: Optional[int] = None

@dataclass
class Instruction:
    offset: int
    opcode_val: int
    opcode_name: str
    operands: List[int]
    annotations: List[str]

@dataclass
class CodeObject:
    id: int
    name: str
    version: int
    names: List[str]
    locals: List[str]
    params: List[Parameter]
    constants: List[Constant]
    instructions: List[Instruction]
    parent_id: Optional[int] = None
    children_ids: List[int] = field(default_factory=list)

# ─── Parser ────────────────────────────────────────────────────────────────────

class ParseError(Exception): pass

class Parser:
    def __init__(self, data: bytes):
        self.data = data
        self.pos  = 0
        self._id  = 0
        self.all: List[CodeObject] = []

    # ── primitives ──
    def u8(self) -> int:
        if self.pos >= len(self.data): raise ParseError(f"EOF reading u8 at {self.pos}")
        v = self.data[self.pos]; self.pos += 1; return v

    def u16(self) -> int:
        if self.pos + 2 > len(self.data): raise ParseError(f"EOF reading u16 at {self.pos}")
        v = struct.unpack_from('<H', self.data, self.pos)[0]; self.pos += 2; return v

    def u32(self) -> int:
        if self.pos + 4 > len(self.data): raise ParseError(f"EOF reading u32 at {self.pos}")
        v = struct.unpack_from('<I', self.data, self.pos)[0]; self.pos += 4; return v

    def string(self, n: int) -> str:
        if self.pos + n > len(self.data): raise ParseError(f"EOF reading string({n}) at {self.pos}")
        s = self.data[self.pos:self.pos+n].decode('utf-8', errors='replace'); self.pos += n; return s

    def string_entry(self) -> str:
        return self.string(self.u16())

    # ── structured items ──
    def parse_param(self) -> Parameter:
        name             = self.string(self.u16())
        kind             = self.u8()
        has_default      = bool(self.u8())
        default_ci       = self.u16()
        local_index      = self.u16()
        return Parameter(name, kind, has_default, default_ci, local_index)

    def parse_value(self, index: int) -> Constant:
        type_id   = self.u8()
        type_name = VALUE_TYPE_NAMES.get(type_id, f"UNKNOWN({type_id})")

        if type_id == 3:  # BOOL — value byte precedes module
            val    = self.u8()
            mod_ln = self.u16()
            module = self.string(mod_ln) if mod_ln else ""
            return Constant(index, type_id, type_name, module, "true" if val else "false")

        # All other types: module first
        mod_ln = self.u16()
        module = self.string(mod_ln) if mod_ln else ""

        if type_id == 1:   # INTEGER (or old CODE_OBJECT = 1)
            saved_pos = self.pos
            _bw = self.u16()
            if _bw == 32:
                val = self.u32()
                return Constant(index, type_id, "INTEGER", module, str(val))
            else:
                # bit-width is not 32 → this is actually a CODE_OBJECT
                # with an older type-byte encoding (CODE_OBJECT=1 in old enum).
                # Rewind and parse the embedded CodeObject.
                self.pos = saved_pos
                child = self.parse_code_object()
                return Constant(index, 13, "CODE_OBJECT", module,
                                f"<CodeObject '{child.name}'>", code_object_id=child.id)

        elif type_id == 2: # FLOAT
            _bw  = self.u16()
            raw  = self.u32()
            fval = struct.unpack('<f', struct.pack('<I', raw))[0]
            return Constant(index, type_id, type_name, module, f"{fval:.6g}")

        elif type_id == 4: # STRING
            vlen = self.u16()
            val  = self.string(vlen)
            return Constant(index, type_id, type_name, module, json.dumps(val))

        elif type_id == 13: # CODE_OBJECT
            child = self.parse_code_object()
            return Constant(index, type_id, type_name, module,
                            f"<CodeObject '{child.name}'>", code_object_id=child.id)

        else:
            return Constant(index, type_id, type_name, module, f"<{type_name}>")

    def _disassemble(self, raw: List[int], co: CodeObject) -> List[Instruction]:
        out, ip, n = [], 0, len(raw)
        while ip < n:
            offset    = ip
            opcode    = raw[ip]; ip += 1
            info      = OPCODES.get(opcode)
            if info is None:
                out.append(Instruction(offset, opcode, f"UNKNOWN({opcode})", [], []))
                continue
            mnemonic, kinds = info
            operands, annots = [], []
            for kind in kinds:
                if ip >= n: break
                op = raw[ip]; ip += 1
                operands.append(op)
                if kind == "const":
                    annots.append(f"({co.constants[op].display})" if op < len(co.constants) else "")
                elif kind == "local":
                    annots.append(f"({co.locals[op]})"           if op < len(co.locals)     else "")
                elif kind == "name":
                    annots.append(f"({co.names[op]})"            if op < len(co.names)      else "")
                elif kind == "binary_op":
                    annots.append(f"({BINARY_OPS.get(op,'?')})")
                elif kind == "unary_op":
                    annots.append(f"({UNARY_OPS.get(op,'?')})")
                elif kind == "jmp":
                    annots.append(f"→{op}")
                else:
                    annots.append("")
            out.append(Instruction(offset, opcode, mnemonic, operands, annots))
        return out

    def parse_code_object(self, parent_id: Optional[int] = None) -> CodeObject:
        co_id   = self._id; self._id += 1
        version = self.u8()
        name    = self.string(self.u16())

        names_n  = self.u16()
        names    = [self.string_entry() for _ in range(names_n)]

        locals_n = self.u16()
        locals_  = [self.string_entry() for _ in range(locals_n)]

        params_n = self.u16()
        params   = [self.parse_param() for _ in range(params_n)]

        # Placeholder object added early so children can reference parent
        co = CodeObject(co_id, name, version, names, locals_, params, [], [], parent_id)
        self.all.append(co)

        consts_n = self.u16()
        for i in range(consts_n):
            c = self.parse_value(i)
            co.constants.append(c)
            if c.code_object_id is not None:
                co.children_ids.append(c.code_object_id)
                for obj in self.all:
                    if obj.id == c.code_object_id:
                        obj.parent_id = co_id

        ops_n = self.u16()
        raw   = [self.u32() for _ in range(ops_n)]
        co.instructions = self._disassemble(raw, co)
        return co

    def parse(self):
        root = self.parse_code_object()
        return root, self.all


# ─── Serialise to JSON-friendly dicts ──────────────────────────────────────────

def obj_to_dict(co: CodeObject) -> dict:
    return {
        "id":       co.id,
        "name":     co.name,
        "version":  co.version,
        "parent":   co.parent_id,
        "children": co.children_ids,
        "names":    co.names,
        "locals":   co.locals,
        "params": [
            {"name": p.name, "kind": PARAM_KINDS.get(p.kind, str(p.kind)),
             "has_default": p.has_default,
             "default_const_index": p.default_const_index,
             "local_index": p.local_index}
            for p in co.params
        ],
        "constants": [
            {"index": c.index, "type": c.type_name, "module": c.module,
             "display": c.display, "co_id": c.code_object_id}
            for c in co.constants
        ],
        "instructions": [
            {"offset": i.offset, "opcode": i.opcode_val, "name": i.opcode_name,
             "operands": i.operands, "annotations": i.annotations}
            for i in co.instructions
        ],
    }


# ─── HTML Generator ────────────────────────────────────────────────────────────

def generate_html(filename: str, all_objects: List[CodeObject]) -> str:
    data_json = json.dumps([obj_to_dict(o) for o in all_objects], indent=2)
    basename  = os.path.basename(filename)

    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>yapl-viz · {basename}</title>
<link rel="preconnect" href="https://fonts.googleapis.com"/>
<link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;600;700&family=Syne:wght@700;800&display=swap" rel="stylesheet"/>
<style>
  *, *::before, *::after {{ box-sizing: border-box; margin: 0; padding: 0; }}

  :root {{
    --bg:       #080b12;
    --surface:  #0f1420;
    --card:     #161c2e;
    --border:   #1e2840;
    --accent:   #3de8a0;
    --accent2:  #f7b731;
    --dim:      #4a5880;
    --text:     #cdd6f4;
    --muted:    #6c7ba8;
    --danger:   #f38ba8;
    --keyword:  #89dceb;
    --number:   #f9e2af;
    --string:   #a6e3a1;
    --comment:  #585b70;
  }}

  html, body {{ height: 100%; overflow: hidden; }}

  body {{
    background: var(--bg);
    color: var(--text);
    font-family: 'JetBrains Mono', monospace;
    font-size: 13px;
    display: flex;
    flex-direction: column;
  }}

  /* ── Header ── */
  header {{
    display: flex;
    align-items: center;
    gap: 16px;
    padding: 0 20px;
    height: 48px;
    background: var(--surface);
    border-bottom: 1px solid var(--border);
    flex-shrink: 0;
  }}
  header .logo {{
    font-family: 'Syne', sans-serif;
    font-weight: 800;
    font-size: 15px;
    color: var(--accent);
    letter-spacing: -0.5px;
  }}
  header .sep {{ color: var(--dim); }}
  header .fname {{ color: var(--muted); font-size: 12px; }}
  header .stats {{ margin-left: auto; color: var(--dim); font-size: 11px; }}

  /* ── Layout ── */
  .main {{
    display: flex;
    flex: 1;
    overflow: hidden;
  }}

  /* ── Graph Panel ── */
  .graph-panel {{
    width: 300px;
    min-width: 160px;
    max-width: 70vw;
    flex-shrink: 0;
    border-right: 1px solid var(--border);
    background: var(--bg);
    display: flex;
    flex-direction: column;
    overflow: hidden;
    position: relative;
  }}
  .panel-label {{
    padding: 10px 16px 8px;
    font-size: 10px;
    text-transform: uppercase;
    letter-spacing: 1.5px;
    color: var(--dim);
    border-bottom: 1px solid var(--border);
    flex-shrink: 0;
    user-select: none;
  }}
  #graph-svg {{
    flex: 1;
    width: 100%;
    height: 100%;
    cursor: grab;
    overflow: hidden;
    display: block;
  }}
  #graph-svg.dragging {{ cursor: grabbing; }}

  /* ── Splitter ── */
  .splitter {{
    width: 5px;
    flex-shrink: 0;
    background: transparent;
    cursor: col-resize;
    position: relative;
    z-index: 20;
    transition: background 0.15s;
  }}
  .splitter:hover, .splitter.dragging {{ background: var(--accent); opacity: 0.5; }}

  /* ── Zoom controls ── */
  .zoom-controls {{
    position: absolute;
    bottom: 10px;
    right: 10px;
    display: flex;
    flex-direction: column;
    gap: 4px;
    z-index: 10;
  }}
  .zoom-btn {{
    width: 26px;
    height: 26px;
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 5px;
    color: var(--muted);
    font-size: 15px;
    line-height: 1;
    cursor: pointer;
    display: flex; align-items: center; justify-content: center;
    transition: color 0.1s, border-color 0.1s;
    user-select: none;
  }}
  .zoom-btn:hover {{ color: var(--accent); border-color: var(--accent); }}

  .node rect {{
    rx: 8;
    ry: 8;
    cursor: pointer;
    transition: filter 0.15s;
  }}
  .node rect:hover {{ filter: brightness(1.25); }}
  .node.selected rect {{ filter: brightness(1.4); stroke: var(--accent) !important; stroke-width: 2px; }}
  .node-title {{ font-family: 'JetBrains Mono', monospace; font-weight: 700; font-size: 12px; }}
  .node-sub   {{ font-family: 'JetBrains Mono', monospace; font-size: 10px; }}

  .edge {{ fill: none; stroke: var(--dim); stroke-width: 1.5px; }}
  .edge-label {{ font-size: 9px; fill: var(--dim); font-family: 'JetBrains Mono', monospace; }}

  /* ── Detail Panel ── */
  .detail-panel {{
    flex: 1;
    overflow-y: auto;
    background: var(--bg);
  }}
  .detail-panel::-webkit-scrollbar {{ width: 6px; }}
  .detail-panel::-webkit-scrollbar-track {{ background: transparent; }}
  .detail-panel::-webkit-scrollbar-thumb {{ background: var(--border); border-radius: 3px; }}

  .detail-header {{
    position: sticky;
    top: 0;
    z-index: 10;
    background: var(--surface);
    border-bottom: 1px solid var(--border);
    padding: 16px 24px 12px;
  }}
  .detail-header .co-name {{
    font-family: 'Syne', sans-serif;
    font-size: 20px;
    font-weight: 800;
    color: var(--accent);
  }}
  .detail-header .co-meta {{
    margin-top: 4px;
    color: var(--muted);
    font-size: 11px;
  }}
  .detail-header .breadcrumb {{
    margin-top: 6px;
    font-size: 10px;
    color: var(--dim);
  }}
  .breadcrumb .crumb {{ color: var(--muted); cursor: pointer; }}
  .breadcrumb .crumb:hover {{ color: var(--accent); }}

  .section {{
    padding: 16px 24px;
    border-bottom: 1px solid var(--border);
  }}
  .section-title {{
    font-size: 10px;
    text-transform: uppercase;
    letter-spacing: 1.5px;
    color: var(--dim);
    margin-bottom: 12px;
  }}

  /* ── Fields grid ── */
  .fields {{
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
    gap: 8px;
  }}
  .field {{
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 6px;
    padding: 8px 10px;
  }}
  .field-key   {{ font-size: 9px; text-transform: uppercase; letter-spacing: 1px; color: var(--dim); margin-bottom: 3px; }}
  .field-value {{ font-size: 13px; color: var(--text); font-weight: 600; }}

  /* ── Tag list ── */
  .tag-list {{ display: flex; flex-wrap: wrap; gap: 5px; }}
  .tag {{
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 4px;
    padding: 2px 8px;
    font-size: 11px;
    color: var(--text);
  }}
  .tag .tag-idx {{ color: var(--dim); margin-right: 4px; }}
  .empty {{ color: var(--dim); font-size: 11px; font-style: italic; }}

  /* ── Constants table ── */
  .const-table {{ width: 100%; border-collapse: collapse; }}
  .const-table th {{
    text-align: left;
    font-size: 9px;
    text-transform: uppercase;
    letter-spacing: 1px;
    color: var(--dim);
    padding: 0 8px 6px 0;
    border-bottom: 1px solid var(--border);
  }}
  .const-table td {{
    padding: 5px 8px 5px 0;
    font-size: 12px;
    border-bottom: 1px solid var(--border);
    vertical-align: top;
  }}
  .const-table tr:last-child td {{ border-bottom: none; }}
  .ci {{ color: var(--dim); width: 28px; }}
  .ct {{ color: var(--accent2); width: 80px; }}
  .cv {{ color: var(--text); }}
  .cv.is-co {{ color: var(--accent); cursor: pointer; text-decoration: underline; text-underline-offset: 2px; }}
  .cm {{ color: var(--muted); font-size: 10px; }}

  /* ── Params table ── */
  .param-table {{ width: 100%; border-collapse: collapse; }}
  .param-table th {{
    text-align: left; font-size: 9px; text-transform: uppercase;
    letter-spacing: 1px; color: var(--dim); padding: 0 10px 6px 0;
    border-bottom: 1px solid var(--border);
  }}
  .param-table td {{
    padding: 5px 10px 5px 0;
    font-size: 12px;
    border-bottom: 1px solid var(--border);
  }}
  .param-table tr:last-child td {{ border-bottom: none; }}
  .pk {{ color: var(--muted); font-size: 10px; }}
  .pd {{ color: var(--accent); }}

  /* ── Disassembly ── */
  .disasm {{
    font-family: 'JetBrains Mono', monospace;
    font-size: 12px;
    line-height: 1.7;
  }}
  .instr {{ display: flex; gap: 0; align-items: baseline; border-radius: 3px; }}
  .instr:hover {{ background: var(--card); }}
  .instr.is-jmp-target {{ background: rgba(61, 232, 160, 0.06); border-left: 2px solid var(--accent); padding-left: 4px; }}
  .i-off  {{ color: var(--dim);     min-width: 36px; flex-shrink: 0; }}
  .i-op   {{ color: var(--keyword); min-width: 160px; flex-shrink: 0; font-weight: 600; }}
  .i-arg  {{ color: var(--number);  min-width: 28px; }}
  .i-ann  {{ color: var(--string);  font-size: 11px; }}
  .i-ann.is-jmp {{ color: var(--accent2); }}

  /* ── No selection ── */
  .no-selection {{
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    height: 100%;
    color: var(--dim);
    gap: 8px;
  }}
  .no-selection .ns-icon {{ font-size: 32px; opacity: 0.4; }}
  .no-selection .ns-text {{ font-size: 12px; }}

  .badge {{
    display: inline-block;
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 999px;
    padding: 1px 7px;
    font-size: 10px;
    color: var(--muted);
    vertical-align: middle;
    margin-left: 6px;
  }}
</style>
</head>
<body>
<header>
  <span class="logo">yapl-viz</span>
  <span class="sep">·</span>
  <span class="fname">{basename}</span>
  <span class="stats" id="hdr-stats"></span>
</header>
<div class="main">
  <div class="graph-panel" id="graph-panel">
    <div class="panel-label">Object Graph</div>
    <svg id="graph-svg" xmlns="http://www.w3.org/2000/svg">
      <g id="svg-root"></g>
    </svg>
    <div class="zoom-controls">
      <div class="zoom-btn" id="btn-zoom-in"  title="Zoom in">+</div>
      <div class="zoom-btn" id="btn-zoom-fit" title="Fit">⊡</div>
      <div class="zoom-btn" id="btn-zoom-out" title="Zoom out">−</div>
    </div>
  </div>
  <div class="splitter" id="splitter"></div>
  <div class="detail-panel" id="detail">
    <div class="no-selection">
      <div class="ns-icon">⬡</div>
      <div class="ns-text">Select a code object in the graph</div>
    </div>
  </div>
</div>

<script>
// ── Data ──────────────────────────────────────────────────────────────────────
const ALL_OBJECTS = {data_json};

const byId = Object.fromEntries(ALL_OBJECTS.map(o => [o.id, o]));
let selectedId = null;

// ── Header stats ──────────────────────────────────────────────────────────────
document.getElementById('hdr-stats').textContent =
  `${{ALL_OBJECTS.length}} code object${{ALL_OBJECTS.length !== 1 ? 's' : ''}}`;

// ── Splitter (resize graph panel) ────────────────────────────────────────────
(function() {{
  const splitter = document.getElementById('splitter');
  const panel    = document.getElementById('graph-panel');
  let dragging = false, startX = 0, startW = 0;

  splitter.addEventListener('mousedown', e => {{
    dragging = true;
    startX = e.clientX;
    startW = panel.offsetWidth;
    splitter.classList.add('dragging');
    document.body.style.userSelect = 'none';
    document.body.style.cursor = 'col-resize';
  }});
  window.addEventListener('mousemove', e => {{
    if (!dragging) return;
    const w = Math.max(160, Math.min(window.innerWidth * 0.7, startW + e.clientX - startX));
    panel.style.width = w + 'px';
  }});
  window.addEventListener('mouseup', () => {{
    if (!dragging) return;
    dragging = false;
    splitter.classList.remove('dragging');
    document.body.style.userSelect = '';
    document.body.style.cursor = '';
  }});
}})();

// ── Graph layout (top-down tree) ──────────────────────────────────────────────
const NODE_W  = 180;
const NODE_H  = 54;
const H_GAP   = 40;
const V_GAP   = 80;

// Pan/zoom state
let vx = 0, vy = 0, vscale = 1;
let graphW = 0, graphH = 0;

function applyTransform() {{
  document.getElementById('svg-root').setAttribute(
    'transform', `translate(${{vx}},${{vy}}) scale(${{vscale}})`
  );
}}

function fitGraph() {{
  const svg = document.getElementById('graph-svg');
  const pw   = svg.clientWidth  || 300;
  const ph   = svg.clientHeight || 400;
  const pad  = 24;
  const sx   = (pw - pad * 2) / graphW;
  const sy   = (ph - pad * 2) / graphH;
  vscale = Math.min(sx, sy, 1);
  vx = (pw - graphW * vscale) / 2;
  vy = pad;
  applyTransform();
}}

function subtreeWidth(id) {{
  const co = byId[id];
  if (!co.children.length) return NODE_W;
  const childSum = co.children.reduce((s, cid) => s + subtreeWidth(cid) + H_GAP, -H_GAP);
  return Math.max(NODE_W, childSum);
}}

function assignPositions(id, x, y, positions) {{
  const co = byId[id];
  positions[id] = {{ x, y }};
  if (!co.children.length) return;
  const childWidths = co.children.map(c => subtreeWidth(c));
  const totalW      = childWidths.reduce((s, w) => s + w + H_GAP, -H_GAP);
  let cx = x - totalW / 2;
  co.children.forEach((cid, i) => {{
    const cw = childWidths[i];
    assignPositions(cid, cx + cw / 2, y + NODE_H + V_GAP, positions);
    cx += cw + H_GAP;
  }});
}}

function buildGraph() {{
  const svg  = document.getElementById('graph-svg');
  const root = document.getElementById('svg-root');
  const roots = ALL_OBJECTS.filter(o => o.parent === null);

  const positions = {{}};
  let ox = 0;
  roots.forEach(r => {{
    const w = subtreeWidth(r.id);
    assignPositions(r.id, ox + w / 2, 20, positions);
    ox += w + H_GAP;
  }});

  const allPos = Object.values(positions);
  graphW = Math.max(...allPos.map(p => p.x + NODE_W / 2)) + 20;
  graphH = Math.max(...allPos.map(p => p.y + NODE_H))     + 20;

  // Draw edges
  ALL_OBJECTS.forEach(co => {{
    co.children.forEach(cid => {{
      const p  = positions[co.id];
      const c  = positions[cid];
      const x1 = p.x, y1 = p.y + NODE_H;
      const x2 = c.x, y2 = c.y;
      const mx = (x1 + x2) / 2, my = (y1 + y2) / 2;
      const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
      path.setAttribute('class', 'edge');
      path.setAttribute('d', `M${{x1}},${{y1}} C${{x1}},${{my}} ${{x2}},${{my}} ${{x2}},${{y2}}`);
      root.appendChild(path);
      const constIdx = co.constants.findIndex(c => c.co_id === cid);
      if (constIdx >= 0) {{
        const lbl = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        lbl.setAttribute('class', 'edge-label');
        lbl.setAttribute('x', mx);
        lbl.setAttribute('y', my - 3);
        lbl.setAttribute('text-anchor', 'middle');
        lbl.textContent = `const[${{constIdx}}]`;
        root.appendChild(lbl);
      }}
    }});
  }});

  // Draw nodes
  ALL_OBJECTS.forEach(co => {{
    const p = positions[co.id];
    const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');
    g.setAttribute('class', 'node');
    g.setAttribute('transform', `translate(${{p.x - NODE_W/2}},${{p.y}})`);
    g.dataset.id = co.id;
    g.addEventListener('click', e => {{ e.stopPropagation(); selectObject(co.id); }});

    const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
    rect.setAttribute('width', NODE_W);
    rect.setAttribute('height', NODE_H);
    rect.setAttribute('rx', 8);
    rect.setAttribute('fill', '#161c2e');
    rect.setAttribute('stroke', '#1e2840');
    rect.setAttribute('stroke-width', '1');
    g.appendChild(rect);

    const title = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    title.setAttribute('class', 'node-title');
    title.setAttribute('x', 12);
    title.setAttribute('y', 22);
    title.setAttribute('fill', '#cdd6f4');
    title.textContent = truncate(co.name, 20);
    g.appendChild(title);

    const sub = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    sub.setAttribute('class', 'node-sub');
    sub.setAttribute('x', 12);
    sub.setAttribute('y', 39);
    sub.setAttribute('fill', '#4a5880');
    sub.textContent = `${{co.params.length}} params · ${{co.constants.length}} consts · ${{co.instructions.length}} ops`;
    g.appendChild(sub);

    root.appendChild(g);
  }});

  // ── Pan & zoom on SVG ────────────────────────────────────────────────────
  let panActive = false, panStartX = 0, panStartY = 0, panVx = 0, panVy = 0;
  let moved = false;

  svg.addEventListener('mousedown', e => {{
    if (e.button !== 0) return;
    panActive = true; moved = false;
    panStartX = e.clientX; panStartY = e.clientY;
    panVx = vx; panVy = vy;
    svg.classList.add('dragging');
  }});
  window.addEventListener('mousemove', e => {{
    if (!panActive) return;
    const dx = e.clientX - panStartX;
    const dy = e.clientY - panStartY;
    if (Math.abs(dx) + Math.abs(dy) > 3) moved = true;
    vx = panVx + dx;
    vy = panVy + dy;
    applyTransform();
  }});
  window.addEventListener('mouseup', () => {{
    panActive = false;
    svg.classList.remove('dragging');
  }});

  svg.addEventListener('wheel', e => {{
    e.preventDefault();
    const rect  = svg.getBoundingClientRect();
    const mx    = e.clientX - rect.left;
    const my    = e.clientY - rect.top;
    const delta = e.deltaY < 0 ? 1.12 : 1 / 1.12;
    vx    = mx - (mx - vx) * delta;
    vy    = my - (my - vy) * delta;
    vscale *= delta;
    applyTransform();
  }}, {{ passive: false }});

  // Zoom buttons
  document.getElementById('btn-zoom-in').addEventListener('click',  () => {{
    const svg = document.getElementById('graph-svg');
    const cx = svg.clientWidth / 2, cy = svg.clientHeight / 2;
    vx = cx - (cx - vx) * 1.25; vy = cy - (cy - vy) * 1.25; vscale *= 1.25;
    applyTransform();
  }});
  document.getElementById('btn-zoom-out').addEventListener('click', () => {{
    const svg = document.getElementById('graph-svg');
    const cx = svg.clientWidth / 2, cy = svg.clientHeight / 2;
    vx = cx - (cx - vx) / 1.25; vy = cy - (cy - vy) / 1.25; vscale /= 1.25;
    applyTransform();
  }});
  document.getElementById('btn-zoom-fit').addEventListener('click', fitGraph);

  // Initial fit (wait for layout)
  setTimeout(fitGraph, 0);

  // Select root
  if (roots.length > 0) selectObject(roots[0].id);
}}

function truncate(s, n) {{
  return s.length > n ? s.slice(0, n-1) + '…' : s;
}}

// ── Selection ─────────────────────────────────────────────────────────────────
function selectObject(id) {{
  selectedId = id;
  document.querySelectorAll('.node').forEach(n => {{
    n.classList.toggle('selected', parseInt(n.dataset.id) === id);
  }});
  renderDetail(byId[id]);
}}

// ── Detail Renderer ───────────────────────────────────────────────────────────
function renderDetail(co) {{
  const d = document.getElementById('detail');

  // Breadcrumb
  const crumbs = [];
  let cur = co;
  while (cur) {{
    crumbs.unshift(cur);
    cur = cur.parent !== null ? byId[cur.parent] : null;
  }}
  const bcHtml = crumbs.map((c, i) => {{
    if (i === crumbs.length - 1) return `<span style="color:var(--text)">${{esc(c.name)}}</span>`;
    return `<span class="crumb" onclick="selectObject(${{c.id}})">${{esc(c.name)}}</span>`;
  }}).join(' <span style="color:var(--dim)">›</span> ');

  const jmpTargets = new Set(
    co.instructions.filter(i => i.name === 'JMP' || i.name === 'JMP_IF_FALSE')
                   .flatMap(i => i.operands)
  );

  const disasmHtml = co.instructions.map(instr => {{
    const isTarget = jmpTargets.has(instr.offset);
    const argHtml  = instr.operands.map((op, i) => {{
      const ann    = instr.annotations[i] || '';
      const isJmp  = (instr.name === 'JMP' || instr.name === 'JMP_IF_FALSE');
      return `<span class="i-arg">${{op}}</span> <span class="i-ann${{isJmp ? ' is-jmp' : ''}}">${{esc(ann)}}</span>`;
    }}).join(' ');
    return `<div class="instr${{isTarget ? ' is-jmp-target' : ''}}">
      <span class="i-off">${{instr.offset}}:</span>
      <span class="i-op">${{esc(instr.name)}}</span>
      ${{argHtml}}
    </div>`;
  }}).join('');

  const constsHtml = co.constants.length === 0 ? '<span class="empty">none</span>' :
    `<table class="const-table">
      <thead><tr><th>#</th><th>type</th><th>value</th><th>module</th></tr></thead>
      <tbody>
      ${{co.constants.map(c => `
        <tr>
          <td class="ci">${{c.index}}</td>
          <td class="ct">${{esc(c.type)}}</td>
          <td class="cv${{c.co_id !== null ? ' is-co' : ''}}"
              ${{c.co_id !== null ? `onclick="selectObject(${{c.co_id}})"` : ''}}>
            ${{esc(c.display)}}
          </td>
          <td class="cm">${{c.module ? esc(c.module) : ''}}</td>
        </tr>`).join('')}}
      </tbody>
    </table>`;

  const paramsHtml = co.params.length === 0 ? '<span class="empty">none</span>' :
    `<table class="param-table">
      <thead><tr><th>name</th><th>kind</th><th>local#</th><th>default</th></tr></thead>
      <tbody>
      ${{co.params.map(p => `
        <tr>
          <td>${{esc(p.name)}}</td>
          <td class="pk">${{esc(p.kind)}}</td>
          <td class="pk">${{p.local_index}}</td>
          <td>${{p.has_default ? `<span class="pd">const[${{p.default_const_index}}]</span>` : '<span class="pk">—</span>'}}</td>
        </tr>`).join('')}}
      </tbody>
    </table>`;

  const namesHtml = co.names.length === 0 ? '<span class="empty">none</span>' :
    `<div class="tag-list">
      ${{co.names.map((n, i) => `<span class="tag"><span class="tag-idx">${{i}}</span>${{esc(n)}}</span>`).join('')}}
    </div>`;

  const localsHtml = co.locals.length === 0 ? '<span class="empty">none</span>' :
    `<div class="tag-list">
      ${{co.locals.map((n, i) => `<span class="tag"><span class="tag-idx">${{i}}</span>${{esc(n)}}</span>`).join('')}}
    </div>`;

  d.innerHTML = `
    <div class="detail-header">
      <div class="co-name">${{esc(co.name)}}</div>
      <div class="co-meta">
        version ${{co.version}}
        &nbsp;·&nbsp; ${{co.constants.length}} constant${{co.constants.length !== 1 ? 's' : ''}}
        &nbsp;·&nbsp; ${{co.locals.length}} local${{co.locals.length !== 1 ? 's' : ''}}
        &nbsp;·&nbsp; ${{co.params.length}} param${{co.params.length !== 1 ? 's' : ''}}
        &nbsp;·&nbsp; ${{co.instructions.length}} instruction${{co.instructions.length !== 1 ? 's' : ''}}
      </div>
      <div class="breadcrumb">${{bcHtml}}</div>
    </div>

    <div class="section">
      <div class="section-title">Fields</div>
      <div class="fields">
        <div class="field"><div class="field-key">name</div><div class="field-value">${{esc(co.name)}}</div></div>
        <div class="field"><div class="field-key">version</div><div class="field-value">${{co.version}}</div></div>
        <div class="field"><div class="field-key">params</div><div class="field-value">${{co.params.length}}</div></div>
        <div class="field"><div class="field-key">locals</div><div class="field-value">${{co.locals.length}}</div></div>
        <div class="field"><div class="field-key">names</div><div class="field-value">${{co.names.length}}</div></div>
        <div class="field"><div class="field-key">constants</div><div class="field-value">${{co.constants.length}}</div></div>
        <div class="field"><div class="field-key">instructions</div><div class="field-value">${{co.instructions.length}}</div></div>
        <div class="field"><div class="field-key">children</div><div class="field-value">${{co.children.length}}</div></div>
      </div>
    </div>

    ${{co.params.length > 0 ? `
    <div class="section">
      <div class="section-title">Parameters <span class="badge">${{co.params.length}}</span></div>
      ${{paramsHtml}}
    </div>` : ''}}

    <div class="section">
      <div class="section-title">Locals <span class="badge">${{co.locals.length}}</span></div>
      ${{localsHtml}}
    </div>

    <div class="section">
      <div class="section-title">Names <span class="badge">${{co.names.length}}</span></div>
      ${{namesHtml}}
    </div>

    <div class="section">
      <div class="section-title">Constants <span class="badge">${{co.constants.length}}</span></div>
      ${{constsHtml}}
    </div>

    <div class="section">
      <div class="section-title">Disassembly <span class="badge">${{co.instructions.length}}</span></div>
      <div class="disasm">${{disasmHtml || '<span class="empty">no instructions</span>'}}</div>
    </div>
  `;
}}

function esc(s) {{
  return String(s)
    .replace(/&/g,'&amp;')
    .replace(/</g,'&lt;')
    .replace(/>/g,'&gt;')
    .replace(/"/g,'&quot;');
}}

// ── Boot ──────────────────────────────────────────────────────────────────────
buildGraph();
</script>
</body>
</html>"""


# ─── Entry point ───────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    input_path  = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else os.path.splitext(input_path)[0] + ".html"

    print(f"  reading  {input_path}")
    data = open(input_path, 'rb').read()
    print(f"  parsing  {len(data)} bytes")

    try:
        parser = Parser(data)
        root, all_objects = parser.parse()
    except ParseError as e:
        print(f"  error    {e}", file=sys.stderr)
        sys.exit(1)

    print(f"  found    {len(all_objects)} code object(s)")
    for co in all_objects:
        indent = "  " * (sum(1 for o in all_objects if _is_ancestor(o.id, co.id, all_objects)))
        print(f"           {indent}· {co.name!r}  ({len(co.constants)} consts, {len(co.instructions)} instrs)")

    html = generate_html(input_path, all_objects)
    open(output_path, 'w', encoding='utf-8').write(html)
    print(f"  written  {output_path}")

    try:
        webbrowser.open(f"file://{os.path.abspath(output_path)}")
    except Exception:
        pass


def _is_ancestor(candidate_id, co_id, all_objects):
    """True if candidate_id is a strict ancestor of co_id."""
    cur = next((o for o in all_objects if o.id == co_id), None)
    while cur and cur.parent_id is not None:
        if cur.parent_id == candidate_id:
            return True
        cur = next((o for o in all_objects if o.id == cur.parent_id), None)
    return False


if __name__ == '__main__':
    main()
