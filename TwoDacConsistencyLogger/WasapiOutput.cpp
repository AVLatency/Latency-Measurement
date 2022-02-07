#include "WasapiOutput.h"
#include <mmdeviceapi.h>
#include <climits>
#include <cmath>
#include <Functiondiscoverykeys_devpkey.h>
#include <format>
#include <comdef.h>

// Needed for AvSetMmThreadCharacteristics and AvRevertMmThreadCharacteristics:
#include <avrt.h>
#pragma comment(lib, "avrt.lib")

const int INT24_MAX = (1 << 23) - 1;

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

WasapiOutput::WasapiOutput(const GeneratedSamples & config) : recordingConfig { config }
{
    this->waveFormat = config.WaveFormat;
}

void WasapiOutput::StartOutput()
{
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

    WasapiOutput* pMySource = this;

    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = 0;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    HANDLE hEvent = NULL;
    HANDLE hTask = NULL;
    UINT32 bufferFrameCount;
    BYTE* pData;
    DWORD flags = 0;
    DWORD taskIndex = 0;

    IPropertyStore* pProps = NULL;
    LPWSTR pwszID = NULL;

    hr = CoInitialize(NULL);

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

        hr = pEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &pDevice);
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

        // Initialize the stream to play at the minimum latency.
        hr = pAudioClient->GetDevicePeriod(NULL, &hnsRequestedDuration);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_EXCLUSIVE,
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            hnsRequestedDuration,
            hnsRequestedDuration,
            waveFormat,
            NULL);
    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
        // Align the buffer if needed, see IAudioClient::Initialize() documentation
        UINT32 nFrames = 0;
        hr = pAudioClient->GetBufferSize(&nFrames);
        EXIT_ON_ERROR(hr)
            hnsRequestedDuration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC / waveFormat->nSamplesPerSec * nFrames + 0.5);
        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_EXCLUSIVE,
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            hnsRequestedDuration,
            hnsRequestedDuration,
            waveFormat,
            NULL);
    }
    EXIT_ON_ERROR(hr)

        // Create an event handle and register it for
        // buffer-event notifications.
        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hEvent == NULL)
    {
        hr = E_FAIL;
        goto Exit;
    }

    hr = pAudioClient->SetEventHandle(hEvent);
    EXIT_ON_ERROR(hr);

    // Get the actual size of the two allocated buffers.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->GetService(
            IID_IAudioRenderClient,
            (void**)&pRenderClient);
    EXIT_ON_ERROR(hr)

        // To reduce latency, load the first buffer with data
        // from the audio source before starting the stream.
        hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
    EXIT_ON_ERROR(hr)

        hr = pMySource->LoadData(bufferFrameCount, pData, &flags);
    EXIT_ON_ERROR(hr)

        hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags);
    EXIT_ON_ERROR(hr)

        // Ask MMCSS to temporarily boost the thread priority
        // to reduce glitches while the low-latency stream plays.
        hTask = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
    if (hTask == NULL)
    {
        hr = E_FAIL;
        EXIT_ON_ERROR(hr)
    }

    hr = pAudioClient->Start();  // Start playing.
    EXIT_ON_ERROR(hr)

        // Each loop fills one of the two buffers.
        while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
        {
            // Wait for next buffer event to be signaled.
            DWORD retval = WaitForSingleObject(hEvent, 2000);
            if (retval != WAIT_OBJECT_0)
            {
                // Event handle timed out after a 2-second wait.
                pAudioClient->Stop();
                hr = ERROR_TIMEOUT;
                goto Exit;
            }

            // Grab the next empty buffer from the audio device.
            hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
            EXIT_ON_ERROR(hr)

                // Load the buffer with data from the audio source.
                hr = LoadData(bufferFrameCount, pData, &flags);
            EXIT_ON_ERROR(hr)

                hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags);
            EXIT_ON_ERROR(hr)
        }

    // Wait for the last buffer to play before stopping.
    Sleep((DWORD)(hnsRequestedDuration / REFTIMES_PER_MILLISEC));

    hr = pAudioClient->Stop();  // Stop playing.
    EXIT_ON_ERROR(hr)

        Exit:
    if (hEvent != NULL)
    {
        CloseHandle(hEvent);
    }
    if (hTask != NULL)
    {
        AvRevertMmThreadCharacteristics(hTask);
    }

    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pRenderClient)
}

WORD WasapiOutput::GetFormatID()
{
    if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        return EXTRACT_WAVEFORMATEX_ID(&(reinterpret_cast<WAVEFORMATEXTENSIBLE*>(waveFormat)->SubFormat));
    }
    else
    {
        return waveFormat->wFormatTag;
    }
}

