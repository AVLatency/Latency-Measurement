#pragma once
#include "IResultsWriter.h"

class ResultsWriter : IResultsWriter
{
public:
	static ResultsWriter Writer;

	virtual void WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string inputFormat) override;
	virtual void WriteFinalResultsLine(bool writeHeader, std::fstream& resultsStream, const AveragedResult& result) override;
};

