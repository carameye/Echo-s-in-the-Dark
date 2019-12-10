#pragma once

#include "common.hpp"
#include <SDL.h>
#include <SDL_mixer.h>

#include "vector"

// a singleton implementation of the SoundSystem
class SoundSystem {

	public:
	typedef std::map<Sound_Effects, int> EffectChannelMap;

	static SoundSystem* get_system();

	// free all the sound and music resources.
	void free_sounds();

	// changes the current background music
	void play_bgm(Music bgm);

	// plays a sound effect
	// if an effect's loop is -1, adds the channel on which the effect is played to m_effect_channels
	void play_sound_effect(Sound_Effects sound_effect, int loops = 0, int fade_in_ms = 0, int channel = -1);

	// stops the sound effect from playing on all channels
	// removes the sound effect from m_effect_channels, if the sound effect is in the map
	void stop_sound_effect(Sound_Effects sound_effect, int fade_out_ms);

	// pause all sound effects, used when opening the menu during a sound effect
	void pause_all_sound_effects();

	// resume playing all sound effects from where originally paused, used when closing the menu
	// after pausing the game on a sound effect
	void resume_all_sound_effects();

	EffectChannelMap* get_effect_channels();

	// since the SoundSystem is a singleton, make sure that you cannot create copies of the SoundSystem
	// delete any implementation for the copy constructor and the copy assignment operator
	SoundSystem(const SoundSystem&) = delete;
	SoundSystem& operator=(const SoundSystem&) = delete;

    private:
	// initialize a new sound system
	SoundSystem();

	// maps of all the music and sounds that can be played
	std::map<Music, Mix_Music*> m_background_music;
	std::map<Sound_Effects, Mix_Chunk*> m_sound_effects;

	// a map of all the channels on which a sound_effect is being played
	EffectChannelMap m_effect_channels;

	int m_channels_allocated = 0;
};

void on_effect_done(int channel);
