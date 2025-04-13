#include <app/app.hpp>
#include <settings/settings.hpp>
#include <iostream>
#include <SDL3/SDL.h>
#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <memory>

#define M_PI 3.14159265358979323846

// Window dimensions
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "SDL Sound Test - Async Audio Demo"

// Structure to record sound events
struct SoundEvent {
    std::string soundName;
    Uint64 timestamp;
};

// Sound structure to manage individual sounds
struct Sound {
    SDL_AudioStream* stream;
    double frequency;
    float gain;
    bool playing;
    Uint64 startTime;
    int durationMs;
    int fadeMs; // Fade out time in milliseconds
    
    Sound(SDL_AudioDeviceID deviceId, double freq, float g = 0.3f, int durMs = 0, int fadeoutMs = 100) 
        : frequency(freq), gain(g), playing(false), startTime(0), durationMs(durMs), fadeMs(fadeoutMs) {
        
        // Set up audio spec for this sound
        SDL_AudioSpec spec;
        SDL_zero(spec);
        spec.format = SDL_AUDIO_F32;
        spec.channels = 1;
        spec.freq = 48000;
        
        // Create the audio stream
        stream = SDL_CreateAudioStream(&spec, &spec);
        
        if (stream) {
            // Set the stream's gain
            SDL_SetAudioStreamGain(stream, gain);
            
            // Bind the stream to the device
            SDL_BindAudioStream(deviceId, stream);
        }
    }
    
    ~Sound() {
        if (stream) {
            SDL_UnbindAudioStream(stream);
            SDL_DestroyAudioStream(stream);
        }
    }
    
