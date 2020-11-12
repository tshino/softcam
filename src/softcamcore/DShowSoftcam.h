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
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv);

    // IAMStreamConfig
    HRESULT SetFormat(AM_MEDIA_TYPE *mt);
    HRESULT GetFormat(AM_MEDIA_TYPE **out_pmt);
    HRESULT GetNumberOfCapabilities(int *out_count, int *out_size);
    HRESULT GetStreamCaps(int index, AM_MEDIA_TYPE **out_pmt, BYTE *out_scc);

    FrameBuffer*    getFrameBuffer();
    bool            valid() const { return m_valid; }
    int             width() const { return m_width; }
    int             height() const { return m_height; }
    float           framerate() const { return m_framerate; }
    void            releaseFrameBuffer();

private:
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
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv);

    // CBasePin
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

    // CBaseOutputPin
    // Ask for buffers of the size appropriate to the agreed media type
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);

    // CSourceStream
    HRESULT FillBuffer(IMediaSample *pms);
    HRESULT GetMediaType(CMediaType *pMediaType);
    HRESULT OnThreadCreate(void);

    //  IKsPropertySet
    HRESULT Set(REFGUID guidPropSet, DWORD dwPropID,
                LPVOID pInstanceData, DWORD cbInstanceData,
                LPVOID pPropData, DWORD cbPropData);
    HRESULT Get(REFGUID guidPropSet, DWORD dwPropID,
                LPVOID pInstanceData, DWORD cbInstanceData,
                LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT QuerySupported(REFGUID guidPropSet, DWORD dwPropID,
                           DWORD *pTypeSupport);

    // IAMStreamConfig
    HRESULT SetFormat(AM_MEDIA_TYPE *mt);
    HRESULT GetFormat(AM_MEDIA_TYPE **out_pmt);
    HRESULT GetNumberOfCapabilities(int *out_count, int *out_size);
    HRESULT GetStreamCaps(int index, AM_MEDIA_TYPE **out_pmt, BYTE *out_scc);

private:
    const bool  m_valid;
    const int   m_width;
    const int   m_height;
    uint64_t    m_frame_counter = 0;
    std::unique_ptr<uint8_t[]>  m_screenshot;

    CCritSec m_critsec;
    CRefTime m_sample_time;
    int m_interval_time = 10;

    Softcam*        getParent();
};


} //namespace softcam
