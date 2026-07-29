#include <iostream>
#include "utilizers.hpp"

int main() 
{
    std::string wePath = FindWallpaperEnginePath();

    std::string wallpaperPath = "E:\\steam\\steamapps\\workshop\\content\\431960\\3568154865\\project.json";
    ChangeWallpaper(wePath, wallpaperPath, 1); // 0 first, 1, second monitor
    return 0;
}