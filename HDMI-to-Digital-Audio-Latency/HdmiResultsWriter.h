#pragma once
#include "IResultsWriter.h"

class HdmiResultsWriter : IResultsWriter
{
public:
	static HdmiResultsWriter Writer;

	virtual void WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string inputFormat) override;
	virtual void WriteFinalResultsLine(bool writeHeader, std::fstream& resultsStream, const AveragedResult& result) override;
};

