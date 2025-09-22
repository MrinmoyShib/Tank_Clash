#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "Constants.hpp"
#include <windows.h>

// -------- Audio Management Functions --------
void initializeAudio();
void playBackgroundMusic();
void pauseBackgroundMusic();
void playDestroySound();
void playFireSound();
void playHitSound();
void stopAllAudio();

#endif // AUDIO_HPP