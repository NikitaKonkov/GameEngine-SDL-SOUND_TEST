#include "soundManager.hpp"
#include "config.hpp"
#include <SDL3/SDL.h>

SoundManager::SoundManager(SDL_AudioDeviceID device)
    : deviceId(device), isRecording(false), recordingStartTime(0), 
      isPlaying(false), playbackStartTime(0), currentEventIndex(0), globalVolume(1.0f) {}

SoundManager::~SoundManager() {
    // The unique_ptrs will clean up the Sound objects
    sounds.clear();
    activeInstances.clear();
}

bool SoundManager::addSound(const std::string& name, double frequency, float gain, int durationMs, int fadeMs) {
    if (sounds.find(name) != sounds.end()) {
        // Sound already exists
        return false;
    }
    
    sounds[name] = std::make_unique<Sound>(deviceId, frequency, gain, durationMs, fadeMs);
    return true;
}

bool SoundManager::playSound(const std::string& name, int durationMs) {
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        return false;
    }
    
    // Create a new instance of this sound
    std::string instanceName = name + "_instance" + std::to_string(instanceCounter++);
    Sound* templateSound = it->second.get();
    
    // Create a new sound instance with the same parameters but apply volume
    auto newInstance = std::make_unique<Sound>(deviceId, templateSound->frequency, 
                                           templateSound->gain * globalVolume, // Apply volume multiplier 
                                           templateSound->durationMs, 
                                           templateSound->fadeMs);
    
    // Play the new instance
    newInstance->play(durationMs);
    
    // Add to active instances
    activeInstances.push_back(std::move(newInstance));
    
    // Clean up finished instances periodically
    if (activeInstances.size() > 20) {
        cleanupFinishedInstances();
    }
    
    return true;
}

bool SoundManager::removeSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        // Sound doesn't exist
        return false;
    }
    
    // Remove the sound
    sounds.erase(it);
    return true;
}

bool SoundManager::recordKeyDown(const std::string& name) {
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        return false;
    }
    
    // Get template sound duration
    int duration = it->second->getDuration();
    
    // Update key state and store play duration
    keyStates[name] = true;
    keyPressTime[name] = SDL_GetTicks();
    keyPlayDuration[name] = duration;
    
    // Play the sound immediately
    playSound(name);
    
    // If recording, add this event
    if (isRecording) {
        SoundEvent event;
        event.soundName = name;
        event.timestamp = SDL_GetTicks() - recordingStartTime;
        event.isKeyDown = true;
        event.volume = globalVolume; // Store current volume level
        event.delay = currentDelay;  // Store the actual current delay value
        recordedEvents.push_back(event);
        SDL_Log("Recorded key down: %s at %llu ms (volume: %.2f, delay: %d ms)", 
                name.c_str(), event.timestamp, globalVolume, currentDelay);
    }
    
    return true;
}

bool SoundManager::recordKeyUp(const std::string& name) {
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        return false;
    }
    
    // Update key state
    keyStates[name] = false;
    
    // If recording, add this event
    if (isRecording) {
        SoundEvent event;
        event.soundName = name;
        event.timestamp = SDL_GetTicks() - recordingStartTime;
        event.isKeyDown = false;
        event.volume = globalVolume; // Store current volume level
        event.delay = currentDelay;  // Store the actual current delay value
        recordedEvents.push_back(event);
        SDL_Log("Recorded key up: %s at %llu ms (volume: %.2f, delay: %d ms)", 
                name.c_str(), event.timestamp, globalVolume, currentDelay);
    }
    
    return true;
}

void SoundManager::cleanupFinishedInstances() {
    // Remove finished instances
    auto it = activeInstances.begin();
    while (it != activeInstances.end()) {
        if (!(*it)->playing) {
            it = activeInstances.erase(it);
        } else {
            ++it;
        }
    }
}

void SoundManager::startRecording() {
    if (!isRecording) {
        recordedEvents.clear();
        keyStates.clear(); // Reset key states
        recordingStartTime = SDL_GetTicks();
        isRecording = true;
        SDL_Log("Recording started");
    }
}

void SoundManager::stopRecording() {
    if (isRecording) {
        isRecording = false;
        
        // Release all keys at the end of recording
        for (const auto& keyState : keyStates) {
            if (keyState.second) { // If key is down
                SoundEvent event;
                event.soundName = keyState.first;
                event.timestamp = SDL_GetTicks() - recordingStartTime;
                event.isKeyDown = false;
                recordedEvents.push_back(event);
            }
        }
        
        keyStates.clear(); // Reset key states
        SDL_Log("Recording stopped - %zu events recorded", recordedEvents.size());
    }
}

