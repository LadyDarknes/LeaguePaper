#include <iostream>
#include <thread>
#include <chrono>
#include "utilizers.hpp"

namespace fs = std::filesystem;

int main() 
{
    std::string wePath = FindWallpaperEnginePath();
    if (wePath.empty()) {
        std::cerr << "Error: wallpaper64.exe not found." << std::endl;
        return 1;
    }

    fs::path webDir = fs::current_path() / "web";
    fs::create_directories(webDir);
    fs::path htmlPath = fs::absolute(webDir / "index.html");
    fs::path bgPath = fs::absolute(webDir / "bg.jpg");

    int monitorIndex = 1;
    int activeSkin = -1;

    std::cout << "LeaguePaper started. Listening to LCU..." << std::endl;

    while (true) {
        LcuInfo lcu = GetLcuInfo();
        if (!lcu.isValid) {
            if (activeSkin != 0) {
                std::cout << "Waiting for League Client..." << std::endl;
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
                std::cout << "ChampSelect update -> Champ: " << targetChamp << " | Skin: " << targetSkin << std::endl;

                std::string customPath = GetCustomWallpaper(targetChamp, targetSkin);
                if (!customPath.empty()) {
                    std::cout << "Applying custom wallpaper: " << customPath << std::endl;
                    ChangeWallpaper(wePath, customPath, monitorIndex);
                } else {
                    if (FetchAndApplySplashArt(targetChamp, targetSkin, bgPath.string())) {
                        std::cout << "Applying live wallpaper..." << std::endl;
                        ChangeWallpaper(wePath, htmlPath.string(), monitorIndex);
                    }
                }

                activeSkin = targetSkin;
            }
        } else {
            if (activeSkin != -1 && activeSkin != 0) {
                activeSkin = -1;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    return 0;
}