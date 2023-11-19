#include <softcamcore/DShowSoftcam.h>
#include <gtest/gtest.h>

#include <memory>
#include <atomic>
#include <thread>
#include <algorithm>


namespace DShowSoftcamTest {
namespace sc = softcam;


std::unique_ptr<sc::FrameBuffer> createFrameBufer(int width, int height, float framerate)
{
    return std::make_unique<sc::FrameBuffer>(sc::FrameBuffer::create(width, height, framerate));
}

void checkMediaType320x240(const AM_MEDIA_TYPE *pmt)
{
    ASSERT_NE( pmt, nullptr );
    EXPECT_EQ( pmt->majortype, MEDIATYPE_Video );
    EXPECT_EQ( pmt->subtype, MEDIASUBTYPE_RGB24 );
    EXPECT_EQ( pmt->bFixedSizeSamples, TRUE );
    EXPECT_EQ( pmt->bTemporalCompression, FALSE );
    EXPECT_EQ( pmt->lSampleSize, 320 * 240 * 3u );
    EXPECT_EQ( pmt->formattype, FORMAT_VideoInfo );
    ASSERT_GE( pmt->cbFormat, sizeof(VIDEOINFOHEADER) );

    ASSERT_NE( pmt->pbFormat, nullptr );
    VIDEOINFOHEADER* pFormat = (VIDEOINFOHEADER*)pmt->pbFormat;
    EXPECT_GT( pFormat->dwBitRate, 0u );
    EXPECT_GT( pFormat->AvgTimePerFrame, 0 );
    EXPECT_GE( pFormat->bmiHeader.biSize, sizeof(BITMAPINFOHEADER) );
    EXPECT_EQ( pFormat->bmiHeader.biWidth, 320 );
    EXPECT_EQ( pFormat->bmiHeader.biHeight, 240 );
    EXPECT_EQ( pFormat->bmiHeader.biPlanes, 1 );
    EXPECT_EQ( pFormat->bmiHeader.biBitCount, 24 );
    EXPECT_EQ( pFormat->bmiHeader.biCompression, (DWORD)BI_RGB );
    EXPECT_EQ( pFormat->bmiHeader.biSizeImage, 320 * 240 * 3u );
}

// This is a dummy GUID representing nothing.
// {12A54BBA-9F51-41B0-B331-0C3B08D1269F}
const GUID SOME_GUID =
{ 0x12a54bba, 0x9f51, 0x41b0, { 0xb3, 0x31, 0xc, 0x3b, 0x8, 0xd1, 0x26, 0x9f } };


class Softcam : public ::testing::Test
{
 protected:
    sc::Softcam*        m_softcam = nullptr;

    virtual void TearDown() override
    {
        if (m_softcam)
        {
            m_softcam->Release();
        }
    }
};

TEST_F(Softcam, CreateInstance)
{
    HRESULT hr = 555;
    CUnknown* ret = sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);

    EXPECT_EQ( hr, S_OK );
    ASSERT_NE( ret, nullptr );

    delete static_cast<sc::Softcam*>(ret);
}

TEST_F(Softcam, QueryInterface)
{
    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    {
        IBaseFilter *ptr = nullptr;
        hr = m_softcam->QueryInterface(IID_IBaseFilter, reinterpret_cast<void**>(&ptr));
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( ptr, m_softcam );
        if (ptr) ptr->Release();
    }{
        IAMovieSetup *ptr = nullptr;
        hr = m_softcam->QueryInterface(IID_IAMovieSetup, reinterpret_cast<void**>(&ptr));
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( ptr, m_softcam );
        if (ptr) ptr->Release();
    }{
        IAMStreamConfig *ptr = nullptr;
        hr = m_softcam->QueryInterface(IID_IAMStreamConfig, reinterpret_cast<void**>(&ptr));
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( ptr, m_softcam );
        if (ptr) ptr->Release();
    }
}

TEST_F(Softcam, AttributesNoServer)
{
    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    EXPECT_EQ( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->valid(), false );
    EXPECT_EQ( m_softcam->width(), 0 );
    EXPECT_EQ( m_softcam->height(), 0 );
    EXPECT_EQ( m_softcam->framerate(), 0.0f );
}

