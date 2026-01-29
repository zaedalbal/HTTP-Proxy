#!/bin/bash

# интеграционные тесты для прокси

# конфигурация
PROXY_HOST="127.0.0.1"
PROXY_PORT="12345"
PROXY_URL="http://${PROXY_HOST}:${PROXY_PORT}"
PROXY_BIN="../../build/proxy"
PROXY_PID=""
TEST_DIR="$(mktemp -d)"
PASSED=0
FAILED=0

# функция для вывода результатов
print_result()
{
    local test_name="$1"
    local result="$2"
    
    if [[ "$result" == "PASS" ]]; then
        echo "[PASS] $test_name"
        ((PASSED++))
    else
        echo "[FAIL] $test_name"
        ((FAILED++))
    fi
}

# запуск прокси сервера
start_proxy()
{
    echo "starting proxy server..."
    
    # проверка наличия исполняемого файла
    if [[ ! -f "$PROXY_BIN" ]]; then
        echo "error: executable file $PROXY_BIN not found"
        echo "first run: bash ./build.sh"
        exit 1
    fi
    
    # запуск в фоновом режиме
    $PROXY_BIN &
    PROXY_PID=$!
    
    # ожидание запуска сервера
    echo "waiting for server to start (pid: $PROXY_PID)..."
    sleep 2
    
    # проверка что процесс запущен
    if ! kill -0 $PROXY_PID 2>/dev/null; then
        echo "error: proxy server failed to start"
        exit 1
    fi
    
    # проверка что порт открыт
    if ! nc -z $PROXY_HOST $PROXY_PORT 2>/dev/null; then
        echo "error: port $PROXY_PORT is not open"
        kill $PROXY_PID 2>/dev/null || true
        exit 1
    fi
    
    echo "proxy server started on ${PROXY_HOST}:${PROXY_PORT}"
}

# остановка прокси сервера
stop_proxy()
{
    if [[ -n "$PROXY_PID" ]] && kill -0 $PROXY_PID 2>/dev/null; then
        echo "stopping proxy server (pid: $PROXY_PID)..."
        kill $PROXY_PID
        wait $PROXY_PID 2>/dev/null || true
    fi
}

# очистка временных файлов
cleanup()
{
    stop_proxy
    rm -rf "$TEST_DIR"
    echo ""
    echo "test results:"
    echo "passed: $PASSED"
    echo "failed: $FAILED"
    
    if [[ $FAILED -gt 0 ]]; then
        exit 1
    fi
}

trap cleanup EXIT INT TERM

