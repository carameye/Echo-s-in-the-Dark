#include "sound_system.hpp"

SoundSystem::SoundSystem() {
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
	{
		fprintf(stderr, "Failed to open audio device\n");
		return;
	}
	// allocate more channels so can play more sounds at once
	m_channels_allocated = Mix_AllocateChannels(MIX_CHANNELS * 2);

	m_background_music[Music::standard] =  Mix_LoadMUS(audio_path("background.wav"));
	m_background_music[Music::menu] = Mix_LoadMUS(audio_path("dreams.wav"));
	m_background_music[Music::level_builder] = Mix_LoadMUS(audio_path("level_builder.wav"));
	m_background_music[Music::ghost_approach] = Mix_LoadMUS(audio_path("ghosts.wav"));
	m_sound_effects[Sound_Effects::robot_hurt] = Mix_LoadWAV(audio_path("robot_hurt.wav"));
	m_sound_effects[Sound_Effects::open_door] = Mix_LoadWAV(audio_path("open_door.wav"));
	m_sound_effects[Sound_Effects::door_locked] = Mix_LoadWAV(audio_path("locked.wav"));
	m_sound_effects[Sound_Effects::rocket] = Mix_LoadWAV(audio_path("rocket.wav"));
	m_sound_effects[Sound_Effects::collision] = Mix_LoadWAV(audio_path("metal_bang.wav"));
	m_sound_effects[Sound_Effects::button_click] = Mix_LoadWAV(audio_path("click.wav"));

	// set the volume for the music and sound effects
	Mix_VolumeMusic((int)(MIX_MAX_VOLUME / 5));
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::robot_hurt], MIX_MAX_VOLUME/2);
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::open_door], MIX_MAX_VOLUME/4);
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::door_locked], MIX_MAX_VOLUME); // locked door effect kind of quiet, so make it louder
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::rocket], MIX_MAX_VOLUME/3);
	Mix_VolumeChunk(m_sound_effects[Sound_Effects::collision], MIX_MAX_VOLUME/5);

	// check that we have correctly loaded bgm and sounds
	for (auto& bgm : m_background_music) {
		if (bgm.second == nullptr) {
			fprintf(stderr, "Failed to get game sounds\n %s\n", Mix_GetError());
			return;
		}
	}

	for (auto& effect : m_sound_effects) {
		if (effect.second == nullptr) {
			fprintf(stderr, "Failed to get game sounds\n %s\n", Mix_GetError());
			return;
		}
	}

	// Playing background music indefinitely. Initialize to be the menu music, as game starts on menu.
	Mix_FadeInMusic(m_background_music[Music::menu], -1, 1500);
};

SoundSystem* SoundSystem::get_system()
{
    static SoundSystem sound_system;

	return &sound_system;
}

void SoundSystem::free_sounds()
{
	// free sound effects
	for (auto& effect : m_sound_effects) {
		if (effect.second != nullptr) {
			Mix_FreeChunk(effect.second);
			effect.second = nullptr;
		}
	}
	m_sound_effects.clear();
	// free background music
	for (auto& bgm : m_background_music) {
		if (bgm.second != nullptr) {
			Mix_FreeMusic(bgm.second);
			bgm.second = nullptr;
		}
	}
	m_background_music.clear();

	Mix_CloseAudio();
}

void SoundSystem::play_bgm(Music bgm)
{
	Mix_FadeInMusic(m_background_music[bgm], -1, 1000);
}

void SoundSystem::play_sound_effect(Sound_Effects effect, int loops, int fade_in_ms, int channel)
{
	if (m_effect_channels.find(effect) != m_effect_channels.end()) {
		// sound effect is already being played. Stop this effect from playing, so we can play a new one
		stop_sound_effect(effect, 0);
	}

	int sound_effect_channel = Mix_FadeInChannel(channel, m_sound_effects[effect], 0, fade_in_ms);

	if (sound_effect_channel == -1) {
		fprintf(stderr, "SoundSystem play_sound_effect error %s\n", Mix_GetError());
		return;
	};

	// if successfully played sound, track it's channel.
	m_effect_channels[effect] = sound_effect_channel;
	Mix_ChannelFinished(on_effect_done);
}

void SoundSystem::stop_sound_effect(Sound_Effects sound_effect, int fade_out_ms)
{
	if (m_effect_channels.find(sound_effect) == m_effect_channels.end()) {
		// sound effect isn't being played
		return;
	}
	Mix_FadeOutChannel(m_effect_channels[sound_effect], fade_out_ms);
	Mix_ChannelFinished(on_effect_done);
}

void SoundSystem::pause_all_sound_effects()
{
	for (int channel = 0; channel < m_channels_allocated; channel++) {
		Mix_Pause(channel);
	}
}

void SoundSystem::resume_all_sound_effects() {
	for (int channel = 0; channel < m_channels_allocated; channel++) {
		Mix_Resume(channel);
	}
}

SoundSystem::EffectChannelMap* SoundSystem::get_effect_channels()
{
	return &m_effect_channels;
}

void on_effect_done(int channel)
{
	// once the sound effect stops playing, stop tracking the channel for that sound effect
	SoundSystem::EffectChannelMap* effect_channels = SoundSystem::get_system()->get_effect_channels();
	SoundSystem::EffectChannelMap::iterator itr = effect_channels->begin();
	while (itr != effect_channels->end()) {
		if (itr->second == channel) {
			effect_channels->erase(itr++);
		} else {
			++itr;
		}
	}
}
