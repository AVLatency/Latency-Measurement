#pragma once
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Media.Audio.h"

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media;
using namespace winrt::Windows::Media::Audio;
using namespace winrt::Windows::Media::MediaProperties;

class AudioGraphWrapper
{
public:
	AudioGraphWrapper();
	AudioGraph graph = nullptr;
	AudioDeviceOutputNode deviceOutputNode = nullptr;
	AudioFrameInputNode frameInputNode = nullptr;
	event_token quantumStartedEventToken;
};
