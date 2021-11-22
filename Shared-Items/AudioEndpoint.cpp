#include "AudioEndpoint.h"



#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

AudioEndpoint::AudioEndpoint(IMMDevice * device, std::string name, std::string id) : Device(device), Name(name), ID(id)
{
    device->AddRef();
}

AudioEndpoint::AudioEndpoint(const AudioEndpoint & other)
{
    Device = other.Device;
    Device->AddRef();
    Name = other.Name;
    ID = other.ID;
}


AudioEndpoint::AudioEndpoint(const AudioEndpoint&& other)
{
    Device = other.Device;
    Device->AddRef();
    Name = other.Name;
    ID = other.ID;
}


AudioEndpoint::~AudioEndpoint()
{
	SAFE_RELEASE(Device);
}
