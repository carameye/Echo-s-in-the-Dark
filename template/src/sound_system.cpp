#include "sound_system.hpp"

SoundSystem::SoundSystem() {
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device\n");
		return;
	}

	m_background_music[Music::standard] =  Mix_LoadMUS(audio_path("background.wav"));
	m_background_music[Music::menu] = Mix_LoadMUS(audio_path("dreams.wav"));
	m_background_music[Music::level_builder] = Mix_LoadMUS(audio_path("Lonelyhood.ogg"));
	m_background_music[Music::ghost_approach] = Mix_LoadMUS(audio_path("ghosts.wav"));
	m_sound_effects[Sound_Effects::robot_hurt] = Mix_LoadWAV(audio_path("robot_hurt.wav"));
	m_sound_effects[Sound_Effects::open_door] = Mix_LoadWAV(audio_path("open_door.wav"));
	m_sound_effects[Sound_Effects::door_locked] = Mix_LoadWAV(audio_path("locked.wav"));
	m_sound_effects[Sound_Effects::rocket] = Mix_LoadWAV(audio_path("rocket.wav"));
	m_sound_effects[Sound_Effects::collision] = Mix_LoadWAV(audio_path("impactMining_000.ogg"));

	// set the volume for the music and sound effects
	Mix_VolumeMusic((int)(MIX_MAX_VOLUME / 5));
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::robot_hurt], MIX_MAX_VOLUME/2);
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::open_door], MIX_MAX_VOLUME/4);
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::door_locked], MIX_MAX_VOLUME); // locked door effect kind of quiet, so make it louder
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::rocket], MIX_MAX_VOLUME/3);
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::collision], MIX_MAX_VOLUME/4);

	// check that we have correctly loaded bgm and sounds
	for (auto& bgm : m_background_music) {
		if (bgm.second == nullptr) {
			fprintf(stderr, "Failed to game sounds\n %s\n", Mix_GetError());
			return;
		}
	}

	for (auto& effect : m_sound_effects) {
		if (effect.second == nullptr) {
			fprintf(stderr, "Failed to game sounds\n %s\n", Mix_GetError());
			return;
		}
	}

	// Playing background music indefinitely. Initialize to be the menu music, as game starts on menu.
	Mix_FadeInMusic(m_background_music[Music::menu], -1, 1500);
};

SoundSystem* SoundSystem::get_system()
{
    if (system == nullptr) {
        system = new SoundSystem();
    }
    return system;
}

void SoundSystem::destroy()
{
	free_sounds();
	delete system;
	system = nullptr;
}

void SoundSystem::free_sounds()
{
	// free sound effects
	for (auto& effect : m_sound_effects) {
		if (effect.second != nullptr) {
			Mix_FreeChunk(effect.second);
			effect.second = nullptr;
			m_sound_effects.erase(effect.first);
		}
	}
	// free background music
	for (auto& bgm : m_background_music) {
		if (bgm.second != nullptr) {
			Mix_FreeMusic(bgm.second);
			bgm.second = nullptr;
			m_background_music.erase(bgm.first);
		}
	}

	Mix_CloseAudio();
}

void SoundSystem::play_bgm(Music bgm)
{
	Mix_FadeInMusic(m_background_music[bgm], -1, 1500);
}

void SoundSystem::play_sound_effect(Sound_Effects effect, int channel, int loops, int fade_in_ms)
{
	Mix_FadeInChannel(channel, m_sound_effects[effect], 0, fade_in_ms);
}

void SoundSystem::stop_sound_effect(int channel, int fade_out_ms)
{
	Mix_FadeOutChannel(channel, fade_out_ms);
}
