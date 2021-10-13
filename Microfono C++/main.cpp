#include <iostream>
#include <fstream>
#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>

#pragma comment(lib, "winmm.lib")

using namespace std;
int main(int argc, char* argv[])
{
	unsigned int nDeviceInput = waveInGetNumDevs();
	void* release = 0;
	cout << "numero di dispositivi input: " << nDeviceInput << endl;

	WAVEINCAPS capabilities;
	WAVEFORMATEX waveFormat;
	waveInGetDevCaps(0, &capabilities, sizeof(WAVEINCAPS));
	wcout << capabilities.szPname << endl;

	//prendo waveFormat
	IMMDeviceEnumerator* enumeratore=NULL;
	IMMDevice* deviceEndpoint = NULL;
	IAudioClient* clientEndpoint = NULL;
	
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	CoInitialize(release);
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

	clientEndpoint->Release();
	deviceEndpoint->Release();
	enumeratore->Release();
	
	//apertura dispositivo
	HWAVEIN handleInput=NULL;
	WAVEHDR waveHeader;
	DWORD flag = 0;

	const unsigned int bufferSize = waveFormat.nChannels * waveFormat.nSamplesPerSec * waveFormat.wBitsPerSample / 8;
	char* bufferAudio = (char*)malloc(bufferSize + 1);
	waveHeader.lpData = (LPSTR)bufferAudio;
	waveHeader.dwBufferLength = bufferSize + 1;
	waveHeader.dwFlags = 0;

	hr=waveInOpen(&handleInput, 0, &waveFormat, NULL, 0, CALLBACK_NULL);

	hr = waveInPrepareHeader(handleInput, &waveHeader, sizeof(WAVEHDR));
	hr=waveInAddBuffer(handleInput, &waveHeader, sizeof(WAVEHDR));
	cout << "Registrazione..." << endl;
	hr=waveInStart(handleInput);
	while (waveHeader.dwBytesRecorded < bufferSize + 1);
	waveInReset(handleInput);
	hr = waveInStop(handleInput);
	hr = waveInUnprepareHeader(handleInput, &waveHeader, sizeof(WAVEHDR));
	hr=waveInClose(handleInput);

	ofstream fileAudio;
	fileAudio.open(".\\audio", ios::out | ios::binary);

	for (int i = 0; i < bufferSize; i+=sizeof(int))
	{
		fileAudio << *((int*)(bufferAudio+i));
	}
	fileAudio.close();

	free(bufferAudio);
	return 0;
}