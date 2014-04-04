#include "Audio.h"
#include <stdio.h>

Audio::Audio() :
  audio_rate(22050),
  audio_format(MIX_DEFAULT_FORMAT),
  audio_channels(2),
  audio_buffers(4096)
{
}

void Audio::Initialize()
{
  //Initialize SDL_mixer with our chosen audio settings
  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) 
  {
    printf("Unable to initialize audio: %s\n", Mix_GetError());
    exit(1);
  }

  //Load our WAV file from disk
  sound = Mix_LoadWAV("Audio/sound.wav");
  music = Mix_LoadMUS("Audio/Rocketbelt.mp3");
  jetpack = Mix_LoadWAV("Audio/Blowtorch.wav");
  ding = Mix_LoadWAV("Audio/Ting.wav");

  if(sound == NULL)
    printf("Unable to load WAV file: %s\n", Mix_GetError());

  if(music == NULL)
    printf("Unable to load MP3 file: %s\n", Mix_GetError());

  if(jetpack == NULL)
    printf("Unable to load WAV file: %s\n", Mix_GetError());

  if(ding == NULL)
    printf("Unable to load WAV file: %s\n", Mix_GetError());

  soundDictionary[SFX_JETPACK] = jetpack;
  soundDictionary[SFX_RING_PASSED] = ding;
  soundDictionary[SFX_FAIL] = sound;
  musicDictionary[MUSIC_BACKGROUND] = music;
}

void Audio::PlaySoundEffect(SoundEffect effect, int loops, int channel)
{
  if(soundDictionary.find(effect) == soundDictionary.end())
  {
    return;
  }
  Mix_PlayChannel(channel, soundDictionary[effect], loops);
}

void Audio::StopSound(int channel)
{
  Mix_HaltChannel(channel);
}

void Audio::PlayMusic(Music music, int loops)
{
  if(musicDictionary.find(music) == musicDictionary.end())
  {
    return;
  }

  Mix_PlayMusic(musicDictionary[music], loops);
}
