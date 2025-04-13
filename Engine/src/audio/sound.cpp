#include "sound.hpp"
#include "config.hpp"

Sound::Sound(SDL_AudioDeviceID deviceId, double freq, float g, int durMs, int fadeoutMs) 
    : frequency(freq), gain(g), playing(false), startTime(0), durationMs(durMs), fadeMs(fadeoutMs) {
    
    // Set up audio spec for this sound
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;
    spec.freq = AUDIO_SAMPLE_RATE;
    
    // Create the audio stream
    stream = SDL_CreateAudioStream(&spec, &spec);
    
    if (stream) {
        // Set the stream's gain
        SDL_SetAudioStreamGain(stream, gain);
        
        // Bind the stream to the device
        SDL_BindAudioStream(deviceId, stream);
    }
}

Sound::~Sound() {
    if (stream) {
        SDL_UnbindAudioStream(stream);
        SDL_DestroyAudioStream(stream);
    }
}

float Sound::applyEnvelope(int sampleIndex, int totalSamples, float value) {
    const int fadeInSamples = 480; // 10ms at 48kHz
    const int fadeOutSamples = (fadeMs * AUDIO_SAMPLE_RATE) / 1000; // Convert fadeMs to samples
    
    // Apply fade-in (first 10ms)
    if (sampleIndex < fadeInSamples) {
        float fadeInFactor = static_cast<float>(sampleIndex) / fadeInSamples;
        value *= fadeInFactor;
    }
    
    // Apply fade-out (last fadeOutSamples)
    if (sampleIndex > totalSamples - fadeOutSamples) {
        float fadeOutFactor = static_cast<float>(totalSamples - sampleIndex) / fadeOutSamples;
        value *= fadeOutFactor;
    }
    
    return value;
}

void Sound::generateSineWave(int durationMs) {
    if (!stream || !durationMs) return;
    
    const int sample_rate = AUDIO_SAMPLE_RATE;
    const double phase_increment = 2.0 * M_PI * frequency / sample_rate;
    const int numSamples = (sample_rate * durationMs) / 1000;
    const int bufferSize = numSamples * sizeof(float);
    
    float* buffer = static_cast<float*>(SDL_malloc(bufferSize));
    if (!buffer) return;
    
    // Generate sine wave with envelope
    double phase = 0.0;
    for (int i = 0; i < numSamples; i++) {
        // Apply amplitude envelope to avoid clicks
        float sampleValue = static_cast<float>(sin(phase));
        buffer[i] = applyEnvelope(i, numSamples, gain * sampleValue);
        
        phase += phase_increment;
        if (phase > 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }
    
    // Add data to the stream
    SDL_PutAudioStreamData(stream, buffer, bufferSize);
    
    // Free the buffer
    SDL_free(buffer);
}

void Sound::play(int durationMs) {
    if (!stream) return;
    
    // Clear any previous audio data
    SDL_ClearAudioStream(stream);
    
    // If duration is provided, use it; otherwise use the default
    int soundDuration = durationMs > 0 ? durationMs : this->durationMs;
    if (soundDuration <= 0) soundDuration = 1000; // Default to 1 second
    
    // Generate the sine wave data
    generateSineWave(soundDuration);
    
    // Mark as playing and record start time
    playing = true;
    startTime = SDL_GetTicks();
    this->durationMs = soundDuration;
}

void Sound::update() {
    if (playing && durationMs > 0) {
        Uint64 currentTime = SDL_GetTicks();
        if (currentTime - startTime >= durationMs) {
            playing = false;
        }
    }
}