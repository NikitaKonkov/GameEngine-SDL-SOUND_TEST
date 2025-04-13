#include "visualizer.hpp"
#include "config.hpp"

void SoundVisualizer::RenderPlayingSounds(SDL_Renderer *renderer, SoundManager& soundManager) {
    const auto& activeInstances = soundManager.getActiveInstances();
    int playingCount = soundManager.getPlayingCount();
    
    // Draw background grid lines
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    for (int i = 0; i < GRID_LINES; i++) {
        int y = 50 + i * (WINDOW_HEIGHT - 100) / GRID_LINES;
        SDL_RenderLine(renderer, 0, y, WINDOW_WIDTH, y);
    }
    
    // If no active instances, just show a placeholder
    if (playingCount == 0) {
        SDL_FRect bar = {
            WINDOW_WIDTH / 2.0f - 50.0f,
            WINDOW_HEIGHT - 50.0f - 20.0f,
            100.0f,
            20.0f
        };
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &bar);
        return;
    }
    
    // Draw active sounds
    int i = 0;
    for (const auto& instance : activeInstances) {
        if (!instance->isPlaying()) continue;
        if (i >= MAX_VISIBLE_BARS) break; // Limit visualization to prevent too many bars
        
        int barHeight = 0;
        SDL_Color color = {0, 0, 0, 255};
        
        // Calculate height based on frequency
        barHeight = static_cast<int>(instance->getFrequency() / 5.0); // Scale for visualization
        if (barHeight > WINDOW_HEIGHT - 100)
            barHeight = WINDOW_HEIGHT - 100;
        
        // Set color based on frequency (low frequencies are blue, high are red)
        float normalizedFreq = instance->getFrequency() / 1000.0f;
        if (normalizedFreq > 1.0f) normalizedFreq = 1.0f;
        
        // Calculate position - spread evenly across the screen
        float xPos = i * (WINDOW_WIDTH / float(MAX_VISIBLE_BARS));
        
        // Calculate width based on number of active sounds
        float width = WINDOW_WIDTH / float(MAX_VISIBLE_BARS) - 5.0f;
        
        color.r = static_cast<Uint8>(normalizedFreq * 255);
        color.g = static_cast<Uint8>((1.0f - normalizedFreq) * 128);
        color.b = static_cast<Uint8>((1.0f - normalizedFreq) * 255);
        color.a = 255;
        
        // Draw the bar
        SDL_FRect bar = {
            xPos,
            WINDOW_HEIGHT - barHeight - 50.0f,
            width,
            static_cast<float>(barHeight)
        };
        
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &bar);
        
        i++;
    }
    
    // Display recording/playback status
    SDL_FRect statusBox = {
        10.0f, 10.0f,
        static_cast<float>(STATUS_BOX_WIDTH), static_cast<float>(STATUS_BOX_HEIGHT)
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
    
    // Display count of playing sounds
    char countText[32];
    snprintf(countText, sizeof(countText), "Playing: %d", playingCount);
    SDL_FRect countBox = {
        10.0f, 50.0f,
        static_cast<float>(STATUS_BOX_WIDTH), static_cast<float>(STATUS_BOX_HEIGHT)
    };
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &countBox);
}