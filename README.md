# HTTP Proxy Server

Асинхронный HTTP/HTTPS прокси-сервер на C++ с использованием **Boost.Asio** и **Boost.Beast**, с админ панелью на Qt6

Поддерживается:

* HTTP/1.1 проксирование первого запроса, затем тунелирование
* HTTPS через метод `CONNECT` (туннелирование)
* Просмотр состояние прокси через админ-панель
(на данный момент админ-панель находится в разработке. Админ-панель не имеет шифрования и имеет возможность только смотреть конфиг и сессии прокси-сервера)

---

## Требования

Для сборки и запуска необходимы:

* Компилятор с поддержкой **C++23**
* **CMake 3.31+**
* **Boost 1.82+**
  (используются `asio`, `beast`, `system`)
* **Google Test** (для сборки и запуска тестов)
* **Qt6** (Widgets, Network)
* **Git**

### Установка зависимостей

**Для Ubuntu/Debian:**

```bash
sudo apt install git cmake g++ libboost-dev libboost-system-dev libboost-log-dev libgtest-dev libssl-dev
```

**Для Fedora:**

```bash
sudo dnf install git cmake gcc-c++ boost-devel gtest-devel openssl-devel
```

**Для Arch Linux / Manjaro:**

```bash
sudo pacman -S git cmake gcc boost gtest openssl
```

(если ваша версия CMake < 3.31, вы можете попробовать поменять CMakeLists.txt)

---

## Клонирование репозитория

```bash
git clone https://github.com/zaedalbal/HTTP-Proxy.git
cd HTTP-Proxy
```

---

## Сборка всех частей проекта

```bash
bash ./build.sh
```

После успешной сборки в каталоге `build` будут созданы каталоги с proxy_server, admin_panel и tools

---

## Ручная сборка проекта

Флаги для сборки

```bash
BUILD_PROXY_SERVER # Сборка прокси-сервера

BUILD_ADMIN_PANEL # Сборка админ-панели

BUILD_TOOLS # Сборка утилит для настройки прокси-сервера
```

Сборка проекта

```bash
mkdir build
cd build
cmake .. -DBUILD_PROXY_SERVER=OFF -DBUILD_ADMIN_PANEL=OFF -DBUILD_TOOLS=OFF
cmake --build . --config Release
```

После успешной сборки в каталоге `build` будут созданы выбранные части проекта.

---

## Запуск прокси из каталога по умолчанию

Из каталога `build/proxy_server`:

```bash
./proxy
```

По умолчанию прокси запускается на порту `12345`.

---

## Запуск админ-панели из каталога по умолчанию

Из каталога `build/admin_panel`

```bash
./admin_panel
```

---

## Конфиг файл

Конфиг файл создается в той же директории, что и исполняемый файл, при первом запуске

Дефолтные значение в конфиг файле

```bash
[proxy]
admin_panel_auth_data_file_name = 'admin_panel_auth_data_file_name.toml'
admin_panel_on = false
admin_panel_port = 54321
blacklist_on = false
blacklisted_hosts_file_name = 'blacklisted_hosts.toml'
host = '0.0.0.0'
log_file_name = 'proxy.log'
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

Формат записей пользователей админ-панели:

```bash
1   [test1]
  1 algorithm = 'PBKDF2-HMAC-SHA256'
  2 hash = 'd5817f1d9a3f0c08710e74b850e25d4b9e5530e0ba3ecdfb00343031964a27f4'
  3 iterations = 255096
  4 login = 'test1'
  5 salt = '01e053e777f71ec923e3806e7cb431cc'
  6
  7 [test2]
  8 algorithm = 'PBKDF2-HMAC-SHA256'
  9 hash = '40d9298374d49ff9cbcf9c052f4fa41929805171d8319c757851b4abd5462237'
 10 iterations = 294579
 11 login = 'test2'
 12 salt = '24659020b7055c3e91d31ea96bc5a5cb'
```

Для изменения файла с пользователями админ-панели используйте скрипт

```bash
admin_panel_auth_data_editor
```

Как пользоваться admin_panel_auth_data_editor:

```bash
Usage:
  add_admin_user add <file.toml> <login> <password> # если запись уже существует, то она обновляется
  add_admin_user remove <file.toml> <login>
