#pragma once

#include <memory>
#include <functional>


namespace softcam {

class Watchdog
{
 public:
    Watchdog() {}
    static Watchdog     createHeartbeat(
                            float                       interval,
                            std::function<void()>       increment);
    static Watchdog     createMonitor(
                            float                       interval,
                            float                       timeout,
                            std::function<unsigned()>   read);

    void    stop();
    bool    alive() const;

 private:
    std::shared_ptr<void>   m_flag_holder;
};

} //namespace softcam
