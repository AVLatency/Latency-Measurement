#pragma once
#include <fstream>
#include "GeneratedSamples.h"
#include "AudioFormat.h"
#include "RecordingResult.h"
#include "AudioEndpoint.h"
#include "AveragedResult.h"

class IResultsWriter
{
public:
	virtual void WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result) = 0;
	virtual void WriteFinalResultsLine(bool writeHeader, std::fstream& resultsStream, const AveragedResult& result) = 0;
};
