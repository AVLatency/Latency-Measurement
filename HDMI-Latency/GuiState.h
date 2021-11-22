#pragma once

enum struct GuiState
{
	GettingStarted,
	SelectAudioDevices,
	AdjustLeftVolume,
	CancellingAdjustLeftVolume,
	AdjustRightVolume,
	CancellingAdjustRightVolume,
	MeasurementConfig,
	Measuring,
	CancellingMeasuring,
	Results
};
