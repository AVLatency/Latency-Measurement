#pragma once
#include <vector>
#include <audioclient.h>

class HdmiWaveFormats
{
public:
	static HdmiWaveFormats Formats;

	std::vector<WAVEFORMATEXTENSIBLE*> AllHDMIExtensibleFormats;
	std::vector<WAVEFORMATEX*> AllHDMIExFormats;

	HdmiWaveFormats();

private:
	void PopulateExtensibleFormats();
	void RecordExtensibleSubFormat(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleChannels(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleChannelMask(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleSamplesPerSec(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleBitsPerSample(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleFormat(WAVEFORMATEXTENSIBLE* extensibleFormat);

	void PopulateExFormats();
	void RecordExFormatTag(WAVEFORMATEX* exFormat);
	void RecordExChannels(WAVEFORMATEX* exFormat);
	void RecordExSamplesPerSec(WAVEFORMATEX* exFormat);
	void RecordExBitsPerSample(WAVEFORMATEX* exFormat);
	void RecordExFormat(WAVEFORMATEX* exFormat);

	void SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample);
};

