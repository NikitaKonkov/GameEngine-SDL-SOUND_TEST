#include <inputs/keyboard.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <string>

namespace Keyboard {

KeyboardManager Input;

KeyboardManager::KeyboardManager(const std::string& configFile) : configFilePath(configFile) {
    loadConfiguration(configFile);
}

bool KeyboardManager::loadConfiguration(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        SDL_Log("Failed to open keyboard configuration file: %s", configFile.c_str());
        return false;
    }

    actionMappings.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == '/') {
            continue;
        }
        
        std::istringstream iss(line);
        std::string actionName, primaryKeyStr, alternateKeyStr;
        
        if (iss >> actionName >> primaryKeyStr >> alternateKeyStr) {
            SDL_Keycode primaryKey = stringToKeycode(primaryKeyStr);
            SDL_Keycode alternateKey = stringToKeycode(alternateKeyStr);
            
            mapAction(actionName, primaryKey, alternateKey);
        }
    }
    
    configFilePath = configFile;
    return true;
}

bool KeyboardManager::saveConfiguration(const std::string& configFile) {
    std::string filePath = configFile.empty() ? configFilePath : configFile;
    
    std::ofstream file(filePath);
    if (!file.is_open()) {
        SDL_Log("Failed to open keyboard configuration file for writing: %s", filePath.c_str());
        return false;
    }
    
    file << "# Keyboard Configuration File\n";
    file << "# Format: ActionName PrimaryKey AlternateKey\n\n";
    
    for (const auto& [actionName, mapping] : actionMappings) {
        file << actionName << " " 
             << keycodeToString(mapping.primaryKey) << " " 
             << keycodeToString(mapping.alternateKey) << "\n";
    }
    
    return true;
}

void KeyboardManager::handleEvent(const SDL_Event& event) {

    if (event.type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event.key.key;
        
        // Check if this is a new key and we're at the limit
        if (keyStates.find(key) == keyStates.end() && keyStates.size() >= MAX_KEY_STATES) {
            // Remove a key that's in RELEASED state (prioritize those)
            bool removed = false;
            for (auto it = keyStates.begin(); it != keyStates.end(); ) {
                if (it->second == KeyState::RELEASED) {
                    SDL_Log("Removing old released key: %s to make room for: %s", 
                           keycodeToString(it->first).c_str(), keycodeToString(key).c_str());
                    it = keyStates.erase(it);
                    removed = true;
                    break;
                } else {
                    ++it;
                }
            }
            
            // If no RELEASED keys, remove the first key (oldest)
            if (!removed && !keyStates.empty()) {
                auto it = keyStates.begin();
                SDL_Log("Removing oldest key: %s to make room for: %s", 
                       keycodeToString(it->first).c_str(), keycodeToString(key).c_str());
                keyStates.erase(it);
            }
        }
        



        // Update state
        if (keyStates.find(key) == keyStates.end() || 
            keyStates[key] == KeyState::RELEASED || 
            keyStates[key] == KeyState::JUST_RELEASED) {
            keyStates[key] = KeyState::JUST_PRESSED;
            
            // Trigger callbacks for actions mapped to this key
            for (auto& [name, mapping] : actionMappings) {
                if (mapping.primaryKey == key || mapping.alternateKey == key) {
                    if (mapping.pressCallback) {
                        mapping.pressCallback();
                    }
                }
            }
        }
    } 




    else if (event.type == SDL_EVENT_KEY_UP) {
        SDL_Keycode key = event.key.key;
        
        // Check if this is a new key and we're at the limit
        if (keyStates.find(key) == keyStates.end() && keyStates.size() >= MAX_KEY_STATES) {
            // Remove a key that's in RELEASED state (prioritize those)
            bool removed = false;
            for (auto it = keyStates.begin(); it != keyStates.end(); ) {
                if (it->second == KeyState::RELEASED) {
                    SDL_Log("Removing old released key: %s to make room for: %s", 
                           keycodeToString(it->first).c_str(), keycodeToString(key).c_str());
                    it = keyStates.erase(it);
                    removed = true;
                    break;
                } else {
                    ++it;
                }
            }
            
            // If no RELEASED keys, remove the first key (oldest)
            if (!removed && !keyStates.empty()) {
                auto it = keyStates.begin();
                SDL_Log("Removing oldest key: %s to make room for: %s", 
                       keycodeToString(it->first).c_str(), keycodeToString(key).c_str());
                keyStates.erase(it);
            }
        }
        
        // Update state
        if (keyStates.find(key) != keyStates.end() &&
           (keyStates[key] == KeyState::PRESSED || 
            keyStates[key] == KeyState::JUST_PRESSED)) {
            keyStates[key] = KeyState::JUST_RELEASED;
            
            // Trigger callbacks for actions mapped to this key
            for (auto& [name, mapping] : actionMappings) {
                if (mapping.primaryKey == key || mapping.alternateKey == key) {
                    if (mapping.releaseCallback) {
                        mapping.releaseCallback();
                    }
                }
            }
        }
    }

    
}

