#include "WasapiOutput.h"

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

}

void WasapiOutput::StopPlayback()
{

}