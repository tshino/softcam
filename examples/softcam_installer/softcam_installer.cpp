#include <windows.h>
#include <string>
#include <cstring>
#include <cstdio>


void Message(const std::string& message)
{
    MessageBoxA(NULL, message.c_str(), "Softcam Installer", MB_OK);
}


std::string ToHex(long x)
{
    char buff[128];
    std::snprintf(buff, sizeof(buff), "%08lx\n", x);
    return buff;
}


HMODULE LoadDLL(const std::string& path)
{
    HMODULE hmod = LoadLibraryA(path.c_str());
    if (!hmod)
    {
        Message("Error: can't load DLL");
        std::exit(1);
    }
    return hmod;
}


template <typename Func>
Func* GetProc(HMODULE hmod, const std::string& name)
{
    Func* func = (Func*)GetProcAddress(hmod, name.c_str());
    if (!func)
    {
        Message("Error: can't find function " + name + " in DLL");
        std::exit(1);
    }
    return func;
}


int WINAPI WinMain(
    HINSTANCE /*hInstance*/,
    HINSTANCE /*hPrevInstance*/,
    LPSTR /*lpszCmdLine*/,
    int /*nCmdShow*/)
{
    if (__argc != 3)
    {
        Message(
            "Usage:\n"
            "   softcam_installer.exe register <softcam.dll path>\n"
            "   softcam_installer.exe unregister <softcam.dll path>");
        return 0;
    }

    std::string cmd = __argv[1], path = __argv[2];

    if (cmd == "register")
    {
        auto hmod = LoadDLL(path);
        auto RegisterServer = GetProc<HRESULT STDAPICALLTYPE()>(hmod, "DllRegisterServer");

        auto hr = RegisterServer();

        if (FAILED(hr))
        {
            Message("Error: registration failed (" + ToHex(hr) + ")");
            return 1;
        }

        Message("softcam.dll has been successfully registered to the system");
        return 0;
    }
    else if (cmd == "unregister")
    {
        auto hmod = LoadDLL(path);
        auto UnregisterServer = GetProc<HRESULT STDAPICALLTYPE()>(hmod, "DllUnregisterServer");

        auto hr = UnregisterServer();

        if (FAILED(hr))
        {
            Message("Error: registration failed (" + ToHex(hr) + ")");
            return 1;
        }

        Message("softcam.dll has been successfully unregistered from the system");
        return 0;
    }
    else
    {
        Message("Error: invalid option");
        return 1;
    }
}
