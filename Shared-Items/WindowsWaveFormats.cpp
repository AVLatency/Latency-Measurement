#include "WindowsWaveFormats.h"
//#include "wmcodecdsp.h"

WindowsWaveFormats WindowsWaveFormats::Formats;

WindowsWaveFormats::WindowsWaveFormats()
{
	PopulateExtensibleFormats();
	PopulateExFormats();

	//PopulateAllDriverSupportedFormats();
}

void WindowsWaveFormats::PopulateExtensibleFormats()
{
	WAVEFORMATEXTENSIBLE extensibleFormat;
	memset(&extensibleFormat, 0, sizeof(WAVEFORMATEXTENSIBLE));
	extensibleFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	extensibleFormat.Format.nChannels = 1;
	extensibleFormat.Format.nSamplesPerSec = 44100;
	SetBitsPerSample(&extensibleFormat.Format, 16);
	extensibleFormat.Format.cbSize = 22;
	extensibleFormat.Samples.wValidBitsPerSample = (WORD)extensibleFormat.Format.wBitsPerSample;
	extensibleFormat.dwChannelMask = 0; // This could be all types of who knows!
	extensibleFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	RecordExtensibleSubFormat(&extensibleFormat);

	PopulateIEC61937Formats();

	for (WAVEFORMATEXTENSIBLE* waveFormat : AllExtensibleFormats)
	{
		AllExtensibleAudioFormats.push_back(new AudioFormat((WAVEFORMATEX*)waveFormat));
	}
}

void WindowsWaveFormats::RecordExtensibleSubFormat(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	extensibleFormat->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	RecordExtensibleChannels(extensibleFormat);

	// PERFORMANCE OPTIMIZATION: Exclude these formats I haven't found any drivers that support them:
	// KSDATAFORMAT_SUBTYPE_DTS_AUDIO;

	// PERFORMANCE OPTIMIZATION: Exclude these formats because I don't know of any consumer electronics
	// that send these types of encodings over the wire:
	// KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	// KSDATAFORMAT_SUBTYPE_ALAW;
	// KSDATAFORMAT_SUBTYPE_MULAW;
	// KSDATAFORMAT_SUBTYPE_ADPCM;
}

void WindowsWaveFormats::RecordExtensibleChannels(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	for (int i = 1; i < 9; i++)
	{
		extensibleFormat->Format.nChannels = (WORD)i;
		RecordExtensibleChannelMask(extensibleFormat);
	}
}

void WindowsWaveFormats::RecordExtensibleChannelMask(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	// This list is based off of the configurations that are possible with the Murideo SEVEN Generator
	// (8K SEVEN Generator User Manual page 38 "AUDIO CHANNEL /SPEAKER CONFIGURATION")
	// https://www.murideo.com/mu-gen-seven-g8k.html
	// https://www.murideo.com/seven-g-training.html
	// A few additional configurations have been added that are common Windows configurations, taken from ksmedia.h

	// Always start with a 0 channel mask to see if the IsFormatSupported function gives us a closest match
	// with actual speaker configurations.
	extensibleFormat->dwChannelMask = 0;
	RecordExtensibleSamplesPerSec(extensibleFormat);

	// This is just a guess on my part as to what speaker configurations might possible come up.
	// Who knows if this is at all necessary.
	switch (extensibleFormat->Format.nChannels)
	{
	case 1:
		// Windows Mono
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_CENTER; // KSAUDIO_SPEAKER_MONO;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 2:
		// 2.0
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT; // KSAUDIO_SPEAKER_STEREO
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 3:
		// 3.0 FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 3.0 RC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 2.1 LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 4:
		// 4.0 Windows Quadrophonic
		// Note: Windows HDMI drivers (NVIDIA and Intel at least) replace SPEAKER_BACK (RLC and RRC) with SPEAKER_SIDE (RL and RR), which
		// is a format that already exists later in this list.
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT; // KSAUDIO_SPEAKER_QUAD
		RecordExtensibleSamplesPerSec(extensibleFormat);
		
		// 4.0 RR_RL_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 4.0 RC_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER; // KSAUDIO_SPEAKER_SURROUND
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 4.0 FRC_FLC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 3.1 FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 3.1 RC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 5:
		// 5.0 RR_RL_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.0 RC_RR_RL_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 5.0 FRC_FLC_FC_FR_LF
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.0 FRC_FLC_RC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 4.1 RC_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 4.1 RR_RL_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 4.1 FRC_FLC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 6:
		// 5.1 Windows DVD
		// Note: Windows HDMI drivers (NVIDIA and Intel at least) replace SPEAKER_BACK (RLC and RRC) with SPEAKER_SIDE (RL and RR), which
		// is a format that already exists later in this list.
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT; // KSAUDIO_SPEAKER_5POINT1
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 5.1 RR_RL_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT; // KSAUDIO_SPEAKER_5POINT1_SURROUND
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.1 RC_RR_RL_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 5.1 FRC_FLC_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.1 FRC_FLC_RC_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.0 RC_RR_RL_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 6.0 RRC_RLC_RR_RL_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.0 FRC_FLC_RC_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.0 FRC_FLC_RR_RL_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 7:
		// 7.0 RRC_RLC_RR_RL_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.1 RC_RR_RL_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 6.1 RRC_RLC_RR_RL_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.1 FRC_FLC_RC_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 6.1 FRC_FLC_RR_RL_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 7.0 FRC_FLC_RR_RL_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 8:
		// 7.1 RRC_RLC_RR_RL_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT; // KSAUDIO_SPEAKER_7POINT1_SURROUND
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 7.1 FRC_FLC_RR_RL_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	}
}

