#include "common.hpp"
#include "sound_player.hpp"

#include <fmod.hpp>

//////////////////////////////////////////////////////////////////////////
// constructor / destructor
//////////////////////////////////////////////////////////////////////////
SoundPlayer::SoundPlayer()
	: m_system(NULL)
	, m_sound(NULL)
	, m_channel(NULL)
{

}

SoundPlayer::~SoundPlayer()
{
	if (m_sound != NULL)
	{
		m_sound->release();
		m_sound = NULL;
	}

	if (m_system != NULL)
	{
		m_system->release();
		m_system = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// public interfaces
//////////////////////////////////////////////////////////////////////////
bool SoundPlayer::Initialize()
{
	FMOD_RESULT result;
	result = FMOD::System_Create(&m_system);
	if (result != FMOD_OK) return false;

	m_system->init(32, FMOD_INIT_NORMAL, 0);
	if (result != FMOD_OK) return false;
	return true;
}

void SoundPlayer::Update(float delta_time)
{
	m_system->update();
}

void SoundPlayer::PlaySound(const tstring& file_path, bool loop /* = false */)
{
	FMOD_RESULT result;
	FMOD_MODE mode = FMOD_SOFTWARE;
	if (loop) mode |= FMOD_LOOP_NORMAL;

	if (m_sound != NULL) m_sound->release();
	result = m_system->createStream(file_path.c_str(), mode, 0, &m_sound);
	if (result != FMOD_OK) return;

	result = m_system->playSound(FMOD_CHANNEL_FREE, m_sound, false, &m_channel);
	if (result != FMOD_OK) return;
}

void SoundPlayer::GetSpectrum(size_t num_sections, std::vector<float> &out_spectrum)
{
	std::vector<float> spectrum_l(num_sections);
	std::vector<float> spectrum_r(num_sections);

	m_system->getSpectrum(&spectrum_l[0], num_sections, 0, FMOD_DSP_FFT_WINDOW_RECT);
	m_system->getSpectrum(&spectrum_r[0], num_sections, 1, FMOD_DSP_FFT_WINDOW_RECT);

	out_spectrum.resize(num_sections);
	for (size_t i = 0; i != num_sections; ++i)
	{
		out_spectrum[i] = (spectrum_l[i] + spectrum_r[i]) * 0.5f;
	}
}

void SoundPlayer::ToggleMute()
{
	if (m_channel)
	{
		bool mute;
		m_channel->getMute(&mute);
		m_channel->setMute(!mute);
	}
}

void SoundPlayer::ChangeVolume(float delta_volume)
{
	if (m_channel)
	{
		float volume;
		m_channel->getVolume(&volume);
		volume += delta_volume;
		if (volume < 0.0f) volume = 0.0f;
		if (volume > 1.0f) volume = 1.0f;
		m_channel->setVolume(volume);
	}
}
