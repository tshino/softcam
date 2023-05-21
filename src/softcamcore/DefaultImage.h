#pragma once

#include <string>
#include <memory>
#include <cstdint>


namespace softcam {


/// Default image
class DefaultImage
{
 public:
    static DefaultImage tryLoad(const std::string& file_path);
    static DefaultImage makeBlankImage(int width, int height);

    explicit operator bool() const { return m_valid; }

    int             width() const { return m_width; }
    int             height() const { return m_height; }
    const void*     imageBits() const { return m_bits.get(); }

 private:
    bool        m_valid = false;
    int         m_width = 0;
    int         m_height = 0;
    std::unique_ptr<std::uint8_t[]> m_bits;
};


} //namespace softcam
