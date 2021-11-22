#pragma once

enum struct GuiState
{
	GettingStarted = 0,
	SelectAudioDevices,
	AdjustVolume,
	CancellingAdjustVolume,
	MeasurementConfig,
	Measuring,
	CancellingMeasuring,
	Results
};