void WindowsWaveFormats::RecordExtensibleSamplesPerSec(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	extensibleFormat->Format.nSamplesPerSec = 44100;
	RecordExtensibleBitsPerSample(extensibleFormat);

	extensibleFormat->Format.nSamplesPerSec = 48000;
	RecordExtensibleBitsPerSample(extensibleFormat);

	extensibleFormat->Format.nSamplesPerSec = 96000;
	RecordExtensibleBitsPerSample(extensibleFormat);

	extensibleFormat->Format.nSamplesPerSec = 192000;
	RecordExtensibleBitsPerSample(extensibleFormat);
}

void WindowsWaveFormats::RecordExtensibleBitsPerSample(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	SetBitsPerSample(&extensibleFormat->Format, 16);
	extensibleFormat->Samples.wValidBitsPerSample = extensibleFormat->Format.wBitsPerSample;
	RecordExtensibleFormat(extensibleFormat);

	SetBitsPerSample(&extensibleFormat->Format, 24);
	extensibleFormat->Samples.wValidBitsPerSample = extensibleFormat->Format.wBitsPerSample;
	RecordExtensibleFormat(extensibleFormat);

	SetBitsPerSample(&extensibleFormat->Format, 32);
	extensibleFormat->Samples.wValidBitsPerSample = extensibleFormat->Format.wBitsPerSample;
	RecordExtensibleFormat(extensibleFormat);
}

void WindowsWaveFormats::RecordExtensibleFormat(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	WAVEFORMATEXTENSIBLE* pWfx = new WAVEFORMATEXTENSIBLE();
	*pWfx = *extensibleFormat;
	AllExtensibleFormats.push_back(pWfx);
}

void WindowsWaveFormats::PopulateIEC61937Formats()
{
	// Currently none supported by WasapiOutput class.
}

void WindowsWaveFormats::RecordIEC61937Format(WAVEFORMATEXTENSIBLE_IEC61937* IEC61937Format)
{
	WAVEFORMATEXTENSIBLE_IEC61937* pWfx = new WAVEFORMATEXTENSIBLE_IEC61937();
	*pWfx = *IEC61937Format;
	AllExtensibleFormats.push_back((WAVEFORMATEXTENSIBLE*)pWfx);
}

void WindowsWaveFormats::PopulateExFormats()
{
	// WAVEFORMATEX are legacy formats that are typically duplicates of the WAVEFORMATEXTENSIBLE ones
	// But they are still important because NVIDIA (and possibly other) drivers only support 24 bit HDMI
	// audio output through these legacy formats.

	WAVEFORMATEX exFormat;
	memset(&exFormat, 0, sizeof(WAVEFORMATEX));
	exFormat.wFormatTag = WAVE_FORMAT_PCM;
	exFormat.nChannels = 1;
	exFormat.nSamplesPerSec = 44100;
	SetBitsPerSample(&exFormat, 16);
	exFormat.cbSize = 0;

	RecordExFormatTag(&exFormat);

	for (WAVEFORMATEX* waveFormat : AllExFormats)
	{
		AllExAudioFormats.push_back(new AudioFormat(waveFormat));
	}
}

