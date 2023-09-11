#pragma once
#include <fstream>
#include "GeneratedSamples.h"
#include "AudioFormat.h"
#include "RecordingResult.h"
#include "AudioEndpoint.h"
#include "AveragedResult.h"
#include <string>

class IResultsWriter
{
public:
	virtual void WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, AudioEndpoint* outputEndpoint, AudioEndpoint* inputEndpoint, RecordingResult& result, std::string inputFormat) = 0;
	virtual void WriteFinalResultsLine(bool writeHeader, std::fstream& resultsStream, const AveragedResult& result) = 0;
};
