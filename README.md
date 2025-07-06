# YAPL

**Yet Another Programming Language** — интерпретируемый язык программирования с динамической типизацией, реализованный на C++.

---

## Описание

Учебный проект для углубления познаний в области разработки ЯП и сопутствующих инструментов. 

---

## Быстрый старт

1. Клонировать репозиторий и подгрузить модули:
   ```bash
   git clone https://github.com/yourname/yapl.git
   cd yapl
   git submodule update --init --recursive
   ```
2. Сборка через CMake:
   ```bash
   mkdir build
   cmake -S . --build build
   cmake --build build --target yapl
   ```
3. Запуск интерпретатора на примере:
   ```bash
   ./yapl path/to/examples/hello_world.yapl
   ```
4. Запуск REPL (экспериментально):
   ```bash
   ./yapl repl
   ```

---

## Структура проекта

*TODO*

---

## Синтаксис
Наличие точки с запятой в конце строк является опциональным.

### Объявление переменных
```yapl
let foo
const bar = 20
let baz, qux
const xyz = 20, fred = "Hello!"
```

### Функции
```yapl
fn add(a, b) {
  return a + b
}

add(1, bar)

const add2 = add // functions are objects too
add2(1, 2)


fn sum(*args) {
   let s = 0
   for arg : args {
      s += arg
   }
   
   return s
}
sum(1, 2, 3, 4, 5, 6, 7, 8)
```

### Управляющие конструкции
```yapl
if a > b {
  print("a больше b")
} else {
  print("b не меньше a")
}

while i < 5 {
  i = i + 1
}

for let i = 0; i < 3; i += 1; {
  print(i)
}

const arr = [5, 6, 7]
for item : arr {
   print(item)
}
```

### User-defined types
```yapl 
class Point {
   fn init(this, x, y) {
      this.x = x
      thix.y = y
   }
   
   fn __str__(this) {
      return "Point { x: " + this.x + ", y: " + this.y + " }"
   }
   
   fn dist(this, other) {
      const dx = other.x - this.x
      const dy = other.y - this.y

      return sqrt(dx*dx + dy*dy)
   }
}

const p1 = Point(0, 0)
const p2 = Point(1, 1)

p1.dist(p2)

p1.x = 10
```

### Built-in types
```yapl 
const arr = [1, 2, 3]
const dict = { 1: "test", "foo": 2, "bar": "baz" }
```

### Modules
```yapl
// module.yapl
fn add(a, b) { return a + b } 
fn sub(a, b) { return a - b } 

export { add, sub }


// main.yapl
import { add, sub } from 'module.yapl'

add(1, 2)
sub(4, 3)
```

---

## План развития

- Компиляция в **байткод** и **нативный код** (LLVM backend).
- Улучшенный **REPL** с автодополнением.
- Интеграция с **LSP**, **форматтером**, **докгеном**.
- Расширение стандартной библиотеки.

---

## Тестирование

Юнит-тесты на Catch2:
```bash
cd build
ctest
```  
(или `ctest --output-on-failure` для подробностей)

---

## Зависимости

- **CMake** 3.29+
- Компилятор C++20 (GCC, MSVC)
- **Catch2** для тестов 


---

*Автор: ваше имя*  
*По вопросам и предложениям: issue / pull request в этом репозитории.*