TEST_F(Softcam, AttributesNormal)
{
    auto fb = createFrameBufer(320, 240, 60);

    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    EXPECT_NE( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->valid(), true );
    EXPECT_EQ( m_softcam->width(), 320 );
    EXPECT_EQ( m_softcam->height(), 240 );
    EXPECT_EQ( m_softcam->framerate(), 60.0f );
}

TEST_F(Softcam, AttributesDeactivatedServer)
{
    auto fb = createFrameBufer(320, 240, 60);
    fb->deactivate();

    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    EXPECT_NE( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->valid(), true );
    EXPECT_EQ( m_softcam->width(), 320 );
    EXPECT_EQ( m_softcam->height(), 240 );
    EXPECT_EQ( m_softcam->framerate(), 60.0f );
}

TEST_F(Softcam, AttributesMultipleReceiver)
{
    auto fb = createFrameBufer(320, 240, 60);

    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();
    sc::Softcam* softcam2 = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( softcam2, nullptr );
    softcam2->AddRef();

    EXPECT_NE( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->valid(), true );
    EXPECT_EQ( m_softcam->width(), 320 );
    EXPECT_EQ( m_softcam->height(), 240 );
    EXPECT_EQ( m_softcam->framerate(), 60.0f );
    EXPECT_NE( softcam2->getFrameBuffer(), nullptr );
    EXPECT_EQ( softcam2->valid(), true );
    EXPECT_EQ( softcam2->width(), 320 );
    EXPECT_EQ( softcam2->height(), 240 );
    EXPECT_EQ( softcam2->framerate(), 60.0f );

    m_softcam->Release();
    m_softcam = nullptr;
    softcam2->Release();
    softcam2 = nullptr;

    fb.reset();
}

TEST_F(Softcam, AttributesRebootingReceiver)
{
    auto fb = createFrameBufer(320, 240, 60);

    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    EXPECT_NE( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->valid(), true );
    EXPECT_EQ( m_softcam->width(), 320 );
    EXPECT_EQ( m_softcam->height(), 240 );
    EXPECT_EQ( m_softcam->framerate(), 60.0f );

    m_softcam->Release();
    m_softcam = nullptr;

    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    EXPECT_NE( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->valid(), true );
    EXPECT_EQ( m_softcam->width(), 320 );
    EXPECT_EQ( m_softcam->height(), 240 );
    EXPECT_EQ( m_softcam->framerate(), 60.0f );

    m_softcam->Release();
    m_softcam = nullptr;

    fb.reset();
}

TEST_F(Softcam, AttributesRebootingSender)
{
    auto fb = createFrameBufer(320, 240, 60);

    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    ASSERT_NE( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->getFrameBuffer()->active(), true );
    EXPECT_EQ( m_softcam->valid(), true );
    EXPECT_EQ( m_softcam->width(), 320 );
    EXPECT_EQ( m_softcam->height(), 240 );
    EXPECT_EQ( m_softcam->framerate(), 60.0f );

    fb->deactivate();

    ASSERT_NE( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->getFrameBuffer()->active(), false );

    m_softcam->releaseFrameBuffer();

    EXPECT_EQ( m_softcam->getFrameBuffer(), nullptr );

    fb.reset();

    EXPECT_EQ( m_softcam->getFrameBuffer(), nullptr );

    fb = createFrameBufer(320, 240, 60);

    ASSERT_NE( m_softcam->getFrameBuffer(), nullptr );
    EXPECT_EQ( m_softcam->getFrameBuffer()->active(), true );
    EXPECT_EQ( m_softcam->valid(), true );
    EXPECT_EQ( m_softcam->width(), 320 );
    EXPECT_EQ( m_softcam->height(), 240 );
    EXPECT_EQ( m_softcam->framerate(), 60.0f );
}

TEST_F(Softcam, AttributesIncompatibleSender)
{
    auto fb = createFrameBufer(320, 240, 60);

    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    EXPECT_NE( m_softcam->getFrameBuffer(), nullptr );

    fb->deactivate();
    m_softcam->releaseFrameBuffer();

    EXPECT_EQ( m_softcam->getFrameBuffer(), nullptr );

    fb.reset();
    fb = createFrameBufer(640, 480, 60);                // <<<

    EXPECT_EQ( m_softcam->getFrameBuffer(), nullptr );    // <<<
    EXPECT_EQ( m_softcam->valid(), true );
    EXPECT_EQ( m_softcam->width(), 320 );
    EXPECT_EQ( m_softcam->height(), 240 );
    EXPECT_EQ( m_softcam->framerate(), 60.0f );
}

