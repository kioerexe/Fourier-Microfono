#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <vector>
#include <pybind11/pybind11.h>
#pragma comment(lib, "winmm.lib")

namespace py = pybind11;
void *reserved;
std::vector<IUnknown*> interfaccie;
IMMDeviceEnumerator* enumeratore=NULL;
IMMDevice* deviceEndpoint = NULL;
IAudioClient* clientEndpoint = NULL;
WAVEFORMATEX waveFormat;

HWAVEIN handleInput = NULL;
WAVEHDR waveHeader;
DWORD flag = 0;
unsigned int bufferSize = 0;
unsigned int nBit = 32;
unsigned int fCampionamento = 44100;
char* bufferAudio=0;
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
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 32;
	waveFormat.nBlockAlign = 4;
	waveFormat.nAvgBytesPerSec = 44100 * 4;
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
	if (bufferAudio != 0) free(bufferAudio);
}

bool OpenDevice(unsigned int idDevice)
{
	bufferSize = waveFormat.nChannels * waveFormat.nSamplesPerSec * waveFormat.wBitsPerSample / 8;
	bufferAudio = (char*)malloc(bufferSize + 1);
	waveHeader.lpData = (LPSTR)bufferAudio;
	waveHeader.dwBufferLength = bufferSize + 1;
	waveHeader.dwFlags = 0;

	waveInOpen(&handleInput, idDevice, &waveFormat, NULL, 0, CALLBACK_NULL);
	waveInPrepareHeader(handleInput, &waveHeader, sizeof(WAVEHDR));
	auto hr=waveInAddBuffer(handleInput, &waveHeader, sizeof(WAVEHDR));
	hr=waveInUnprepareHeader(handleInput, &waveHeader, sizeof(WAVEHDR));

	if (hr == S_OK) return true;
	return false;
}

py::tuple Rec()
{
	waveInPrepareHeader(handleInput, &waveHeader, sizeof(WAVEHDR));
	waveInStart(handleInput);
	while (waveHeader.dwBytesRecorded < bufferSize + 1);
	waveInReset(handleInput);
	waveInStop(handleInput);
	waveInUnprepareHeader(handleInput, &waveHeader, sizeof(WAVEHDR));

	py::list dati;
	for (int i = 0; i < bufferSize; i+=sizeof(int))
		dati.append(*((int*)(bufferAudio + i)));

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
	m.def("rec", &Rec, py::return_value_policy::automatic_reference);
	m.def("close", &Close);
	m.attr("nBitCampionamento") = nBit;
	m.attr("fCampionamento") = fCampionamento;
}