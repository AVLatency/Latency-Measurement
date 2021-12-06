#pragma once
#include <fstream>
#include "GeneratedSamples.h"
#include "AudioFormat.h"
#include "RecordingResult.h"

class IResultsWriter
{
public:
	virtual void WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const GeneratedSamples& generatedSamples, AudioFormat* audioFormat, int inputSampleRate, RecordingResult& result) = 0;
};