TEST_F(Softcam, IAMStreamConfigNoServer)
{
    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    IAMStreamConfig *amsc = m_softcam;
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
}

TEST_F(Softcam, IAMStreamConfigNullPointer)
{
    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    IAMStreamConfig *amsc = m_softcam;
    hr = amsc->GetFormat(nullptr);
    EXPECT_EQ( hr, E_POINTER );

    hr = amsc->SetFormat(nullptr);
    EXPECT_EQ( hr, E_POINTER );

    int count = 55, size = 77;
    hr = amsc->GetNumberOfCapabilities(nullptr, &size);
    EXPECT_EQ( hr, E_POINTER );
    hr = amsc->GetNumberOfCapabilities(&count, nullptr);
    EXPECT_EQ( hr, E_POINTER );
    hr = amsc->GetNumberOfCapabilities(nullptr, nullptr);
    EXPECT_EQ( hr, E_POINTER );

    BYTE scc[sizeof(VIDEO_STREAM_CONFIG_CAPS)];
    AM_MEDIA_TYPE *pmt = nullptr;
    hr = amsc->GetStreamCaps(0, nullptr, scc);
    EXPECT_EQ( hr, E_POINTER );
    hr = amsc->GetStreamCaps(0, &pmt, nullptr);
    EXPECT_EQ( hr, E_POINTER );
    hr = amsc->GetStreamCaps(0, nullptr, nullptr);
    EXPECT_EQ( hr, E_POINTER );
}

TEST_F(Softcam, IAMStreamConfigNormal)
{
    auto fb = createFrameBufer(320, 240, 60);

    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    IAMStreamConfig *amsc = m_softcam;
    AM_MEDIA_TYPE *pmt = nullptr;
    hr = amsc->GetFormat(&pmt);
    EXPECT_EQ( hr, S_OK );
    checkMediaType320x240( pmt );

    hr = amsc->SetFormat(pmt);
    EXPECT_EQ( hr, S_OK );
    DeleteMediaType(pmt);
    pmt = nullptr;

    int count = 55, size = 77;
    hr = amsc->GetNumberOfCapabilities(&count, &size);
    EXPECT_EQ( hr, S_OK );
    EXPECT_EQ( count, 1 );
    EXPECT_GE( size, (int)sizeof(VIDEO_STREAM_CONFIG_CAPS) );

    size = (std::max)((int)sizeof(VIDEO_STREAM_CONFIG_CAPS), size);
    std::unique_ptr<BYTE[]> scc(new BYTE[size]);
    hr = amsc->GetStreamCaps(0, &pmt, scc.get());
    EXPECT_EQ( hr, S_OK );
    checkMediaType320x240( pmt );

    DeleteMediaType(pmt);
    pmt = nullptr;
}

TEST_F(Softcam, IBaseFilterEnumPins)
{
    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    IBaseFilter *base_filter = nullptr;
    hr = m_softcam->QueryInterface(IID_IBaseFilter, reinterpret_cast<void**>(&base_filter));
    EXPECT_EQ( hr, S_OK );
    ASSERT_NE( base_filter, nullptr );

    IEnumPins *enum_pins = nullptr;
    hr = base_filter->EnumPins(&enum_pins);
    base_filter->Release();
    EXPECT_EQ( hr, S_OK );
    ASSERT_NE( enum_pins, nullptr );

    IPin *pins[1] = {};
    ULONG fetched = 0;
    hr = enum_pins->Next(1, pins, &fetched);
    EXPECT_EQ( hr, S_OK );
    EXPECT_EQ( fetched, 1u );
    EXPECT_NE( pins[0], nullptr );

    if (pins[0]) pins[0]->Release();

    hr = enum_pins->Next(1, pins, &fetched);
    EXPECT_EQ( hr, S_FALSE );
    EXPECT_EQ( fetched, 0u );

    enum_pins->Release();
}