void KeyboardManager::update() {
    for (auto& [key, state] : keyStates) {
        std::string keyName = keycodeToString(key);
        if (state == KeyState::JUST_PRESSED) {
            SDL_Log("Key %s JUST_PRESSED", keyName.c_str());
            state = KeyState::PRESSED;
        } 
        else if (state == KeyState::JUST_RELEASED) {
            state = KeyState::RELEASED;
        }
        
        // Handle hold callbacks
        if (state == KeyState::PRESSED) {
            SDL_Log("Key %s PRESSED", keyName.c_str());
            
            for (auto& [name, mapping] : actionMappings) {
                if ((mapping.primaryKey == key || mapping.alternateKey == key) && mapping.holdCallback) {
                    mapping.holdCallback();
                }
            }
        }
    }
}

KeyState KeyboardManager::getKeyState(SDL_Keycode keyCode) const {
    auto it = keyStates.find(keyCode);
    if (it != keyStates.end()) {
        return it->second;
    }
    return KeyState::RELEASED;
}

bool KeyboardManager::isKeyPressed(SDL_Keycode keyCode) const {
    KeyState state = getKeyState(keyCode);
    return state == KeyState::PRESSED || state == KeyState::JUST_PRESSED;
}

bool KeyboardManager::isKeyJustPressed(SDL_Keycode keyCode) const {
    return getKeyState(keyCode) == KeyState::JUST_PRESSED;
}

bool KeyboardManager::isKeyJustReleased(SDL_Keycode keyCode) const {
    return getKeyState(keyCode) == KeyState::JUST_RELEASED;
}

bool KeyboardManager::mapAction(const std::string& actionName, SDL_Keycode primaryKey, SDL_Keycode alternateKey) {
    // Check if we're at the limit for action mappings and this is a new action
    if (actionMappings.find(actionName) == actionMappings.end() && actionMappings.size() >= MAX_KEY_STATES) {
        // Remove the first action mapping to make room
        if (!actionMappings.empty()) {
            auto it = actionMappings.begin();
            SDL_Log("Removing old action mapping: %s to make room for: %s", 
                   it->first.c_str(), actionName.c_str());
            actionMappings.erase(it);
        }
    }

    actionMappings[actionName] = ActionMapping{
        actionName,
        primaryKey,
        alternateKey
    };
    return true;
}

bool KeyboardManager::isActionPressed(const std::string& actionName) const {
    auto it = actionMappings.find(actionName);
    if (it != actionMappings.end()) {
        return isKeyPressed(it->second.primaryKey) || 
               (it->second.alternateKey != SDLK_UNKNOWN && isKeyPressed(it->second.alternateKey));
    }
    return false;
}

bool KeyboardManager::isActionJustPressed(const std::string& actionName) const {
    auto it = actionMappings.find(actionName);
    if (it != actionMappings.end()) {
        return isKeyJustPressed(it->second.primaryKey) || 
               (it->second.alternateKey != SDLK_UNKNOWN && isKeyJustPressed(it->second.alternateKey));
    }
    return false;
}

bool KeyboardManager::isActionJustReleased(const std::string& actionName) const {
    auto it = actionMappings.find(actionName);
    if (it != actionMappings.end()) {
        return isKeyJustReleased(it->second.primaryKey) || 
               (it->second.alternateKey != SDLK_UNKNOWN && isKeyJustReleased(it->second.alternateKey));
    }
    return false;
}

void KeyboardManager::registerActionCallback(
    const std::string& actionName,
    std::function<void()> pressCallback,
    std::function<void()> releaseCallback,
    std::function<void()> holdCallback) {
    
    auto it = actionMappings.find(actionName);
    if (it != actionMappings.end()) {
        if (pressCallback) it->second.pressCallback = pressCallback;
        if (releaseCallback) it->second.releaseCallback = releaseCallback;
        if (holdCallback) it->second.holdCallback = holdCallback;
    }
}

