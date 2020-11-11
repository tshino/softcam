#include <softcamcore/SenderAPI.h>
#include <gtest/gtest.h>

#include <softcamcore/FrameBuffer.h>
#include <atomic>
#include <thread>
#include <chrono>


namespace {
namespace sc = softcam;
namespace sender = softcam::sender;

#define SLEEP(msec) \
        std::this_thread::sleep_for(std::chrono::milliseconds(msec))
#define WAIT_FOR_FLAG_CHANGE(atomic_flag, last_value) [&]{ \
            while (atomic_flag == last_value) { SLEEP(1); } \
        }()

TEST(WaitForConnection, ShouldBlockUntilReceiverConnected)
{
    auto handle = sender::CreateCamera(320, 240);
    std::atomic<int> flag = 0;

    std::thread th([&]
    {
        WAIT_FOR_FLAG_CHANGE(flag, 0);
        SLEEP(10);
        EXPECT_EQ( flag, 1 );

        auto fb = sc::FrameBuffer::open();
        ASSERT_TRUE( fb );

        WAIT_FOR_FLAG_CHANGE(flag, 1);
        EXPECT_EQ( flag, 2 );
    });

    flag = 1;
    bool ret = sender::WaitForConnection(handle);
    flag = 2;

    th.join();
    EXPECT_EQ( ret, true );
    sender::DeleteCamera(handle);
}

TEST(WaitForConnection, ShouldTimeout)
{
    const float TIMEOUT = 0.5f;

    auto handle = sender::CreateCamera(320, 240);
    std::atomic<int> flag = 0;

    std::thread th([&]
    {
        WAIT_FOR_FLAG_CHANGE(flag, 0);
        SLEEP(10);
        EXPECT_EQ( flag, 1 );

        WAIT_FOR_FLAG_CHANGE(flag, 1);
        EXPECT_EQ( flag, 2 );
    });

    flag = 1;
    bool ret = sender::WaitForConnection(handle, TIMEOUT);
    flag = 2;

    th.join();
    EXPECT_EQ( ret, false );
    sender::DeleteCamera(handle);
}

TEST(WaitForConnection, InvalidArgs)
{
    bool ret = sender::WaitForConnection(nullptr);
    EXPECT_EQ( ret, false );
}

} //namespace
