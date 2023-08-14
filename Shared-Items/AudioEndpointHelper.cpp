#include "AudioEndpointHelper.h"
#include <Functiondiscoverykeys_devpkey.h>
#include <comdef.h>


#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

/// <summary>
/// Taken from https://docs.microsoft.com/en-us/windows/win32/coreaudio/device-properties
/// </summary>
std::vector<AudioEndpoint> AudioEndpointHelper::GetAudioEndPoints(EDataFlow type)
{
	std::vector<AudioEndpoint> result;

    HRESULT hr = S_OK;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDeviceCollection* pCollection = NULL;
    IMMDevice* pEndpoint = NULL;
    IPropertyStore* pProps = NULL;
    LPWSTR pwszID = NULL;

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    EXIT_ON_ERROR(hr)

        hr = pEnumerator->EnumAudioEndpoints(
            type, DEVICE_STATE_ACTIVE,
            &pCollection);
    EXIT_ON_ERROR(hr)

        UINT  count;
    hr = pCollection->GetCount(&count);
    EXIT_ON_ERROR(hr)

        //if (count == 0)
        //{
        //    printf("No endpoints found.\n");
        //}

    // Each loop prints the name of an endpoint device.
    for (ULONG i = 0; i < count; i++)
    {
        // Get pointer to endpoint number i.
        hr = pCollection->Item(i, &pEndpoint);
        EXIT_ON_ERROR(hr)

            // Get the endpoint ID string.
            hr = pEndpoint->GetId(&pwszID);
        EXIT_ON_ERROR(hr)

            hr = pEndpoint->OpenPropertyStore(
                STGM_READ, &pProps);
        EXIT_ON_ERROR(hr)

            PROPVARIANT varName;
        // Initialize container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(
            PKEY_Device_FriendlyName, &varName);
        EXIT_ON_ERROR(hr)

            //// Print endpoint friendly name and endpoint ID.
            //printf("Endpoint %d: \"%S\" (%S)\n",
            //    i, varName.pwszVal, pwszID);
        
        AudioEndpoint endpoint(pEndpoint, (char*)_bstr_t(varName.pwszVal), (char*)_bstr_t(pwszID));
        result.push_back(endpoint);

        CoTaskMemFree(pwszID);
        pwszID = NULL;
        PropVariantClear(&varName);
        SAFE_RELEASE(pProps)
            SAFE_RELEASE(pEndpoint)
    }
    SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pCollection)

        return result;

Exit:
    printf("Error!\n");
    CoTaskMemFree(pwszID);
    SAFE_RELEASE(pEnumerator)
        SAFE_RELEASE(pCollection)
        SAFE_RELEASE(pEndpoint)
        SAFE_RELEASE(pProps)

    return result;
}

/// <summary>
/// Taken from https://learn.microsoft.com/en-us/windows/win32/coreaudio/rendering-a-stream
/// </summary>
AudioEndpoint* AudioEndpointHelper::GetDefaultAudioEndPoint(EDataFlow type)
{
    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pEndpoint = NULL;
    IPropertyStore* pProps = NULL;
    LPWSTR pwszID = NULL;

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator);
    if (!FAILED(hr))
    {
        hr = pEnumerator->GetDefaultAudioEndpoint(type, eMultimedia, &pEndpoint); // eMultimedia matches AudioGraphOutput Render::AudioRenderCategory::Media
        if (!FAILED(hr))
        {
            // Get the endpoint ID string.
            hr = pEndpoint->GetId(&pwszID);
            if (!FAILED(hr))
            {
                hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
                if (!FAILED(hr))
                {
                    PROPVARIANT varName;
                    // Initialize container for property value.
                    PropVariantInit(&varName);

                    // Get the endpoint's friendly-name property.
                    hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
                    if (!FAILED(hr))
                    {
                        AudioEndpoint* result = new AudioEndpoint(pEndpoint, (char*)_bstr_t(varName.pwszVal), (char*)_bstr_t(pwszID));

                        SAFE_RELEASE(pEnumerator)
                        SAFE_RELEASE(pEndpoint)
                        SAFE_RELEASE(pProps)
                        return result;
                    }
                }
            }
        }
    }

    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pEndpoint)
    SAFE_RELEASE(pProps)
    return nullptr;
}

int AudioEndpointHelper::GetInputMixFormatSampleRate(const AudioEndpoint& endpoint)
{
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    HRESULT hr;
    IAudioClient* pAudioClient = NULL;
    WAVEFORMATEX* pWaveFormat = NULL;
    int sampleRate = 0;

    hr = CoInitialize(NULL);
    EXIT_ON_ERROR(hr)

    hr = endpoint.Device->Activate(
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->GetMixFormat(&pWaveFormat);
    EXIT_ON_ERROR(hr)
    
    sampleRate = pWaveFormat->nSamplesPerSec;

Exit:
    CoTaskMemFree(pWaveFormat);
    SAFE_RELEASE(pAudioClient)
    CoUninitialize();

    return sampleRate;
}