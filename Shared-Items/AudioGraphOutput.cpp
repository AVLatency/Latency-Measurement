#include "AudioGraphOutput.h"
#include "TestConfiguration.h"
#include "winrt/Windows.Media.MediaProperties.h"

int AudioGraphOutput::lastSampleRate = 0;

AudioGraphOutput::AudioGraphOutput(bool loop, bool firstChannelOnly, float* audioSamples, int audioSamplesLength)
	: loop(loop), firstChannelOnly(firstChannelOnly), audioSamples(audioSamples), audioSamplesLength(audioSamplesLength)
{
}

AudioGraphOutput::~AudioGraphOutput()
{
	DestroyGraph();
}

void AudioGraphOutput::StartPlayback()
{
	sampleIndex = 0;
	playbackInProgress = true;
	DestroyGraph();
	audioGraph = new AudioGraphWrapper();
	CreateGraphAsync(audioGraph).get();
	StartPlaybackAsync().get();
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (stopRequested)
		{
			break;
		}
	}
	DestroyGraph();
	playbackInProgress = false;
}

IAsyncAction AudioGraphOutput::CreateGraphAsync(AudioGraphWrapper* audioGraph)
{
	AudioGraphSettings settings(Render::AudioRenderCategory::Media); // Matches AudioEndpointHelper::GetDefaultAudioEndPoint eMultimedia
	// Explicitly disabling audio processing to ensure the test waveform isn't excessively modified
	// This is already going to be the case for LowestLatency mode
	settings.DesiredRenderDeviceAudioProcessing(AudioProcessing::Raw);
	// Using lowest latency to align as close as possible with audio input, which speeds up tests
	settings.QuantumSizeSelectionMode(QuantumSizeSelectionMode::LowestLatency);
	CreateAudioGraphResult graphCreateResult = co_await AudioGraph::CreateAsync(settings);

	if (graphCreateResult.Status() == AudioGraphCreationStatus::Success)
	{
		audioGraph->graph = graphCreateResult.Graph();
		lastSampleRate = audioGraph->graph.EncodingProperties().SampleRate();
	}
	else
	{
		printf(std::format("Error: Could not create AudioGraph. Status: {}\n", (int)graphCreateResult.Status()).c_str());
	}
}

void AudioGraphOutput::DestroyGraph()
{
	if (audioGraph != nullptr)
	{
		if (audioGraph->frameInputNode != nullptr)
		{
			audioGraph->frameInputNode.QuantumStarted(audioGraph->quantumStartedEventToken); // unregisters event handler
			audioGraph->frameInputNode.Stop();
		}
		if (audioGraph->deviceOutputNode != nullptr)
		{
			audioGraph->deviceOutputNode.Stop();
		}
		if (audioGraph->graph != nullptr)
		{
			audioGraph->graph.Stop();
		}
		delete audioGraph;
	}
}

IAsyncAction AudioGraphOutput::StartPlaybackAsync()
{
	CreateAudioDeviceOutputNodeResult createDeviceResult = co_await audioGraph->graph.CreateDeviceOutputNodeAsync();
	if (createDeviceResult.Status() == AudioDeviceNodeCreationStatus::Success)
	{
		audioGraph->deviceOutputNode = createDeviceResult.DeviceOutputNode();

		// Create the FrameInputNode at the same format as the graph
		AudioEncodingProperties nodeEncodingProperties = audioGraph->graph.EncodingProperties();
		audioGraph->frameInputNode = audioGraph->graph.CreateFrameInputNode(nodeEncodingProperties);

		audioGraph->frameInputNode.AddOutgoingConnection(audioGraph->deviceOutputNode);

		// Initialize the Frame Input Node in the stopped state
		audioGraph->frameInputNode.Stop();

		// Hook up an event handler so we can start generating samples when needed
		// This event is triggered when the node is required to provide data
		audioGraph->quantumStartedEventToken = audioGraph->frameInputNode.QuantumStarted([=](auto&& sender, auto&& args)
			{
				AudioOutputCallback(sender, args);
			});

		audioGraph->graph.Start();

		if (audioGraph->frameInputNode)
		{
			audioGraph->frameInputNode.Start();
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
	if (lastSampleRate == 0)
	{
		winrt::init_apartment();
		AudioGraphWrapper* audioGraph = new AudioGraphWrapper();
		CreateGraphAsync(audioGraph).get();
		lastSampleRate = audioGraph->graph.EncodingProperties().SampleRate();
		delete audioGraph;
	}
	return lastSampleRate;
}
