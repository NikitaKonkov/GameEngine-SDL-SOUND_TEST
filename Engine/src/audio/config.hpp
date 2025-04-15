#pragma once

// Window dimensions
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "SDL Sound Test - Async Audio Demo"

// Audio settings
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_CHANNELS 4

// Visualization settings
#define MAX_VISIBLE_BARS 100
#define GRID_LINES 10
#define STATUS_BOX_HEIGHT 30
#define STATUS_BOX_WIDTH 200

// Mixer settings
#define DEFAULT_DELAY_MS 100  // Default delay value
#define MIN_DELAY_MS 10       // Minimum allowed delay
#define MAX_DELAY_MS 500      // Maximum allowed delay
#define DELAY_STEP_MS 10      // Step size for delay adjustment