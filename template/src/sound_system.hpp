#pragma once

#include "common.hpp"
#include <SDL.h>
#include <SDL_mixer.h>

std::map<Music, Mix_Music*> m_background_music;

// a singleton implementation of the SoundSystem
class SoundSystem {

	static SoundSystem* system;

    public:
	static SoundSystem* get_system();

	// stops all the current playing sounds and destroys the current sound system
	void destroy();

	// manage current sounds and music
	void play_bgm(Music bgm);
	void play_sound_effect(Sound_Effects sound_effect, int channel = -1, int loops = 0, int fade_in_ms = 0);
	void stop_sound_effect(int channel, int fade_out_ms);

    private:
	// initialize a new sound system
	SoundSystem();

	// free all the sound and music resources. Called on destroy.
	void free_sounds();

	// maps of all the music and sounds that can be played
	std::map<Sound_Effects, Mix_Chunk*> m_sound_effects;
};
