#include <softcamcore/DefaultImage.h>
#include <gtest/gtest.h>


namespace {
namespace sc = softcam;


TEST(DefaultImage, FileNotFound) {
    auto img = sc::DefaultImage::tryLoad("not_a_valid_file_path");

    EXPECT_FALSE( img );
    EXPECT_EQ( img.width(), 0 );
    EXPECT_EQ( img.height(), 0 );
    EXPECT_EQ( img.imageBits(), nullptr );
}

TEST(DefaultImage, BlankImage) {
    auto img = sc::DefaultImage::makeBlankImage(320, 200);

    EXPECT_TRUE( img );
    EXPECT_EQ( img.width(), 320 );
    EXPECT_EQ( img.height(), 200 );

    ASSERT_NE( img.imageBits(), nullptr );
    EXPECT_EQ( ((const std::uint8_t*)img.imageBits())[0], 0x00 );
    EXPECT_EQ( ((const std::uint8_t*)img.imageBits())[1], 0x00 );
    EXPECT_EQ( ((const std::uint8_t*)img.imageBits())[2], 0x00 );
    EXPECT_EQ( ((const std::uint8_t*)img.imageBits())[200*320*3-1], 0x00 );
}


} //namespace