    // Apply amplitude envelope to the sample (fadeIn/fadeOut)
    float applyEnvelope(int sampleIndex, int totalSamples, float value) {
        const int fadeInSamples = 480; // 10ms at 48kHz
        const int fadeOutSamples = (fadeMs * 48000) / 1000; // Convert fadeMs to samples
        
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
    
    // Fill the stream with sine wave data with smooth envelope
    void generateSineWave(int durationMs) {
        if (!stream || !durationMs) return;
        
        const int sample_rate = 48000;  // CD quality
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
            buffer[i] = applyEnvelope(i, numSamples, 0.3f * sampleValue);
            
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
    
    // Start playing the sound
    void play(int durationMs = 0) {
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
    
    // Update playing state
    void update() {
        if (playing && durationMs > 0) {
            Uint64 currentTime = SDL_GetTicks();
            if (currentTime - startTime >= durationMs) {
                playing = false;
            }
        }
    }
};

// Sound manager class
class SoundManager {
private:
    SDL_AudioDeviceID deviceId;
    std::map<std::string, std::unique_ptr<Sound>> sounds;
    
    // Recording variables
    bool isRecording;
    Uint64 recordingStartTime;
    std::vector<SoundEvent> recordedEvents;
    bool isPlaying;
    Uint64 playbackStartTime;
    size_t currentEventIndex;
    
public:
    SoundManager(SDL_AudioDeviceID device) 
        : deviceId(device), isRecording(false), recordingStartTime(0), 
          isPlaying(false), playbackStartTime(0), currentEventIndex(0) {}
    
    ~SoundManager() {
        // The unique_ptrs will clean up the Sound objects
        sounds.clear();
    }
    
    // Add a new sound
    bool addSound(const std::string& name, double frequency, float gain = 0.3f, int durationMs = 1000, int fadeMs = 100) {
        if (sounds.find(name) != sounds.end()) {
            // Sound already exists
            return false;
        }
        
        sounds[name] = std::make_unique<Sound>(deviceId, frequency, gain, durationMs, fadeMs);
        return true;
    }
    
    // Play a sound
    bool playSound(const std::string& name, int durationMs = 0) {
        auto it = sounds.find(name);
        if (it == sounds.end()) {
            return false;
        }
        
        it->second->play(durationMs);
        
        // If recording, add this event
        if (isRecording) {
            SoundEvent event;
            event.soundName = name;
            event.timestamp = SDL_GetTicks() - recordingStartTime;
            recordedEvents.push_back(event);
            SDL_Log("Recorded event: %s at %llu ms", name.c_str(), event.timestamp);
        }
        
        return true;
    }
    
    // Start recording
    void startRecording() {
        if (!isRecording) {
            recordedEvents.clear();
            recordingStartTime = SDL_GetTicks();
            isRecording = true;
            SDL_Log("Recording started");
        }
    }
    
    // Stop recording
    void stopRecording() {
        if (isRecording) {
            isRecording = false;
            SDL_Log("Recording stopped - %zu events recorded", recordedEvents.size());
        }
    }
    
    // Start playback of recorded events
    void startPlayback() {
        if (!recordedEvents.empty() && !isPlaying) {
            isPlaying = true;
            playbackStartTime = SDL_GetTicks();
            currentEventIndex = 0;
            SDL_Log("Playback started - %zu events to play", recordedEvents.size());
        } else if (recordedEvents.empty()) {
            SDL_Log("No recorded events to play");
        }
    }
    
    // Stop playback
    void stopPlayback() {
        if (isPlaying) {
            isPlaying = false;
            SDL_Log("Playback stopped");
        }
    }
    
    // Update playback (check for events to play)
    void updatePlayback() {
        if (isPlaying && currentEventIndex < recordedEvents.size()) {
            Uint64 currentTime = SDL_GetTicks() - playbackStartTime;
            
            // Play all events due at this time
            while (currentEventIndex < recordedEvents.size() && 
                   recordedEvents[currentEventIndex].timestamp <= currentTime) {
                const SoundEvent& event = recordedEvents[currentEventIndex];
                playSound(event.soundName);
                SDL_Log("Playing recorded event: %s at %llu ms", event.soundName.c_str(), currentTime);
                currentEventIndex++;
            }
            
            // Check if we've reached the end
            if (currentEventIndex >= recordedEvents.size()) {
                SDL_Log("Playback completed");
                // Keep isPlaying true to prevent repeating
            }
        }
    }
    
    // Update all sounds
    void update() {
        for (auto& pair : sounds) {
            pair.second->update();
        }
        
        // Update playback if needed
        if (isPlaying) {
            updatePlayback();
        }
    }
    
    // Get number of sounds currently playing
    int getPlayingCount() {
        int count = 0;
        for (const auto& pair : sounds) {
            if (pair.second->playing) {
                count++;
            }
        }
        return count;
    }
    
    // Is recording in progress?
    bool isCurrentlyRecording() const {
        return isRecording;
    }
    
    // Is playback in progress?
    bool isCurrentlyPlaying() const {
        return isPlaying;
    }
    
    // Give access to the sounds for rendering
    const std::map<std::string, std::unique_ptr<Sound>>& getSounds() const {
        return sounds;
    }
};

// Function to render visualization of the playing sounds
void RenderPlayingSounds(SDL_Renderer *renderer, SoundManager& soundManager, const std::map<std::string, std::unique_ptr<Sound>>& sounds) {
    const int barWidth = WINDOW_WIDTH / (sounds.size() > 0 ? sounds.size() : 1);
    int i = 0;
    
    for (const auto& pair : sounds) {
        const Sound& sound = *pair.second;
        
        int barHeight = 0;
        SDL_Color color = {0, 0, 0, 255}; // Black for not playing
        
        if (sound.playing) {
            // Calculate height based on frequency
            barHeight = static_cast<int>(sound.frequency / 5.0); // Scale for visualization
            if (barHeight > WINDOW_HEIGHT - 100)
                barHeight = WINDOW_HEIGHT - 100;
            
            // Set color based on frequency (low frequencies are blue, high are red)
            float normalizedFreq = sound.frequency / 1000.0f;
            if (normalizedFreq > 1.0f) normalizedFreq = 1.0f;
            
            color.r = static_cast<Uint8>(normalizedFreq * 255);
            color.g = static_cast<Uint8>((1.0f - normalizedFreq) * 128);
            color.b = static_cast<Uint8>((1.0f - normalizedFreq) * 255);
            color.a = 255;
        }
        
        // Draw the bar
        SDL_FRect bar = {
            static_cast<float>(i * barWidth),
            WINDOW_HEIGHT - barHeight - 50.0f,
            static_cast<float>(barWidth - 5),
            static_cast<float>(barHeight)
        };
        
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &bar);
        
        i++;
    }
    
    // Display recording/playback status
    const int boxHeight = 30;
    const int boxWidth = 200;
    
    SDL_FRect statusBox = {
        10.0f, 10.0f,
        static_cast<float>(boxWidth), static_cast<float>(boxHeight)
    };
    
    if (soundManager.isCurrentlyRecording()) {
        // Red for recording
        SDL_SetRenderDrawColor(renderer, 255, 40, 40, 200);
    } else if (soundManager.isCurrentlyPlaying()) {
        // Green for playback
        SDL_SetRenderDrawColor(renderer, 40, 255, 40, 200);
    } else {
        // Black for idle
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    }
    
    SDL_RenderFillRect(renderer, &statusBox);
}

int main(int argc, char* argv[]) {
    // Initialize SDL with both video and audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return -1;
    }
    
    SDL_Log("Vulkan SDL Game Engine started successfully.");
    
    // Create window
    SDL_Window *window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    
    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Audio spec
    SDL_AudioSpec audioSpec;
    SDL_zero(audioSpec);
    audioSpec.format = SDL_AUDIO_F32;
    audioSpec.channels = 2;      // stereo
    audioSpec.freq = 48000;      // 48KHz
    
    // Open the audio device
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec);
    if (!audioDevice) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open audio device: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Define a sequence of frequencies for musical notes
    std::vector<double> frequencies = {
        247.48, // C3
        261.63, // C4
        293.66, // D4
        329.63, // E4
        349.23, // F4
        392.00, // G4
        440.00, // A4
        493.88, // B4
        523.25, // C5
        554.37, // C#5
    };
    