SDL_Keycode KeyboardManager::stringToKeycode(const std::string& keyName) {
    // Standard keys
    if (keyName == "ESCAPE") return SDLK_ESCAPE;
    if (keyName == "RETURN") return SDLK_RETURN;
    if (keyName == "SPACE") return SDLK_SPACE;
    if (keyName == "TAB") return SDLK_TAB;
    if (keyName == "BACKSPACE") return SDLK_BACKSPACE;
    
    // Letters
    if (keyName == "A") return SDLK_A;
    if (keyName == "B") return SDLK_B;
    if (keyName == "C") return SDLK_C;
    if (keyName == "D") return SDLK_D;
    if (keyName == "E") return SDLK_E;
    if (keyName == "F") return SDLK_F;
    if (keyName == "G") return SDLK_G;
    if (keyName == "H") return SDLK_H;
    if (keyName == "I") return SDLK_I;
    if (keyName == "J") return SDLK_J;
    if (keyName == "K") return SDLK_K;
    if (keyName == "L") return SDLK_L;
    if (keyName == "M") return SDLK_M;
    if (keyName == "N") return SDLK_N;
    if (keyName == "O") return SDLK_O;
    if (keyName == "P") return SDLK_P;
    if (keyName == "Q") return SDLK_Q;
    if (keyName == "R") return SDLK_R;
    if (keyName == "S") return SDLK_S;
    if (keyName == "T") return SDLK_T;
    if (keyName == "U") return SDLK_U;
    if (keyName == "V") return SDLK_V;
    if (keyName == "W") return SDLK_W;
    if (keyName == "X") return SDLK_X;
    if (keyName == "Y") return SDLK_Y;
    if (keyName == "Z") return SDLK_Z;

    
    // Function keys
    if (keyName == "F1") return SDLK_F1;
    if (keyName == "F2") return SDLK_F2;
    if (keyName == "F3") return SDLK_F3;
    if (keyName == "F4") return SDLK_F4;
    if (keyName == "F5") return SDLK_F5;
    if (keyName == "F6") return SDLK_F6;
    if (keyName == "F7") return SDLK_F7;
    if (keyName == "F8") return SDLK_F8;
    if (keyName == "F9") return SDLK_F9;
    if (keyName == "F10") return SDLK_F10;
    if (keyName == "F11") return SDLK_F11;
    if (keyName == "F12") return SDLK_F12;
    
    // Arrow keys
    if (keyName == "UP") return SDLK_UP;
    if (keyName == "DOWN") return SDLK_DOWN;
    if (keyName == "LEFT") return SDLK_LEFT;
    if (keyName == "RIGHT") return SDLK_RIGHT;
    
    // Additional keys (add more as needed)
    if (keyName == "LSHIFT") return SDLK_LSHIFT;
    if (keyName == "RSHIFT") return SDLK_RSHIFT;
    if (keyName == "LCTRL") return SDLK_LCTRL;
    if (keyName == "RCTRL") return SDLK_RCTRL;
    if (keyName == "LALT") return SDLK_LALT;
    if (keyName == "RALT") return SDLK_RALT;
    
    return SDLK_UNKNOWN;
}

std::string KeyboardManager::keycodeToString(SDL_Keycode keycode) {
    switch (keycode) {
        case SDLK_ESCAPE: return "ESCAPE";
        case SDLK_RETURN: return "RETURN";
        case SDLK_SPACE: return "SPACE";
        case SDLK_TAB: return "TAB";
        case SDLK_BACKSPACE: return "BACKSPACE";
        
        case SDLK_A: return "A";
        case SDLK_B: return "B";
        case SDLK_C: return "C";
        case SDLK_D: return "D";
        case SDLK_E: return "E";
        case SDLK_F: return "F";
        case SDLK_G: return "G";
        case SDLK_H: return "H";
        case SDLK_I: return "I";
        case SDLK_J: return "J";
        case SDLK_K: return "K";
        case SDLK_L: return "L";
        case SDLK_M: return "M";
        case SDLK_N: return "N";
        case SDLK_O: return "O";
        case SDLK_P: return "P";
        case SDLK_Q: return "Q";
        case SDLK_R: return "R";
        case SDLK_S: return "S";
        case SDLK_T: return "T";
        case SDLK_U: return "U";
        case SDLK_V: return "V";
        case SDLK_W: return "W";
        case SDLK_X: return "X";
        case SDLK_Y: return "Y";
        case SDLK_Z: return "Z";
        
        case SDLK_F1: return "F1";
        case SDLK_F2: return "F2";
        case SDLK_F3: return "F3";
        case SDLK_F4: return "F4";
        case SDLK_F5: return "F5";
        case SDLK_F6: return "F6";
        case SDLK_F7: return "F7";
        case SDLK_F8: return "F8";
        case SDLK_F9: return "F9";
        case SDLK_F10: return "F10";
        case SDLK_F11: return "F11";
        case SDLK_F12: return "F12";
        
        case SDLK_UP: return "UP";
        case SDLK_DOWN: return "DOWN";
        case SDLK_LEFT: return "LEFT";
        case SDLK_RIGHT: return "RIGHT";
        
        case SDLK_LSHIFT: return "LSHIFT";
        case SDLK_RSHIFT: return "RSHIFT";
        case SDLK_LCTRL: return "LCTRL";
        case SDLK_RCTRL: return "RCTRL";
        case SDLK_LALT: return "LALT";
        case SDLK_RALT: return "RALT";
        
        case SDLK_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}

} // namespace Keyboard