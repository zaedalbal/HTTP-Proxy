#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>
#include "network/user_traffic_manager.hpp"

class UserTrafficManagerTest : public ::testing::Test
{
protected:
    User_traffic_manager manager_;
};

// получение существующего пользователя
TEST_F(UserTrafficManagerTest, GetExistingUser)
{
    std::string ip = "192.168.1.1";
    
    auto limiter1 = manager_.get_or_create_user(ip);
    auto limiter2 = manager_.get_or_create_user(ip);
    
    // должны получить один и тот же объект
    EXPECT_EQ(limiter1.get(), limiter2.get());
}

// разные ip получают разные лимитеры
TEST_F(UserTrafficManagerTest, DifferentIpsDifferentLimiters)
{
    auto limiter1 = manager_.get_or_create_user("192.168.1.1");
    auto limiter2 = manager_.get_or_create_user("192.168.1.2");
    
    EXPECT_NE(limiter1.get(), limiter2.get());
}

// weak_ptr автоматически удаляется когда shared_ptr уничтожен
TEST_F(UserTrafficManagerTest, WeakPtrCleanup)
{
    std::string ip = "192.168.1.1";
    
    {
        auto limiter = manager_.get_or_create_user(ip);
        // limiter уничтожится при выходе из scope
    }
    
    // создаем нового - должен быть новый объект
    auto new_limiter = manager_.get_or_create_user(ip);
    ASSERT_NE(new_limiter, nullptr);
}

// несколько пользователей одновременно
TEST_F(UserTrafficManagerTest, MultipleUsers)
{
    std::vector<std::shared_ptr<Traffic_limiter>> limiters;
    
    for (int i = 0; i < 10; ++i)
    {
        std::string ip = "192.168.1." + std::to_string(i);
        limiters.push_back(manager_.get_or_create_user(ip));
    }
    
    // все должны быть уникальными
    for (size_t i = 0; i < limiters.size(); ++i)
    {
        for (size_t j = i + 1; j < limiters.size(); ++j)
        {
            EXPECT_NE(limiters[i].get(), limiters[j].get());
        }
    }
}

// пустой ip
TEST_F(UserTrafficManagerTest, EmptyIp)
{
    auto limiter = manager_.get_or_create_user("");
    ASSERT_NE(limiter, nullptr);
}

// одинаковые запросы возвращают тот же limiter
TEST_F(UserTrafficManagerTest, ConsistentReturns)
{
    std::string ip = "10.0.0.1";
    
    auto limiter1 = manager_.get_or_create_user(ip);
    auto limiter2 = manager_.get_or_create_user(ip);
    auto limiter3 = manager_.get_or_create_user(ip);
    
    EXPECT_EQ(limiter1.get(), limiter2.get());
    EXPECT_EQ(limiter2.get(), limiter3.get());
}

