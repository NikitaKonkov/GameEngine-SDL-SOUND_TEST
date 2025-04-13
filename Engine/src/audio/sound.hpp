#pragma once

#include <SDL3/SDL.h>
#include <cmath>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Sound structure to manage individual sounds
class Sound {
private:
    SDL_AudioStream* stream;
    double frequency;
    float gain;
    bool playing;
    Uint64 startTime;
    int durationMs;
    int fadeMs; // Fade out time in milliseconds
    
    // Apply amplitude envelope to the sample (fadeIn/fadeOut)
    float applyEnvelope(int sampleIndex, int totalSamples, float value);
    
    // Fill the stream with sine wave data with smooth envelope
    void generateSineWave(int durationMs);
    
public:
    Sound(SDL_AudioDeviceID deviceId, double freq, float gain = 0.3f, int durMs = 0, int fadeoutMs = 100);
    ~Sound();
    
    // Start playing the sound
    void play(int durationMs = 0);
    
    // Update playing state
    void update();
    
    // Getters for sound properties
    double getFrequency() const { return frequency; }
    float getGain() const { return gain; }
    bool isPlaying() const { return playing; }
    int getDuration() const { return durationMs; }
    int getFadeTime() const { return fadeMs; }
    
    // Friend class for SoundManager to access private members
    friend class SoundManager;
};