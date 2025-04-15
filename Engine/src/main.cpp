#include <iostream>
#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <ctime>
#include <sstream>
#include <iomanip>

// Our modular audio system includes
#include "audio/sound.hpp"
#include "audio/soundManager.hpp"
#include "audio/visualizer.hpp"
#include "audio/config.hpp"

// Global delay variable that can be accessed by both main and SoundManager
int currentDelay = DEFAULT_DELAY_MS;

// Generate a unique filename based on timestamp
std::string generateFilename() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << "c:\\Users\\nikit\\Desktop\\C++ Development\\GameEngine-SDL-SOUND_TEST\\recordings\\";
    oss << "music_recording_";
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    oss << ".txt";
    return oss.str();
}

// Helper function to handle note key events
void handleNoteKeyEvent(SoundManager &soundManager, const std::vector<double> &frequencies, int key, bool isKeyDown) {
    int noteIndex = 0;
    
    // Handle numeric keys 1-9
    if (key >= SDLK_1 && key <= SDLK_9) {
        noteIndex = key - SDLK_1;
    }
    // Handle letter keys for additional notes
    else {
        // Map QWERTZUIOPASDFGHJKL to note indices starting after the number keys
        switch (key) {
            case SDLK_Q: noteIndex = 9; break;
            case SDLK_W: noteIndex = 10; break;
            case SDLK_E: noteIndex = 11; break;
            case SDLK_R: noteIndex = 12; break;
            case SDLK_T: noteIndex = 13; break;
            case SDLK_Z: noteIndex = 14; break;  // Z for QWERTZ layout
            case SDLK_Y: noteIndex = 14; break;  // Y for QWERTY layout (alternative)
            case SDLK_U: noteIndex = 15; break;
            case SDLK_I: noteIndex = 16; break;
            case SDLK_O: noteIndex = 17; break;
            case SDLK_P: noteIndex = 18; break;
            case SDLK_A: noteIndex = 19; break;
            case SDLK_S: noteIndex = 20; break;
            case SDLK_D: noteIndex = 21; break;
            case SDLK_F: noteIndex = 22; break;
            case SDLK_G: noteIndex = 23; break;
            case SDLK_H: noteIndex = 24; break;
            case SDLK_J: noteIndex = 25; break;
            case SDLK_K: noteIndex = 26; break;
            case SDLK_L: noteIndex = 27; break;
            default: return; // Unknown key, don't process
        }
    }
    
    // Ensure we don't access frequencies out of bounds
    if (noteIndex >= frequencies.size()) {
        SDL_Log("Note index %d out of bounds (max: %zu)", noteIndex, frequencies.size() - 1);
        return;
    }
    
    std::string noteName = "note" + std::to_string(noteIndex);

    if (isKeyDown) {
        soundManager.recordKeyDown(noteName);
        SDL_Log("Key down: note %d (%.2f Hz)", noteIndex + 1, frequencies[noteIndex]);
    } else {
        soundManager.recordKeyUp(noteName);
        SDL_Log("Key up: note %d", noteIndex + 1);
    }
}

// Helper function to handle chord key events
void handleChordKeyEvent(SoundManager &soundManager, bool isKeyDown) {
    const std::vector<std::string> chordNotes = {"chord1", "chord2", "chord3"};

    for (const auto &note : chordNotes) {
        if (isKeyDown) {
            soundManager.recordKeyDown(note);
        } else {
            soundManager.recordKeyUp(note);
        }
    }

    SDL_Log(isKeyDown ? "Playing C major chord" : "C major chord released");
}