TEST_F(Softcam, IBaseFilterQueryFilterInfo)
{
    HRESULT hr = 555;
    m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
    ASSERT_NE( m_softcam, nullptr );
    m_softcam->AddRef();

    IBaseFilter *base_filter = nullptr;
    hr = m_softcam->QueryInterface(IID_IBaseFilter, reinterpret_cast<void**>(&base_filter));
    EXPECT_EQ( hr, S_OK );
    ASSERT_NE( base_filter, nullptr );

    FILTER_INFO info;
    std::memset(&info, 99, sizeof(info));
    hr = base_filter->QueryFilterInfo(&info);
    EXPECT_EQ( hr, S_OK );
    auto eos = std::find(std::begin(info.achName), std::end(info.achName), 0);
    EXPECT_NE( eos, std::end(info.achName) );
    EXPECT_EQ( info.pGraph, nullptr );

    hr = base_filter->QueryFilterInfo(nullptr);
    EXPECT_EQ( hr, E_POINTER );

    base_filter->Release();
}


class SoftcamStream : public ::testing::Test
{
 protected:
    sc::Softcam*        m_softcam = nullptr;
    IPin*               m_pins[1] = {};
    sc::SoftcamStream*  m_stream = nullptr;

    void SetUpSoftcamStream()
    {
        HRESULT hr = 555;
        m_softcam = (sc::Softcam*)sc::Softcam::CreateInstance(nullptr, SOME_GUID, &hr);
        if (m_softcam == nullptr)
        {
            return;
        }
        m_softcam->AddRef();

        IBaseFilter *base_filter = nullptr;
        hr = m_softcam->QueryInterface(IID_IBaseFilter, reinterpret_cast<void**>(&base_filter));
        if (hr != S_OK || base_filter == nullptr)
        {
            return;
        }

        IEnumPins *enum_pins = nullptr;
        hr = base_filter->EnumPins(&enum_pins);
        base_filter->Release();
        if (hr != S_OK || enum_pins == nullptr)
        {
            return;
        }

        ULONG fetched = 0;
        hr = enum_pins->Next(1, m_pins, &fetched);
        enum_pins->Release();
        if (hr != S_OK || m_pins[0] == nullptr)
        {
            return;
        }
        m_stream = static_cast<sc::SoftcamStream*>(m_pins[0]);
    }

    virtual void TearDown() override
    {
        if (m_pins[0])
        {
            m_pins[0]->Release();
        }
        if (m_softcam)
        {
            m_softcam->Release();
        }
    }
};

TEST_F(SoftcamStream, QueryInterface)
{
    SetUpSoftcamStream();

    ASSERT_NE( m_pins[0], nullptr );
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;
    {
        IPin *ptr = nullptr;
        hr = m_pins[0]->QueryInterface(IID_IPin, reinterpret_cast<void**>(&ptr));
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( ptr, m_stream );
        if (ptr) ptr->Release();
    }{
        IQualityControl *ptr = nullptr;
        hr = m_pins[0]->QueryInterface(IID_IQualityControl, reinterpret_cast<void**>(&ptr));
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( ptr, m_stream );
        if (ptr) ptr->Release();
    }{
        IKsPropertySet *ptr = nullptr;
        hr = m_pins[0]->QueryInterface(IID_IKsPropertySet, reinterpret_cast<void**>(&ptr));
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( ptr, m_stream );
        if (ptr) ptr->Release();
    }{
        IAMStreamConfig *ptr = nullptr;
        hr = m_pins[0]->QueryInterface(IID_IAMStreamConfig, reinterpret_cast<void**>(&ptr));
        EXPECT_EQ( hr, S_OK );
        EXPECT_EQ( ptr, m_stream );
        if (ptr) ptr->Release();
    }
}

TEST_F(SoftcamStream, IPinEnumMediaTypes)
{
    auto fb = createFrameBufer(320, 240, 60);
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    IPin *pin = m_stream;
    IEnumMediaTypes *enum_media_types = nullptr;
    hr = pin->EnumMediaTypes(&enum_media_types);
    EXPECT_EQ( hr, S_OK );
    ASSERT_NE( enum_media_types, nullptr );

    AM_MEDIA_TYPE *ppmt[1] = {};
    ULONG fetched = 0;
    hr = enum_media_types->Next(1, ppmt, &fetched);
    EXPECT_EQ( hr, S_OK );
    EXPECT_EQ( fetched, 1u );
    checkMediaType320x240( ppmt[0] );

    DeleteMediaType(ppmt[0]);
    ppmt[0] = nullptr;

    hr = enum_media_types->Next(1, ppmt, &fetched);
    EXPECT_EQ( hr, S_FALSE );
    EXPECT_EQ( fetched, 0u );

    enum_media_types->Release();
}