HRESULT WasapiOutput::LoadData(UINT32 bufferFrameCount, BYTE* pData, DWORD* flags)
{
    if (sampleIndex >= recordingConfig.samplesLength)
    {
        *flags = AUDCLNT_BUFFERFLAGS_SILENT;
        return S_OK;
    }

    WORD numChannels = waveFormat->nChannels;
    if (GetFormatID() == WAVE_FORMAT_IEEE_FLOAT && waveFormat->wBitsPerSample == 32)
    {
        float* castData = (float*)pData;
        for (UINT32 i = 0; i < bufferFrameCount * numChannels; i += numChannels)
        {
            if (sampleIndex < recordingConfig.samplesLength)
            {
                for (int c = 0; c < numChannels; c++)
                {
                    if (c == 0)
                    {
                        castData[i + c] = recordingConfig.samples[sampleIndex];
                    }
                    else
                    {
                        castData[i + c] = 0;
                    }
                }
                sampleIndex++;
            }
            else
            {
                for (int c = 0; c < numChannels; c++)
                {
                    castData[i + c] = 0;
                }
            }
        }
    }
    else if (GetFormatID() == WAVE_FORMAT_PCM && waveFormat->wBitsPerSample == 16)
    {
        // This is confirmed to be Little Endian on my computer!
        // (settings bytes manually as big endian results in garbage, but setting
        // them little endian manually works perfectly)

        INT16* castData = (INT16*)pData;
        for (UINT32 i = 0; i < bufferFrameCount * numChannels; i += numChannels)
        {
            if (sampleIndex < recordingConfig.samplesLength)
            {
                for (int c = 0; c < numChannels; c++)
                {
                    if (c == 0)
                    {
                        castData[i + c] = (INT16)(round(recordingConfig.samples[sampleIndex] * SHRT_MAX));
                    }
                    else
                    {
                        castData[i + c] = 0;
                    }
                }
                sampleIndex++;
            }
            else
            {
                for (int c = 0; c < numChannels; c++)
                {
                    castData[i + c] = 0;
                }
            }
        }
    }
    else if (GetFormatID() == WAVE_FORMAT_PCM && waveFormat->wBitsPerSample == 24)
    {
        // nBlockAlign and bufferFrameCount are only used for the buffer size!
        int dataLength = bufferFrameCount * waveFormat->nBlockAlign;
        // Actual frame size is 32 bits for 24 bit audio:
        int bytesPerSampleWithPadding = 32 / 8;
        int bytesPerFrame = bytesPerSampleWithPadding * waveFormat->nChannels;

        for (int i = 0; i < dataLength; i += bytesPerFrame)
        {
            if (sampleIndex < recordingConfig.samplesLength)
            {
                int thirtyTwoBit = (int)round(recordingConfig.samples[sampleIndex] * INT24_MAX);

                for (int c = 0; c < numChannels; c++)
                {
                    int channelOffset = c * bytesPerSampleWithPadding;
                    if (c == 0)
                    {
                        // Left channel (write audio data)
                        pData[i + channelOffset + 0] = 0; // padding
                        pData[i + channelOffset + 1] = thirtyTwoBit; // little endian, least significant first
                        pData[i + channelOffset + 2] = thirtyTwoBit >> 8;
                        pData[i + channelOffset + 3] = thirtyTwoBit >> 16;
                        pData[i + channelOffset + 3] |= (thirtyTwoBit >> 31) << 7; // negative bit
                    }
                    else
                    {
                        // Other channels (write zero audio data)
                        pData[i + channelOffset + 0] = 0; // padding
                        pData[i + channelOffset + 1] = 0; // little endian, least significant first
                        pData[i + channelOffset + 2] = 0;
                        pData[i + channelOffset + 3] = 0;
                    }
                }
                sampleIndex++;
            }
            else
            {
                for (int c = 0; c < numChannels; c++)
                {
                    int channelOffset = c * bytesPerSampleWithPadding;

                    pData[i + channelOffset + 0] = 0; // padding
                    pData[i + channelOffset + 1] = 0; // little endian, least significant first
                    pData[i + channelOffset + 2] = 0;
                    pData[i + channelOffset + 3] = 0;
                }
            }
        }
    }
    else if (GetFormatID() == WAVE_FORMAT_PCM && waveFormat->wBitsPerSample == 32)
    {
        INT32* castData = (INT32*)pData;
        for (UINT32 i = 0; i < bufferFrameCount * numChannels; i += numChannels)
        {
            if (sampleIndex < recordingConfig.samplesLength)
            {
                for (int c = 0; c < numChannels; c++)
                {
                    if (c == 0)
                    {
                        castData[i + c] = (INT32)(round(recordingConfig.samples[sampleIndex] * INT_MAX));
                    }
                    else
                    {
                        castData[i + c] = 0;
                    }
                }
                sampleIndex++;
            }
            else
            {
                for (int c = 0; c < numChannels; c++)
                {
                    castData[i + c] = 0;
                }
            }
        }
    }
    else
    {
        *flags = AUDCLNT_BUFFERFLAGS_SILENT;
        return -1; // TODO: a proper error message?
    }
    
    *flags = sampleIndex < recordingConfig.samplesLength ? 0 : AUDCLNT_BUFFERFLAGS_SILENT;
    return S_OK;
}