Below is one way to “print” (or list) all the keys and their frequencies for a standard 88‑key piano (“medium size piano”). A normal piano has 88 keys from A0 up to C8. One common approach is to build two arrays—one with the note names and one with the corresponding frequencies (calculated using the formula based on A4 = 440 Hz)—or simply use a pre‐calculated lookup table.

Below is an example list showing each key (from key number 1 to 88) along with its typical note name and frequency:

| Key # | Note          | Frequency (Hz) |
|-------|---------------|----------------|
| 1     | A0            | 27.50          |
| 2     | A♯0 / B♭0     | 29.14          |
| 3     | B0            | 30.87          |
| 4     | C1            | 32.70          |
| 5     | C♯1 / D♭1     | 34.65          |
| 6     | D1            | 36.71          |
| 7     | D♯1 / E♭1     | 38.89          |
| 8     | E1            | 41.20          |
| 9     | F1            | 43.65          |
| 10    | F♯1 / G♭1     | 46.25          |
| 11    | G1            | 48.99          |
| 12    | G♯1 / A♭1     | 51.91          |
| 13    | A1            | 55.00          |
| 14    | A♯1 / B♭1     | 58.27          |
| 15    | B1            | 61.74          |
| 16    | C2            | 65.41          |
| 17    | C♯2 / D♭2     | 69.30          |
| 18    | D2            | 73.42          |
| 19    | D♯2 / E♭2     | 77.78          |
| 20    | E2            | 82.41          |
| 21    | F2            | 87.31          |
| 22    | F♯2 / G♭2     | 92.50          |
| 23    | G2            | 97.99          |
| 24    | G♯2 / A♭2     | 103.83         |
| 25    | A2            | 110.00         |
| 26    | A♯2 / B♭2     | 116.54         |
| 27    | B2            | 123.47         |
| 28    | C3            | 130.81         |
| 29    | C♯3 / D♭3     | 138.59         |
| 30    | D3            | 146.83         |
| 31    | D♯3 / E♭3     | 155.56         |
| 32    | E3            | 164.81         |
| 33    | F3            | 174.61         |
| 34    | F♯3 / G♭3     | 184.99         |
| 35    | G3            | 195.99         |
| 36    | G♯3 / A♭3     | 207.65         |
| 37    | A3            | 220.00         |
| 38    | A♯3 / B♭3     | 233.08         |
| 39    | B3            | 246.94         |
| 40    | C4 (Middle C) | 261.63         |
| 41    | C♯4 / D♭4     | 277.18         |
| 42    | D4            | 293.66         |
| 43    | D♯4 / E♭4     | 311.13         |
| 44    | E4            | 329.63         |
| 45    | F4            | 349.23         |
| 46    | F♯4 / G♭4     | 369.99         |
| 47    | G4            | 391.99         |
| 48    | G♯4 / A♭4     | 415.30         |
| 49    | A4            | 440.00         |
| 50    | A♯4 / B♭4     | 466.16         |
| 51    | B4            | 493.88         |
| 52    | C5            | 523.25         |
| 53    | C♯5 / D♭5     | 554.37         |
| 54    | D5            | 587.33         |
| 55    | D♯5 / E♭5     | 622.25         |
| 56    | E5            | 659.26         |
| 57    | F5            | 698.46         |
| 58    | F♯5 / G♭5     | 739.99         |
| 59    | G5            | 783.99         |
| 60    | G♯5 / A♭5     | 830.61         |
| 61    | A5            | 880.00         |
| 62    | A♯5 / B♭5     | 932.33         |
| 63    | B5            | 987.77         |
| 64    | C6            | 1046.50        |
| 65    | C♯6 / D♭6     | 1108.73        |
| 66    | D6            | 1174.66        |
| 67    | D♯6 / E♭6     | 1244.51        |
| 68    | E6            | 1318.51        |
| 69    | F6            | 1396.91        |
| 70    | F♯6 / G♭6     | 1479.98        |
| 71    | G6            | 1567.98        |
| 72    | G♯6 / A♭6     | 1661.22        |
| 73    | A6            | 1760.00        |
| 74    | A♯6 / B♭6     | 1864.66        |
| 75    | B6            | 1975.53        |
| 76    | C7            | 2093.00        |
| 77    | C♯7 / D♭7     | 2217.46        |
| 78    | D7            | 2349.32        |
| 79    | D♯7 / E♭7     | 2489.02        |
| 80    | E7            | 2637.02        |
| 81    | F7            | 2793.83        |
| 82    | F♯7 / G♭7     | 2959.96        |
| 83    | G7            | 3135.96        |
| 84    | G♯7 / A♭7     | 3322.44        |
| 85    | A7            | 3520.00        |
| 86    | A♯7 / B♭7     | 3729.31        |
| 87    | B7            | 3951.07        |
| 88    | C8            | 4186.01        |

