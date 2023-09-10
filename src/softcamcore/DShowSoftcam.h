#pragma once

#include <memory>
#include <baseclasses/streams.h>
#include "FrameBuffer.h"


namespace softcam {


class Softcam : public CSource, public IAMStreamConfig
{
public:
    static CUnknown* CreateInstance(
                    LPUNKNOWN   lpunk,
                    const GUID& clsid,
                    HRESULT*    phr);

    // IUnknown Methods
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv) override;

    // IAMStreamConfig
    HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *mt) override;
    HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **out_pmt) override;
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *out_count, int *out_size) override;
    HRESULT STDMETHODCALLTYPE GetStreamCaps(int index, AM_MEDIA_TYPE **out_pmt, BYTE *out_scc) override;

    FrameBuffer*    getFrameBuffer();
    bool            valid() const { return m_valid; }
    int             width() const { return m_width; }
    int             height() const { return m_height; }
    float           framerate() const { return m_framerate; }
    void            releaseFrameBuffer();

private:
    CCritSec    m_critsec;
    FrameBuffer m_frame_buffer;
    const bool  m_valid;
    const int   m_width;
    const int   m_height;
    const float m_framerate;

    Softcam(LPUNKNOWN lpunk, const GUID& clsid, HRESULT *phr);
};


class SoftcamStream : public CSourceStream, public IKsPropertySet, public IAMStreamConfig
{
 public:
    SoftcamStream(HRESULT *phr, Softcam *pParent, LPCWSTR pPinName);
    ~SoftcamStream();

    // IUnknown
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv) override;

    // CBasePin
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q) override;

    // CBaseOutputPin
    // Ask for buffers of the size appropriate to the agreed media type
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties) override;

    // CSourceStream
    HRESULT FillBuffer(IMediaSample *pms) override;
    HRESULT GetMediaType(CMediaType *pMediaType) override;
    HRESULT OnThreadCreate(void) override;

    //  IKsPropertySet
    HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwPropID,
                                  LPVOID pInstanceData, DWORD cbInstanceData,
                                  LPVOID pPropData, DWORD cbPropData) override;
    HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID,
                                  LPVOID pInstanceData, DWORD cbInstanceData,
                                  LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned) override;
    HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID,
                                             DWORD *pTypeSupport) override;

    // IAMStreamConfig
    HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *mt) override;
    HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **out_pmt) override;
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *out_count, int *out_size) override;
    HRESULT STDMETHODCALLTYPE GetStreamCaps(int index, AM_MEDIA_TYPE **out_pmt, BYTE *out_scc) override;

private:
    const bool  m_valid;
    const int   m_width;
    const int   m_height;
    uint64_t    m_frame_counter = 0;
    std::unique_ptr<uint8_t[]>  m_screenshot;

    CCritSec m_critsec;
    CRefTime m_sample_time;
    long m_interval_time_msec = 10;

    Softcam*        getParent();
};


} //namespace softcam
