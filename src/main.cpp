#include <iostream>
#include <thread>
#include <chrono>
#include "utilizers.hpp"

namespace fs = std::filesystem;

int main() 
{
    std::cout << "========================================" << std::endl;
    std::cout << "          LEAGUE PAPER BRIDGE           " << std::endl;
    std::cout << "========================================" << std::endl;

    std::string wePath = FindWallpaperEnginePath();
    if (wePath.empty()) {
        std::cerr << "[ERROR] Wallpaper Engine (wallpaper64.exe) not found!" << std::endl;
        std::cerr << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "[SUCCESS] Wallpaper Engine found: " << wePath << std::endl;

    // Create web directory if missing
    fs::path webDir = fs::current_path() / "web";
    fs::create_directories(webDir);
    fs::path htmlPath = fs::absolute(webDir / "index.html");
    fs::path bgPath = fs::absolute(webDir / "bg.jpg");

    int monitorIndex = 1; // Default to monitor 1 (second monitor), change as needed
    int activeSkin = -1;

    std::cout << "[INFO] Listening for League of Legends Client (LCU)..." << std::endl;

    while (true) {
        LcuInfo lcu = GetLcuInfo();
        if (!lcu.isValid) {
            if (activeSkin != 0) {
                std::cout << "[STATUS] League Client not running. Waiting..." << std::endl;
                activeSkin = 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            continue;
        }

        SelectionInfo sel = GetCurrentSelection(lcu);

        if (sel.phase == "ChampSelect") {
            int targetChamp = sel.championId != 0 ? sel.championId : sel.championPickIntent;
            int targetSkin = sel.selectedSkinId != 0 ? sel.selectedSkinId : (targetChamp * 1000);

            if (targetSkin > 0 && targetSkin != activeSkin) {
                std::cout << "[EVENT] ChampSelect Active -> Champion ID: " << targetChamp 
                          << " | Skin ID: " << targetSkin << std::endl;

                // 1. Check Custom/Drive Wallpaper
                std::string customPath = GetCustomWallpaper(targetChamp, targetSkin);
                if (!customPath.empty()) {
                    std::cout << "[WALLPAPER] Found Custom Wallpaper: " << customPath << std::endl;
                    ChangeWallpaper(wePath, customPath, monitorIndex);
                } else {
                    // 2. Fallback to Data Dragon / CommunityDragon Splash + Web Snow Canvas
                    if (FetchAndApplySplashArt(targetChamp, targetSkin, bgPath.string())) {
                        std::cout << "[WALLPAPER] Applying HTML Snow Canvas Wallpaper..." << std::endl;
                        ChangeWallpaper(wePath, htmlPath.string(), monitorIndex);
                    } else {
                        std::cerr << "[ERROR] Failed to fetch splash art!" << std::endl;
                    }
                }

                activeSkin = targetSkin;
            }
        } else {
            if (activeSkin != -1 && activeSkin != 0) {
                std::cout << "[STATUS] Phase: " << sel.phase << " (Not in ChampSelect)" << std::endl;
                activeSkin = -1;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    return 0;
}