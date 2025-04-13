#include <app/app.hpp>
#include <Inputs/keyboard.hpp>
#include <settings/settings.hpp>
#include <stdexcept>
#include <iostream>



App::App(const std::string& title, int width, int height)
    : title(title), width(width), height(height), window(nullptr), running(false) {
}

App::~App() {
    // Renderer is automatically cleaned up by the unique_ptr
    
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    SDL_Quit();
}

bool App::initialize() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create window with Vulkan support
    window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    
    try {
        // Create the renderer
        renderer = std::make_unique<Renderer>(window);
    }
    catch (const std::exception& e) {
        std::cerr << "Renderer initialization failed: " << e.what() << std::endl;
        return false;
    }
    
    running = true;
    return true;
}

void App::run() {
    
    while (running) {
        processEvents();
        
        update();
        
        render();
    }

}

void App::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
        // Handle window events
        else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            // Get the new window size
            int newWidth, newHeight;
            SDL_GetWindowSizeInPixels(window, &newWidth, &newHeight);
            
            // Update the stored dimensions
            width = newWidth;
            height = newHeight;
            
            // Let the renderer know about the resize
            if (renderer) {
                renderer->handleWindowResize(width, height);
            }
        }
        
        // Process keyboard input through your input system
        // Only pass relevant keyboard events to the keyboard manager
        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            // Send events to the generic keyboard manager
            Keyboard::Input.handleEvent(event);
            
        }
    }
}

void App::update() {
    // Update input state
    Keyboard::Input.update();
    
    // Add game logic updates here
    // This is where you would update game state, physics, etc.
}

void App::render() {
    // Let the renderer draw the current frame
    renderer->drawFrame();
}