void SoundManager::startPlayback() {
    if (!recordedEvents.empty() && !isPlaying) {
        isPlaying = true;
        playbackStartTime = SDL_GetTicks();
        currentEventIndex = 0;
        keyStates.clear(); // Reset key states for playback
        SDL_Log("Playback started - %zu events to play", recordedEvents.size());
    } else if (recordedEvents.empty()) {
        SDL_Log("No recorded events to play");
    }
}

void SoundManager::stopPlayback() {
    if (isPlaying) {
        isPlaying = false;
        keyStates.clear(); // Reset key states
        SDL_Log("Playback stopped");
    }
}

void SoundManager::updatePlayback() {
    if (isPlaying && currentEventIndex < recordedEvents.size()) {
        Uint64 currentTime = SDL_GetTicks() - playbackStartTime;
        
        // Process all events due at this time
        while (currentEventIndex < recordedEvents.size() && 
               recordedEvents[currentEventIndex].timestamp <= currentTime) {
            const SoundEvent& event = recordedEvents[currentEventIndex];
            
            // Apply the delay setting from this event to match recording conditions
            if (event.delay != currentDelay) {
                // Temporarily update global delay to match what was recorded
                currentDelay = event.delay;
                SDL_Log("Playback: using delay of %d ms from recording", currentDelay);
            }
            
            if (event.isKeyDown) {
                // Key down event - play the sound and update state
                auto it = sounds.find(event.soundName);
                if (it != sounds.end()) {
                    // Store key press info for continuous playback
                    keyStates[event.soundName] = true;
                    keyPressTime[event.soundName] = SDL_GetTicks();
                    keyPlayDuration[event.soundName] = it->second->getDuration();
                    
                    // Temporarily adjust volume to match recorded level
                    float originalVolume = globalVolume;
                    globalVolume = event.volume;
                    
                    // Play the sound
                    playSound(event.soundName);
                    
                    // Restore original volume
                    globalVolume = originalVolume;
                }
                SDL_Log("Playback: key down %s at %llu ms (volume: %.2f)", event.soundName.c_str(), currentTime, event.volume);
            } else {
                // Key up event - just update state
                keyStates[event.soundName] = false;
                SDL_Log("Playback: key up %s at %llu ms", event.soundName.c_str(), currentTime);
            }
            
            currentEventIndex++;
        }
        
        // Check if we've reached the end
        if (currentEventIndex >= recordedEvents.size()) {
            SDL_Log("Playback completed");
            keyStates.clear(); // Reset key states
            // Keep isPlaying true to prevent repeating
        }
        
        // Also check for continuous playback of held keys during playback
        updateContinuousPlayback();
    }
}

void SoundManager::updateContinuousPlayback() {
    // Get current time once for all operations
    Uint64 currentTime = SDL_GetTicks();
    
    // Use the currentDelay variable instead of DEFAULT_DELAY_MS for the replay interval
    // This allows the user-adjusted delay to affect continuous playback
    
    // Process all held keys with a consistent approach
    for (auto& keyPair : keyStates) {
        const std::string& name = keyPair.first;
        bool isKeyDown = keyPair.second;
        
        // Skip keys that aren't being held down
        if (!isKeyDown) {
            continue;
        }
        
        // Check if we need to initialize timing for this key
        if (keyPressTime.find(name) == keyPressTime.end()) {
            // First encounter with this key - initialize timing data
            keyPressTime[name] = currentTime;
            continue; // Skip playing on first press - it's handled by recordKeyDown
        }
        
        // Check if enough time has passed to replay this sound
        Uint64 lastPlayTime = keyPressTime[name];
        Uint64 elapsedTime = currentTime - lastPlayTime;
        
        // Replay at fixed intervals using the current delay value
        if (elapsedTime >= currentDelay) {
            // Play the sound again
            playSound(name);
            
            // Update the last play time for just this key
            keyPressTime[name] = currentTime;
            
            SDL_Log("Replaying held key: %s (interval: %llu ms, current delay: %d ms)", 
                    name.c_str(), elapsedTime, currentDelay);
        }
    }
}