    // Create the sound manager
    SoundManager soundManager(audioDevice);
    
    // Add sounds for each note with 100ms fadeout
    for (size_t i = 0; i < frequencies.size(); i++) {
        std::string noteName = "note" + std::to_string(i);
        soundManager.addSound(noteName, frequencies[i], 0.3f, 2000, 100); // 2 second duration, 100ms fadeout
    }
    
    // Create a chord sound with 200ms fadeout for smoother chord endings
    soundManager.addSound("chord1", 261.63, 0.2f, 3000, 200); // C4 for 3 seconds, 200ms fadeout
    soundManager.addSound("chord2", 329.63, 0.2f, 3000, 200); // E4 for 3 seconds, 200ms fadeout 
    soundManager.addSound("chord3", 392.00, 0.2f, 3000, 200); // G4 for 3 seconds, 200ms fadeout
    
    // Main loop flag
    bool quit = false;
    
    // Event handler
    SDL_Event e;
    
    // Display instructions
    SDL_Log("Press keys 1-8 to play individual notes");
    SDL_Log("Press C to play a C major chord");
    SDL_Log("Press S to start/stop recording");
    SDL_Log("Press D to play back recorded music");
    SDL_Log("Press A to quit");
    
    // While application is running
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e)) {
            // User requests quit
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            // User presses a key
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                switch (e.key.key) {
                    case SDLK_A:
                        SDL_Log("'A' key pressed. Exiting...");
                        quit = true;
                        break;
                        
                    case SDLK_S:
                        // Toggle recording
                        if (soundManager.isCurrentlyRecording()) {
                            soundManager.stopRecording();
                        } else {
                            soundManager.startRecording();
                        }
                        break;
                        
                    case SDLK_D:
                        // Start playback of recorded music
                        if (!soundManager.isCurrentlyPlaying()) {
                            soundManager.startPlayback();
                        } else {
                            soundManager.stopPlayback();
                        }
                        break;
                    case SDLK_0:
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                    case SDLK_5:
                    case SDLK_6:
                    case SDLK_7:
                    case SDLK_8:
                    case SDLK_9: {
                        int noteIndex = e.key.key - SDLK_0;
                        std::string noteName = "note" + std::to_string(noteIndex);
                        soundManager.playSound(noteName);
                        SDL_Log("Playing note %d (%.2f Hz)", noteIndex + 1, frequencies[noteIndex]);
                        break;
                    }
                    case SDLK_C:
                        // Play a C major chord (C, E, G together)
                        soundManager.playSound("chord1");
                        soundManager.playSound("chord2");
                        soundManager.playSound("chord3");
                        SDL_Log("Playing C major chord");
                        break;
                }
            }
        }
        
        // Update sound states
        soundManager.update();
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        
        // Render the visualization using properly accessed sounds
        RenderPlayingSounds(renderer, soundManager, soundManager.getSounds());
        
        // Update screen
        SDL_RenderPresent(renderer);
        
        // Small delay to reduce CPU usage
        SDL_Delay(10);
    }
    
    // Clean up
    SDL_CloseAudioDevice(audioDevice);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}