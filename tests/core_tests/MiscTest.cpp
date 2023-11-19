#include <softcamcore/Misc.h>
#include <gtest/gtest.h>

#include <cstring>
#include <thread>
#include <atomic>
#include <cmath>


namespace MiscTest {
namespace sc = softcam;

const char SHMEM_NAME[] = "shmemtest";
const char MUTEX_NAME[] = "shmemtest_mutex";
const char ANOTHER_NAME[] = "shmemtest2";
const unsigned long SHMEM_SIZE = 888;
const char SOME_DATA[] = "Hello, world!";
const char ANOTHER_DATA[] = "12345";

#define WAIT_FOR(expr) [&]{ \
            while (!(expr)) { sc::Timer::sleep(0.01f); } \
        }()


TEST(Timer, Basic1) {
    sc::Timer timer;
    auto t1 = timer.get();
    auto t2 = timer.get();
    auto t3 = timer.get();

    EXPECT_GE( t1, 0.0f );
    EXPECT_GE( t2, t1 );
    EXPECT_GE( t3, t2 );
}

TEST(Timer, Sleep) {
    sc::Timer timer;
    EXPECT_NO_THROW({ sc::Timer::sleep(0.2f); });
    auto t = timer.get();

    EXPECT_GT( t, 0.0f );
}

TEST(Timer, SleepAccuracy) {

    float sum = 0.0f, sum_squared = 0.0f;
    for (int i = 0; i < 10; i++)
    {
        const float expected = (float)i * 0.005f;

        sc::Timer timer;
        sc::Timer::sleep(expected);
        float actual = timer.get();

        float error = actual - expected;
        sum += error;
        sum_squared += error * error;
    }

    float mean = sum / 10.0f;
    float variance = sum_squared / 10.0f - mean * mean;

    EXPECT_LT( std::fabs(mean), 0.003f );
    EXPECT_LT( std::sqrt(variance), 0.003f );
}

TEST(Timer, Rewind) {
    sc::Timer timer;
    EXPECT_NO_THROW({ timer.rewind(0.2f); });
    auto t = timer.get();

    EXPECT_LT( t, 0.0f );
}

TEST(Timer, Reset) {
    sc::Timer timer;
    timer.sleep(0.2f);
    auto t1 = timer.get();
    timer.reset();
    auto t2 = timer.get();

    EXPECT_GE( t2, 0.0f );
    EXPECT_LT( t2, t1 );
}

TEST(NamedMutex, Basic)
{
    std::atomic<int> signal = 0;

    std::thread th1([&]
    {
        sc::NamedMutex mutex(MUTEX_NAME);
        mutex.lock();
        signal = 1;
        WAIT_FOR( signal >= 3 );
        mutex.unlock();
    });
    std::thread th2([&]
    {
        sc::NamedMutex mutex(MUTEX_NAME);
        WAIT_FOR( signal >= 2 );
        mutex.lock();
        signal = 4;
        mutex.unlock();
    });

    WAIT_FOR( signal >= 1 );
    EXPECT_EQ( signal.load(), 1 );
    signal = 2;
    sc::Timer::sleep(0.1f);
    EXPECT_EQ( signal.load(), 2 );
    signal = 3;
    WAIT_FOR( signal >= 4 );
    EXPECT_EQ( signal.load(), 4 );

    th1.join();
    th2.join();
}

TEST(SharedMemory, Basic1) {
    auto shmem = sc::SharedMemory::create(SHMEM_NAME, SHMEM_SIZE);

    EXPECT_TRUE( shmem );
    EXPECT_NE( shmem.get(), nullptr );
    EXPECT_GE( shmem.size(), SHMEM_SIZE );
}

TEST(SharedMemory, Basic2) {
    auto view1 = sc::SharedMemory::create(SHMEM_NAME, SHMEM_SIZE);
    auto view2 = sc::SharedMemory::open(SHMEM_NAME);

    EXPECT_TRUE( view1 );
    EXPECT_TRUE( view2 );
    EXPECT_NE( view1.get(), nullptr );
    EXPECT_NE( view2.get(), nullptr );
    EXPECT_NE( view1.get(), view2.get() );
    EXPECT_GE( view1.size(), SHMEM_SIZE );
    EXPECT_GE( view2.size(), SHMEM_SIZE );
}

