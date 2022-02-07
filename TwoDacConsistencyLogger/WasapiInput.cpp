#include "WasapiInput.h"
#include <mmdeviceapi.h>
#include <cmath>
#include <cstdio>
#include <Functiondiscoverykeys_devpkey.h>
#include <comdef.h>
#include <format>

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

WasapiInput::WasapiInput(const GeneratedSamples& config)
{
    testWaveDurationInSeconds = config.TestWaveDurationInSeconds();
}

WasapiInput::~WasapiInput()
{
    if (recordedAudio != NULL)
    {
        delete[] recordedAudio;
    }
}

UINT16 WasapiInput::GetFormatID()
{
    return EXTRACT_WAVEFORMATEX_ID(&waveFormat.SubFormat);
}

void WasapiInput::StartRecording()
{
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const IID IID_IAudioClient = __uuidof(IAudioClient);

    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pWaveFormat = NULL;
    UINT32 packetLength = 0;
    BOOL bDone = FALSE;
    BYTE* pData;
    DWORD flags;
    bool pastInitialDiscontinuity = false;

    IPropertyStore* pProps = NULL;
    LPWSTR pwszID = NULL;

    hr = CoInitialize(NULL);

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

    hr = pEnumerator->GetDefaultAudioEndpoint(
        eCapture, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)


    // Get the endpoint ID string.
    hr = pDevice->GetId(&pwszID);
    EXIT_ON_ERROR(hr)
    hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    EXIT_ON_ERROR(hr)
    PROPVARIANT varName;
    // Initialize container for property value.
    PropVariantInit(&varName);
    // Get the endpoint's friendly-name property.
    hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
    EXIT_ON_ERROR(hr)
    DeviceName = std::format("{} ({})", (char*)_bstr_t(varName.pwszVal), (char*)_bstr_t(pwszID)); // endpoint friendly name and endpoint ID.
    CoTaskMemFree(pwszID);
    pwszID = NULL;
    PropVariantClear(&varName);
    SAFE_RELEASE(pProps)

    hr = pDevice->Activate(
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

    // TODO: make this exclusive mode so I can have the software select the highest sample rate(??)
    // Or just show the user what their current sample rate is and tell them to change it in the windows control panel
    // if they want more options.

    hr = pAudioClient->GetMixFormat(&pWaveFormat);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        hnsRequestedDuration,
        0,
        pWaveFormat,
        NULL);
    EXIT_ON_ERROR(hr)

    // Get the size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetService(
        IID_IAudioCaptureClient,
        (void**)&pCaptureClient);
    EXIT_ON_ERROR(hr)

    // Notify the audio sink which format to use.
    hr = SetFormat(pWaveFormat);
    EXIT_ON_ERROR(hr)

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC *
    bufferFrameCount / pWaveFormat->nSamplesPerSec;

    hr = pAudioClient->Start();  // Start recording.
    EXIT_ON_ERROR(hr)

    // Each loop fills about half of the shared buffer.
    while (bDone == FALSE)
    {
        // Sleep for half the buffer duration.
        Sleep(hnsActualDuration / REFTIMES_PER_MILLISEC / 2);

        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        EXIT_ON_ERROR(hr)

        while (packetLength != 0)
        {
            // Get the available data in the shared buffer.
            hr = pCaptureClient->GetBuffer(
                &pData,
                &numFramesAvailable,
                &flags, NULL, NULL); // TODO: pu64QPCPosition parameter could be used for Bluetooth latency testing!
            EXIT_ON_ERROR(hr)

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;  // Tell CopyData to write silence.
            }
            if (flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
            {
                if (pastInitialDiscontinuity)
                {
                    ThrowAwayRecording();
                }
            }
            else
            {
                pastInitialDiscontinuity = true;
            }

            // Copy the available capture data to the audio sink.
            hr = CopyData(
                pData, numFramesAvailable, &bDone);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            EXIT_ON_ERROR(hr)

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            EXIT_ON_ERROR(hr)
        }
    }

    hr = pAudioClient->Stop();  // Stop recording.
    EXIT_ON_ERROR(hr)

    Exit:
    CoUninitialize();
    CoTaskMemFree(pWaveFormat);
    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pCaptureClient)
}

