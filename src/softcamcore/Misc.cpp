#include "Misc.h"

#include <windows.h>
#include <cmath>


namespace {

const auto close = [](void* ptr) { if (ptr) CloseHandle(ptr); };
const auto unmap = [](void* ptr) { if (ptr) UnmapViewOfFile(ptr); };

} //namespace

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
    Sleep((int)(seconds * 1000.0f));
}


NamedMutex::NamedMutex(const char* name) :
    m_handle(CreateMutexA(nullptr, false, name), close)
{
}

void NamedMutex::lock()
{
    WaitForSingleObject(m_handle.get(), INFINITE);
}

void NamedMutex::unlock()
{
    ReleaseMutex(m_handle.get());
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

SharedMemory::SharedMemory(const char* name, unsigned long size) :
    m_handle(
        CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, name),
        close),
    m_address(
        (m_handle && GetLastError() != ERROR_ALREADY_EXISTS)
            ? MapViewOfFile(m_handle.get(), FILE_MAP_WRITE, 0, 0, 0) : nullptr,
        unmap),
    m_size(m_address ? size : (release(), 0))
{
}

SharedMemory::SharedMemory(const char* name) :
    m_handle(
        OpenFileMappingA(FILE_MAP_WRITE, false, name),
        close),
    m_address(
        m_handle
            ? MapViewOfFile(m_handle.get(), FILE_MAP_WRITE, 0, 0, 0) : nullptr,
        unmap)
{
    MEMORY_BASIC_INFORMATION meminfo;
    if (m_address && 0 < VirtualQuery(m_address.get(), &meminfo, sizeof(meminfo)))
    {
        m_size = (unsigned long)meminfo.RegionSize;
    }
    else
    {
        release();
    }
}

void
SharedMemory::release()
{
    m_size = 0;
    m_address.reset();
    m_handle.reset();
}


} //namespace softcam