TEST(SharedMemory, Basic3) {
    auto view1 = sc::SharedMemory::create(SHMEM_NAME, SHMEM_SIZE);
    ASSERT_TRUE( view1 );
    std::memcpy(view1.get(), SOME_DATA, sizeof(SOME_DATA));

    auto view2 = sc::SharedMemory::open(SHMEM_NAME);
    ASSERT_TRUE( view2 );
    EXPECT_EQ( std::memcmp(view2.get(), SOME_DATA, sizeof(SOME_DATA)), 0 );

    auto mutex = sc::NamedMutex(MUTEX_NAME);
    mutex.lock();
    std::memcpy(view1.get(), ANOTHER_DATA, sizeof(ANOTHER_DATA));
    mutex.unlock();

    mutex.lock();
    EXPECT_EQ( std::memcmp(view2.get(), ANOTHER_DATA, sizeof(ANOTHER_DATA)), 0 );
    mutex.unlock();
}

TEST(SharedMemory, InvalidArgs) {
    {
        auto shmem = sc::SharedMemory::create(SHMEM_NAME, 0);
        EXPECT_FALSE( shmem );
        EXPECT_EQ( shmem.get(), nullptr );
        EXPECT_EQ( shmem.size(), 0u );
    }{
        auto shmem = sc::SharedMemory::create("\\", SHMEM_SIZE);
        EXPECT_FALSE( shmem );
        EXPECT_EQ( shmem.get(), nullptr );
        EXPECT_EQ( shmem.size(), 0u );
    }{
        auto view1 = sc::SharedMemory::create(SHMEM_NAME, SHMEM_SIZE);
        auto view2 = sc::SharedMemory::open(ANOTHER_NAME);
        EXPECT_FALSE( view2 );
        EXPECT_EQ( view2.get(), nullptr );
        EXPECT_EQ( view2.size(), 0u );
    }
}

TEST(SharedMemory, OpenBeforeCreateFails) {
    auto view1 = sc::SharedMemory::open(SHMEM_NAME);
    auto view2 = sc::SharedMemory::create(SHMEM_NAME, SHMEM_SIZE);

    EXPECT_FALSE( view1 );
    EXPECT_TRUE( view2 );
    EXPECT_EQ( view1.get(), nullptr );
    EXPECT_NE( view2.get(), nullptr );
    EXPECT_EQ( view1.size(), 0u );
    EXPECT_GE( view2.size(), SHMEM_SIZE );
}

TEST(SharedMemory, MultipleCreateFails) {
    auto view1 = sc::SharedMemory::create(SHMEM_NAME, SHMEM_SIZE);
    auto view2 = sc::SharedMemory::create(SHMEM_NAME, SHMEM_SIZE);

    EXPECT_TRUE( view1 );
    EXPECT_FALSE( view2 );
    EXPECT_EQ( view2.get(), nullptr );
    EXPECT_EQ( view2.size(), 0u );
}

TEST(SharedMemory, MultipleOpenSucceeds) {
    auto view1 = sc::SharedMemory::create(SHMEM_NAME, SHMEM_SIZE);
    auto view2 = sc::SharedMemory::open(SHMEM_NAME);
    auto view3 = sc::SharedMemory::open(SHMEM_NAME);

    EXPECT_TRUE( view1 );
    EXPECT_TRUE( view2 );
    EXPECT_TRUE( view3 );
    EXPECT_NE( view1.get(), nullptr );
    EXPECT_NE( view2.get(), nullptr );
    EXPECT_NE( view3.get(), nullptr );
    EXPECT_NE( view1.get(), view2.get() );
    EXPECT_NE( view1.get(), view3.get() );
    EXPECT_NE( view2.get(), view3.get() );
    EXPECT_GE( view1.size(), SHMEM_SIZE );
    EXPECT_GE( view2.size(), SHMEM_SIZE );
    EXPECT_GE( view3.size(), SHMEM_SIZE );
}

} //namespace MiscTest
