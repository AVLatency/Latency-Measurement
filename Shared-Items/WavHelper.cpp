#include "WavHelper.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <audioclient.h>
using namespace std;

namespace little_endian_io
{
    template <typename Word>
    std::ostream& write_word(std::ostream& outs, Word value, unsigned size = sizeof(Word))
    {
        for (; size; --size, value >>= 8)
            outs.put(static_cast <char> (value & 0xFF));
        return outs;
    }
}
using namespace little_endian_io;

void WavHelper::SaveWavFile(std::string path, std::string filename, float* samples, int samplesLength, int samplesPerSecond, int numInputChannels, int numOutputChannels, bool firstChannelOnly, float volume, int loopCount)
{
    filesystem::create_directories(filesystem::path(path));

    // http://www.topherlee.com/software/pcm-tut-wavformat.html
    // https://www.cplusplus.com/forum/beginner/166954/
    // https://gist.github.com/csukuangfj/c1d1d769606260d436f8674c30662450

    ofstream f(std::format("{}/{}", path, filename), ios::binary);

    unsigned short bitsPerSample = 16;

    // Write the file headers
    f << "RIFF----WAVEfmt ";     // (file size to be filled in later)
    write_word(f, 16, 4);  // size of the fmt chunk
    write_word(f, 1, 2);  // data waveFormat: PCM - integer samples
    write_word(f, numOutputChannels, 2);
    write_word(f, samplesPerSecond, 4);  // samples per second (Hz)
    write_word(f, samplesPerSecond * numOutputChannels * (bitsPerSample / 8), 4);  // bytes per second
    write_word(f, (bitsPerSample / 8) * numOutputChannels, 2);  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word(f, bitsPerSample, 2);  // number of bits per sample (use a multiple of 8)

    // Write the data chunk header
    size_t data_chunk_pos = f.tellp();
    f << "data----";  // (chunk size to be filled in later)

    // Write the audio samples
    for (int loop = 0; loop < loopCount; loop++)
    {
        if (numInputChannels == 1)
        {
            for (int i = 0; i < samplesLength; i++)
            {
                for (int c = 0; c < numOutputChannels; c++)
                {
                    if (c == 0 || !firstChannelOnly)
                    {
                        write_word(f, (INT16)(round(samples[i] * volume * SHRT_MAX)), 2);
                    }
                    else
                    {
                        write_word(f, (INT16)0, 2);
                    }
                }
            }
        }
        else
        {
            for (int i = 0; i < samplesLength; i++)
            {
                write_word(f, (INT16)(round(samples[i] * volume * SHRT_MAX)), 2);
            }
        }
    }

    // (We'll need the final file size to fix the chunk sizes above)
    size_t file_length = f.tellp();

    // Fix the file header to contain the proper RIFF chunk file size, which is (file size - 8) bytes
    f.seekp(0 + 4); // first four are the "RIFF" characters
    write_word(f, file_length - 8, 4);

    // Fix the data chunk header to contain the data size
    f.seekp(data_chunk_pos + (unsigned long long)4); // first four are the "data" characters
    write_word(f, file_length - (data_chunk_pos + 8), 4); // size of all the actual audio data in bytes. (Adding 8 to account for the "data----" characters)
}