# тест 1: базовый http get запрос
test_http_get()
{
    local output
    output=$(curl -x $PROXY_URL \
        -s -w "\n%{http_code}" \
        --max-time 10 \
        http://example.com/ 2>&1)
    
    local http_code=$(echo "$output" | tail -n1)
    
    if [[ "$http_code" == "200" ]]; then
        print_result "http get request to example.com" "PASS"
    else
        print_result "http get request to example.com (code: $http_code)" "FAIL"
    fi
}

# тест 2: http get с заголовками
test_http_headers()
{
    local output
    output=$(curl -x $PROXY_URL \
        -s -w "\n%{http_code}" \
        -H "User-Agent: ProxyTest/1.0" \
        -H "Accept: text/html" \
        --max-time 10 \
        http://httpbin.org/headers 2>&1)
    
    local http_code=$(echo "$output" | tail -n1)
    
    if [[ "$http_code" == "200" ]]; then
        print_result "http request with custom headers" "PASS"
    else
        print_result "http request with custom headers (code: $http_code)" "FAIL"
    fi
}

# тест 3 https через connect метод
test_https_connect()
{
    local output
    output=$(curl -x $PROXY_URL \
        -s -w "\n%{http_code}" \
        --max-time 10 \
        https://www.google.com/ 2>&1)
    
    local http_code=$(echo "$output" | tail -n1)
    
    if [[ "$http_code" == "200" ]]; then
        print_result "https connect to google.com" "PASS"
    else
        print_result "https connect to google.com (code: $http_code)" "FAIL"
    fi
}

# тест 4 несколько последовательных запросов
test_sequential_requests()
{
    local success=0
    
    for i in {1..5}; do
        local output
        output=$(curl -x $PROXY_URL \
            -s -w "\n%{http_code}" \
            --max-time 10 \
            http://httpbin.org/get 2>&1)
        
        local http_code=$(echo "$output" | tail -n1)
        
        if [[ "$http_code" == "200" ]]; then
            ((success++))
        fi
    done
    
    if [[ $success -eq 5 ]]; then
        print_result "5 sequential requests" "PASS"
    else
        print_result "5 sequential requests (successful: $success/5)" "FAIL"
    fi
}

# тест 5 параллельные запросы
test_parallel_requests()
{
    local pids=()
    local temp_files=()
    
    # запуск 3 параллельных запросов
    for i in {1..3}; do
        local temp_file="$TEST_DIR/parallel_$i.txt"
        temp_files+=("$temp_file")
        
        (curl -x $PROXY_URL \
            -s -w "\n%{http_code}" \
            --max-time 15 \
            http://httpbin.org/delay/1 > "$temp_file" 2>&1) &
        
        pids+=($!)
    done
    
    # ожидание завершения всех запросов
    for pid in "${pids[@]}"; do
        wait $pid
    done
    
    # проверка результатов
    local success=0
    for temp_file in "${temp_files[@]}"; do
        if [[ -f "$temp_file" ]]; then
            local http_code=$(tail -n1 "$temp_file")
            if [[ "$http_code" == "200" ]]; then
                ((success++))
            fi
        fi
    done
    
    if [[ $success -eq 3 ]]; then
        print_result "3 parallel requests" "PASS"
    else
        print_result "3 parallel requests (successful: $success/3)" "FAIL"
    fi
}

# тест 6: разные http методы
test_http_methods()
{
    local methods=("GET" "POST" "PUT" "DELETE")
    local success=0
    
    for method in "${methods[@]}"; do
        local output
        output=$(curl -x $PROXY_URL \
            -X $method \
            -s -w "\n%{http_code}" \
            --max-time 10 \
            http://httpbin.org/${method,,} 2>&1)
        
        local http_code=$(echo "$output" | tail -n1)
        
        if [[ "$http_code" == "200" ]]; then
            ((success++))
        fi
    done
    
    if [[ $success -eq 4 ]]; then
        print_result "different http methods (get/post/put/delete)" "PASS"
    else
        print_result "different http methods (successful: $success/4)" "FAIL"
    fi
}

# тест 7: post запрос с данными
test_http_post_data()
{
    local output
    output=$(curl -x $PROXY_URL \
        -X POST \
        -d "test=data&field=value" \
        -s -w "\n%{http_code}" \
        --max-time 10 \
        http://httpbin.org/post 2>&1)
    
    local http_code=$(echo "$output" | tail -n1)
    
    if [[ "$http_code" == "200" ]]; then
        print_result "post request with data" "PASS"
    else
        print_result "post request with data (code: $http_code)" "FAIL"
    fi
}

# тест 8: редирект
test_http_redirect()
{
    local output
    output=$(curl -x $PROXY_URL \
        -L \
        -s -w "\n%{http_code}" \
        --max-time 10 \
        http://httpbin.org/redirect/2 2>&1)
    
    local http_code=$(echo "$output" | tail -n1)
    
    if [[ "$http_code" == "200" ]]; then
        print_result "handling http redirects" "PASS"
    else
        print_result "handling http redirects (code: $http_code)" "FAIL"
    fi
}

# тест 9: несуществующий хост
test_invalid_host()
{
    local output
    output=$(curl -x $PROXY_URL \
        -s -w "\n%{http_code}" \
        --max-time 10 \
        http://this-host-does-not-exist-12345.com/ 2>&1)
    
    local http_code=$(echo "$output" | tail -n1)
    
    # ожидаем ошибку (не 200)
    if [[ "$http_code" != "200" && "$http_code" != "000" ]]; then
        print_result "handling non-existent host" "PASS"
    else
        print_result "handling non-existent host (unexpected code: $http_code)" "FAIL"
    fi
}

# основная функция запуска тестов
main()
{
    echo "=== integration tests for proxy ==="
    echo ""
    
    # запуск прокси
    start_proxy
    
    echo ""
    echo "running tests..."
    echo ""
    
    # http тесты
    test_http_get
    test_http_headers
    test_http_methods
    test_http_post_data
    test_http_redirect
    
    # https тесты
    test_https_connect
    
    # тесты производительности
    test_sequential_requests
    test_parallel_requests
    
    # негативные тесты
    test_invalid_host
    
    echo ""
}

# проверка зависимостей
check_dependencies()
{
    local deps=("curl" "nc")
    
    for dep in "${deps[@]}"; do
        if ! command -v $dep &> /dev/null; then
            echo "error: $dep is not installed"
            echo "install with: sudo apt install curl netcat-openbsd"
            exit 1
        fi
    done
}

check_dependencies
main