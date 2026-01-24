#!/bin/bash

if [[ -d "build" ]]; # проверка существования деректории build
then
    echo "Removing previous build directory"
    rm -rf build
fi

echo "Creating build directory..."
mkdir build
cd build || exit 1 # если cd завершится неудачно, то выдать ошибку

echo "Configuration project..."
if ! cmake ..;
then
    echo "Error in configuration project"
    exit 1
fi

echo "Starting build project..."
if ! cmake --build . --config Release;
then
    echo "Error in building project"
    exit 1
fi

echo "done!"

echo "Run ./proxy to start the server"