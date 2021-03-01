#include "Misc.h"

#include <windows.h>
#include <cmath>
#include <cassert>


namespace softcam {


Timer::Timer()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&m_clock);
    QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
}

float Timer::get()
{
    std::uint64_t now;
    QueryPerformanceCounter((LARGE_INTEGER*)&now);
    float elapsed = (float)((double)int64_t(now - m_clock) / (double)m_frequency);
    return elapsed;
}

void Timer::rewind(float delta)
{
    uint64_t delta_clock = (uint64_t)std::round(delta * (double)m_frequency);
    m_clock += delta_clock;
}

void Timer::reset()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&m_clock);
}

void Timer::sleep(float seconds)
{
    if (seconds <= 0.0f)
    {
        return;
    }
    unsigned delay_msec = (unsigned)std::round(seconds * 1000.0f);
    if (delay_msec == 0)
    {
        delay_msec = 1;
    }
    HANDLE e = CreateEventA(nullptr, false, false, nullptr);
    if (e == nullptr)
    {
        // fallback to older API
        Sleep(delay_msec);
        return;
    }
    MMRESULT ret = timeSetEvent(
                    delay_msec,
                    1,
                    (LPTIMECALLBACK)e,
                    0,
                    TIME_ONESHOT | TIME_CALLBACK_EVENT_SET);
    if (ret == 0)
    {
        // fallback to older API
        CloseHandle(e);
        Sleep(delay_msec);
        return;
    }
    WaitForSingleObject(e, INFINITE);
    CloseHandle(e);
}


NamedMutex::NamedMutex(const char* name) :
    m_handle(CreateMutexA(nullptr, false, name), closeHandle)
{
    assert( m_handle.get() != nullptr && "Creating a named mutex failed" );
}

void NamedMutex::lock()
{
    WaitForSingleObject(m_handle.get(), INFINITE);
}

void NamedMutex::unlock()
{
    bool ret = ReleaseMutex(m_handle.get());

    assert( ret == true && "Tried to release a mutex that is not locked" );
    (void)ret;
}

void NamedMutex::closeHandle(void* ptr)
{
    if (ptr)
    {
        #ifndef NDEBUG
        // checks for the error of closing still owned mutex
        bool ret1 = ReleaseMutex(ptr);
        assert( ret1 == false && "Tried to delete a mutex that is locked" );
        #endif

        bool ret2 = CloseHandle(ptr);

        assert( ret2 == true && "CloseHandle() for a mutex failed" );
        (void)ret2;
    }
}

SharedMemory
SharedMemory::create(const char* name, unsigned long size)
{
    return SharedMemory(name, size);
}

SharedMemory
SharedMemory::open(const char* name)
{
    return SharedMemory(name);
}

SharedMemory::SharedMemory(const char* name, unsigned long size)
{
    m_handle.reset(
        CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, name),
        closeHandle);
    if (m_handle && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        m_address.reset(
            MapViewOfFile(m_handle.get(), FILE_MAP_WRITE, 0, 0, 0),
            unmap);
        if (m_address)
        {
            m_size = size;
            return;
        }
    }
    release();
}

SharedMemory::SharedMemory(const char* name)
{
    m_handle.reset(
        OpenFileMappingA(FILE_MAP_WRITE, false, name),
        closeHandle);
    if (m_handle)
    {
        m_address.reset(
            MapViewOfFile(m_handle.get(), FILE_MAP_WRITE, 0, 0, 0),
            unmap);
        if (m_address)
        {
            MEMORY_BASIC_INFORMATION meminfo;
            if (0 < VirtualQuery(m_address.get(), &meminfo, sizeof(meminfo)))
            {
                m_size = (unsigned long)meminfo.RegionSize;
                return;
            }
        }
    }
    release();
}

void
SharedMemory::release()
{
    m_size = 0;
    m_address.reset();
    m_handle.reset();
}

void
SharedMemory::closeHandle(void* ptr)
{
    if (ptr)
    {
        bool ret = CloseHandle(ptr);

        assert( ret == true && "CloseHandle() for a shared memory failed" );
        (void)ret;
    }
}

void
SharedMemory::unmap(void* ptr)
{
    if (ptr)
    {
        UnmapViewOfFile(ptr);
    }
}

} //namespace softcam
