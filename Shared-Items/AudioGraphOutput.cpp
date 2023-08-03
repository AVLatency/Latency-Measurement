#include "AudioGraphOutput.h"
#include "TestConfiguration.h"
#include "winrt/Windows.Media.MediaProperties.h"

AudioGraphOutput::AudioGraphOutput(bool loop, bool firstChannelOnly, float* audioSamples, int audioSamplesLength)
	: loop(loop), firstChannelOnly(firstChannelOnly), audioSamples(audioSamples), audioSamplesLength(audioSamplesLength)
{
}

AudioGraphOutput::~AudioGraphOutput()
{
}

void AudioGraphOutput::StartPlayback()
{
	playbackInProgress = true;
	StartPlaybackAsync().get();
	while (audioGraph)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (stopRequested)
		{
			if (frameInputNode)
			{
				frameInputNode.QuantumStarted(quantumStartedEventToken); // unregisters event handler
				frameInputNode.Stop();
				frameInputNode.Close();
			}

			if (audioGraph)
			{
				audioGraph.Stop();
			}

			break;
		}
	}
	playbackInProgress = false;
}

IAsyncAction AudioGraphOutput::StartPlaybackAsync()
{
	AudioGraphSettings settings(Render::AudioRenderCategory::Media);
	// Explicitly disabling audio processing to ensure the test waveform isn't excessively modified
	// This is already going to be the case for LowestLatency mode
	settings.DesiredRenderDeviceAudioProcessing(AudioProcessing::Raw);
	// Using lowest latency to align as close as possible with audio input, which speeds up tests
	settings.QuantumSizeSelectionMode(QuantumSizeSelectionMode::LowestLatency);
	CreateAudioGraphResult graphCreateResult = co_await AudioGraph::CreateAsync(settings);

	if (graphCreateResult.Status() == AudioGraphCreationStatus::Success)
	{
		audioGraph = graphCreateResult.Graph();

		CreateAudioDeviceOutputNodeResult createDeviceResult = co_await audioGraph.CreateDeviceOutputNodeAsync();
		if (createDeviceResult.Status() == AudioDeviceNodeCreationStatus::Success)
		{
			deviceOutputNode = createDeviceResult.DeviceOutputNode();

			// Create the FrameInputNode at the same format as the graph
			AudioEncodingProperties nodeEncodingProperties = audioGraph.EncodingProperties();
			frameInputNode = audioGraph.CreateFrameInputNode(nodeEncodingProperties);

			frameInputNode.AddOutgoingConnection(deviceOutputNode);

			// Initialize the Frame Input Node in the stopped state
			frameInputNode.Stop();

			// Hook up an event handler so we can start generating samples when needed
			// This event is triggered when the node is required to provide data
			quantumStartedEventToken = frameInputNode.QuantumStarted([=](auto&& sender, auto&& args)
				{
					AudioOutputCallback(sender, args);
				});

			audioGraph.Start();

			if (frameInputNode)
			{
				frameInputNode.Start();
			}
			else
			{
				printf("Error: No frameInputNode\n");
				playbackInProgress = false;
			}
		}
		else // could not create output node
		{
			printf(std::format("Error: Could not create output device. Status: {}\n", (int)createDeviceResult.Status()).c_str());
			playbackInProgress = false;
		}
	}
	else
	{
		printf(std::format("Error: Could not create AudioGraph. Status: {}\n", (int)graphCreateResult.Status()).c_str());
		playbackInProgress = false;
	}
}

//////////////////////////////////////////////////////////////////////////
void AudioGraphOutput::AudioOutputCallback(AudioFrameInputNode sender, FrameInputNodeQuantumStartedEventArgs args)
{
	// Use the FrameInputNodeQuantumStartedEventArgs.RequiredSamples property
	// to determine how many samples are required to fill the quantum with data.
	// Pass an AudioFrame into the AddFrame method to provide the required audio samples.

	// Need to know how many samples are required. In this case, the node is running at the same rate as the rest of the graph
	// For minimum latency, only provide the required amount of samples. Extra samples will introduce additional latency.
	uint32_t numSamplesNeeded = args.RequiredSamples();

	if (numSamplesNeeded != 0)
	{
		AudioEncodingProperties properties = sender.EncodingProperties();
		uint32_t channelCount = properties.ChannelCount();
		unsigned int bufferSize = numSamplesNeeded * channelCount * sizeof(float); // samples in audio graph are always 32-bit float
		AudioFrame frame(bufferSize);
		{ // intentionally changing scope so that buffer and reference will be unlocked before sender.AddFrame(frame) call.
			AudioBuffer buffer = frame.LockBuffer(AudioBufferAccessMode::Write);
			IMemoryBufferReference reference = buffer.CreateReference();
			float* outputFloatData = (float*)reference.data();

			float volume = Mute ? 0 : TestConfiguration::OutputVolume;
			for (int i = 0; i < numSamplesNeeded * channelCount; i += channelCount)
			{
				if (!FinishedPlayback(true))
				{
					for (int c = 0; c < channelCount; c++)
					{
						if (!firstChannelOnly || c == 0)
						{
							outputFloatData[i + c] = audioSamples[sampleIndex] * volume;
						}
						else
						{
							outputFloatData[i + c] = 0;
						}
					}
					sampleIndex++;
				}
				else
				{
					for (int c = 0; c < channelCount; c++)
					{
						outputFloatData[i + c] = 0;
					}
				}
			}
		} // intentionally changing scope so that buffer and reference will be unlocked before sender.AddFrame(frame) call.
		sender.AddFrame(frame);
	}

	if (FinishedPlayback(true))
	{
		stopRequested = true;
	}
}

void AudioGraphOutput::StopPlayback()
{
	stopRequested = true;
}

bool AudioGraphOutput::FinishedPlayback(bool loopIfNeeded)
{
    bool endOfSamplesReached = sampleIndex >= audioSamplesLength;
    if (loop)
    {
        if (loopIfNeeded && endOfSamplesReached)
        {
            sampleIndex = 0;
        }
        return false;
    }
    else
    {
        return endOfSamplesReached;
    }
}

int AudioGraphOutput::CurrentWindowsSampleRate()
{
	// TODO: Some audio graph singleton that is lazy loaded and blocks until the async thing has finished loading.
	// This audio graph would be used by everything for the lifetime of the app?
	//return AudioGraphSingleton->audioGraph.SamplesPerQuantum;
    return 96000; // TODO
}