void WindowsWaveFormats::RecordExFormatTag(WAVEFORMATEX* exFormat)
{
	exFormat->wFormatTag = WAVE_FORMAT_PCM;
	RecordExChannels(exFormat);

	// PERFORMANCE OPTIMIZATION: Exclude these formats because the encoders are not yet implemented:
	// WAVE_FORMAT_DOLBY_AC3_SPDIF
	// WAVE_FORMAT_DTS

	// PERFORMANCE OPTIMIZATION: Exclude these formats I haven't found any drivers that support them:
	// WAVE_FORMAT_DOLBY_AC2;
	// WAVE_FORMAT_DOLBY_AC4;
	// WAVE_FORMAT_DTS_DS;
	// WAVE_FORMAT_DTS2;

	// PERFORMANCE OPTIMIZATION: Exclude these formats because I don't know of any consumer electronics
	// that send these types of encodings over the wire:
	// WAVE_FORMAT_ADPCM;
	// WAVE_FORMAT_IEEE_FLOAT;
	// WAVE_FORMAT_ALAW;
	// WAVE_FORMAT_MULAW;
}

void WindowsWaveFormats::RecordExChannels(WAVEFORMATEX* exFormat)
{
	// "For exclusive-mode formats, the method queries the device driver.
	// Some device drivers will report that they support a 1-channel or 2-channel
	// PCM format if the format is specified by a stand-alone WAVEFORMATEX structure, 
	// but will reject the same format if it is specified by a WAVEFORMATEXTENSIBLE structure.
	// To obtain reliable results from these drivers, exclusive-mode applications
	// should call IsFormatSupported twice for each 1-channel or 2-channel PCM format—one
	// call should use a stand-alone WAVEFORMATEX structure to specify the format, and the
	// other call should use a WAVEFORMATEXTENSIBLE structure to specify the same format."
	// Source: https://docs.microsoft.com/en-us/windows/win32/coreaudio/device-formats
	//
	// tl;dr: WAVEFORMATEX only needs to be populated with 1 or 2 channel PCM formats.
	// The rest of the formats can be handled by the WAVEFORMATEXTENSIBLE formats.

	// Buuuuuut... Microsoft documentation isn't the most trustworthly, so we'll look for
	// any number of channels anyway:

	for (int i = 1; i < 9; i++)
	{
		exFormat->nChannels = (WORD)i;
		RecordExSamplesPerSec(exFormat);
	}
}

void WindowsWaveFormats::RecordExSamplesPerSec(WAVEFORMATEX* exFormat)
{
	exFormat->nSamplesPerSec = 44100;
	RecordExBitsPerSample(exFormat);

	exFormat->nSamplesPerSec = 48000;
	RecordExBitsPerSample(exFormat);

	exFormat->nSamplesPerSec = 96000;
	RecordExBitsPerSample(exFormat);

	exFormat->nSamplesPerSec = 192000;
	RecordExBitsPerSample(exFormat);
}

void WindowsWaveFormats::RecordExBitsPerSample(WAVEFORMATEX* exFormat)
{
	SetBitsPerSample(exFormat, 16);
	RecordExFormat(exFormat);

	SetBitsPerSample(exFormat, 24);
	RecordExFormat(exFormat);

	SetBitsPerSample(exFormat, 32);
	RecordExFormat(exFormat);
}

void WindowsWaveFormats::RecordExFormat(WAVEFORMATEX* exFormat)
{
	WAVEFORMATEX* pWfx = new WAVEFORMATEX();
	*pWfx = *exFormat;
	AllExFormats.push_back(pWfx);
}

void WindowsWaveFormats::SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample)
{
	wfx->wBitsPerSample = bitsPerSample;
	wfx->nBlockAlign = wfx->wBitsPerSample / 8 * wfx->nChannels;
	wfx->nAvgBytesPerSec = wfx->nBlockAlign * wfx->nSamplesPerSec;
}

