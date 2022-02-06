#include "HdmiResultsWriter.h"
#include "TestNotes.h"
#include <imgui.h>

HdmiResultsWriter HdmiResultsWriter::Writer;

void HdmiResultsWriter::WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string inputFormat)
{
	// TODO: copy this over from another project
}

void HdmiResultsWriter::WriteFinalResultsLine(bool writeHeader, std::fstream& resultsStream, const AveragedResult& result)
{
	// TODO: copy this over from another project
}
