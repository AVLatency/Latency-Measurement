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
    : endpoint(endpoint), loop(loop), bufferDurationInSeconds(bufferDurationInSeconds)
{
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

    hr = CoInitialize(NULL);
    EXIT_ON_ERROR(hr)

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
        while (bDone == FALSE && !stopRequested)
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
                    if (pastInitialDiscontinuity && !loop)
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

    CoUninitialize();
    
    recordingInProgress = false;
}

UINT16 WasapiInput::GetFormatID()
{
    return EXTRACT_WAVEFORMATEX_ID(&waveFormat.SubFormat);
}

HRESULT WasapiInput::SetFormat(WAVEFORMATEX* wfex)
{
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
    printf("WasapiInput::SetFormat\nFormat type: 0x%.4x\nChannels: %u\nSamples Per Sec: %u\nAvg Bytes Per Sec: %u\nBlock Align: %u\nBits Per Sample: %u\ncbSize: %u\nValid Bits Per Sample: %u\nSamples Per Block: %u\nChannel Mask: 0x%.8x\n",
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
    else if (waveFormat.Format.nChannels < 2)
    {
        return -1; // Only one input is useless to these latency measuremnet tools
    }

    recordingBufferLength = recordedAudioNumChannels * round(waveFormat.Format.nSamplesPerSec * bufferDurationInSeconds);
    recordingBuffer1 = new float[recordingBufferLength];
    if (loop)
    {
        recordingBuffer2 = new float[recordingBufferLength];
    }

    return S_OK;
}

HRESULT WasapiInput::CopyData(BYTE* pData, UINT32 bufferFrameCount, BOOL* bDone)
{
    if (FinishedRecording(false))
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
            for (int c = 0; c < recordedAudioNumChannels; c++)
            {
                CurrentBuffer()[recordedAudioIndex] = pData == NULL ? 0.0f : castData[i + c];
                recordedAudioIndex++;
                if (FinishedRecording(loop))
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
            for (int c = 0; c < recordedAudioNumChannels; c++)
            {
                CurrentBuffer()[recordedAudioIndex] = pData == NULL ? 0.0f : castData[i + c] / (float)SHRT_MIN * -1.0f;
                recordedAudioIndex++;
                if (FinishedRecording(loop))
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
        // nBlockAlign and bufferFrameCount are only used for the buffer size!
        int dataLength = bufferFrameCount * waveFormat.Format.nBlockAlign;
        // Actual frame size is 32 bits for 24 bit audio:
        int bytesPerSampleWithPadding = 32 / 8;
        int bytesPerFrame = bytesPerSampleWithPadding * waveFormat.Format.nChannels;

        unsigned int* castData = (unsigned int*)pData;

        for (int i = 0; i < dataLength; i += bytesPerFrame)
        {
            for (int c = 0; c < recordedAudioNumChannels; c++)
            {
                float thisSample = 0;
                if (pData != NULL)
                {
                    // TODO: Test this!
                    // Turn this padded 24 bit value into a 32 bit value
                    unsigned int uintValue = castData[i + c]; 
                    uintValue &= ~(unsigned int(1) << 31); // clear the negative bit
                    uintValue <<= 8; // trash the padding
                    uintValue |= (castData[i + c] & (unsigned int(1) << 31)); // bring back the negative bit

                    thisSample = (int)uintValue / (float)INT_MIN * -1.0f;
                }
                CurrentBuffer()[recordedAudioIndex] = thisSample;
                recordedAudioIndex++;
                if (FinishedRecording(loop))
                {
                    *bDone = true;
                    return S_OK;
                }
            }
        }
        return S_OK;
    }
    else if (GetFormatID() == WAVE_FORMAT_PCM && waveFormat.Format.wBitsPerSample == 32)
    {
        INT32* castData = (INT32*)pData;
        for (UINT32 i = 0; i < bufferFrameCount * numChannels; i += numChannels)
        {
            for (int c = 0; c < recordedAudioNumChannels; c++)
            {
                CurrentBuffer()[recordedAudioIndex] = pData == NULL ? 0.0f : castData[i + c] / (float)INT_MIN * -1.0f;
                recordedAudioIndex++;
                if (FinishedRecording(loop))
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

bool WasapiInput::FinishedRecording(bool flipBuffersIfNeeded)
{
    bool endOfBufferReached = recordedAudioIndex >= recordingBufferLength;
    if (loop)
    {
        if(flipBuffersIfNeeded && endOfBufferReached)
        {
            recordedAudioIndex = 0;
            recordingToBuffer1 = !recordingToBuffer1;
        }
        return false;
    }
    else
    {
        return endOfBufferReached;
    }
}

void WasapiInput::ThrowAwayRecording()
{
    // Write easily identifyable garbage to the recording and set it to be completed
    RecordingFailed = true;
    bool high = true;
    for (int i = 0; i < recordingBufferLength;)
    {
        CurrentBuffer()[i] = high ? 1 : -1;
        i++;
        if (recordingBufferLength % 2 == 0)
        {
            CurrentBuffer()[i] = high ? 1 : -1;
            i++;
        }
        high = !high;
    }
    recordedAudioIndex = recordingBufferLength;
}

void WasapiInput::StopRecording()
{
    stopRequested = true;
}

float* WasapiInput::CurrentBuffer()
{
    return recordingToBuffer1 ? recordingBuffer1 : recordingBuffer2;
}
