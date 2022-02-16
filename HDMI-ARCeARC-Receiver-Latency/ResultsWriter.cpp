#include "ResultsWriter.h"
#include "TestNotes.h"
#include <imgui.h>

ResultsWriter ResultsWriter::Writer;

void ResultsWriter::WriteIndividualRecordingResults(bool writeHeader, std::fstream& detailedResultsStream, const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, RecordingResult& result, std::string inputFormat)
{
	// TODO: copy this over from another project
}

void ResultsWriter::WriteFinalResultsLine(bool writeHeader, std::fstream& resultsStream, const AveragedResult& result)
{
	// TODO: copy this over from another project
}
