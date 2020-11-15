#include "Watchdog.h"

#include <thread>
#include <atomic>
#include "Misc.h"


namespace softcam {

Watchdog Watchdog::createHeartbeat(
                            float                       interval,
                            std::function<void()>       increment)
{
    struct Heartbeat
    {
        std::atomic<bool>   m_quit = false;
        std::atomic<bool>   m_alive = true; // dummy
        std::thread         m_thread;
        ~Heartbeat()
        {
            m_quit = true;
            m_thread.join();
        }
    };

    auto heartbeat = std::make_shared<Heartbeat>();
    auto quit = &heartbeat->m_quit;

    heartbeat->m_thread = std::thread([quit, interval, increment]
    {
        while (!quit->load())
        {
            Timer::sleep(interval);
            increment();
        }
    });

    std::shared_ptr<std::atomic<bool>> holder(heartbeat, &heartbeat->m_alive);

    Watchdog wd;
    wd.m_flag_holder = holder;
    return wd;
}

Watchdog Watchdog::createMonitor(
                            float                       interval,
                            float                       timeout,
                            std::function<unsigned()>   read)
{
    struct Monitor
    {
        std::atomic<bool>   m_quit = false;
        std::atomic<bool>   m_alive = true;
        std::thread         m_thread;
        ~Monitor()
        {
            m_quit = true;
            m_thread.join();
        }
    };

    auto monitor = std::make_shared<Monitor>();
    auto ptr = monitor.get();

    monitor->m_thread = std::thread([ptr, interval, timeout, read]
    {
        unsigned last_value = read();
        Timer timer;
        while (!ptr->m_quit.load())
        {
            Timer::sleep(interval);
            unsigned value = read();
            if (last_value != value)
            {
                last_value = value;
                timer.reset();
            }
            if (timeout < timer.get())
            {
                ptr->m_alive = false;
                break;
            }
        }
    });

    std::shared_ptr<std::atomic<bool>> holder(monitor, &monitor->m_alive);

    Watchdog wd;
    wd.m_flag_holder = holder;
    return wd;
}

void Watchdog::stop()
{
    m_flag_holder.reset();
}

bool Watchdog::alive() const
{
    if (m_flag_holder)
    {
        std::atomic<bool> *flag = static_cast<std::atomic<bool>*>(m_flag_holder.get());
        return flag->load();
    }
    return false;
}

} //namespace softcam
