#pragma once
#include "IResultsWriter.h"

class HdmiResultsWriter : IResultsWriter
{
public:
	static HdmiResultsWriter Writer;

	virtual void WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const GeneratedSamples& generatedSamples, AudioFormat* audioFormat, int inputSampleRate, RecordingResult& result) override;
};

