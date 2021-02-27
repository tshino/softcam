#pragma once

#include <memory>
#include <cstdint>


namespace softcam {


/// Time Measurement and Sleep
class Timer
{
 public:
    Timer();

    float   get();
    void    rewind(float delta);
    void    reset();

    static void     sleep(float seconds);

 private:
    std::uint64_t   m_clock;
    std::uint64_t   m_frequency;
};


/// Inter-process Mutex
class NamedMutex
{
 public:
    explicit NamedMutex(const char* name);

    void        lock();
    void        unlock();

 private:
    std::shared_ptr<void>   m_handle;

    static void closeHandle(void*);
};


/// Inter-process Shared Memory
class SharedMemory
{
 public:
    SharedMemory() {}
    static SharedMemory create(const char* name, unsigned long size);
    static SharedMemory open(const char* name);

    explicit operator bool() const { return get() != nullptr; }

    unsigned long   size() const { return m_size; }
    void*           get() { return m_address.get(); }
    const void*     get() const { return m_address.get(); }

 private:
    std::shared_ptr<void>   m_handle;
    std::shared_ptr<void>   m_address;
    unsigned long           m_size = 0;

    explicit SharedMemory(const char* name, unsigned long size);
    explicit SharedMemory(const char* name);
    void    release();

    static void closeHandle(void*);
    static void unmap(void* ptr);
};


} //namespace softcam
