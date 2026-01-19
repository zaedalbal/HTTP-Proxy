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
sudo apt install git cmake g++ libboost-dev libboost-system-dev libgtest-dev libboost-log-dev
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

## Конфиг файл

Конфиг файл создается в той же директории, что и исполняемый файл, при первом запуске

Дефолтные значение в конфиг файле

```bash
[proxy]
host = '0.0.0.0'
log_file_name = 'proxy.log' # в логи записываются только заголовки
log_file_size_bytes = 16777216
log_on = false
max_bandwidth_per_sec = 2097152
max_connections = 256 # max connections на данный момент не используется
port = 12345
timeout_milliseconds = 10000
```

---

## Использование прокси

Для работы с прокси укажите IP-адрес и порт прокси-сервера в настройках вашего браузера или приложения

После настройки весь трафик браузера/приложения будет проходить через установленный прокси-сервер

---

## Структура проекта

```text
include/          # Заголовочные файлы
lib/              # Библиотеки
src/              # Исходные файлы
tests/            # Тесты (для запуска тестов: ctest -V)
CMakeLists.txt    # Конфигурация сборки
```

---

## Примечания

* Прокси не выполняет анализ или модификацию HTTPS-трафика
* Поддерживается только HTTP/1.1