HRESULT WasapiInput::SetFormat(WAVEFORMATEX* wfex)
{
    //TODO: a whole bunch of validation of this, as explained here: https://matthewvaneerde.wordpress.com/2011/09/09/how-to-validate-and-log-a-waveformatex/
    // Documentation here: https://docs.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible
    // Example here: https://gist.github.com/mhamilt/859e57f064e4d5b2bb8e78ae55439d70#file-wasapi-console-cpp-L64

    if (wfex->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        //typedef struct {
        //	WAVEFORMATEX    Format;
        //	union {
        //		WORD wValidBitsPerSample;       /* bits of precision  */
        //		WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
        //		WORD wReserved;                 /* If neither applies, set to zero. */
        //	} Samples;
        //	DWORD           dwChannelMask;      /* which channels are */
        //  GUID            SubFormat;
        waveFormat = *reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(wfex);
    }
    else
    {
        //typedef struct tWAVEFORMATEX
        //{
        //	WORD    wFormatTag;        /* waveFormat type */
        //	WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
        //	DWORD   nSamplesPerSec;    /* sample rate */
        //	DWORD   nAvgBytesPerSec;   /* for buffer estimation */
        //	WORD    nBlockAlign;       /* block size of data */
        //	WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
        //	WORD    cbSize;            /* The count in bytes of the size of
        //									extra information (after cbSize) */

        // This is just an old-style WAVEFORMATEX struct, so convert it to the new WAVEFORMATEXTENSIBLE waveFormat:
        waveFormat.Format = *wfex;
        waveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        waveFormat.Samples.wReserved = 0;
        waveFormat.Samples.wSamplesPerBlock = 0;
        waveFormat.Samples.wValidBitsPerSample = waveFormat.Format.wBitsPerSample;
        waveFormat.dwChannelMask = 0;
        INIT_WAVEFORMATEX_GUID(&waveFormat.SubFormat, wfex->wFormatTag);
    }

#ifdef _DEBUG
    printf("TestRecordingSink::SetFormat\nFormat type: 0x%.4x\nChannels: %u\nSamples Per Sec: %u\nAvg Bytes Per Sec: %u\nBlock Align: %u\nBits Per Sample: %u\ncbSize: %u\nValid Bits Per Sample: %u\nSamples Per Block: %u\nChannel Mask: 0x%.8x\n",
        GetFormatID(),
        waveFormat.Format.nChannels,
        waveFormat.Format.nSamplesPerSec,
        waveFormat.Format.nAvgBytesPerSec,
        waveFormat.Format.nBlockAlign,
        waveFormat.Format.wBitsPerSample,
        waveFormat.Format.cbSize,
        waveFormat.Samples.wValidBitsPerSample,
        waveFormat.Samples.wSamplesPerBlock,
        waveFormat.dwChannelMask);
#endif

    if (waveFormat.Samples.wValidBitsPerSample != waveFormat.Format.wBitsPerSample)
    {
        return -1; // Not supported yet!
    }
    else if (waveFormat.Format.nChannels != 2)
    {
        return -1; // Not supported!
    }

    recordedAudioLength = recordedAudioNumChannels * round(waveFormat.Format.nSamplesPerSec * testWaveDurationInSeconds);
    recordedAudio = new float[recordedAudioLength];

    return S_OK;
}

