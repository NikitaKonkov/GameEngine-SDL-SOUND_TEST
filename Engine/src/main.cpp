#include <iostream>
#include <SDL3/SDL.h>
#include <vector>
#include <memory>

// Our modular audio system includes
#include "audio/sound.hpp"
#include "audio/soundManager.hpp"
#include "audio/visualizer.hpp"
#include "audio/config.hpp"

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
    SDL_Log("You can play multiple notes simultaneously - each press creates a new sound instance");
    
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
                        
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                    case SDLK_5:
                    case SDLK_6:
                    case SDLK_7:
                    case SDLK_8: {
                        int noteIndex = e.key.key - SDLK_1;
                        std::string noteName = "note" + std::to_string(noteIndex);
                        
                        // Record key down event and play sound
                        soundManager.recordKeyDown(noteName);
                        SDL_Log("Key down: note %d (%.2f Hz)", noteIndex + 1, frequencies[noteIndex]);
                        break;
                    }
                    case SDLK_C:
                        // Play a C major chord (C, E, G together)
                        soundManager.recordKeyDown("chord1");
                        soundManager.recordKeyDown("chord2");
                        soundManager.recordKeyDown("chord3");
                        SDL_Log("Playing C major chord");
                        break;
                }
            }
            // User releases a key
            else if (e.type == SDL_EVENT_KEY_UP) {
                switch (e.key.key) {
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                    case SDLK_5:
                    case SDLK_6:
                    case SDLK_7:
                    case SDLK_8: {
                        int noteIndex = e.key.key - SDLK_1;
                        std::string noteName = "note" + std::to_string(noteIndex);
                        
                        // Record key up event (no sound)
                        soundManager.recordKeyUp(noteName);
                        SDL_Log("Key up: note %d", noteIndex + 1);
                        break;
                    }
                    case SDLK_C:
                        // Record chord key releases
                        soundManager.recordKeyUp("chord1");
                        soundManager.recordKeyUp("chord2");
                        soundManager.recordKeyUp("chord3");
                        SDL_Log("C major chord released");
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
        SDL_Delay(10);
    }
    
    // Clean up
    SDL_CloseAudioDevice(audioDevice);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}