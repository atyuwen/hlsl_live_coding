#ifndef _SOUND_PLAYER_INCLUDED_HPP_
#define _SOUND_PLAYER_INCLUDED_HPP_

#include <vector>
#include <boost/shared_ptr.hpp>

class SoundPlayer;
typedef boost::shared_ptr<SoundPlayer> SoundPlayerPtr;

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
}

class SoundPlayer
{
public:
	SoundPlayer();
	virtual ~SoundPlayer();

public:
	bool Initialize();
	void Update(float delta_time);

	void PlaySound(const tstring& file_path, bool loop = false);
	
	bool GetMute() const;
	void SetMute(bool mute);
	void ChangeVolume(float delta_volume);

	void GetSpectrum(size_t num_sections, std::vector<float> &out_spectrum);

private:
	FMOD::System* m_system;
	FMOD::Sound* m_sound;
	FMOD::Channel* m_channel;
};

#endif  // _SOUND_PLAYER_INCLUDED_HPP_