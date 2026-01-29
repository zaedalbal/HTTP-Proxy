#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include "network/traffic_limiter.hpp"

class TrafficLimiterTest : public ::testing::Test
{
protected:
    static constexpr uint64_t BYTES_PER_SEC = 1000000; // 1 мб/сек
};

// базовая инициализация
TEST_F(TrafficLimiterTest, Construction)
{
    EXPECT_NO_THROW(Traffic_limiter limiter(BYTES_PER_SEC));
}

// начальное состояние позволяет брать токены
TEST_F(TrafficLimiterTest, InitialTokens)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    // можем взять токены сразу (должно быть max_tokens_ = bytes_per_sec * 1.5)
    std::size_t allowed = limiter.acquire(BYTES_PER_SEC);
    EXPECT_GT(allowed, 0);
    EXPECT_LE(allowed, BYTES_PER_SEC * 1.5);
}

// acquire возвращает не больше чем запрошено
TEST_F(TrafficLimiterTest, AcquireReturnsRequestedAmount)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    std::size_t want = 1000;
    std::size_t allowed = limiter.acquire(want);
    
    EXPECT_LE(allowed, want);
}

// acquire уменьшает количество доступных токенов
TEST_F(TrafficLimiterTest, AcquireDepletesTokens)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    std::size_t first = limiter.acquire(BYTES_PER_SEC);
    std::size_t second = limiter.acquire(BYTES_PER_SEC);
    
    // второй запрос должен получить меньше так как токены израсходованы
    EXPECT_LT(second, first);
}

// полное исчерпание токенов
TEST_F(TrafficLimiterTest, TokenDepletion)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    // исчерпываем все токены
    std::size_t total = 0;
    while (true)
    {
        std::size_t allowed = limiter.acquire(BYTES_PER_SEC);
        if (allowed == 0)
        {
            break;
        }
        total += allowed;
    }
    
    EXPECT_GT(total, 0);
    
    // следующий запрос должен вернуть 0
    EXPECT_EQ(limiter.acquire(1000), 0);
}

// refill восстанавливает токены со временем
TEST_F(TrafficLimiterTest, RefillRestoresTokens)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    // исчерпываем токены
    while (limiter.acquire(BYTES_PER_SEC) > 0);
    
    // ждем немного для refill
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // теперь должны быть доступны новые токены
    std::size_t allowed = limiter.acquire(1000);
    EXPECT_GT(allowed, 0);
}

// rate limiting работает приблизительно правильно
TEST_F(TrafficLimiterTest, RateLimitingApproximate)
{
    uint64_t rate = 100000; // 100 кб/сек
    Traffic_limiter limiter(rate);
    
    auto start = std::chrono::steady_clock::now();
    std::size_t total_acquired = 0;
    
    // пытаемся получить больше чем rate за 0.5 секунды
    while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(500))
    {
        std::size_t allowed = limiter.acquire(10000);
        total_acquired += allowed;
        if (allowed == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    // за 0.5 сек должны получить примерно rate * 0.5 + burst (max_tokens)
    // проверяем что не получили слишком много
    uint64_t expected_max = rate * 0.5 + rate * 1.5; // время * rate + burst
    EXPECT_LE(total_acquired, expected_max * 1.2); // 20% погрешность
}

// несколько маленьких запросов
TEST_F(TrafficLimiterTest, MultipleSmallAcquires)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    for (int i = 0; i < 10; ++i)
    {
        std::size_t allowed = limiter.acquire(1000);
        EXPECT_GT(allowed, 0);
    }
}

// acquire с нулевым запросом
TEST_F(TrafficLimiterTest, AcquireZero)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    std::size_t allowed = limiter.acquire(0);
    EXPECT_EQ(allowed, 0);
}

// очень большой запрос
TEST_F(TrafficLimiterTest, AcquireVeryLarge)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    std::size_t huge = 1000000000; // 1 гб
    std::size_t allowed = limiter.acquire(huge);
    
    // должен вернуть максимум max_tokens_
    EXPECT_LE(allowed, BYTES_PER_SEC * 1.5);
}

// потокобезопасность - параллельные вызовы acquire
TEST_F(TrafficLimiterTest, ThreadSafety)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    std::vector<std::thread> threads;
    std::atomic<std::size_t> total_acquired{0};
    
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([&limiter, &total_acquired]()
        {
            for (int j = 0; j < 100; ++j)
            {
                std::size_t allowed = limiter.acquire(1000);
                total_acquired += allowed;
            }
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // проверяем что получили разумное количество
    EXPECT_GT(total_acquired.load(), 0);
}

// быстрые последовательные запросы
TEST_F(TrafficLimiterTest, RapidSequentialAcquires)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    std::size_t count = 0;
    for (int i = 0; i < 1000; ++i)
    {
        if (limiter.acquire(100) > 0)
        {
            ++count;
        }
    }
    
    EXPECT_GT(count, 0);
}

// низкий rate
TEST_F(TrafficLimiterTest, LowRate)
{
    uint64_t low_rate = 1000; // 1 кб/сек
    Traffic_limiter limiter(low_rate);
    
    std::size_t allowed = limiter.acquire(100);
    EXPECT_GT(allowed, 0);
    EXPECT_LE(allowed, 100);
}

// высокий rate
TEST_F(TrafficLimiterTest, HighRate)
{
    uint64_t high_rate = 100000000; // 100 мб/сек
    Traffic_limiter limiter(high_rate);
    
    std::size_t allowed = limiter.acquire(10000000);
    EXPECT_GT(allowed, 0);
}

// refill после длительной паузы
TEST_F(TrafficLimiterTest, RefillAfterLongPause)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    // исчерпываем токены
    while (limiter.acquire(BYTES_PER_SEC) > 0);
    
    // долгая пауза
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // должны восстановиться до max_tokens_
    std::size_t allowed = limiter.acquire(BYTES_PER_SEC * 2);
    EXPECT_GT(allowed, 0);
    EXPECT_LE(allowed, BYTES_PER_SEC * 1.5); // max_tokens
}

// постепенное восстановление
TEST_F(TrafficLimiterTest, GradualRefill)
{
    Traffic_limiter limiter(BYTES_PER_SEC);
    
    // исчерпываем
    while (limiter.acquire(BYTES_PER_SEC) > 0);
    
    std::vector<std::size_t> amounts;
    
    // проверяем что со временем получаем все больше
    for (int i = 0; i < 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        amounts.push_back(limiter.acquire(100000));
    }
    
    // хотя бы некоторые должны быть > 0
    std::size_t non_zero = 0;
    for (auto amt : amounts)
    {
        if (amt > 0) ++non_zero;
    }
    
    EXPECT_GT(non_zero, 0);
}