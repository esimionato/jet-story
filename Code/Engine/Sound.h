#ifndef __SOUND_H
#define __SOUND_H

#pragma once

#define _USING_V110_SDK71_ 1
#include <string>
#include "fmod.hpp"


// This is not exposed to Game
namespace Audio
{
	extern FMOD_SYSTEM* g_pFMOD_system;

	bool Init();
	void ShutDown();

	void Update();
}	// namespace Audio


class Sound
{
public:
	Sound(const std::string& fileName);
	~Sound();

	bool IsOK() const { return m_pFMOD_sound != nullptr; }

	bool IsPlaying() const;
	bool Play(int nLoopCount);
	void Stop();

	void Pause();
	void Resume();

private:
	FMOD_SOUND*		m_pFMOD_sound;
	FMOD_CHANNEL*	m_pFMOD_channel;
};


#endif	// #ifndef __SOUND_H
