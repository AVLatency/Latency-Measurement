#include "WasapiOutput.h"

// Needed for AvSetMmThreadCharacteristics and AvRevertMmThreadCharacteristics:
#include <avrt.h>
#pragma comment(lib, "avrt.lib")

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

WasapiOutput::WasapiOutput(const AudioEndpoint& endpoint, bool loop, float* audioSamples, int audioSamplesLength, WAVEFORMATEX* waveFormat)
	: endpoint(endpoint), loop(loop), audioSamples(audioSamples), audioSamplesLength(audioSamplesLength), waveFormat(waveFormat)
{
	
}

WasapiOutput::~WasapiOutput()
{
	delete[] audioSamples;
}

void WasapiOutput::StartPlayback()
{
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = 0;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    HANDLE hEvent = NULL;
    HANDLE hTask = NULL;
    UINT32 bufferFrameCount;
    BYTE* pData;
    DWORD flags = 0;
    DWORD taskIndex = 0;

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

        hr = pEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &pDevice);
    EXIT_ON_ERROR(hr)

        hr = pDevice->Activate(
            IID_IAudioClient, CLSCTX_ALL,
            NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

        // Call a helper function to negotiate with the audio
        // device for an exclusive-mode stream format.
        hr = GetStreamFormat(pAudioClient, &pwfx);
    EXIT_ON_ERROR(hr)

        // Initialize the stream to play at the minimum latency.
        hr = pAudioClient->GetDevicePeriod(NULL, &hnsRequestedDuration);
    EXIT_ON_ERROR(hr)

        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_EXCLUSIVE,
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            hnsRequestedDuration,
            hnsRequestedDuration,
            pwfx,
            NULL);
    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
        // Align the buffer if needed, see IAudioClient::Initialize() documentation
        UINT32 nFrames = 0;
        hr = pAudioClient->GetBufferSize(&nFrames);
        EXIT_ON_ERROR(hr)
            hnsRequestedDuration = (REFERENCE_TIME)((double)REFTIMES_PER_SEC / pwfx->nSamplesPerSec * nFrames + 0.5);
        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_EXCLUSIVE,
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            hnsRequestedDuration,
            hnsRequestedDuration,
            pwfx,
            NULL);
    }
    EXIT_ON_ERROR(hr)

        // Tell the audio source which format to use.
        hr = pMySource->SetFormat(pwfx);
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
                hr = pMySource->LoadData(bufferFrameCount, pData, &flags);
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
    CoTaskMemFree(pwfx);
    SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pDevice)
        SAFE_RELEASE(pAudioClient)
        SAFE_RELEASE(pRenderClient)
}

void WasapiOutput::StopPlayback()
{

}