#include <softcamcore/DShowSoftcam.h>
#include <gtest/gtest.h>

#include <memory>


namespace {
namespace sc = softcam;


std::unique_ptr<sc::FrameBuffer> createFrameBufer(int width, int height, float framerate)
{
    return std::make_unique<sc::FrameBuffer>(sc::FrameBuffer::create(width, height, framerate));
}

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
        EXPECT_EQ( (IBaseFilter*)ptr, softcam );
    }{
        void *ptr = nullptr;
        hr = softcam->QueryInterface(IID_IAMovieSetup, &ptr);
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( (IAMovieSetup*)ptr, softcam );
    }{
        void *ptr = nullptr;
        hr = softcam->QueryInterface(IID_IAMStreamConfig, &ptr);
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( (IAMStreamConfig*)ptr, softcam );
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
    auto fb = createFrameBufer(320, 240, 60);

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

TEST(Softcam, AttributesDeactivatedServer)
{
    auto fb = createFrameBufer(320, 240, 60);
    fb->deactivate();

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

TEST(Softcam, AttributesMultipleReceiver)
{
    auto fb = createFrameBufer(320, 240, 60);

    HRESULT hr = 555;
    sc::Softcam* softcam1 = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam1, nullptr );
    softcam1->AddRef();
    sc::Softcam* softcam2 = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam2, nullptr );
    softcam2->AddRef();

    EXPECT_NE( softcam1->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam1->valid(), true );
    EXPECT_EQ( softcam1->width(), 320 );
    EXPECT_EQ( softcam1->height(), 240 );
    EXPECT_EQ( softcam1->framerate(), 60.0f );
    EXPECT_NE( softcam2->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam2->valid(), true );
    EXPECT_EQ( softcam2->width(), 320 );
    EXPECT_EQ( softcam2->height(), 240 );
    EXPECT_EQ( softcam2->framerate(), 60.0f );

    softcam1->Release();
    softcam2->Release();
}

TEST(Softcam, AttributesRebootingReceiver)
{
    auto fb = createFrameBufer(320, 240, 60);

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

    softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam, nullptr );
    softcam->AddRef();

    EXPECT_NE( softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam->valid(), true );
    EXPECT_EQ( softcam->width(), 320 );
    EXPECT_EQ( softcam->height(), 240 );
    EXPECT_EQ( softcam->framerate(), 60.0f );

    softcam->Release();
}

TEST(Softcam, AttributesRebootingSender)
{
    auto fb = createFrameBufer(320, 240, 60);

    HRESULT hr = 555;
    sc::Softcam* softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam, nullptr );
    softcam->AddRef();

    ASSERT_NE( softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam->getFrameBuffer()->active(), true );
    EXPECT_EQ( softcam->valid(), true );
    EXPECT_EQ( softcam->width(), 320 );
    EXPECT_EQ( softcam->height(), 240 );
    EXPECT_EQ( softcam->framerate(), 60.0f );

    fb->deactivate();

    ASSERT_NE( softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam->getFrameBuffer()->active(), false );

    softcam->releaseFrameBuffer();

    EXPECT_EQ( softcam->getFrameBuffer(), nullptr );

    fb.reset();

    EXPECT_EQ( softcam->getFrameBuffer(), nullptr );

    fb = createFrameBufer(320, 240, 60);

    ASSERT_NE( softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam->getFrameBuffer()->active(), true );
    EXPECT_EQ( softcam->valid(), true );
    EXPECT_EQ( softcam->width(), 320 );
    EXPECT_EQ( softcam->height(), 240 );
    EXPECT_EQ( softcam->framerate(), 60.0f );

    softcam->Release();
}

TEST(Softcam, IAMStreamConfigNoServer)
{
    HRESULT hr = 555;
    sc::Softcam* softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam, nullptr );
    softcam->AddRef();

    IAMStreamConfig *amsc = softcam;
    AM_MEDIA_TYPE *pmt = nullptr;
    hr = amsc->GetFormat(&pmt);
    EXPECT_EQ( hr, E_FAIL );
    EXPECT_EQ( pmt, nullptr );

    AM_MEDIA_TYPE mt{};
    hr = amsc->SetFormat(&mt);
    EXPECT_EQ( hr, E_FAIL );

    int count = 55, size = 77;
    hr = amsc->GetNumberOfCapabilities(&count, &size);
    EXPECT_EQ( hr, E_FAIL );
    EXPECT_EQ( count, 55 );
    EXPECT_EQ( size, 77 );

    BYTE scc[sizeof(VIDEO_STREAM_CONFIG_CAPS)];
    hr = amsc->GetStreamCaps(0, &pmt, scc);
    EXPECT_EQ( hr, E_FAIL );

    softcam->Release();
}


} //namespace
