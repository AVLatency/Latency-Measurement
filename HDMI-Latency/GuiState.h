#pragma once

enum struct GuiState
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
