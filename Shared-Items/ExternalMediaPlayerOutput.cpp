#include "ExternalMediaPlayerOutput.h"
#include <thread>
//#include <shellapi.h>
#include <format>

ExternalMediaPlayerOutput::ExternalMediaPlayerOutput(std::string filePath, bool loop) : filePath(filePath), loop(loop)
{
}

ExternalMediaPlayerOutput::~ExternalMediaPlayerOutput()
{
}

void ExternalMediaPlayerOutput::StartPlayback()
{
    do
    {
        system(std::format("\"\"C:\\Program Files (x86)\\K-Lite Codec Pack\\MPC-HC64\\mpc-hc64.exe\" \"{}\" /play /close \"", filePath).c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (stopRequested.load(std::memory_order_acquire))
        {
            break;
        }
    } while (loop);
}

void ExternalMediaPlayerOutput::StopPlayback()
{
	stopRequested.store(true, std::memory_order_release);
}
