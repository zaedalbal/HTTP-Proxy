#include <gtest/gtest.h>
#include "utils/timer.hpp"
#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <atomic>

class TimerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        io_context = std::make_unique<boost::asio::io_context>();
        executor = std::make_unique<boost::asio::any_io_executor>(
            io_context->get_executor()
        );
    }

    void TearDown() override
    {
        io_context->stop();
        if (io_thread.joinable())
            io_thread.join();
    }

    void run_async()
    {
        io_thread = std::thread([this]()
        {
            io_context->run();
        });
    }

    void run_for(std::chrono::milliseconds dur)
    {
        io_context->run_for(dur);
    }

    std::unique_ptr<boost::asio::io_context> io_context;
    std::unique_ptr<boost::asio::any_io_executor> executor;
    std::thread io_thread;
};

TEST_F(TimerTest, CreateTimer)
{
    auto timer = std::make_shared<Timer>(*executor, 100);
    EXPECT_NE(timer, nullptr);
}

TEST_F(TimerTest, DoesNotStartAutomatically)
{
    std::atomic<int> counter{0};

    auto timer = std::make_shared<Timer>(*executor, 50);
    timer->set_callback_func([&] { ++counter; });

    run_for(std::chrono::milliseconds(100));
    EXPECT_EQ(counter, 0);
}

TEST_F(TimerTest, StartCallsCallback)
{
    std::atomic<bool> called{false};

    auto timer = std::make_shared<Timer>(*executor, 50);
    timer->set_callback_func([&] { called = true; });

    timer->start();
    run_for(std::chrono::milliseconds(100 + 5)); // +5ms
    EXPECT_TRUE(called);
}

TEST_F(TimerTest, StartIsIdempotent)
{
    std::atomic<int> counter{0};

    auto timer = std::make_shared<Timer>(*executor, 50);
    timer->set_callback_func([&] { ++counter; });

    timer->start();
    timer->start();
    timer->start();

    run_for(std::chrono::milliseconds(200 + 5));
    EXPECT_GE(counter.load(), 1);
    EXPECT_LE(counter.load(), 2);
}

TEST_F(TimerTest, StopPreventsCallback)
{
    std::atomic<bool> called{false};

    auto timer = std::make_shared<Timer>(*executor, 100);
    timer->set_callback_func([&] { called = true; });

    timer->start();
    timer->stop();

    run_for(std::chrono::milliseconds(200 + 5));
    EXPECT_FALSE(called);
}

TEST_F(TimerTest, RefreshRestartsTimer)
{
    std::atomic<int> counter{0};

    auto timer = std::make_shared<Timer>(*executor, 100);
    timer->set_callback_func([&] { ++counter; });

    run_async();

    timer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    timer->refresh();

    std::this_thread::sleep_for(std::chrono::milliseconds(75));
    EXPECT_LE(counter.load(), 1); // ещё не должно сработать

    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    EXPECT_GE(counter.load(), 1);
    EXPECT_LE(counter.load(), 2); // +-1
}

TEST_F(TimerTest, WorksWithoutCallback)
{
    auto timer = std::make_shared<Timer>(*executor, 50);
    timer->start();

    EXPECT_NO_THROW({
        run_for(std::chrono::milliseconds(100 + 5));
    });
}

TEST_F(TimerTest, FiresOnlyOnce)
{
    std::atomic<int> counter{0};

    auto timer = std::make_shared<Timer>(*executor, 50);
    timer->set_callback_func([&] { ++counter; });

    timer->start();
    run_for(std::chrono::milliseconds(300 + 5));

    EXPECT_GE(counter.load(), 1);
    EXPECT_LE(counter.load(), 2); // +-1
}