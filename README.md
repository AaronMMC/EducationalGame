# SynType Arcade

A competitive arcade typing combat game built with C++ and SFML.

---

## Features

- **4 Playable Avatars** with unique combat abilities
- **Multiplayer VS** — fight a CPU opponent with HP bars, damage, and multipliers
- **Single Player** — Score Attack and Endless Siege with Boss Battles every 10 waves
- **Score + Multiplier System** — combo streaks boost your multiplier; drop below threshold and lose it
- **VS Screen** — dramatic two-sided connection animation before each match
- **Profile** — customize callsign, accent color, active unit; track wins/losses/best stats
- **Settings** — music/sfx volume, time limit, scanlines, dark mode, glitch effects

---

## Avatar Abilities

| Avatar   | Tag        | Special                                                           |
|----------|------------|-------------------------------------------------------------------|
| PHOENIX  | COMEBACK   | Round 3 forces multiplier to minimum x3. Weak early, lethal late |
| BLAZE    | VOLATILE   | Gains multiplier fastest, but full reset below 90% accuracy      |
| VIRUS    | SABOTEUR   | Every 2s scrambles 5 random characters on the opponent's screen  |
| SENTINEL | HEAD START | Opponent cannot type for the first 4 seconds of each round       |

---

## Prerequisites

- **CMake** 3.20+
- **C++17** compiler: MSVC 2019+, GCC 9+, or Clang 10+
- **Git** (CMake FetchContent downloads SFML automatically)
- Internet connection on first build (to fetch SFML ~30MB)

---

## Build Instructions

### Windows (Visual Studio / MSVC)

```bat
git clone <your-repo-url> SynType
cd SynType
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Run from the `build/Release/` folder:
```bat
cd build\Release
SynType.exe
```

### Windows (MinGW / MSYS2)

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make -j4
./SynType
```

### Linux

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./SynType
```

### macOS

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.logicalcpu)
./SynType
```

---

## Fonts (Required for best visuals)

The game uses three fonts. Without them it falls back to system fonts
(Consolas on Windows, DejaVu on Linux, Menlo on macOS).

For the full arcade look, download these **free** fonts and place them
in an `assets/` folder next to your executable:

| File                        | Font            | Download                                         |
|-----------------------------|-----------------|--------------------------------------------------|
| `ShareTechMono-Regular.ttf` | Share Tech Mono | https://fonts.google.com/specimen/Share+Tech+Mono |
| `Orbitron-Bold.ttf`         | Orbitron        | https://fonts.google.com/specimen/Orbitron        |
| `Rajdhani-SemiBold.ttf`     | Rajdhani        | https://fonts.google.com/specimen/Rajdhani        |

### Quick download (Linux/macOS)

```bash
mkdir -p assets
# Install gfonts CLI or just wget the files:
curl -L "https://fonts.gstatic.com/s/sharetechmono/v15/J7aHnp1uDWRBEqV98dVQztYldFcLowEF.ttf" \
     -o assets/ShareTechMono-Regular.ttf
curl -L "https://fonts.gstatic.com/s/orbitron/v29/yMJMMIlzdpvBhQQL_SC3X9yhF25-T1nyGy6BoWgz.ttf" \
     -o assets/Orbitron-Bold.ttf
curl -L "https://fonts.gstatic.com/s/rajdhani/v15/LDI2apCSOBg7S-QT7pb4JMGgsB5T.ttf" \
     -o assets/Rajdhani-SemiBold.ttf
```

---

## Project Structure

```
SynType/
├── CMakeLists.txt
├── README.md
├── assets/               ← put fonts here
└── src/
    ├── main.cpp           ← window + screen manager loop
    ├── GameState.h/cpp    ← all shared state, damage, multiplier logic
    ├── Stats.h/cpp        ← WPM, accuracy, timer
    ├── Avatar.h/cpp       ← avatar definitions and tuning values
    ├── TextSnippets.h/cpp ← typing text pool (normal + boss)
    ├── NetworkSim.h/cpp   ← CPU opponent simulation (swap for real net)
    ├── UIHelpers.h/cpp    ← drawing primitives, colors, panels, HP bars
    └── Screens/
        ├── IScreen.h           ← base interface
        ├── TitleScreen.*       ← main menu
        ├── AvatarSelectScreen.*← choose your unit
        ├── VSScreen.*          ← connection animation + countdown
        ├── GameScreen.*        ← core typing gameplay
        ├── ResultScreen.*      ← match results
        ├── ProfileScreen.*     ← customize + stats
        ├── SettingsScreen.*    ← audio, time, visual toggles
        └── SPSelectScreen.*    ← Score Attack vs Endless select
```

---

## Adding Real Multiplayer

`NetworkSim` is a drop-in stub for a real network layer. To add LAN/online play:

1. Replace `NetworkSim::tick()` with a UDP/TCP receive loop
2. Send local `typedText.size()`, `stats.getWPM()`, `stats.getAccuracy()`,
   `multiplier` to the opponent each frame
3. Receive the same four values from the opponent
4. Virus sabotage: send the `obfuscatedIndices` set over the network
5. Sentinel freeze: send a `frozenUntil` timestamp at match start

Recommended libraries: **ENet** (reliable UDP), **SFML Network** (TCP), or **GameNetworkingSockets**.

---

## Snippets File

You can add custom typing texts by editing `TextSnippets.cpp`.
The `normal()` pool is used in standard rounds; `boss()` is used in boss waves.
Keep snippets under ~200 characters for comfortable single-screen display.

---

## Controls

| Key         | Action                          |
|-------------|---------------------------------|
| Any key     | Start typing / begin round      |
| Backspace   | Delete last character           |
| Tab         | Insert 4 spaces                 |
| Enter       | Insert newline                  |
| Escape      | Return to previous screen       |

---

## License

MIT — do whatever you want with it.
