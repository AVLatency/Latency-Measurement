#pragma once
#include <fstream>
#include "GeneratedSamples.h"
#include "AudioFormat.h"
#include "RecordingResult.h"
#include "AudioEndpoint.h"

class IResultsWriter
{
public:
	virtual void WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, AudioFormat* audioFormat, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result) = 0;
};

