#include <app/app.hpp>
#include <settings/settings.hpp>
#include <iostream>
#include <SDL3/SDL.h>
#include <cmath>
#include <vector>

#define M_PI 3.14159265358979323846

// Window dimensions
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "SDL Sound Test"

// Audio callback function to generate a sine wave
void AudioCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    // Get the current frequency from userdata
    double *frequency = static_cast<double*>(userdata);
    static double phase = 0.0;
    
    // Generate a sine wave with the current frequency
    const int sample_rate = 48000;  // CD quality
    const double phase_increment = 2.0 * M_PI * (*frequency) / sample_rate;
    
    // Calculate how many samples we need
    const int sample_size = sizeof(float);
    const int num_samples = additional_amount / sample_size;
    
    // Allocate buffer for our samples
    float *buffer = static_cast<float*>(SDL_malloc(num_samples * sample_size));
    if (!buffer) {
        return;
    }
    
    // Fill the buffer with sine wave data
    for (int i = 0; i < num_samples; ++i) {
        buffer[i] = static_cast<float>(0.3 * sin(phase)); // 0.3 for volume (30%)
        phase += phase_increment;
        if (phase > 2.0 * M_PI) {
            phase -= 2.0 * M_PI; // Keep phase in the range [0, 2π]
        }
    }
    
    // Add data to the audio stream
    SDL_PutAudioStreamData(stream, buffer, num_samples * sample_size);
    
    // Free our buffer
    SDL_free(buffer);
}

// Function to render text about the current frequency
void RenderText(SDL_Renderer *renderer, double frequency) {
    // In a real application, you would render text here
    // Since SDL3 doesn't have built-in text rendering, we'll just draw a
    // visual representation of the frequency using colored rectangles
    
    int barHeight = static_cast<int>(frequency / 10.0); // Scale for visualization
    if (barHeight > WINDOW_HEIGHT - 100)
        barHeight = WINDOW_HEIGHT - 100;
    
    SDL_FRect bar = {
        WINDOW_WIDTH / 2.0f - 50.0f, 
        WINDOW_HEIGHT - barHeight - 50.0f, 
        100.0f, 
        barHeight * 1.0f
    };
    
    // Set color based on frequency (low frequencies are blue, high are red)
    float normalizedFreq = frequency / 1000.0f; // Normalize to 0-1 range for common frequencies
    if (normalizedFreq > 1.0f) normalizedFreq = 1.0f;
    
    SDL_SetRenderDrawColor(renderer, 
                          static_cast<Uint8>(normalizedFreq * 255), // Red component
                          static_cast<Uint8>((1.0f - normalizedFreq) * 128), // Green component
                          static_cast<Uint8>((1.0f - normalizedFreq) * 255), // Blue component
                          255); // Alpha
    
    SDL_RenderFillRect(renderer, &bar);
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
    audioSpec.channels = 1;      // mono
    audioSpec.freq = 48000;      // 48KHz
    
    // Define a sequence of frequencies to play (musical notes)
    std::vector<double> frequencies = {
        261.63, // C4
        293.66, // D4
        329.63, // E4
        349.23, // F4
        392.00, // G4
        440.00, // A4
        493.88, // B4
        523.25  // C5
    };
    
    // Current frequency to use (this will be passed to our callback)
    double currentFrequency = frequencies[0];
    
    // Open audio device stream with our callback
    SDL_AudioStream *audioStream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 
        &audioSpec, 
        AudioCallback,
        &currentFrequency
    );
    
    if (!audioStream) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open audio stream: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    
    // Start playing
    SDL_ResumeAudioStreamDevice(audioStream);
    
    SDL_Log("Playing a sequence of tones with 1-second intervals...");
    
    // Play each frequency for 1 second
    size_t currentNoteIndex = 0;
    Uint64 lastNoteChangeTime = SDL_GetTicks();
    
    // Main loop flag
    bool quit = false;
    
    // Event handler
    SDL_Event e;
    
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
                // If 'A' key is pressed
                if (e.key.key == SDLK_A) {
                    SDL_Log("'A' key pressed. Exiting...");
                    quit = true;
                }
                // If space key is pressed, change to the next note
                else if (e.key.key == SDLK_SPACE) {
                    currentNoteIndex = (currentNoteIndex + 1) % frequencies.size();
                    currentFrequency = frequencies[currentNoteIndex];
                    SDL_Log("Playing frequency: %.2f Hz", currentFrequency);
                    lastNoteChangeTime = SDL_GetTicks();
                }
            }
        }
        
        // Check if it's time to change notes (every 1 second) in auto mode
        Uint64 currentTime = SDL_GetTicks();
        if (currentTime - lastNoteChangeTime >= 1000) {
            currentNoteIndex = (currentNoteIndex + 1) % frequencies.size();
            currentFrequency = frequencies[currentNoteIndex];
            SDL_Log("Playing frequency: %.2f Hz", currentFrequency);
            lastNoteChangeTime = currentTime;
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        
        // Render text about current frequency
        RenderText(renderer, currentFrequency);
        
        // Update screen
        SDL_RenderPresent(renderer);
        
        // Small delay to reduce CPU usage
        SDL_Delay(10);
    }
    
    // Clean up
    SDL_DestroyAudioStream(audioStream);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}