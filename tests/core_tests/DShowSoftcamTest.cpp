#include <softcamcore/DShowSoftcam.h>
#include <gtest/gtest.h>


namespace {
namespace sc = softcam;

// {12A54BBA-9F51-41B0-B331-0C3B08D1269F}
const GUID SOME_GUID =
{ 0x12a54bba, 0x9f51, 0x41b0, { 0xb3, 0x31, 0xc, 0x3b, 0x8, 0xd1, 0x26, 0x9f } };


TEST(Softcam, CreateInstance)
{
    HRESULT hr = 555;
    CUnknown* ret = sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);

    EXPECT_EQ( hr, S_OK );
    ASSERT_NE( ret, nullptr );

    static_cast<sc::Softcam*>(ret)->Release();
}

TEST(Softcam, QueryInterface)
{
    HRESULT hr = 555;
    sc::Softcam* softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam, nullptr );

    {
        void *ptr = nullptr;
        hr = softcam->QueryInterface(IID_IAMStreamConfig, &ptr);
        EXPECT_EQ( hr, S_OK );
        ASSERT_NE( ptr, nullptr );
    }

    softcam->Release();
}


} //namespace
