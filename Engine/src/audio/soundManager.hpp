#pragma once

#include "sound.hpp"
#include <SDL3/SDL.h> // Include SDL header for SDL_AudioDeviceID
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

// External reference to currentDelay (defined in main.cpp)
extern int currentDelay;

// Structure to record sound events
struct SoundEvent {
    std::string soundName;
    Uint64 timestamp;
    bool isKeyDown; // true for key down, false for key up
    float volume;   // Store volume level with each event
    int delay;      // Store the current delay setting
};

// Sound manager class
class SoundManager {
private:
    SDL_AudioDeviceID deviceId;
    std::map<std::string, std::unique_ptr<Sound>> sounds;
    std::vector<std::unique_ptr<Sound>> activeInstances;
    int instanceCounter = 0;
    
    // Key tracking for recording
    std::map<std::string, bool> keyStates; // Tracks if a key is currently pressed
    std::map<std::string, Uint64> keyPressTime; // Tracks when a key was last pressed
    std::map<std::string, int> keyPlayDuration; // Tracks how long a key's sound should play
    
    // Recording variables
    bool isRecording;
    Uint64 recordingStartTime;
    std::vector<SoundEvent> recordedEvents;
    bool isPlaying;
    Uint64 playbackStartTime;
    size_t currentEventIndex;
    
    // Volume control
    float globalVolume = 1.0f;  // Default volume level (100%)
    
    // Update playback (check for events to play)
    void updatePlayback();
    
    // Play continuous sounds for held keys
    void updateContinuousPlayback();
    
public:
    SoundManager(SDL_AudioDeviceID device);
    ~SoundManager();
    
    // Add a new sound template
    bool addSound(const std::string& name, double frequency, float gain = 0.3f, int durationMs = 1000, int fadeMs = 100);
    
    // Play a sound (always creates a new instance)
    bool playSound(const std::string& name, int durationMs = 0);
    
    // Record key up/down events
    bool recordKeyDown(const std::string& name);
    bool recordKeyUp(const std::string& name);
    
    // Clean up any finished sound instances
    void cleanupFinishedInstances();
    
    // Recording and playback control
    void startRecording();
    void stopRecording();
    void startPlayback();
    void stopPlayback();
    
    // Update all sounds
    void update();
    
    // Get number of sounds currently playing
    int getPlayingCount();
    
    // Status getters
    bool isCurrentlyRecording() const { return isRecording; }
    bool isCurrentlyPlaying() const { return isPlaying; }
    
    // Volume control
    void adjustVolume(float delta);
    float getVolume() const { return globalVolume; }
    
    // Delay control
    void setDelay(int delay); // Add a method to directly set the delay
    
    // Access to sound collections for rendering
    const std::vector<std::unique_ptr<Sound>>& getActiveInstances() const { return activeInstances; }
    const std::map<std::string, std::unique_ptr<Sound>>& getSounds() const { return sounds; }
    
    // Save recorded events to a file
    bool saveRecordingToFile(const std::string& filename);
    
    // Load recorded events from a file
    bool loadRecordingFromFile(const std::string& filename);
    
    // Add a new method to remove a sound
    bool removeSound(const std::string& name);
};