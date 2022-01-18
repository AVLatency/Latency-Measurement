#pragma once

enum struct MeasurementToolGuiState
{
	GettingStarted = 0,
	SelectAudioDevices,
	AdjustVolume,
	CancellingAdjustVolume,
	FinishingAdjustVolume,
	MeasurementConfig,
	Measuring,
	CancellingMeasuring,
	Results
};
