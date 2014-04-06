#ifndef AUDIO_H
#define AUDIO_H

#include "SDL.h"
#include "SDL_mixer.h"
#include <map>

class Audio
{
public:
  enum SoundEffect
  {
    SFX_JETPACK,
    SFX_RING_PASSED,
    SFX_FAIL
  };
  enum Music
  {
    MUSIC_BACKGROUND
  };
  Audio();
  void Initialize();
  void PlaySoundEffect(SoundEffect effect, int loops, int channel);
  void StopSound(int channel);
  void PlayMusic(Music music, int loops);
private:
  int audio_rate;      //Frequency of audio playback
  Uint16 audio_format; //Format of the audio we're playing
  int audio_channels;  //2 channels = stereo
  int audio_buffers;   //Size of the audio buffers in memory
  Mix_Chunk *sound;
  Mix_Music *music;
  Mix_Chunk *jetpack;
  Mix_Chunk *ding;
  std::map<SoundEffect, Mix_Chunk*> soundDictionary;
  std::map<Music, Mix_Music*> musicDictionary;
};

#endif