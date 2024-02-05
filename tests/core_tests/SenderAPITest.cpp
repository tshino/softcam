#include <softcamcore/SenderAPI.h>
#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <chrono>
#include <cmath>
#include <softcamcore/FrameBuffer.h>
#include <softcamcore/Misc.h>


namespace SenderAPITest {
namespace sc = softcam;
namespace sender = softcam::sender;

#define SLEEP_MS(msec) \
        std::this_thread::sleep_for(std::chrono::milliseconds(msec))
#define WAIT_FOR_FLAG_CHANGE(atomic_flag, last_value) [&]{ \
            while (atomic_flag == last_value) { SLEEP_MS(1); } \
        }()

TEST(SenderCreateCamera, Basic)
{
    {
        auto handle = sender::CreateCamera(320, 240, 60);
        EXPECT_TRUE( handle );

        auto fb = sc::FrameBuffer::open();
        EXPECT_EQ( fb.width(), 320 );
        EXPECT_EQ( fb.height(), 240 );
        EXPECT_EQ( fb.framerate(), 60 );

        EXPECT_NO_THROW({ sender::DeleteCamera(handle); });
    }{
        auto handle = sender::CreateCamera(1920, 1080, 30);
        EXPECT_TRUE( handle );

        auto fb = sc::FrameBuffer::open();
        EXPECT_EQ( fb.width(), 1920 );
        EXPECT_EQ( fb.height(), 1080 );
        EXPECT_EQ( fb.framerate(), 30 );

        EXPECT_NO_THROW({ sender::DeleteCamera(handle); });
    }
}

TEST(SenderCreateCamera, FramerateIsOptional) {
    auto handle = sender::CreateCamera(320, 240);
    EXPECT_TRUE( handle );

    auto fb = sc::FrameBuffer::open();
    EXPECT_EQ( fb.framerate(), 60 );

    sender::DeleteCamera(handle);
}

TEST(SenderCreateCamera, ZeroMeansUnlimitedVariableFramerate) {
    auto handle = sender::CreateCamera(320, 240, 0.0f);
    EXPECT_TRUE( handle );

    auto fb = sc::FrameBuffer::open();
    EXPECT_EQ( fb.framerate(), 0.0f );

    sender::DeleteCamera(handle);
}

TEST(SenderCreateCamera, InvalidArgs) {
    {
        auto handle = sender::CreateCamera(0, 240, 60);
        EXPECT_FALSE( handle );
        sender::DeleteCamera(handle);
    }{
        auto handle = sender::CreateCamera(320, 0, 60);
        EXPECT_FALSE( handle );
        sender::DeleteCamera(handle);
    }{
        auto handle = sender::CreateCamera(0, 0, 60);
        EXPECT_FALSE( handle );
        sender::DeleteCamera(handle);
    }{
        auto handle = sender::CreateCamera(-320, 240, 60);
        EXPECT_FALSE( handle );
        sender::DeleteCamera(handle);
    }{
        auto handle = sender::CreateCamera(320, -240, 60);
        EXPECT_FALSE( handle );
        sender::DeleteCamera(handle);
    }{
        auto handle = sender::CreateCamera(320, 240, -60);
        EXPECT_FALSE( handle );
        sender::DeleteCamera(handle);
    }
}

TEST(SenderDeleteCamera, InvalidArgs)
{
    auto handle = sender::CreateCamera(320, 240);
    int x = 0;
    EXPECT_NO_THROW({ sender::DeleteCamera(&x); });
    EXPECT_NO_THROW({ sender::DeleteCamera(nullptr); });
    sender::DeleteCamera(handle); // correct
    EXPECT_NO_THROW({ sender::DeleteCamera(handle); }); // double free
}