HRESULT WasapiInput::CopyData(BYTE* pData, UINT32 bufferFrameCount, BOOL* bDone)
{
    if (FinishedRecording())
    {
        *bDone = true;
        return S_OK;
    }

    WORD numChannels = waveFormat.Format.nChannels;
    if (GetFormatID() == WAVE_FORMAT_IEEE_FLOAT && waveFormat.Format.wBitsPerSample == 32)
    {
        float* castData = (float*)pData;
        for (UINT32 i = 0; i < bufferFrameCount * numChannels; i += numChannels)
        {
            for (int c = 0; c < numChannels; c++)
            {
                recordedAudio[recordedAudioIndex] = pData == NULL ? 0.0f : castData[i + c];
                recordedAudioIndex++;
                if (FinishedRecording())
                {
                    *bDone = true;
                    return S_OK;
                }
            }
        }

        return S_OK;
    }
    else if (GetFormatID() == WAVE_FORMAT_PCM && waveFormat.Format.wBitsPerSample == 16)
    {
        // This is confirmed to be Little Endian on my computer!
        // (settings bytes manually as big endian results in garbage, but setting
        // them little endian manually works perfectly)

        INT16* castData = (INT16*)pData;
        for (UINT32 i = 0; i < bufferFrameCount * numChannels; i += numChannels)
        {
            for (int c = 0; c < numChannels; c++)
            {
                recordedAudio[recordedAudioIndex] = pData == NULL ? 0.0f : castData[i + c] / (float)SHRT_MIN * -1.0f;
                recordedAudioIndex++;
                if (FinishedRecording())
                {
                    *bDone = true;
                    return S_OK;
                }
            }
        }

        return S_OK;
    }
    else if (GetFormatID() == WAVE_FORMAT_PCM && waveFormat.Format.wBitsPerSample == 24)
    {
        //// nBlockAlign and bufferFrameCount are only used for the buffer size!
        //int dataLength = bufferFrameCount * waveFormat.Format.nBlockAlign;
        //// Actual frame size is 32 bits for 24 bit audio:
        //int bytesPerSampleWithPadding = 32 / 8;
        //int bytesPerFrame = bytesPerSampleWithPadding * waveFormat.Format.nChannels;

        //for (int i = 0; i < dataLength; i += bytesPerFrame)
        //{
        //	int thirtyTwoBit = (int)round(sin(1) * INT24_MAX);

        //	for (int c = 0; c < numChannels; c++)
        //	{
        //		int channelOffset = c * bytesPerSampleWithPadding;

        //		pData[i + channelOffset + 0] = 0; // padding
        //		pData[i + channelOffset + 1] = thirtyTwoBit; // little endian, least significant first
        //		pData[i + channelOffset + 2] = thirtyTwoBit >> 8;
        //		pData[i + channelOffset + 3] = thirtyTwoBit >> 16;
        //		pData[i + channelOffset + 3] |= (thirtyTwoBit >> 31) << 7; // negative bit
        //	}
        //}

        //return S_OK;
        return -1; // TODO: write this when I'm making something public facing.
    }
    else if (GetFormatID() == WAVE_FORMAT_PCM && waveFormat.Format.wBitsPerSample == 32)
    {
        INT32* castData = (INT32*)pData;
        for (UINT32 i = 0; i < bufferFrameCount * numChannels; i += numChannels)
        {
            for (int c = 0; c < numChannels; c++)
            {
                recordedAudio[recordedAudioIndex] = pData == NULL ? 0.0f : castData[i + c] / (float)INT_MIN * -1.0f;
                recordedAudioIndex++;
                if (FinishedRecording())
                {
                    *bDone = true;
                    return S_OK;
                }
            }
        }

        return S_OK;
    }
    else
    {
        return -1;
    }
}

bool WasapiInput::FinishedRecording()
{
    return recordedAudioIndex >= recordedAudioLength;
}

void WasapiInput::ThrowAwayRecording()
{
    // Write garbage to the recording and set it to be completed
    RecordingFailed = true;
    bool high = true;
    for (int i = 0; i < recordedAudioLength;)
    {
        recordedAudio[i] = high ? 1 : -1;
        i++;
        if (recordedAudioLength % 2 == 0)
        {
            recordedAudio[i] = high ? 1 : -1;
            i++;
        }
        high = !high;
    }
    recordedAudioIndex = recordedAudioLength;
}
