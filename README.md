# HTTP Proxy Server

Асинхронный HTTP/HTTPS прокси-сервер на C++ с использованием **Boost.Asio** и **Boost.Beast**.

Поддерживается:

* HTTP/1.1 проксирование
* HTTPS через метод `CONNECT` (туннелирование)

---

## Требования

Для сборки и запуска необходимы:

* Компилятор с поддержкой **C++23**
* **CMake 3.31+**
* **Boost 1.82+**
  (используются `asio`, `beast`, `system`)
* **Google Test** (для сборки и запуска тестов)
* Git

### Установка зависимостей

**Для Ubuntu/Debian:**

```bash
sudo apt install git cmake g++ libboost-dev libboost-system-dev libgtest-dev
```

**Для Fedora:**

```bash
sudo dnf install git cmake gcc-c++ boost-devel gtest-devel
```

**Для Arch Linux / Manjaro:**

```bash
sudo pacman -S git cmake gcc boost gtest
```

---

## Клонирование репозитория

```bash
git clone https://github.com/zaedalbal/HTTP-Proxy.git
cd HTTP-Proxy
```

---

## Сборка проекта

```bash
bash ./build.sh
```

После успешной сборки в каталоге `build` будет создан исполняемый файл прокси.

---

## Ручная сборка проекта

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

После успешной сборки в каталоге `build` будет создан исполняемый файл прокси.

---

## Запуск прокси

Из каталога `build`:

```bash
./proxy
```

По умолчанию прокси запускается на порту `12345`.

---

## Структура проекта

```text
include/          # Заголовочные файлы
src/              # Исходные файлы
CMakeLists.txt    # Конфигурация сборки
```

---

## Примечания

* Прокси не выполняет анализ или модификацию HTTPS-трафика
* Поддерживается только HTTP/1.1