// потокобезопасность - параллельное создание пользователей
TEST_F(UserTrafficManagerTest, ThreadSafetyCreation)
{
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<Traffic_limiter>> limiters(10);
    
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([this, i, &limiters]()
        {
            std::string ip = "192.168.1." + std::to_string(i);
            limiters[i] = manager_.get_or_create_user(ip);
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // все limiters должны быть созданы
    for (const auto& limiter : limiters)
    {
        EXPECT_NE(limiter, nullptr);
    }
}

// потокобезопасность - параллельный доступ к одному ip
TEST_F(UserTrafficManagerTest, ThreadSafetySameIp)
{
    std::string ip = "192.168.1.1";
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<Traffic_limiter>> limiters(10);
    
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([this, &ip, i, &limiters]()
        {
            limiters[i] = manager_.get_or_create_user(ip);
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // все должны указывать на один объект
    for (size_t i = 1; i < limiters.size(); ++i)
    {
        EXPECT_EQ(limiters[0].get(), limiters[i].get());
    }
}

// пересоздание после уничтожения
TEST_F(UserTrafficManagerTest, RecreateAfterDestruction)
{
    std::string ip = "192.168.1.1";
    Traffic_limiter* first_ptr = nullptr;
    
    {
        auto limiter1 = manager_.get_or_create_user(ip);
        first_ptr = limiter1.get();
    }
    
    // создаем снова после уничтожения
    auto limiter2 = manager_.get_or_create_user(ip);
    
    // должен быть новый объект (хотя адрес может совпасть)
    ASSERT_NE(limiter2, nullptr);
}

// ipv6 адреса
TEST_F(UserTrafficManagerTest, Ipv6Addresses)
{
    auto limiter1 = manager_.get_or_create_user("2001:0db8:85a3::8a2e:0370:7334");
    auto limiter2 = manager_.get_or_create_user("fe80::1");
    
    ASSERT_NE(limiter1, nullptr);
    ASSERT_NE(limiter2, nullptr);
    EXPECT_NE(limiter1.get(), limiter2.get());
}

// специальные символы в ip (некорректный но валидный string)
TEST_F(UserTrafficManagerTest, SpecialCharactersInIp)
{
    auto limiter = manager_.get_or_create_user("not-a-valid-ip!");
    ASSERT_NE(limiter, nullptr);
}

// большое количество пользователей
TEST_F(UserTrafficManagerTest, ManyUsers)
{
    std::vector<std::shared_ptr<Traffic_limiter>> limiters;
    
    for (int i = 0; i < 1000; ++i)
    {
        std::string ip = "10.0." + std::to_string(i / 256) + "." + std::to_string(i % 256);
        limiters.push_back(manager_.get_or_create_user(ip));
    }
    
    EXPECT_EQ(limiters.size(), 1000);
    
    // проверяем что все не nullptr
    for (const auto& limiter : limiters)
    {
        EXPECT_NE(limiter, nullptr);
    }
}

// use_count проверка
TEST_F(UserTrafficManagerTest, UseCount)
{
    std::string ip = "192.168.1.1";
    
    auto limiter1 = manager_.get_or_create_user(ip);
    EXPECT_GE(limiter1.use_count(), 1);
    
    {
        auto limiter2 = manager_.get_or_create_user(ip);
        // теперь два shared_ptr
        EXPECT_GE(limiter1.use_count(), 2);
    }
    
    // один shared_ptr снова
    EXPECT_GE(limiter1.use_count(), 1);
}

// limiter функционален после получения из manager
TEST_F(UserTrafficManagerTest, LimiterFunctional)
{
    auto limiter = manager_.get_or_create_user("192.168.1.1");
    
    ASSERT_NE(limiter, nullptr);
    
    // пробуем использовать limiter
    std::size_t allowed = limiter->acquire(1000);
    EXPECT_GT(allowed, 0);
}

// разные manager создают независимые limiters
TEST_F(UserTrafficManagerTest, IndependentManagers)
{
    User_traffic_manager manager2;
    
    std::string ip = "192.168.1.1";
    auto limiter1 = manager_.get_or_create_user(ip);
    auto limiter2 = manager2.get_or_create_user(ip);
    
    EXPECT_NE(limiter1.get(), limiter2.get());
}

// стресс тест - создание и уничтожение
TEST_F(UserTrafficManagerTest, StressTest)
{
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i)
    {
        threads.emplace_back([this]()
        {
            for (int j = 0; j < 100; ++j)
            {
                std::string ip = "192.168." + std::to_string(j % 10) + "." + std::to_string(j);
                auto limiter = manager_.get_or_create_user(ip);
                limiter->acquire(100);
                // limiter автоматически уничтожается
            }
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
}

// длинные ip строки
TEST_F(UserTrafficManagerTest, LongIpStrings)
{
    std::string long_ip(1000, 'x');
    auto limiter = manager_.get_or_create_user(long_ip);
    
    ASSERT_NE(limiter, nullptr);
    
    // получаем тот же
    auto limiter2 = manager_.get_or_create_user(long_ip);
    EXPECT_EQ(limiter.get(), limiter2.get());
}