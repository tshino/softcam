#include "DefaultImage.h"

#include <utility>


namespace softcam {


DefaultImage
DefaultImage::tryLoad(const std::string& file_path)
{
    // TODO: check the file and try load image from the file
    return {};
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