void WindowsWaveFormats::PopulateAllDriverSupportedFormats()
{
#if _DEBUG
	// The following code can be used to determine what wave formats your driver might support.
	// My drivers seem to support the following:
	// DOLBY_AC3_SPDIF (48 kHz after encoding)
	// DTS (48 kHz after encoding)

	WAVEFORMATEX exFormat;
	memset(&exFormat, 0, sizeof(WAVEFORMATEX));
	exFormat.wFormatTag = WAVE_FORMAT_PCM;
	exFormat.nChannels = 2;
	exFormat.nSamplesPerSec = 48000;
	SetBitsPerSample(&exFormat, 16);
	exFormat.cbSize = 0;

	exFormat.wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
	RecordExSamplesPerSec(&exFormat);
	exFormat.wFormatTag = WAVE_FORMAT_DTS;
	RecordExSamplesPerSec(&exFormat);
	exFormat.wFormatTag = WAVE_FORMAT_DOLBY_AC2;
	RecordExSamplesPerSec(&exFormat);
	exFormat.wFormatTag = WAVE_FORMAT_DOLBY_AC4;
	RecordExSamplesPerSec(&exFormat);
	exFormat.wFormatTag = WAVE_FORMAT_DTS_DS;
	RecordExSamplesPerSec(&exFormat);
	exFormat.wFormatTag = WAVE_FORMAT_DTS2;
	RecordExSamplesPerSec(&exFormat);
	exFormat.wFormatTag = WAVE_FORMAT_DVM;
	RecordExSamplesPerSec(&exFormat);

	exFormat.wFormatTag = 0xe06d8033; // KSDATAFORMAT_SUBTYPE_DTS_AUDIO
	RecordExSamplesPerSec(&exFormat);

	exFormat.wFormatTag = 0x00001608; // MEDIASUBTYPE_MPEG_ADTS_AAC
	RecordExSamplesPerSec(&exFormat);

	exFormat.wFormatTag = 0x00001600; // MEDIASUBTYPE_NOKIA_MPEG_ADTS_AAC
	RecordExSamplesPerSec(&exFormat);

	exFormat.wFormatTag = 0x0000160A; // MEDIASUBTYPE_VODAFONE_MPEG_ADTS_AAC
	RecordExSamplesPerSec(&exFormat);

	exFormat.wFormatTag = 0xa2e58eb7; // MEDIASUBTYPE_DTS_HD
	RecordExSamplesPerSec(&exFormat);

	exFormat.wFormatTag = 0xa7fb87af; // MEDIASUBTYPE_DOLBY_DDPLUS
	RecordExSamplesPerSec(&exFormat);

	exFormat.wFormatTag = 0xeb27cec4; // MEDIASUBTYPE_DOLBY_TRUEHD
	RecordExSamplesPerSec(&exFormat);

	WAVEFORMATEXTENSIBLE extensibleFormat;
	memset(&extensibleFormat, 0, sizeof(WAVEFORMATEXTENSIBLE));
	extensibleFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	extensibleFormat.Format.nChannels = 2;
	extensibleFormat.Format.nSamplesPerSec = 48000;
	SetBitsPerSample(&extensibleFormat.Format, 16);
	extensibleFormat.Format.cbSize = 22;
	extensibleFormat.Samples.wValidBitsPerSample = (WORD)extensibleFormat.Format.wBitsPerSample;
	extensibleFormat.dwChannelMask = 0; // This could be all types of who knows!

	extensibleFormat.SubFormat = KSDATAFORMAT_SUBTYPE_DTS_AUDIO;
	RecordExtensibleFormat(&extensibleFormat);

	// TODO: somehow fill these in(?)
	//extensibleFormat.SubFormat = MEDIASUBTYPE_MPEG_ADTS_AAC;
	//RecordExtensibleFormat(&extensibleFormat);

	//extensibleFormat.SubFormat = MEDIASUBTYPE_NOKIA_MPEG_ADTS_AAC;
	//RecordExtensibleFormat(&extensibleFormat);

	//extensibleFormat.SubFormat = MEDIASUBTYPE_VODAFONE_MPEG_ADTS_AAC;
	//RecordExtensibleFormat(&extensibleFormat);

	//extensibleFormat.SubFormat = MEDIASUBTYPE_DTS2;
	//RecordExtensibleFormat(&extensibleFormat);

	//extensibleFormat.SubFormat = MEDIASUBTYPE_DTS_HD;
	//RecordExtensibleFormat(&extensibleFormat);

	//extensibleFormat.SubFormat = MEDIASUBTYPE_DOLBY_DDPLUS;
	//RecordExtensibleFormat(&extensibleFormat);

	//extensibleFormat.SubFormat = MEDIASUBTYPE_DOLBY_TRUEHD;
	//RecordExtensibleFormat(&extensibleFormat);

	//extensibleFormat.SubFormat = MEDIASUBTYPE_DVM;
	//RecordExtensibleFormat(&extensibleFormat);

	// The following code can be used to determine what IEC61937 formats your driver might support.
	// My drivers seem to support the following:
	// IEC61937_DOLBY_DIGITAL_PLUS (192 kHz after encoding)
	// IEC61937_DOLBY_DIGITAL_AC3 (192 kHz after encoding)
	// IEC61937_DOLBY_MLP_MAT10 (192 kHz after encoding)
	// IEC61937_DTS_HD (192 kHz after encoding)
	// IEC61937_DTS (192 kHz after encoding)

	WAVEFORMATEXTENSIBLE_IEC61937 wfext;
	wfext.FormatExt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfext.FormatExt.Format.nChannels = 2;              // One IEC 60958 Line.
	wfext.FormatExt.Format.nSamplesPerSec = 192000;    // Link runs at 192 KHz.
	wfext.FormatExt.Format.nAvgBytesPerSec = 768000;   // 192 KHz * 4.
	wfext.FormatExt.Format.nBlockAlign = 4;            // 16 bits * 2 channels.
	wfext.FormatExt.Format.wBitsPerSample = 16;        // Always at 16 bits over IEC 60958.
	wfext.FormatExt.Format.cbSize = 34;                // Indicates that Format is part of a WAVEFORMATEXTENSIBLE_IEC61937 structure.
	wfext.FormatExt.Samples.wValidBitsPerSample = 16;
	wfext.FormatExt.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;    // Dolby 5.1 Surround.
	wfext.dwEncodedSamplesPerSec = 48000;                       // Sample rate of encoded content.
	wfext.dwEncodedChannelCount = 6;                            // Encoded data contains 6 channels.
	wfext.dwAverageBytesPerSec = 0;                             // Ignored for this format.

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS_ATMOS;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MAT20;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MAT21;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DTS;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DTS_HD;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DTSX_E1;
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DTSX_E2;
	RecordIEC61937Format(&wfext);

	// The following format configurations are taken from this website:
	// https://learn.microsoft.com/en-us/windows/win32/coreaudio/representing-formats-for-iec-61937-transmissions

	wfext.FormatExt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfext.FormatExt.Format.nChannels = 2;              // One IEC 60958 Line.
	wfext.FormatExt.Format.nSamplesPerSec = 192000;    // Link runs at 192 KHz.
	wfext.FormatExt.Format.nAvgBytesPerSec = 768000;   // 192 KHz * 4.
	wfext.FormatExt.Format.nBlockAlign = 4;            // 16 bits * 2 channels.
	wfext.FormatExt.Format.wBitsPerSample = 16;        // Always at 16 bits over IEC 60958.
	wfext.FormatExt.Format.cbSize = 34;                // Indicates that Format is part of a WAVEFORMATEXTENSIBLE_IEC61937 structure.
	wfext.FormatExt.Samples.wValidBitsPerSample = 16;
	wfext.FormatExt.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;    // Dolby 5.1 Surround.
	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS;
	wfext.dwEncodedSamplesPerSec = 48000;                       // Sample rate of encoded content.
	wfext.dwEncodedChannelCount = 6;                            // Encoded data contains 6 channels.
	wfext.dwAverageBytesPerSec = 0;                             // Ignored for this format.
	RecordIEC61937Format(&wfext);

	// This one is supported by my HDMI audio driver:
	wfext.FormatExt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfext.FormatExt.Format.nChannels = 8;                // Four IEC 60958 Lines.
	wfext.FormatExt.Format.nSamplesPerSec = 192000;      // Link runs at 192 KHz.
	wfext.FormatExt.Format.nAvgBytesPerSec = 3072000;    // 192 KHz * 16.
	wfext.FormatExt.Format.nBlockAlign = 16;             // 16-bits * 8 channels.
	wfext.FormatExt.Format.wBitsPerSample = 16;          // Always at 16 bits over IEC 60958.
	wfext.FormatExt.Format.cbSize = 34;                  // Indicates that Format is part of a WAVEFORMATEXTENSIBLE_IEC61937 structure.
	wfext.FormatExt.Samples.wValidBitsPerSample = 16;
	wfext.FormatExt.dwChannelMask = KSAUDIO_SPEAKER_7POINT1;    // Dolby 7.1 Surround.
	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP; // This structure indicates MLP (MAT 1.0) support.
	wfext.dwEncodedSamplesPerSec = 96000;                       // Sample rate of encoded content.
	wfext.dwEncodedChannelCount = 8;                            // Encoded data contains 8 channels.
	wfext.dwAverageBytesPerSec = 0;                             // Ignored for this format.
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfext.FormatExt.Format.nChannels = 8;                // Four IEC 60958 Lines.
	wfext.FormatExt.Format.nSamplesPerSec = 192000;      // Link runs at 192 KHz.
	wfext.FormatExt.Format.nAvgBytesPerSec = 3072000;    // 192 KHz * 16.
	wfext.FormatExt.Format.nBlockAlign = 16;             // 16-bits * 8 channels.
	wfext.FormatExt.Format.wBitsPerSample = 16;          // Always at 16 bits over IEC 60958.
	wfext.FormatExt.Format.cbSize = 34;                  // Indicates that Format is part of a WAVEFORMATEXTENSIBLE_IEC61937 structure.
	wfext.FormatExt.Samples.wValidBitsPerSample = 16;
	wfext.FormatExt.dwChannelMask = KSAUDIO_SPEAKER_7POINT1;    // Dolby 7.1 Surround.
	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MAT20; // This structure indicates MAT 2.0 support.
	wfext.dwEncodedSamplesPerSec = 96000;                       // Sample rate of encoded content.
	wfext.dwEncodedChannelCount = 8;                            // Encoded data contains 8 channels.
	wfext.dwAverageBytesPerSec = 0;                             // Ignored for this format.
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfext.FormatExt.Format.nChannels = 8;                // Four IEC 60958 Lines.
	wfext.FormatExt.Format.nSamplesPerSec = 192000;      // Link runs at 192 KHz.
	wfext.FormatExt.Format.nAvgBytesPerSec = 3072000;    // 192 KHz * 16.
	wfext.FormatExt.Format.nBlockAlign = 16;             // 16-bits * 8 channels.
	wfext.FormatExt.Format.wBitsPerSample = 16;          // Always at 16 bits over IEC 60958.
	wfext.FormatExt.Format.cbSize = 34;                  // Indicates that Format is part of a WAVEFORMATEXTENSIBLE_IEC61937 structure.
	wfext.FormatExt.Samples.wValidBitsPerSample = 16;
	wfext.FormatExt.dwChannelMask = KSAUDIO_SPEAKER_7POINT1;    // Dolby 7.1 Surround.
	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MAT21; // This structure indicates MAT 2.1 support.
	wfext.dwEncodedSamplesPerSec = 96000;                       // Sample rate of encoded content.
	wfext.dwEncodedChannelCount = 8;                            // Encoded data contains 8 channels.
	wfext.dwAverageBytesPerSec = 0;                             // Ignored for this format.
	RecordIEC61937Format(&wfext);

	wfext.FormatExt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfext.FormatExt.Format.nChannels = 2;             // One IEC 60958 Line.
	wfext.FormatExt.Format.nSamplesPerSec = 96000;    // Link runs at 96 KHz.
	wfext.FormatExt.Format.nAvgBytesPerSec = 384000;  // 96 KHz * 4.
	wfext.FormatExt.Format.nBlockAlign = 4;           // 16 bits * 8 channels.
	wfext.FormatExt.Format.wBitsPerSample = 16;       // Always at 16 bits over link.
	wfext.FormatExt.Format.cbSize = 34;               // Indicates that Format is part of a WAVEFORMATEXTENSIBLE_IEC61937 structure.
	wfext.FormatExt.Samples.wValidBitsPerSample = 16;
	wfext.FormatExt.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;    // 5.1 Surround.
	wfext.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_IEC61937_WMA_PRO;
	wfext.dwEncodedSamplesPerSec = 96000;                       // Sample rate of encoded content.
	wfext.dwEncodedChannelCount = 6;                            // Encoded data contains 6 channels.
	wfext.dwAverageBytesPerSec = 0;                             // Ignored for this format.
	RecordIEC61937Format(&wfext);

#endif
}
