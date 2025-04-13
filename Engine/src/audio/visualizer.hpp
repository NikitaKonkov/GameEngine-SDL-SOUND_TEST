#pragma once

#include <SDL3/SDL.h>
#include "soundManager.hpp"
#include "config.hpp"

// Class to handle sound visualizations
class SoundVisualizer {
public:
    // Render visualization of the playing sounds
    static void RenderPlayingSounds(SDL_Renderer *renderer, SoundManager& soundManager);
};