#include "DefaultImage.h"

#include <utility>
#include <locale.h>
#include <shlwapi.h>
#include <wincodec.h>


#pragma comment(lib, "Windowscodecs.lib")


namespace {

std::wstring ToWString(const std::string& str)
{
    size_t len = 0, converted = 0;
    _locale_t loc = _create_locale(LC_ALL, "");
    _mbstowcs_s_l(&len, nullptr, 0, str.data(), str.size() + 1, loc);
    std::wstring wstr(len, L'\0');
    _mbstowcs_s_l(&converted, (wchar_t*)wstr.data(), len, str.data(), _TRUNCATE, loc);
    _free_locale(loc);
    return std::wstring(wstr.c_str());
}

}


namespace softcam {


DefaultImage
DefaultImage::tryLoad(const std::string& file_path)
{
    if (!PathFileExistsA(file_path.c_str()))
    {
        return {};
    }
    if (FAILED(CoInitialize(nullptr)))
    {
        return {};
    }
    DefaultImage img;
    IWICImagingFactory* factory = nullptr;
    if (!FAILED(CoCreateInstance(
            CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory, (void**)&factory)))
    {
        IWICBitmapDecoder* decoder = nullptr;
        if (!FAILED(factory->CreateDecoderFromFilename(
                ToWString(file_path).c_str(), nullptr, GENERIC_READ,
                WICDecodeMetadataCacheOnDemand, &decoder)))
        {
            IWICBitmapFrameDecode* decode = nullptr;
            if (!FAILED(decoder->GetFrame(0, &decode)))
            {
                IWICFormatConverter* converter = nullptr;
                if (!FAILED(factory->CreateFormatConverter(&converter)))
                {
                    do
                    {
                        if (FAILED(converter->Initialize(
                                decode, GUID_WICPixelFormat24bppBGR, WICBitmapDitherTypeNone,
                                nullptr, 0.0, WICBitmapPaletteTypeCustom)))
                        {
                            break;
                        }
                        UINT w, h;
                        if (FAILED(converter->GetSize(&w, &h)))
                        {
                            break;
                        }
                        if (w < 1 || w > 16384 || w % 4 != 0 ||
                            h < 1 || h > 16384 || h % 4 != 0)
                        {
                            break;
                        }
                        auto bits = std::make_unique<std::uint8_t[]>(h*w*3);
                        if (FAILED(converter->CopyPixels(nullptr, w*3, h*w*3, bits.get())))
                        {
                            break;
                        }
                        img.m_valid = true;
                        img.m_width = (int)w;
                        img.m_height = (int)h;
                        img.m_bits = std::move(bits);
                    } while (false);
                    converter->Release();
                }
                decode->Release();
            }
            decoder->Release();
        }
        factory->Release();
    }
    CoUninitialize();
    return img;
}

DefaultImage
DefaultImage::makeBlankImage(int width, int height)
{
    DefaultImage img;
    img.m_valid = true;
    img.m_width = width;
    img.m_height = height;

    auto buffer_size = height * width * 3;
    img.m_bits = std::make_unique<std::uint8_t[]>(buffer_size);
    std::memset(img.m_bits.get(), 0x00, buffer_size);

    return std::move(img);
}


} //namespace softcam
