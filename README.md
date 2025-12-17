# HTTP Proxy Server

Асинхронный HTTP прокси-сервер на C++ с использованием Boost.Asio и Boost.Beast
(https пока что не поддерживается)

## Требования

- C++20 (для корутин)
- CMake 3.31+
- Boost

## Сборка

```bash
mkdir build && cd build
cmake ..
make
```

## Запуск

```bash
./proxy
```

## Структура проекта

```
include/          # Заголовочные файлы
src/             # Исходные файлы
CMakeLists.txt   # Конфигурация сборки
```