TEST(SenderSendFrame, Basic)
{
    const float TIMEOUT = 1.0f;
    const unsigned char COLOR_VALUE = 123;

    auto handle = sender::CreateCamera(320, 240);
    std::atomic<int> flag = 0;

    std::thread th([&]
    {
        auto fb = sc::FrameBuffer::open();
        ASSERT_TRUE( fb );

        EXPECT_EQ( fb.frameCounter(), 0 );
        flag = 1;

        fb.waitForNewFrame(0, TIMEOUT);
        EXPECT_EQ( fb.frameCounter(), 1 );

        unsigned char image[320 * 240 * 3];
        uint64_t frame_counter = 0;
        fb.transferToDIB(image, &frame_counter);
        EXPECT_EQ( image[0], COLOR_VALUE );
        EXPECT_EQ( image[320 * 240 * 3 - 1], COLOR_VALUE );
        EXPECT_EQ( frame_counter, 1 );
    });

    WAIT_FOR_FLAG_CHANGE(flag, 0);

    unsigned char image[320 * 240 * 3] = {};
    std::memset(image, COLOR_VALUE, sizeof(image));
    sender::SendFrame(handle, image);

    th.join();
    sender::DeleteCamera(handle);
}

TEST(SenderSendFrame, SendsFirstFrameImmediately)
{
    auto handle = sender::CreateCamera(320, 240);
    unsigned char image[320 * 240 * 3] = {};

    sc::Timer timer;
    sender::SendFrame(handle, image);   // first
    auto lap = timer.get();

    EXPECT_LE( lap, 0.002f );

    sender::DeleteCamera(handle);
}

TEST(SenderSendFrame, SendsEveryFrameImmediatelyIfZeroFramerate)
{
    const float FRAMERATE = 0.0f;
    auto handle = sender::CreateCamera(320, 240, FRAMERATE);
    unsigned char image[320 * 240 * 3] = {};

    sc::Timer timer;
    sender::SendFrame(handle, image);
    auto lap = timer.get();

    EXPECT_LE( lap, 0.002f );

    timer.reset();
    sender::SendFrame(handle, image);
    lap = timer.get();

    EXPECT_LE( lap, 0.002f );

    timer.reset();
    sender::SendFrame(handle, image);
    lap = timer.get();

    EXPECT_LE( lap, 0.002f );

    sender::DeleteCamera(handle);
}

TEST(SenderSendFrame, KeepsProperInterval)
{
    const float FRAMERATE = 20.0f;
    const float INTERVAL = 1.0f / FRAMERATE;
    auto handle = sender::CreateCamera(320, 240, FRAMERATE);
    unsigned char image[320 * 240 * 3] = {};

    sender::SendFrame(handle, image);   // first

    sc::Timer timer;
    sender::SendFrame(handle, image);   // second
    auto lap1 = timer.get();

    EXPECT_GE( lap1, INTERVAL * 1.0f - 0.010f );
    EXPECT_LE( lap1, INTERVAL * 1.0f + 0.010f );

    sender::SendFrame(handle, image);   // third
    auto lap2 = timer.get();

    EXPECT_GE( lap2, INTERVAL * 2.0f - 0.010f );
    EXPECT_LE( lap2, INTERVAL * 2.0f + 0.010f );

    sender::DeleteCamera(handle);
}

TEST(SenderSendFrame, KeepsProperIntervalEvenIfFirstFrameDelayed)
{
    const float FRAMERATE = 20.0f;
    const float INTERVAL = 1.0f / FRAMERATE;
    auto handle = sender::CreateCamera(320, 240, FRAMERATE);
    unsigned char image[320 * 240 * 3] = {};

    SLEEP_MS(30);  // delay

    sender::SendFrame(handle, image);   // first

    sc::Timer timer;
    sender::SendFrame(handle, image);   // second
    auto lap1 = timer.get();

    EXPECT_GE( lap1, INTERVAL * 1.0f - 0.010f );
    EXPECT_LE( lap1, INTERVAL * 1.0f + 0.010f );

    sender::SendFrame(handle, image);   // third
    auto lap2 = timer.get();

    EXPECT_GE( lap2, INTERVAL * 2.0f - 0.010f );
    EXPECT_LE( lap2, INTERVAL * 2.0f + 0.010f );

    sender::DeleteCamera(handle);
}

