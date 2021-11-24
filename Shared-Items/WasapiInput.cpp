#include "WasapiInput.h"
#include <mmdeviceapi.h>

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }


WasapiInput::WasapiInput(const AudioEndpoint& endpoint, bool loop, double bufferDurationInSeconds)
    : endpoint(endpoint)
{
    this->loop = loop;
    this->bufferDurationInSeconds = bufferDurationInSeconds;
}

WasapiInput::~WasapiInput()
{
    if (recordingBuffer1 != nullptr)
    {
        delete[] recordingBuffer1;
    }
    if (recordingBuffer2 != nullptr)
    {
        delete[] recordingBuffer2;
    }
}

void WasapiInput::StartRecording()
{
    recordingInProgress = true;
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    REFERENCE_TIME hnsActualDuration;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;
    WAVEFORMATEX* pWaveFormat = NULL;
    UINT32 packetLength = 0;
    BOOL bDone = FALSE;
    BYTE* pData;
    DWORD flags;
    bool pastInitialDiscontinuity = false;

        hr = endpoint.Device->Activate(
            IID_IAudioClient, CLSCTX_ALL,
            NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

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
    CoTaskMemFree(pWaveFormat);
        SAFE_RELEASE(pAudioClient)
        SAFE_RELEASE(pCaptureClient)
}

HRESULT WasapiInput::SetFormat(WAVEFORMATEX* wfex)
{
    return 0;
}

HRESULT WasapiInput::CopyData(BYTE* pData, UINT32 bufferFrameCount, BOOL* bDone)
{

    return 0;
}

void WasapiInput::ThrowAwayRecording()
{

}

void WasapiInput::StopRecording()
{
    stopRequested = true;
}
