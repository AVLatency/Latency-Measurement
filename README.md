# AV Latency.com GitHub Repository

Welcome to the GitHub repository for [AV Latency.com](https://avlatency.com)! Here you will find software that can be used for measuring latency of consumer electronics [sink devices](https://avlatency.com/terminology/#sink-device) along with its source code and the research that backs it.

## About Audio & Video Latency, Input Lag, and Lip Sync Error

Visit [AV Latency.com](https://avlatency.com) if you want to learn more about audio & video latency, input lag, and lip sync error of consumer electronics.

## Research
Visit the [Wiki of this GitHub repository](https://github.com/AVLatency/Latency-Measurement/wiki) to view research results, such as reference measurements of HDMI audio extractors and audio devices.

## Compiling
The project can be built using Visual Studio with the "Desktop development with C++" workload. The latest "Universal Windows Platform development" workload may also be required to compile the [AudioGraphOutput class](https://github.com/AVLatency/Latency-Measurement/blob/main/Shared-Items/AudioGraphOutput.cpp), which uses a very new `IMemoryBufferReference::data()` function.

## License

All of the software in this repository is released under the [MIT license](https://github.com/AVLatency/Latency-Measurement/blob/main/LICENSE), which means you can do most anything you'd like with it, including commercial use, modifications, and distribution. This software uses the [Dear ImGui](https://github.com/ocornut/imgui) library for its graphical interfaces, which is also released under the [MIT license](https://github.com/ocornut/imgui/blob/7f38773b738e8d37b1a3c1d627205821300ef765/LICENSE.txt).