void SoundManager::update() {
    // Update template sounds (not really necessary but keeping for consistency)
    for (auto& pair : sounds) {
        pair.second->update();
    }
    
    // Update active instances
    for (auto& instance : activeInstances) {
        instance->update();
    }
    
    // Update playback if needed
    if (isPlaying) {
        updatePlayback();
    } else {
        // Update continuous playback for held keys (when not in playback mode)
        updateContinuousPlayback();
    }
    
    // Clean up finished instances periodically
    if (SDL_GetTicks() % 2000 == 0) {
        cleanupFinishedInstances();
    }
}

int SoundManager::getPlayingCount() {
    int count = 0;
    for (const auto& instance : activeInstances) {
        if (instance->playing) {
            count++;
        }
    }
    return count;
}

bool SoundManager::saveRecordingToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        SDL_Log("Failed to open file for saving: %s", filename.c_str());
        return false;
    }
    
    // Write a header
    file << "# Sound Recording - Timestamp(ms),SoundName,Action(D/U),Volume,Delay" << std::endl;
    
    // Write each event with shortened representations
    for (const auto& event : recordedEvents) {
        // Shorten note and chord names (note1 -> n1, chord1 -> c1)
        std::string shortName = event.soundName;
        if (shortName.find("note") == 0) {
            shortName = "n" + shortName.substr(4);  // Replace "note" with "n"
        } else if (shortName.find("chord") == 0) {
            shortName = "c" + shortName.substr(5);  // Replace "chord" with "c"
        }
        
        file << event.timestamp << "," 
             << shortName << ","
             << (event.isKeyDown ? "D" : "U") << ","
             << event.volume << ","
             << event.delay << std::endl;
    }
    
    file.close();
    SDL_Log("Successfully saved recording to: %s", filename.c_str());
    return true;
}

bool SoundManager::loadRecordingFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        SDL_Log("Failed to open file for loading: %s", filename.c_str());
        return false;
    }

    // Clear any existing recording
    recordedEvents.clear();
    
    std::string line;
    
    // Skip header line
    std::getline(file, line);
    
    // Read event data
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        
        // Parse CSV format
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        size_t pos3 = line.find(',', pos2 + 1);
        size_t pos4 = line.find(',', pos3 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            SoundEvent event;
            event.timestamp = std::stoull(line.substr(0, pos1));
            
            // Extract and expand shortened sound name
            std::string shortName = line.substr(pos1 + 1, pos2 - pos1 - 1);
            if (shortName[0] == 'n' && shortName.length() > 1) {
                event.soundName = "note" + shortName.substr(1);  // Expand "n1" to "note1"
            } else if (shortName[0] == 'c' && shortName.length() > 1) {
                event.soundName = "chord" + shortName.substr(1);  // Expand "c1" to "chord1"
            } else {
                event.soundName = shortName;  // Keep as is if not matching pattern
            }
            
            // Parse action character
            std::string action = line.substr(pos2 + 1, pos3 - pos2 - 1);
            event.isKeyDown = (action == "D");
            
            // Parse volume data
            try {
                event.volume = std::stof(line.substr(pos3 + 1, pos4 - pos3 - 1));
            }
            catch(...) {
                event.volume = 1.0f; // Default volume if parsing fails
            }
            
            // Parse delay data if available
            try {
                if (pos4 != std::string::npos) {
                    event.delay = std::stoi(line.substr(pos4 + 1));
                } else {
                    event.delay = DEFAULT_DELAY_MS; // Use default if not specified
                }
            }
            catch(...) {
                event.delay = DEFAULT_DELAY_MS; // Default delay if parsing fails
            }
            
            // Add event to the collection
            recordedEvents.push_back(event);
        }
    }
    
    file.close();
    SDL_Log("Successfully loaded %zu events from file: %s", recordedEvents.size(), filename.c_str());
    return true;
}

// Volume control implementation
void SoundManager::adjustVolume(float delta) {
    globalVolume += delta;
    
    // Clamp volume between 0.0 and 2.0 (0% to 200%)
    if (globalVolume < 0.0f) globalVolume = 0.0f;
    if (globalVolume > 2.0f) globalVolume = 2.0f;
    
    SDL_Log("Volume adjusted to %.1f%%", globalVolume * 100.0f);
}

// Add a method to directly set the current delay value
void SoundManager::setDelay(int delay) {
    // Ensure the delay is within valid range
    if (delay < MIN_DELAY_MS) delay = MIN_DELAY_MS;
    if (delay > MAX_DELAY_MS) delay = MAX_DELAY_MS;
    
    // Update the global delay variable
    currentDelay = delay;
    
    SDL_Log("SoundManager: Delay set to %d ms", currentDelay);
}