TEST_F(SoftcamStream, IKsPropertySet)
{
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    IKsPropertySet *ksps = m_stream;
    hr = ksps->Set(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                   nullptr, 0, nullptr, 0);
    EXPECT_EQ( hr, E_NOTIMPL );

    hr = ksps->Get(AM_KSPROPSETID_TSRateChange, 0, 0, 0, 0, 0, 0);
    EXPECT_EQ( hr, E_PROP_SET_UNSUPPORTED );

    hr = ksps->Get(AMPROPSETID_Pin, 999, 0, 0, 0, 0, 0);
    EXPECT_EQ( hr, E_PROP_ID_UNSUPPORTED );

    hr = ksps->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                   nullptr, 0, nullptr, 0, nullptr);
    EXPECT_EQ( hr, E_POINTER );

    DWORD cbReturned = 99;
    hr = ksps->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                   nullptr, 0, nullptr, 0, &cbReturned);
    EXPECT_EQ( hr, S_OK );
    EXPECT_EQ( cbReturned, sizeof(GUID) );

    int x = 999;
    hr = ksps->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                   nullptr, 0, &x, sizeof(x), nullptr);
    EXPECT_EQ( hr, E_UNEXPECTED );
    EXPECT_EQ( x, 999 );

    GUID prop{};
    hr = ksps->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                   nullptr, 0, &prop, sizeof(prop), nullptr);
    EXPECT_EQ( hr, S_OK );
    EXPECT_EQ( prop, PIN_CATEGORY_CAPTURE );

    hr = ksps->QuerySupported(AM_KSPROPSETID_TSRateChange, 0, nullptr);
    EXPECT_EQ( hr, E_PROP_SET_UNSUPPORTED );

    hr = ksps->QuerySupported(AMPROPSETID_Pin, 999, nullptr);
    EXPECT_EQ( hr, E_PROP_ID_UNSUPPORTED );

    DWORD typeSupported = 99;
    hr = ksps->QuerySupported(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                              &typeSupported);
    EXPECT_EQ( hr, S_OK );
    EXPECT_EQ( typeSupported, (DWORD)KSPROPERTY_SUPPORT_GET );
}

TEST_F(SoftcamStream, IAMStreamConfigNoServer)
{
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    IAMStreamConfig *amsc = m_stream;
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
}

TEST_F(SoftcamStream, IAMStreamConfigNullPointer)
{
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    IAMStreamConfig *amsc = m_stream;
    hr = amsc->GetFormat(nullptr);
    EXPECT_EQ( hr, E_POINTER );

    hr = amsc->SetFormat(nullptr);
    EXPECT_EQ( hr, E_POINTER );

    int count = 55, size = 77;
    hr = amsc->GetNumberOfCapabilities(nullptr, &size);
    EXPECT_EQ( hr, E_POINTER );
    hr = amsc->GetNumberOfCapabilities(&count, nullptr);
    EXPECT_EQ( hr, E_POINTER );
    hr = amsc->GetNumberOfCapabilities(nullptr, nullptr);
    EXPECT_EQ( hr, E_POINTER );

    BYTE scc[sizeof(VIDEO_STREAM_CONFIG_CAPS)];
    AM_MEDIA_TYPE *pmt = nullptr;
    hr = amsc->GetStreamCaps(0, nullptr, scc);
    EXPECT_EQ( hr, E_POINTER );
    hr = amsc->GetStreamCaps(0, &pmt, nullptr);
    EXPECT_EQ( hr, E_POINTER );
    hr = amsc->GetStreamCaps(0, nullptr, nullptr);
    EXPECT_EQ( hr, E_POINTER );
}

