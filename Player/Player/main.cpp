#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include "windows.h"
#include "mmsystem.h"
#include "mmreg.h"

// some song information
#include "Synth.h"
#define INCLUDE_NODES
#include "64k2Patch.h"
#include "64k2Song.h"

#define SAMPLE_RATE 44100
#define SAMPLE_TYPE float
SAMPLE_TYPE lpSoundBuffer[MAX_SAMPLES*2 + 44100*60]; // add safety buffer for 60s 
HWAVEOUT hWaveOut;

/////////////////////////////////////////////////////////////////////////////////
// initialized data
/////////////////////////////////////////////////////////////////////////////////

WAVEFORMATEX WaveFMT =
{
	WAVE_FORMAT_IEEE_FLOAT,
	2, // channels
	SAMPLE_RATE, // samples per sec
	SAMPLE_RATE*sizeof(SAMPLE_TYPE)*2, // bytes per sec
	sizeof(SAMPLE_TYPE)*2, // block alignment;
	sizeof(SAMPLE_TYPE)*8, // bits per sample
	0 // extension not needed
};

WAVEHDR WaveHDR = 
{
	(LPSTR)lpSoundBuffer, 
	MAX_SAMPLES*sizeof(SAMPLE_TYPE)*2,			// MAX_SAMPLES*sizeof(float)*2(stereo)
	0, 
	0, 
	0, 
	0, 
	0, 
	0
};

MMTIME MMTime = 
{ 
	TIME_SAMPLES,
	0
};

/////////////////////////////////////////////////////////////////////////////////
// crt emulation
/////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
	int _fltused = 1;
}

/////////////////////////////////////////////////////////////////////////////////
// entry point for the executable if msvcrt is not used
/////////////////////////////////////////////////////////////////////////////////

#if defined _DEBUG
void main(void)
#else
void mainCRTStartup(void)
#endif
{
	// init synth and start filling the buffer 
	_64klang_Init(SynthStream, SynthNodes, SynthMonoConstantOffset, SynthStereoConstantOffset, SynthMaxOffset);
#if 1
	// threaded buffer fill with 5 seconds heads up
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_64klang_Render, lpSoundBuffer, 0, 0);
	Sleep(5000);
#else
	// sequential (blocking) buffer fill aka precalc
	_64klang_Render(lpSoundBuffer);
#endif	

	// start audio playback
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL);
	waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	do
	{
		waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
		Sleep(128);
	} while ((MMTime.u.sample < MAX_SAMPLES) && !GetAsyncKeyState(VK_ESCAPE));

	// done
	ExitProcess(0);
}
