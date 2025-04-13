#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>
#include <fstream>
#include <iostream>

struct Settings {
    int screenWidth     = 1920;      // screen width
    int screenHeight    = 1080;      // screen height
    bool fullscreen     = false;    // fullscreen mode
    bool vsync          = true;     // vertical sync
    int maxFPS          = 60;       // maximum frames per second
    int audioVolume     = 100;      // audio volume (0-100)
    
    // Load settings from file
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Could not open settings file: " << filename << std::endl;
            return false;
        }else{
            std::cout << "Settings file opened successfully: " << filename << std::endl;
        }
        
        std::string line, key, value;
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;
            
            // Find the separator
            size_t pos = line.find("=");
            if (pos == std::string::npos) continue;
            
            key = line.substr(0, pos);
            value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Parse the settings
            if (key == "screenWidth") screenWidth = std::stoi(value);
            else if (key == "screenHeight") screenHeight = std::stoi(value);
            else if (key == "fullscreen") fullscreen = (value == "true" || value == "1");
            else if (key == "vsync") vsync = (value == "true" || value == "1");
            else if (key == "maxFPS") maxFPS = std::stoi(value);
            else if (key == "audioVolume") audioVolume = std::stoi(value);
        }
        
        return true;
    }
    
    // Save settings to file
    bool saveToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Could not open settings file for writing: " << filename << std::endl;
            return false;
        }
        
        file << "# Game Engine Settings\n";
        file << "screenWidth = " << screenWidth << "\n";
        file << "screenHeight = " << screenHeight << "\n";
        file << "fullscreen = " << (fullscreen ? "true" : "false") << "\n";
        file << "vsync = " << (vsync ? "true" : "false") << "\n";
        file << "maxFPS = " << maxFPS << "\n";
        file << "audioVolume = " << audioVolume << "\n";
        
        return true;
    }
};

// Global instance
extern Settings g_settings;

// Default settings as a function
inline Settings getDefaultSettings() {
    Settings defaults;
    // defaults already have the default values from initialization
    return defaults;
}

#endif // SETTINGS_HPP