TEST_F(SoftcamStream, IAMStreamConfigNormal)
{
    auto fb = createFrameBufer(320, 240, 60);
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    IAMStreamConfig *amsc = m_stream;
    AM_MEDIA_TYPE *pmt = nullptr;
    hr = amsc->GetFormat(&pmt);
    EXPECT_EQ( hr, S_OK );
    checkMediaType320x240( pmt );

    hr = amsc->SetFormat(pmt);
    EXPECT_EQ( hr, S_OK );
    DeleteMediaType(pmt);
    pmt = nullptr;

    int count = 55, size = 77;
    hr = amsc->GetNumberOfCapabilities(&count, &size);
    EXPECT_EQ( hr, S_OK );
    EXPECT_EQ( count, 1 );
    EXPECT_GE( size, (int)sizeof(VIDEO_STREAM_CONFIG_CAPS) );

    size = (std::max)((int)sizeof(VIDEO_STREAM_CONFIG_CAPS), size);
    std::unique_ptr<BYTE[]> scc(new BYTE[size]);
    hr = amsc->GetStreamCaps(0, &pmt, scc.get());
    EXPECT_EQ( hr, S_OK );
    checkMediaType320x240( pmt );

    DeleteMediaType(pmt);
    pmt = nullptr;
}

class MediaSampleMock : public CUnknown, public IMediaSample
{
 public:
    BYTE*       m_ptr;
    std::size_t m_size;
    MediaSampleMock(BYTE* ptr, std::size_t size) :
        CUnknown("", this), m_ptr(ptr), m_size(size)
    {
    }

    DECLARE_IUNKNOWN

    virtual HRESULT STDMETHODCALLTYPE GetPointer(BYTE **ppBuffer) override
    {
        *ppBuffer = m_ptr;
        return S_OK;
    }
    virtual long STDMETHODCALLTYPE GetSize() override
    {
        return (long)m_size;
    }
    virtual HRESULT STDMETHODCALLTYPE GetTime(REFERENCE_TIME *pTimeStart, REFERENCE_TIME *pTimeEnd) override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE SetTime(REFERENCE_TIME *pTimeStart, REFERENCE_TIME *pTimeEnd) override
    {
        return S_OK;
    }
    virtual HRESULT STDMETHODCALLTYPE IsSyncPoint() override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE SetSyncPoint(BOOL bIsSyncPoint) override
    {
        return S_OK;
    }
    virtual HRESULT STDMETHODCALLTYPE IsPreroll() override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE SetPreroll(BOOL bIsPreroll) override
    {
        return E_NOTIMPL;
    }
    virtual long STDMETHODCALLTYPE GetActualDataLength() override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE SetActualDataLength(long __MIDL__IMediaSample0000) override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE GetMediaType(AM_MEDIA_TYPE **ppMediaType) override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE SetMediaType(AM_MEDIA_TYPE *pMediaType) override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE IsDiscontinuity() override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE SetDiscontinuity(BOOL bDiscontinuity) override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE GetMediaTime(LONGLONG *pTimeStart, LONGLONG *pTimeEnd) override
    {
        return E_NOTIMPL;
    }
    virtual HRESULT STDMETHODCALLTYPE SetMediaTime(LONGLONG *pTimeStart, LONGLONG *pTimeEnd) override
    {
        return E_NOTIMPL;
    }
};

TEST_F(SoftcamStream, CSourceStreamFillBufferTimeouts)
{
    auto fb = createFrameBufer(320, 240, 60);
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    std::vector<BYTE> buffer(320 * 240 * 3, 123);
    MediaSampleMock media_sample(buffer.data(), buffer.size());

    hr = m_stream->FillBuffer(&media_sample);
    EXPECT_EQ( hr, NOERROR );
    EXPECT_TRUE( std::all_of(buffer.begin(), buffer.end(),
                             [](BYTE b) { return b == 0; }) );
}

