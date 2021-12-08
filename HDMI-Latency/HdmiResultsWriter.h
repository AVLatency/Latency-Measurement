#pragma once
#include "IResultsWriter.h"

class HdmiResultsWriter : IResultsWriter
{
public:
	static HdmiResultsWriter Writer;

	virtual void WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, AudioFormat* audioFormat, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result) override;
};

