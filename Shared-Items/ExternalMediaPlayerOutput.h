#pragma once
#include "AbstractOutput.h"
#include <string>

class ExternalMediaPlayerOutput : public AbstractOutput
{
public:
	ExternalMediaPlayerOutput(std::string filePath, bool loop);
	~ExternalMediaPlayerOutput();

	void StartPlayback();
	void StopPlayback();

private:
	std::atomic<bool> stopRequested = false;
	std::string filePath;
	bool loop;
};

