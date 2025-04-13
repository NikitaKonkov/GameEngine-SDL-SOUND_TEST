#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include <renderer/renderer.hpp>

class App {
public:
    // Constructor with App title and window dimensions
    App(const std::string& title, int width, int height);
    
    // Destructor for cleanup
    ~App();
    
    // Initialize the App
    bool initialize();
    
    // Run the App main loop
    void run();
    
    // Process input events
    void processEvents();
    
    // Update App state
    void update();
    
    // Render the current frame
    void render();

private:
    // Window properties
    std::string title;
    int width;
    int height;
    
    // SDL window handle
    SDL_Window* window;
    
    // Renderer instance
    std::unique_ptr<Renderer> renderer;
    
    // App state
    bool running;
};