```

---

## Использование прокси

Для работы с прокси укажите IP-адрес и порт прокси-сервера в настройках вашего браузера или приложения

После настройки весь трафик браузера/приложения будет проходить через установленный прокси-сервер

---

## Структура проекта

```text
HTTP-Proxy/
├── proxy_server/          # Основной прокси-сервер
│   ├── include/           # Заголовочные файлы сервера
│   │   ├── admin_panel_bridge/  # Связь сервера с админ-панелью
│   │   │   ├── admin_panel_bridge.hpp
│   │   │   └── admin_panel_session.hpp  # Обработка сессий админ-панели
│   │   ├── config/        # Конфигурация прокси
│   │   │   └── proxy_config.hpp
│   │   ├── globals/       # Глобальные переменные и константы
│   │   │   └── globals.hpp
│   │   ├── logger/        # Логирование
│   │   │   └── logger.hpp
│   │   ├── network/       # Сетевые компоненты прокси
│   │   │   ├── server.hpp
│   │   │   ├── session.hpp
│   │   │   ├── traffic_limiter.hpp
│   │   │   ├── user_traffic_manager.hpp
│   │   │   └── analyze_request.hpp
│   │   ├── proxy_facade/  # Структуры и фасады для управления сессиями
│   │   │   ├── proxy_facade.hpp
│   │   │   └── session_struct.hpp
│   │   └── utils/         # Вспомогательные утилиты
│   │       └── timer.hpp
│   ├── src/               # Исходные файлы сервера
│   │   ├── admin_panel_bridge/  # Реализация взаимодействия с панелью
│   │   │   ├── admin_panel_bridge.cpp
│   │   │   └── admin_panel_session.cpp
│   │   ├── config/
│   │   │   └── proxy_config.cpp
│   │   ├── globals/
│   │   │   └── globals.cpp
│   │   ├── logger/
│   │   │   └── logger.cpp
│   │   ├── main.cpp        # Точка входа в прокси
│   │   ├── network/
│   │   │   ├── server.cpp
│   │   │   ├── session.cpp
│   │   │   ├── traffic_limiter.cpp
│   │   │   ├── user_traffic_manager.cpp
│   │   │   └── analyze_request.cpp
│   │   ├── proxy_facade/
│   │   │   └── proxy_facade.cpp
│   │   └── utils/
│   │       └── timer.cpp
│   └── tests/             # Тесты сервера
│       ├── integration_tests/  # Интеграционные тесты
│       │   └── integration_tests.sh
│       └── unit_tests/    # Юнит-тесты отдельных модулей
│           ├── config/
│           ├── logger/
│           ├── network/
│           └── utils/
│
├── admin_panel/            # Qt6 админ-панель
│   ├── include/           # Заголовки виджетов, страниц и взаимодействия с сервером
│   │   ├── mainwindow.hpp
│   │   ├── pages/          # Страницы панели
│   │   │   ├── AuthenticationPage/
│   │   │   │   └── AuthenticationPageWidget.hpp
│   │   │   ├── ConfigPage/
│   │   │   │   └── ConfigPageWidget.hpp
│   │   │   ├── LogPage/
│   │   │   │   └── LogPageWidget.hpp
│   │   │   └── ProxySessionsPage/
│   │   │       ├── ProxySessionsPageWidget.hpp
│   │   │       ├── SessionsModel/
│   │   │       │   └── SessionsModel.hpp
│   │   │       └── structSessionInfo.hpp
│   │   └── server_interaction/  # Классы для общения с сервером
│   │       ├── RequestFacade.hpp
│   │       ├── ResponseHandler.hpp
│   │       └── ServerInteraction.hpp
│   └── src/               # Реализация админ-панели
│       ├── main.cpp
│       ├── mainwindow.cpp
│       ├── pages/         # Реализация страниц
│       └── server_interaction/  # Реализация взаимодействия с сервером
│
├── tools/                 # Утилиты
│   └── src/
│       └── admin_panel_auth_data_editor.cpp  # Управление аккаунтами админ-панели
│
├── lib/tomlplusplus/      # Встроенная библиотека TOML
├── blacklist-editor.sh    # Скрипт для редактирования черного списка
├── build.sh               # Скрипт для сборки всего проекта
├── CMakeLists.txt         # Главный CMake-файл проекта
└── README.md              # Документация
```

---

## Примечания

* Прокси не выполняет анализ или модификацию HTTPS-трафика
* Поддерживается только HTTP/1.1

## P.S

Данный прокси не предназначен для обхода блокировок или DPI-систем.

Прокси НЕ шифрует, НЕ маскирует, НЕ модифицирует и НЕ дробит заголовки HTTP/HTTPS.

Админ-панель находится в разработке
(на данный момент не имеет шифрования и имеет возможность только смотреть конфиг и сессии прокси-сервера).