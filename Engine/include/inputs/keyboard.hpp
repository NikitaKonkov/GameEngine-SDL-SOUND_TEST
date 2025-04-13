#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>
#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <fstream>
#include <sstream>

namespace Keyboard {

enum class KeyState {
    RELEASED,
    JUST_PRESSED,
    PRESSED,
    JUST_RELEASED
};

struct ActionMapping {
    std::string actionName;
    SDL_Keycode primaryKey;
    SDL_Keycode alternateKey;
    std::function<void()> pressCallback;
    std::function<void()> releaseCallback;
    std::function<void()> holdCallback;
};

class KeyboardManager {
private:
    static constexpr size_t MAX_KEY_STATES = 0; // Maximum allowed key states
    std::unordered_map<SDL_Keycode, KeyState> keyStates;
    std::unordered_map<std::string, ActionMapping> actionMappings;
    std::string configFilePath;

public:
    KeyboardManager(const std::string& configFile = "resources/keyboard_config.txt");
    ~KeyboardManager() = default;

    bool loadConfiguration(const std::string& configFile);
    bool saveConfiguration(const std::string& configFile = "");
    
    // Update key states
    void handleEvent(const SDL_Event& event);
    void update(); // Call each frame to update JUST_PRESSED -> PRESSED, JUST_RELEASED -> RELEASED
    
    // Direct key state checking
    KeyState getKeyState(SDL_Keycode keyCode) const;
    bool isKeyPressed(SDL_Keycode keyCode) const;
    bool isKeyJustPressed(SDL_Keycode keyCode) const;
    bool isKeyJustReleased(SDL_Keycode keyCode) const;
    
    // Action-based interface
    bool mapAction(const std::string& actionName, SDL_Keycode primaryKey, SDL_Keycode alternateKey = SDLK_UNKNOWN);
    bool isActionPressed(const std::string& actionName) const;
    bool isActionJustPressed(const std::string& actionName) const;
    bool isActionJustReleased(const std::string& actionName) const;
    
    // Callback registration
    void registerActionCallback(const std::string& actionName, 
                               std::function<void()> pressCallback = nullptr,
                               std::function<void()> releaseCallback = nullptr,
                               std::function<void()> holdCallback = nullptr);
                               
    // Helper function to convert string to SDL_Keycode
    static SDL_Keycode stringToKeycode(const std::string& keyName);
    static std::string keycodeToString(SDL_Keycode keycode);
};

// Global instance
extern KeyboardManager Input;

} // namespace Engine

#endif // KEYBOARD_HPP