#pragma once
#include <vector>
#include <audioclient.h>

class WindowsWaveFormats
{
public:
	static WindowsWaveFormats Formats;

	std::vector<WAVEFORMATEXTENSIBLE*> AllExtensibleFormats;
	std::vector<WAVEFORMATEX*> AllExFormats;

	WindowsWaveFormats();

private:
	void PopulateExtensibleFormats();
	void RecordExtensibleSubFormat(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleChannels(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleChannelMask(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleSamplesPerSec(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleBitsPerSample(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleFormat(WAVEFORMATEXTENSIBLE* extensibleFormat);

	void PopulateIEC61937Formats();
	void RecordIEC61937Format(WAVEFORMATEXTENSIBLE_IEC61937* IEC61937Format);

	void PopulateExFormats();
	void RecordExFormatTag(WAVEFORMATEX* exFormat);
	void RecordExChannels(WAVEFORMATEX* exFormat);
	void RecordExSamplesPerSec(WAVEFORMATEX* exFormat);
	void RecordExBitsPerSample(WAVEFORMATEX* exFormat);
	void RecordExFormat(WAVEFORMATEX* exFormat);

	void SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample);

	void PopulateExDolbyDigitalAC3();
};

