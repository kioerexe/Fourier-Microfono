#include "micPython.h"
#pragma comment(lib, "winmm.lib")

//funione per creare buffer nell'heap
void BufferAudio::createBuffer(unsigned int sizeb)
{
	this->size = sizeb + 1;
	this->bufferAddress = (char*)malloc(this->size);
}

//delocazione del buffer
void BufferAudio::freeBuffer()
{
	free(this->bufferAddress);
}

//ritorna l'indirizzo del buffer
char* BufferAudio::getAddress()
{
	return this->bufferAddress;
}

//ritorna la dimensione del buffer
unsigned int BufferAudio::getSize()
{
	return this->size;
}
 
namespace py = pybind11;

void *reserved;
std::vector<IUnknown*> interfaccie;
IMMDeviceEnumerator* enumeratore=NULL;
IMMDevice* deviceEndpoint = NULL;
IAudioClient* clientEndpoint = NULL;
WAVEFORMATEX waveFormat;
HWAVEIN handleInput = NULL;
WAVEHDR waveHeader;

unsigned int nBit = 32;
unsigned int fCampionamento = 44100;
struct BufferAudio buffer1;


py::list enumDevicesInput()
{
	unsigned int nDeviceInput = waveInGetNumDevs();
	void* release = 0;
	py::list devicesNames;
	WAVEINCAPS capabilities;
	for (int i = 0; i < nDeviceInput; i++)
	{
		waveInGetDevCaps(i, &capabilities, sizeof(WAVEINCAPS));
		devicesNames.append(capabilities.szPname);
	}
	return devicesNames;
}

bool Inizializza()
{
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	CoInitialize(reserved);
	auto hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&enumeratore);
	enumeratore->GetDefaultAudioEndpoint(eCapture, eConsole, &deviceEndpoint);
	deviceEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&clientEndpoint);
	//clientEndpoint->GetMixFormat(&waveFormat);
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = fCampionamento;
	waveFormat.wBitsPerSample = nBit;
	waveFormat.nBlockAlign = nBit / 8;
	waveFormat.nAvgBytesPerSec = fCampionamento * nBit / 8;
	waveFormat.cbSize = 0;

	interfaccie.push_back(enumeratore);
	interfaccie.push_back(deviceEndpoint);
	interfaccie.push_back(clientEndpoint);

	if (deviceEndpoint != NULL)
		return true;
	else
		return false;
}

void Release()
{
	for (int i = 0; i < interfaccie.capacity(); i++)
	{
		interfaccie[i]->Release();
	}
}

bool OpenDevice(unsigned int idDevice)
{
	auto hr=waveInOpen(&handleInput, idDevice, &waveFormat, NULL, 0, CALLBACK_NULL);

	if (hr == S_OK) return true;
	return false;
}

py::tuple RecOnce(float t)
{
	unsigned int bufferSize = waveFormat.nChannels * t * waveFormat.nSamplesPerSec * waveFormat.wBitsPerSample / 8;
	buffer1.createBuffer(bufferSize);
	waveHeader.lpData = (LPSTR)buffer1.getAddress();
	waveHeader.dwBufferLength = buffer1.getSize();
	waveHeader.dwFlags = 0;

	waveInPrepareHeader(handleInput, &waveHeader, sizeof(WAVEHDR));
	waveInAddBuffer(handleInput, &waveHeader, sizeof(WAVEHDR));
	waveInStart(handleInput);
	while (waveHeader.dwBytesRecorded < bufferSize + 1);
	waveInReset(handleInput);
	waveInStop(handleInput);
	waveInUnprepareHeader(handleInput, &waveHeader, sizeof(WAVEHDR));

	py::list dati;
	for (int i = 0; i < bufferSize; i+=sizeof(int))
		dati.append(*((int*)(buffer1.getAddress() + i)));
	buffer1.freeBuffer();

	return py::tuple(dati);
}

bool Close()
{
	if (waveInClose(handleInput) == S_OK) return true;
	return false;
}

PYBIND11_MODULE(micPython, m)
{
	m.def("enumdevices", &enumDevicesInput);
	m.def("initialize", &Inizializza);
	m.def("release", &Release);
	m.def("opendevice", &OpenDevice, py::arg("idDevice")=0);
	m.def("rec", &RecOnce, py::return_value_policy::automatic_reference);
	m.def("close", &Close);
	m.attr("nBitCampionamento") = nBit;
	m.attr("fCampionamento") = fCampionamento;
}