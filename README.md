# HTTP Proxy Server

Асинхронный HTTP/HTTPS прокси-сервер на C++ с использованием **Boost.Asio** и **Boost.Beast**.

Поддерживается:

* HTTP/1.1 проксирование первого запроса, затем тунелирование
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

(если ваша версия CMake < 3.31, вы можете попробовать поменять CMakeLists.txt)

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
blacklist_on = false
blacklisted_hosts_file_name = 'blacklisted_hosts.toml'
host = '0.0.0.0'
log_file_name = 'proxy.log' # в логи записываются только заголовки
log_file_size_bytes = 16777216
log_on = false
max_bandwidth_per_sec = 2097152
max_connections = 256
port = 12345
timeout_milliseconds = 10000
```

Формат черного списка

```bash
[blacklist]
hosts = [
    "rkn.gov.ru",
    "max.ru",
    "vk.com"
]
```

Для изменения черного списка используйте скрипт:

```bash
blacklist-editor.sh
```

Как пользоваться blacklist-editor.sh:

```bash
blacklist-editor.sh --help

Usage:
  ./blacklist-editor.sh <file> add <host>
  ./blacklist-editor.sh <file> remove <host>
  ./blacklist-editor.sh <file> check <host>
  ./blacklist-editor.sh <file> list
  ./blacklist-editor.sh --help

Description:
  Utility for managing blacklist hosts in TOML format.

Arguments:
  <file>    Path to blacklisted_hosts.toml
  <host>    Domain name (e.g. example.com)

Commands:
  add       Add host to blacklist
  remove    Remove host from blacklist
  check     Check if host exists in blacklist
  list      List all blacklisted hosts
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

## P.S

Данный прокси не предназначен для обхода блокировок или DPI-систем.

Прокси НЕ шифрует, НЕ маскирует, НЕ модифицирует и НЕ дробит заголовки HTTP/HTTPS.