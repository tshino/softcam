#include <softcamcore/Watchdog.h>
#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <chrono>


namespace WatchdogTest {
namespace sc = softcam;

#define SLEEP_S(sec) \
        std::this_thread::sleep_for(std::chrono::milliseconds((int)((sec) * 1000)))


TEST(Watchdog, Basic)
{
    const float HEARTBEAT_INTERVAL = 0.01f;
    const float MONITOR_INTERVAL = 0.01f;
    const float MONITOR_TIMEOUT = 0.20f;

    std::atomic<unsigned>   signal = 0;

    auto heartbeat = sc::Watchdog::createHeartbeat(
                        HEARTBEAT_INTERVAL,
                        [&] { ++signal; });
    auto monitor = sc::Watchdog::createMonitor(
                        MONITOR_INTERVAL,
                        MONITOR_TIMEOUT,
                        [&] { return signal.load(); });

    SLEEP_S(MONITOR_INTERVAL * 2);
    EXPECT_EQ( monitor.alive(), true );

    SLEEP_S(MONITOR_TIMEOUT * 2);
    EXPECT_EQ( monitor.alive(), true );

    heartbeat.stop();

    SLEEP_S(MONITOR_TIMEOUT + MONITOR_INTERVAL * 3);
    EXPECT_EQ( monitor.alive(), false );
}

TEST(Watchdog, Constructors)
{
    {
        sc::Watchdog wd;
        EXPECT_EQ( wd.alive(), false );
    }
    {
        sc::Watchdog hb = sc::Watchdog::createHeartbeat(0.1f, []{});
        EXPECT_EQ( hb.alive(), true );
    }
    {
        sc::Watchdog mon = sc::Watchdog::createMonitor(0.1f, 1.0f, []{ return 0; });
        EXPECT_EQ( mon.alive(), true );
    }
}

TEST(Watchdog, HeartbeatStops)
{
    auto heartbeat = sc::Watchdog::createHeartbeat(0.1f, []{});

    EXPECT_EQ( heartbeat.alive(), true );

    heartbeat.stop();
    EXPECT_EQ( heartbeat.alive(), false );
}

TEST(Watchdog, MonitorStops)
{
    auto monitor = sc::Watchdog::createMonitor(0.1f, 1.0f, [] { return 0; });

    EXPECT_EQ( monitor.alive(), true );

    monitor.stop();
    EXPECT_EQ( monitor.alive(), false );
}

TEST(Watchdog, HeartbeatUpdatesSignal)
{
    const float HEARTBEAT_INTERVAL = 0.01f;

    std::atomic<unsigned>   signal = 0;

    auto heartbeat = sc::Watchdog::createHeartbeat(
                        HEARTBEAT_INTERVAL,
                        [&] { ++signal; });

    SLEEP_S(HEARTBEAT_INTERVAL * 3);
    EXPECT_EQ( heartbeat.alive(), true );
    auto value1 = signal.load();
    EXPECT_NE( value1, 0u );

    SLEEP_S(HEARTBEAT_INTERVAL * 3);
    EXPECT_EQ( heartbeat.alive(), true );
    auto value2 = signal.load();
    EXPECT_NE( value2, value1 );

    heartbeat.stop();
    auto value3 = signal.load();
    EXPECT_EQ( heartbeat.alive(), false );

    SLEEP_S(HEARTBEAT_INTERVAL * 3);
    auto value4 = signal.load();
    EXPECT_EQ( value3, value4 );
}

TEST(Watchdog, MonitorContinues)
{
    const float MONITOR_INTERVAL = 0.01f;
    const float MONITOR_TIMEOUT = 0.20f;

    std::atomic<unsigned>   signal = 0;

    auto monitor = sc::Watchdog::createMonitor(
                        MONITOR_INTERVAL,
                        MONITOR_TIMEOUT,
                        [&] { return signal.load(); });

    for (int i = 0; i < 2 * MONITOR_TIMEOUT / MONITOR_INTERVAL; i++)
    {
        SLEEP_S(MONITOR_INTERVAL);
        ++signal;
        EXPECT_EQ( monitor.alive(), true );
    }
}

TEST(Watchdog, MonitorTimeouts)
{
    const float MONITOR_INTERVAL = 0.01f;
    const float MONITOR_TIMEOUT = 0.20f;

    auto monitor = sc::Watchdog::createMonitor(
                        MONITOR_INTERVAL,
                        MONITOR_TIMEOUT,
                        [&] { return 0; });

    SLEEP_S(MONITOR_INTERVAL * 3);
    EXPECT_EQ( monitor.alive(), true );

    SLEEP_S(MONITOR_TIMEOUT);
    EXPECT_EQ( monitor.alive(), false );
}

} //namespace WatchdogTest