---

### Example C++ Code to Print the Keys

Below is a simple C++ snippet that stores the note names and frequencies (for the 88-key piano) and prints them:

```cpp
#include <iostream>
#include <vector>
#include <string>

struct PianoKey {
    std::string note;
    double frequency;
};

int main() {
    std::vector<PianoKey> keys = {
        {"A0", 27.50}, {"A#/Bb0", 29.14}, {"B0", 30.87},
        {"C1", 32.70}, {"C#/Db1", 34.65}, {"D1", 36.71},
        {"D#/Eb1", 38.89}, {"E1", 41.20}, {"F1", 43.65},
        {"F#/Gb1", 46.25}, {"G1", 48.99}, {"G#/Ab1", 51.91},
        {"A1", 55.00}, {"A#/Bb1", 58.27}, {"B1", 61.74},
        {"C2", 65.41}, {"C#/Db2", 69.30}, {"D2", 73.42},
        {"D#/Eb2", 77.78}, {"E2", 82.41}, {"F2", 87.31},
        {"F#/Gb2", 92.50}, {"G2", 97.99}, {"G#/Ab2", 103.83},
        {"A2", 110.00}, {"A#/Bb2", 116.54}, {"B2", 123.47},
        {"C3", 130.81}, {"C#/Db3", 138.59}, {"D3", 146.83},
        {"D#/Eb3", 155.56}, {"E3", 164.81}, {"F3", 174.61},
        {"F#/Gb3", 184.99}, {"G3", 195.99}, {"G#/Ab3", 207.65},
        {"A3", 220.00}, {"A#/Bb3", 233.08}, {"B3", 246.94},
        {"C4", 261.63}, {"C#/Db4", 277.18}, {"D4", 293.66},
        {"D#/Eb4", 311.13}, {"E4", 329.63}, {"F4", 349.23},
        {"F#/Gb4", 369.99}, {"G4", 391.99}, {"G#/Ab4", 415.30},
        {"A4", 440.00}, {"A#/Bb4", 466.16}, {"B4", 493.88},
        {"C5", 523.25}, {"C#/Db5", 554.37}, {"D5", 587.33},
        {"D#/Eb5", 622.25}, {"E5", 659.26}, {"F5", 698.46},
        {"F#/Gb5", 739.99}, {"G5", 783.99}, {"G#/Ab5", 830.61},
        {"A5", 880.00}, {"A#/Bb5", 932.33}, {"B5", 987.77},
        {"C6", 1046.50}, {"C#/Db6", 1108.73}, {"D6", 1174.66},
        {"D#/Eb6", 1244.51}, {"E6", 1318.51}, {"F6", 1396.91},
        {"F#/Gb6", 1479.98}, {"G6", 1567.98}, {"G#/Ab6", 1661.22},
        {"A6", 1760.00}, {"A#/Bb6", 1864.66}, {"B6", 1975.53},
        {"C7", 2093.00}, {"C#/Db7", 2217.46}, {"D7", 2349.32},
        {"D#/Eb7", 2489.02}, {"E7", 2637.02}, {"F7", 2793.83},
        {"F#/Gb7", 2959.96}, {"G7", 3135.96}, {"G#/Ab7", 3322.44},
        {"A7", 3520.00}, {"A#/Bb7", 3729.31}, {"B7", 3951.07},
        {"C8", 4186.01}
    };

    for (size_t i = 0; i < keys.size(); ++i) {
        std::cout << "Key " << i+1 << ": " 
                  << keys[i].note << " - " 
                  << keys[i].frequency << " Hz" 
                  << std::endl;
    }

    return 0;
}
```

---

### Summary

- A “medium size piano” is generally a standard 88‑key instrument.
- The keys run from A0 (27.50 Hz) to C8 (4186.01 Hz).
- The table (or the code above) shows each key’s note name along with its typical tuned frequency (assuming standard equal temperament tuning with A4 = 440 Hz).

This should provide you with a complete overview and a way to programmatically list the available keys on an 88‑key piano.