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
    int noteIndex = key - SDLK_1;
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
    
    // Define a sequence of frequencies for musical notes
    std::vector<double> frequencies = {
        247.48, // B3
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
        soundManager.addSound(noteName, frequencies[i], 0.3f, 500, 100); // 500ms duration, 100ms fadeout
    }
    
    // Create a chord sound with 200ms fadeout for smoother chord endings
    soundManager.addSound("chord1", 261.63, 0.2f, 5000, 200); // C4 for 3 seconds, 200ms fadeout
    soundManager.addSound("chord2", 329.63, 0.2f, 5000, 200); // E4 for 3 seconds, 200ms fadeout 
    soundManager.addSound("chord3", 392.00, 0.2f, 5000, 200); // G4 for 3 seconds, 200ms fadeout
    
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
                        handleNoteKeyEvent(soundManager, frequencies, e.key.key, true);
                        break;

                    case SDLK_KP_1:
                        handleChordKeyEvent(soundManager, true);
                        break;
                }
            }
            // User releases a key
            else if (e.type == SDL_EVENT_KEY_UP) {
                switch (e.key.key) {
                    case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                    case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
                        handleNoteKeyEvent(soundManager, frequencies, e.key.key, false);
                        break;

                    case SDLK_KP_1:
                        handleChordKeyEvent(soundManager, false);
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
        SDL_Delay(DELAY_MS);
    }
    
    // Clean up
    SDL_CloseAudioDevice(audioDevice);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}