TEST(SenderSendFrame, KeepsProperIntervalEvenIfSomeFramesDelayed)
{
    const float FRAMERATE = 20.0f;
    const float INTERVAL = 1.0f / FRAMERATE;
    auto handle = sender::CreateCamera(320, 240, FRAMERATE);
    unsigned char image[320 * 240 * 3] = {};

    sender::SendFrame(handle, image);   // first

    sc::Timer timer;

    SLEEP_MS(30);  // delay
    sender::SendFrame(handle, image);   // second
    auto lap1 = timer.get();

    EXPECT_GE( lap1, INTERVAL * 1.0f - 0.010f );
    EXPECT_LE( lap1, INTERVAL * 1.0f + 0.010f );

    SLEEP_MS(20);  // delay
    sender::SendFrame(handle, image);   // third
    auto lap2 = timer.get();

    EXPECT_GE( lap2, INTERVAL * 2.0f - 0.010f );
    EXPECT_LE( lap2, INTERVAL * 2.0f + 0.010f );

    sender::DeleteCamera(handle);
}

TEST(SenderSendFrame, InvalidArgs)
{
    auto handle = sender::CreateCamera(320, 240);
    unsigned char image[320 * 240 * 3] = {};

    EXPECT_NO_THROW({ sender::SendFrame(nullptr, nullptr); });
    EXPECT_NO_THROW({ sender::SendFrame(nullptr, image); });
    EXPECT_NO_THROW({ sender::SendFrame(handle, nullptr); });

    auto fb = sc::FrameBuffer::open();
    EXPECT_EQ( fb.frameCounter(), 0 );

    sender::SendFrame(handle, image); // ok. ++frame_counter
    EXPECT_EQ( fb.frameCounter(), 1 );

    sender::DeleteCamera(handle);

    EXPECT_NO_THROW({ sender::SendFrame(handle, image); });
    EXPECT_EQ( fb.frameCounter(), 1 );
}

TEST(SenderWaitForConnection, ShouldBlockUntilReceiverConnected)
{
    auto handle = sender::CreateCamera(320, 240);
    std::atomic<int> flag = 0;

    std::thread th([&]
    {
        WAIT_FOR_FLAG_CHANGE(flag, 0);
        SLEEP_MS(10);
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

TEST(SenderWaitForConnection, ShouldTimeout)
{
    const float TIMEOUT = 0.5f;

    auto handle = sender::CreateCamera(320, 240);
    std::atomic<int> flag = 0;

    std::thread th([&]
    {
        WAIT_FOR_FLAG_CHANGE(flag, 0);
        SLEEP_MS(10);
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

TEST(SenderWaitForConnection, InvalidArgs)
{
    bool ret = sender::WaitForConnection(nullptr);
    EXPECT_EQ( ret, false );

    auto handle = sender::CreateCamera(320, 240);
    sender::DeleteCamera(handle);

    ret = sender::WaitForConnection(handle);
    EXPECT_EQ( ret, false );
}

TEST(SenderIsConnected, ReturnsFalseIfNotConnectedEver)
{
    auto handle = sender::CreateCamera(320, 240);

    bool ret = sender::IsConnected(handle);

    EXPECT_EQ( ret, false );
    sender::DeleteCamera(handle);
}

TEST(SenderIsConnected, ReturnsTrueIfAlreadyConnected)
{
    auto handle = sender::CreateCamera(320, 240);

    auto fb = sc::FrameBuffer::open();
    ASSERT_TRUE( fb );

    bool ret = sender::IsConnected(handle);

    EXPECT_EQ( ret, true );
    sender::DeleteCamera(handle);
}

TEST(SenderIsConnected, InvalidArgs)
{
    bool ret = sender::IsConnected(nullptr);
    EXPECT_EQ( ret, false );

    auto handle = sender::CreateCamera(320, 240);
    sender::DeleteCamera(handle);

    ret = sender::IsConnected(handle);
    EXPECT_EQ( ret, false );
}

} //namespace SenderAPITest
