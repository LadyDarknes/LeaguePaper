#include <iostream>
#include "utilizers.hpp"

int main() 
{
    std::string wePath = FindWallpaperEnginePath();

    std::string wallpaperPath = "E:\\steam\\steamapps\\workshop\\content\\431960\\3568154865";
    ChangeWallpaper(wePath, wallpaperPath, -1);
    return 0;
}