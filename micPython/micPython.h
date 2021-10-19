#pragma once

#ifndef MICPYTHON_H
#define MICPYTHON_H

#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <vector>
#include <pybind11/pybind11.h>
#include <thread>

namespace py = pybind11;
void funcLista(struct BufferAudio, py::list *);
py::list enumDevicesInput();
bool Inizializza();
void Release();
bool OpenDevice(unsigned int);
py::tuple RecOnce(float);
void Rec();
bool Close();

struct BufferAudio {
private:
	unsigned int size;
	char* bufferAddress;
public:
	void createBuffer(unsigned int);
	void freeBuffer(void);
	unsigned int getSize(void);
	char* getAddress(void);
};

#endif