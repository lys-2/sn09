#include <Windows.h>

#include <stdio.h>

#include <mmsystem.h>
#include "sn.h"
#pragma comment(lib, "winmm.lib")

void PrintMidiDevices()
{
	UINT nMidiDeviceNum;
	MIDIINCAPS incaps;
	MIDIOUTCAPS outcaps;

	nMidiDeviceNum = midiInGetNumDevs();
	if (nMidiDeviceNum == 0) {
		fprintf(stderr, "midiInGetNumDevs() return 0...");
		return;
	}

	printf("MIDI input devices:\n");
	for (unsigned int i = 0; i < midiInGetNumDevs(); i++) {
		midiInGetDevCaps(i, &incaps, sizeof(MIDIINCAPS));
	printf(L"    %d : name = %s\n", i, incaps.szPname);
	}

	printf("MIDI output devices:\n");
	for (unsigned int i = 0; i < midiOutGetNumDevs(); i++) {
		midiOutGetDevCaps(i, &outcaps, sizeof(MIDIINCAPS));
	printf(L"    %d : name = %s\n", i, outcaps.szPname);
	}

	printf("\n");
}

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	unsigned char status = LOBYTE(dwParam1);
	unsigned char note = HIBYTE(dwParam1);
	unsigned char velocity = LOBYTE(HIWORD(dwParam1));
	switch (wMsg) {
	case MIM_OPEN:
		printf("wMsg=MIM_OPEN\n");
		break;
	case MIM_CLOSE:
		printf("wMsg=MIM_CLOSE\n");
		break;
	case MIM_DATA:
		if ( status==144 || status == 132 || status == 148) play(note);
		printf("%i, %i, %i\n", status, note, velocity);
		break;
	case MIM_LONGDATA:
		printf("wMsg=MIM_LONGDATA\n");
		break;
	case MIM_ERROR:
		printf("wMsg=MIM_ERROR\n");
		break;
	case MIM_LONGERROR:
		printf("wMsg=MIM_LONGERROR\n");
		break;
	case MIM_MOREDATA:
		printf("wMsg=MIM_MOREDATA\n");
		break;
	default:
		printf("wMsg = unknown\n");
		break;
	}
	return;
}

int wmidi(int argc, char* argv[])
{
	HMIDIIN hMidiDevice = NULL;;
	DWORD nMidiPort = 0;
	UINT nMidiDeviceNum;
	MMRESULT rv;

	PrintMidiDevices();

	nMidiDeviceNum = midiInGetNumDevs();
	if (nMidiDeviceNum == 0) {
		fprintf(stderr, "midiInGetNumDevs() return 0...");

	}

	rv = midiInOpen(&hMidiDevice, nMidiPort, (DWORD_PTR)(void*)MidiInProc, 0, CALLBACK_FUNCTION);
	if (rv != MMSYSERR_NOERROR) {
		fprintf(stderr, "midiInOpen() failed...rv=%d", rv);

	}

	midiInStart(hMidiDevice);


/*	midiInStop(hMidiDevice);
	midiInClose(hMidiDevice);
	hMidiDevice = NULL;*/

	return 0;
}