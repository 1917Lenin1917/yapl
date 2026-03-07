from __future__ import annotations

import argparse
import fnmatch
import io
import os
import subprocess
import sys
import threading
import time
from collections.abc import Iterable
from http import HTTPStatus
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


LIVE_RELOAD_SCRIPT = """<script>
(() => {
  const reloadConnection = new EventSource("/events");
  reloadConnection.onmessage = (event) => {
    if (event.data === "reload") {
      window.location.reload();
    }
  };
  reloadConnection.onerror = () => {
    setTimeout(() => window.location.reload(), 1000);
  };
})();
</script>"""


class LiveReloadState:
    def __init__(self) -> None:
        self.version = 0
        self.condition = threading.Condition()

    def trigger_reload(self) -> None:
        with self.condition:
            self.version += 1
            self.condition.notify_all()

    def wait_for_reload(self, last_seen_version: int, timeout: float = 30.0) -> int:
        with self.condition:
            if self.version != last_seen_version:
                return self.version

            self.condition.wait(timeout=timeout)
            return self.version


live_reload_state = LiveReloadState()


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("build", type=Path)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8000)
    parser.add_argument("--interval", type=float, default=0.5)
    parser.add_argument(
        "--ignore",
        action="append",
        default=["*.tmp", "*.swp", "*~", ".DS_Store"],
    )
    return parser.parse_args()


def should_ignore(path: Path, ignore_patterns: Iterable[str]) -> bool:
    path_text = path.as_posix()
    file_name = path.name

    for pattern in ignore_patterns:
        if fnmatch.fnmatch(file_name, pattern) or fnmatch.fnmatch(path_text, pattern):
            return True

    return False


def snapshot_directory(root: Path, ignore_patterns: Iterable[str]) -> dict[str, int]:
    state: dict[str, int] = {}

    if not root.exists():
        return state

    for file_path in root.rglob("*"):
        if not file_path.is_file():
            continue

        relative_path = file_path.relative_to(root)

        if should_ignore(relative_path, ignore_patterns):
            continue

        try:
            modified_time = file_path.stat().st_mtime_ns
        except OSError:
            continue

        state[str(file_path.resolve())] = modified_time

    return state


def build_docs(source: Path, build: Path) -> bool:
    command = [sys.executable, "-m", "sphinx", "build", str(source), str(build)]
    print(f"[live-docs] {' '.join(command)}")
    completed_process = subprocess.run(command)
    success = completed_process.returncode == 0

    if success:
        print(f"[live-docs] build succeeded: {build}")
    else:
        print(f"[live-docs] build failed with exit code {completed_process.returncode}")

    return success


class LiveReloadRequestHandler(SimpleHTTPRequestHandler):
    def do_GET(self) -> None:
        if self.path == "/events":
            self.handle_events()
            return

        super().do_GET()

    def handle_events(self) -> None:
        self.send_response(HTTPStatus.OK)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Connection", "keep-alive")
        self.end_headers()

        last_seen_version = live_reload_state.version

        try:
            while True:
                current_version = live_reload_state.wait_for_reload(last_seen_version)

                if current_version != last_seen_version:
                    self.wfile.write(b"data: reload\n\n")
                    self.wfile.flush()
                    last_seen_version = current_version
                else:
                    self.wfile.write(b": keepalive\n\n")
                    self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError):
            return

    def send_head(self):
        html_file = self.resolve_html_file()

        if html_file is not None:
            return self.send_injected_html(html_file)

        return super().send_head()

    def resolve_html_file(self) -> Path | None:
        translated_path = Path(self.translate_path(self.path))

        if translated_path.is_dir():
            for index_name in ("index.html", "index.htm"):
                index_file = translated_path / index_name
                if index_file.is_file():
                    return index_file
            return None

        if translated_path.is_file() and translated_path.suffix.lower() in {".html", ".htm"}:
            return translated_path

        return None

    def send_injected_html(self, file_path: Path):
        try:
            html = file_path.read_text(encoding="utf-8")
        except OSError:
            self.send_error(HTTPStatus.NOT_FOUND, "File not found")
            return None

        if LIVE_RELOAD_SCRIPT not in html:
            if "</body>" in html:
                html = html.replace("</body>", f"{LIVE_RELOAD_SCRIPT}</body>")
            else:
                html = f"{html}{LIVE_RELOAD_SCRIPT}"

        encoded_html = html.encode("utf-8")

        self.send_response(HTTPStatus.OK)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(encoded_html)))
        self.end_headers()

        return io.BytesIO(encoded_html)


def serve_directory(build: Path, host: str, port: int) -> ThreadingHTTPServer:
    def create_handler(*args, **kwargs):
        return LiveReloadRequestHandler(*args, directory=str(build), **kwargs)

    server = ThreadingHTTPServer((host, port), create_handler)
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()

    print(f"[live-docs] serving on http://{host}:{port}")
    return server


def watch_and_rebuild(source: Path, build: Path, interval: float, ignore_patterns: list[str]) -> None:
    previous_state = snapshot_directory(source, ignore_patterns)

    while True:
        time.sleep(interval)
        current_state = snapshot_directory(source, ignore_patterns)

        if current_state == previous_state:
            continue

        changed_paths = sorted(set(previous_state) ^ set(current_state))
        changed_paths.extend(
            path
            for path, modified_time in current_state.items()
            if previous_state.get(path) not in (None, modified_time)
        )

        if changed_paths:
            print("[live-docs] changes detected:")
            for changed_path in dict.fromkeys(changed_paths):
                print(f"[live-docs]   {changed_path}")

        if build_docs(source, build):
            live_reload_state.trigger_reload()

        previous_state = current_state


def main() -> None:
    arguments = parse_arguments()

    source = arguments.source.resolve()
    build = arguments.build.resolve()

    os.makedirs(build, exist_ok=True)

    initial_build_succeeded = build_docs(source, build)
    server = serve_directory(build, arguments.host, arguments.port)

    if initial_build_succeeded:
        live_reload_state.trigger_reload()

    try:
        watch_and_rebuild(source, build, arguments.interval, arguments.ignore)
    except KeyboardInterrupt:
        print("\n[live-docs] stopping")
    finally:
        server.shutdown()
        server.server_close()


if __name__ == "__main__":
    main()
