# LeaguePaper

Bridge utility for League of Legends (LCU) and Wallpaper Engine.

## Overview
LeaguePaper monitors the League of Legends client phase during Champion Select and dynamically updates desktop wallpapers via Wallpaper Engine.

## Features
- **LCU Auto-Discovery:** Parses LCU `lockfile` to establish authenticated HTTPS requests.
- **ChampSelect Event Listener:** Detects hovered and locked champions/skins in real-time.
- **Custom Wallpaper Overrides:** Prioritizes local/drive media files (`custom/` directory).
- **Riot Data Dragon Fallback:** Automatically fetches HD splash art and renders it with a lightweight Canvas particle snow effect.
- **Zero Third-Party Dependencies:** Built using standard C++20 and native Windows WinINet API.

## Building
Run `build.bat` using CMake and Ninja:
```cmd
.\build.bat
```