// Helper function to handle drum key events
void handleDrumKeyEvent(SoundManager &soundManager, int key, bool isKeyDown) {
    std::string drumName;
    
    // Map numeric keypad keys to different drum sounds
    switch (key) {
        case SDLK_KP_2: drumName = "kick0"; break;       // Bass drum
        case SDLK_KP_3: drumName = "kick1"; break;      // Snare drum
        case SDLK_KP_4: drumName = "kick2"; break;      // Hi-hat
        case SDLK_KP_5: drumName = "kick3"; break;       // High tom
        case SDLK_KP_6: drumName = "kick4"; break;       // Mid tom
        case SDLK_KP_7: drumName = "kick5"; break;      // Crash cymbal
        case SDLK_KP_8: drumName = "kick6"; break;       // Ride cymbal
        case SDLK_KP_9: drumName = "kick7"; break;       // Hand clap
        default: return; // Unknown key, don't process
    }
    
    // Process both key down and key up events for drums, just like chords
    if (isKeyDown) {
        soundManager.recordKeyDown(drumName);
        SDL_Log("Drum hit: %s", drumName.c_str());
    } else {
        soundManager.recordKeyUp(drumName);
        SDL_Log("Drum released: %s", drumName.c_str());
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL with both video and audio
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
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
    audioSpec.channels = AUDIO_CHANNELS;
    audioSpec.freq = AUDIO_SAMPLE_RATE;
    
    // Open the audio device
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec);
    if (!audioDevice) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open audio device: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Define a sequence of frequencies for musical notes - expanded to include deeper notes
    std::vector<double> frequencies = {
        65.0,  // C2
        70.0,  // C#2/Db2
        75.0,  // D2
        80.0,  // D#2/Eb2
        85.0,  // E2
        90.0,  // F2
        95.0,  // F#2/Gb2
        100.0,  // G2
        105.0, // G#2/Ab2
        110.0, // A2
        115.0, // A#2/Bb2
        120.0, // B2
        125.0, // C3
        130.0, // C#3/Db3
        135.0, // D3
        140.0, // D#3/Eb3
        145.0, // E3
        150.0, // F3
        155.0, // F#3/Gb3
        160.0, // G3
        165.0, // G#3/Ab3
        170.0, // A3
        175.0, // A#3/Bb3
        180.0, // B3
        185.0, // C4
        190.0, // C#4/Db4
        195.0, // D4
        200.0  // D#4/Eb4
    };
    
    // Base frequencies to be able to reset or modify the scale
    const std::vector<double> baseFrequencies = frequencies;
    
    // Frequency adjustment amount
    const double FREQ_ADJUSTMENT = 5.0;
    
    // Current frequency shift (to track how much we've adjusted)
    double currentFreqShift = 0.0;
    
    // Create the sound manager
    SoundManager soundManager(audioDevice);
    
    // Function to update all sound frequencies
    auto updateAllSoundFrequencies = [&]() {
        // Remove all existing note sounds
        for (size_t i = 0; i < frequencies.size(); i++) {
            std::string noteName = "note" + std::to_string(i);
            soundManager.removeSound(noteName);
        }
        
        // Add sounds with new frequencies
        for (size_t i = 0; i < frequencies.size(); i++) {
            std::string noteName = "note" + std::to_string(i);
            soundManager.addSound(noteName, frequencies[i], 0.3f, 500, 100);
        }
        
        SDL_Log("Frequency shift: %.1f Hz", currentFreqShift);
    };
    
    // Add sounds for each note with 100ms fadeout
    for (size_t i = 0; i < frequencies.size(); i++) {
        std::string noteName = "note" + std::to_string(i);
        soundManager.addSound(noteName, frequencies[i], 0.3f, 500, 100); // 500ms duration, 100ms fadeout
    }
    
    // Create a chord sound with 200ms fadeout for smoother chord endings
    soundManager.addSound("chord1", 130.81, 0.2f, 5000, 200); // C3 for 3 seconds, 200ms fadeout
    soundManager.addSound("chord2", 164.81, 0.2f, 5000, 200); // E3 for 3 seconds, 200ms fadeout 
    soundManager.addSound("chord3", 195.99, 0.2f, 5000, 200); // G3 for 3 seconds, 200ms fadeout
    
    // Add drum sounds with appropriate characteristics
    // Each drum has unique frequency, gain, duration and fadeout parameters to create different percussive sounds
    soundManager.addSound("kick0", 50.00, 0.8f, 120, 80);     // Bass drum - deeper sound, strong attack, short decay
    soundManager.addSound("kick1", 55.00, 0.8f, 120, 80);     // Bass drum - deeper sound, strong attack, short decay
    soundManager.addSound("kick2", 60.00, 0.8f, 120, 80);     // Bass drum - deeper sound, strong attack, short decay
    soundManager.addSound("kick3", 65.00, 0.8f, 120, 80);     // Bass drum - deeper sound, strong attack, short decay
    soundManager.addSound("kick4", 70.00, 0.8f, 120, 80);     // Bass drum - deeper sound, strong attack, short decay
    soundManager.addSound("kick5", 75.00, 0.8f, 120, 80);     // Bass drum - deeper sound, strong attack, short decay
    soundManager.addSound("kick6", 80.00, 0.8f, 120, 80);     // Bass drum - deeper sound, strong attack, short decay
    soundManager.addSound("kick7", 85.00, 0.8f, 120, 80);     // Bass drum - deeper sound, strong attack, short decay

    // Main loop flag
    bool quit = false;
    
    // Event handler
    SDL_Event e;
    
    // File path for loading recordings
    const std::string recordingFilePath = "c:\\Users\\nikit\\Desktop\\C++ Development\\GameEngine-SDL-SOUND_TEST\\recordings\\1.txt";
    
    // Volume control settings
    const float VOLUME_STEP = 0.05f; // 5% volume adjustment per wheel tick
    
    // Display instructions
    SDL_Log("Press keys 1-8 to play individual notes");
    SDL_Log("Press C to play a C major chord");
    SDL_Log("Press S to start/stop recording");
    SDL_Log("Press D to play back recorded music");
    SDL_Log("Press F to save your recorded music to a file");
    SDL_Log("Press L to load and play music from 1.txt");
    SDL_Log("Press A to quit");
    SDL_Log("Scroll mouse wheel up/down to adjust volume");
    SDL_Log("You can play multiple notes simultaneously - each press creates a new sound instance");
    SDL_Log("NumPad 2-9 keys for drum sounds (Kick, Snare, HiHat, Toms, Cymbals, Clap)");
    SDL_Log("Press M to increase all frequencies by 200 Hz");
    SDL_Log("Press N to decrease all frequencies by 200 Hz");
    SDL_Log("Press V to decrease delay by 10ms");
    SDL_Log("Press B to increase delay by 10ms");
    
    // While application is running
    // While application is running
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e)) {
            // User requests quit
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
            // User scrolls mouse wheel for volume control
            else if (e.type == SDL_EVENT_MOUSE_WHEEL) {
                float volumeDelta = e.wheel.y * VOLUME_STEP;
                soundManager.adjustVolume(volumeDelta);
            }
            // User presses a key
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                switch (e.key.key) {
                    case SDLK_ESCAPE:
                        SDL_Log("'ESC' key pressed. Exiting...");
                        quit = true;
                        break;
                        
                    case SDLK_KP_PLUS:
                        // Toggle recording
                        if (soundManager.isCurrentlyRecording()) {
                            soundManager.stopRecording();
                        } else {
                            soundManager.startRecording();
                        }
                        break;
                        
                    case SDLK_KP_MINUS:
                        // Start playback of recorded music
                        if (!soundManager.isCurrentlyPlaying()) {
                            soundManager.startPlayback();
                        } else {
                            soundManager.stopPlayback();
                        }
                        break;
                        
                    case SDLK_KP_ENTER:
                        // Save recording to file
                        {
                            std::string filename = generateFilename();
                            if (soundManager.saveRecordingToFile(filename)) {
                                SDL_Log("Recording saved to %s", filename.c_str());
                            } else {
                                SDL_Log("Failed to save recording");
                            }
                        }
                        break;
                        
                    case SDLK_KP_0:
                        // Load recording from file and start playback
                        if (soundManager.loadRecordingFromFile(recordingFilePath)) {
                            SDL_Log("Loaded recording from %s", recordingFilePath.c_str());
                            soundManager.startPlayback();
                        } else {
                            SDL_Log("Failed to load recording from %s", recordingFilePath.c_str());
                        }
                        break;
                        
                    case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                    case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
                    case SDLK_Q: case SDLK_W: case SDLK_E: case SDLK_R: case SDLK_T: 
                    case SDLK_Z: case SDLK_U: case SDLK_I: case SDLK_O: case SDLK_P:
                    case SDLK_A: case SDLK_S: case SDLK_D: case SDLK_F: case SDLK_G: 
                    case SDLK_H: case SDLK_J: case SDLK_K: case SDLK_L:
                        handleNoteKeyEvent(soundManager, frequencies, e.key.key, true);
                        break;

                    case SDLK_KP_1:
                        handleChordKeyEvent(soundManager, true);
                    case SDLK_KP_2: case SDLK_KP_3: case SDLK_KP_4:
                    case SDLK_KP_5: case SDLK_KP_6: case SDLK_KP_7:
                    case SDLK_KP_8: case SDLK_KP_9:
                        handleDrumKeyEvent(soundManager, e.key.key, true);

                        break;
                        
                    case SDLK_M:
                        // Increase all frequencies by 200 Hz
                        currentFreqShift += FREQ_ADJUSTMENT;
                        for (size_t i = 0; i < frequencies.size(); i++) {
                            frequencies[i] = baseFrequencies[i] + currentFreqShift;
                        }
                        updateAllSoundFrequencies();
                        break;
                        
                    case SDLK_N:
                        // Decrease all frequencies by 200 Hz, but don't go below 20 Hz
                        if (currentFreqShift >= FREQ_ADJUSTMENT) {
                            currentFreqShift -= FREQ_ADJUSTMENT;
                            for (size_t i = 0; i < frequencies.size(); i++) {
                                frequencies[i] = baseFrequencies[i] + currentFreqShift;
                            }
                            updateAllSoundFrequencies();
                        } else if (currentFreqShift > 0) {
                            // Reset to base frequencies if we can't decrease by full amount
                            currentFreqShift = 0;
                            frequencies = baseFrequencies;
                            updateAllSoundFrequencies();
                        }
                        break;
                        
                    case SDLK_V:
                        // Decrease delay by 10ms
                        if (currentDelay > MIN_DELAY_MS) {
                            currentDelay -= DELAY_STEP_MS;
                            soundManager.setDelay(currentDelay); // Use the setDelay method
                        }
                        break;
                        
                    case SDLK_B:
                        // Increase delay by 10ms
                        if (currentDelay < MAX_DELAY_MS) {
                            currentDelay += DELAY_STEP_MS;
                            soundManager.setDelay(currentDelay); // Use the setDelay method
                        }
                        break;
                }
            }
            // User releases a key
            else if (e.type == SDL_EVENT_KEY_UP) {
                switch (e.key.key) {
                    case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                    case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
                    case SDLK_Q: case SDLK_W: case SDLK_E: case SDLK_R: case SDLK_T: 
                    case SDLK_Z: case SDLK_U: case SDLK_I: case SDLK_O: case SDLK_P:
                    case SDLK_A: case SDLK_S: case SDLK_D: case SDLK_F: case SDLK_G: 
                    case SDLK_H: case SDLK_J: case SDLK_K: case SDLK_L:
                        handleNoteKeyEvent(soundManager, frequencies, e.key.key, false);
                        break;

                    case SDLK_KP_1:
                        handleChordKeyEvent(soundManager, false);
                        break;
                        
                    case SDLK_KP_2: case SDLK_KP_3: case SDLK_KP_4:
                    case SDLK_KP_5: case SDLK_KP_6: case SDLK_KP_7:
                    case SDLK_KP_8: case SDLK_KP_9:
                        handleDrumKeyEvent(soundManager, e.key.key, false);
                        break;
                }
            }
        }
        
        // Update sound states
        soundManager.update();
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        
        // Render the visualization
        SoundVisualizer::RenderPlayingSounds(renderer, soundManager);
        
        // Update screen
        SDL_RenderPresent(renderer);
        
        // Small delay to reduce CPU usage
        SDL_Delay(currentDelay);
    }
    
    // Clean up
    SDL_CloseAudioDevice(audioDevice);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}