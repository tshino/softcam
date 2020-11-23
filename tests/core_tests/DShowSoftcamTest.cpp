#include <softcamcore/DShowSoftcam.h>
#include <gtest/gtest.h>


namespace {
namespace sc = softcam;


// This is a dummy GUID representing nothing.
// {12A54BBA-9F51-41B0-B331-0C3B08D1269F}
const GUID SOME_GUID =
{ 0x12a54bba, 0x9f51, 0x41b0, { 0xb3, 0x31, 0xc, 0x3b, 0x8, 0xd1, 0x26, 0x9f } };


TEST(Softcam, CreateInstance)
{
    HRESULT hr = 555;
    CUnknown* ret = sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);

    EXPECT_EQ( hr, S_OK );
    ASSERT_NE( ret, nullptr );

    delete static_cast<sc::Softcam*>(ret);
}

TEST(Softcam, QueryInterface)
{
    HRESULT hr = 555;
    sc::Softcam* softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam, nullptr );
    softcam->AddRef();

    {
        void *ptr = nullptr;
        hr = softcam->QueryInterface(IID_IBaseFilter, &ptr);
        EXPECT_EQ( hr, S_OK );
        ASSERT_NE( ptr, nullptr );
    }{
        void *ptr = nullptr;
        hr = softcam->QueryInterface(IID_IAMovieSetup, &ptr);
        EXPECT_EQ( hr, S_OK );
        ASSERT_NE( ptr, nullptr );
    }{
        void *ptr = nullptr;
        hr = softcam->QueryInterface(IID_IAMStreamConfig, &ptr);
        EXPECT_EQ( hr, S_OK );
        ASSERT_NE( ptr, nullptr );
    }

    softcam->Release();
}

TEST(Softcam, AttributesNoServer)
{
    HRESULT hr = 555;
    sc::Softcam* softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam, nullptr );
    softcam->AddRef();

    EXPECT_EQ( softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam->valid(), false );
    EXPECT_EQ( softcam->width(), 0 );
    EXPECT_EQ( softcam->height(), 0 );
    EXPECT_EQ( softcam->framerate(), 0.0f );

    softcam->Release();
}

TEST(Softcam, AttributesNormal)
{
    auto fb = sc::FrameBuffer::create(320, 240, 60);

    HRESULT hr = 555;
    sc::Softcam* softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam, nullptr );
    softcam->AddRef();

    EXPECT_NE( softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam->valid(), true );
    EXPECT_EQ( softcam->width(), 320 );
    EXPECT_EQ( softcam->height(), 240 );
    EXPECT_EQ( softcam->framerate(), 60.0f );

    softcam->Release();
}


} //namespace