TEST_F(SoftcamStream, CSourceStreamFillBufferNormal)
{
    auto fb = createFrameBufer(320, 240, 60);
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    auto TEST_INPUT1 = [](int x, int y, int c)
    {
        return (x + y * 2 + c * 85) % 256;
    };
    auto TEST_INPUT2 = [](int x, int y, int c)
    {
        return (x * 3 + y + c * 85) % 256;
    };

    std::atomic<int> pos = 0;

    std::thread th([&]
    {
        // Bottom-to-Top BGR image
        std::vector<BYTE> buffer(320 * 240 * 3, 123);
        MediaSampleMock media_sample(buffer.data(), buffer.size());

        hr = m_stream->FillBuffer(&media_sample);
        EXPECT_EQ( hr, NOERROR );

        int error_count = 0;
        for (int y = 0; y < 240; y++)
        {
            const BYTE* actual = &buffer[3 * 320 * (239 - y)];
            for (int x = 0; x < 320; x++)
            {
                if (actual[x * 3 + 0] != TEST_INPUT1(x, y, 0) ||
                    actual[x * 3 + 1] != TEST_INPUT1(x, y, 1) ||
                    actual[x * 3 + 2] != TEST_INPUT1(x, y, 2))
                {
                    error_count += 1;
                }
            }
        }
        EXPECT_EQ( error_count, 0 );

        pos = 1;

        hr = m_stream->FillBuffer(&media_sample);
        EXPECT_EQ( hr, NOERROR );

        error_count = 0;
        for (int y = 0; y < 240; y++)
        {
            const BYTE* actual = &buffer[3 * 320 * (239 - y)];
            for (int x = 0; x < 320; x++)
            {
                if (actual[x * 3 + 0] != TEST_INPUT2(x, y, 0) ||
                    actual[x * 3 + 1] != TEST_INPUT2(x, y, 1) ||
                    actual[x * 3 + 2] != TEST_INPUT2(x, y, 2))
                {
                    error_count += 1;
                }
            }
        }
        EXPECT_EQ( error_count, 0 );
    });

    // Top-to-Bottom BGR image
    std::vector<BYTE> input(320 * 240 * 3);
    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 320; x++)
        {
            input[3 * (x + 320 * y) + 0] = (BYTE)TEST_INPUT1(x, y, 0);
            input[3 * (x + 320 * y) + 1] = (BYTE)TEST_INPUT1(x, y, 1);
            input[3 * (x + 320 * y) + 2] = (BYTE)TEST_INPUT1(x, y, 2);
        }
    }
    fb->write(input.data());

    while (pos != 1) { sc::Timer::sleep(0.001f); }

    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 320; x++)
        {
            input[3 * (x + 320 * y) + 0] = (BYTE)TEST_INPUT2(x, y, 0);
            input[3 * (x + 320 * y) + 1] = (BYTE)TEST_INPUT2(x, y, 1);
            input[3 * (x + 320 * y) + 2] = (BYTE)TEST_INPUT2(x, y, 2);
        }
    }
    fb->write(input.data());

    th.join();
}

TEST_F(SoftcamStream, CSourceStreamGetMediaTypeNoServer)
{
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    CMediaType mt;
    hr = m_stream->GetMediaType(&mt);
    EXPECT_EQ( hr, E_FAIL );
}

TEST_F(SoftcamStream, CSourceStreamGetMediaTypeNullPointer)
{
    auto fb = createFrameBufer(320, 240, 60);
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    hr = m_stream->GetMediaType(nullptr);
    EXPECT_EQ( hr, E_POINTER );
}

TEST_F(SoftcamStream, CSourceStreamGetMediaTypeNormal)
{
    auto fb = createFrameBufer(320, 240, 60);
    SetUpSoftcamStream();
    ASSERT_NE( m_stream, nullptr );
    HRESULT hr;

    CMediaType mt;
    hr = m_stream->GetMediaType(&mt);
    EXPECT_EQ( hr, NOERROR );

    checkMediaType320x240(&mt);
}

TEST_F(SoftcamStream, getFrameBuffer_must_not_lock_the_filter_state)
{
    auto fb = createFrameBufer(320, 240, 60);
    SetUpSoftcamStream();

    CAutoLock lock(m_softcam->pStateLock()); // 1st lock
    std::atomic<int> done{false};
    std::thread th([&]
    {
        // A deadlock occurs if this code attempts to get lock.
        m_softcam->getFrameBuffer();
        done = true;
    });

    for (int i = 0; !done && i < 20; i++) { sc::Timer::sleep(0.050f); }
    ASSERT_TRUE( done );
    th.join();
}

TEST_F(SoftcamStream, releaseFrameBuffer_must_not_lock_the_filter_state)
{
    auto fb = createFrameBufer(320, 240, 60);
    SetUpSoftcamStream();

    CAutoLock lock(m_softcam->pStateLock()); // 1st lock
    std::atomic<int> done{false};
    std::thread th([&]
    {
        // A deadlock occurs if this code attempts to get lock.
        m_softcam->releaseFrameBuffer();
        done = true;
    });

    for (int i = 0; !done && i < 20; i++) { sc::Timer::sleep(0.050f); }
    ASSERT_TRUE( done );
    th.join();
}


} //namespace DShowSoftcamTest
