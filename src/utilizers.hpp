#ifndef UTILIZERS_H
#define UTILIZERS_H

#include <string>
#include <windows.h>
#include <filesystem>

std::string ReadRegistryString(HKEY hKeyParent, const std::string& subKey, const std::string& valueName);
std::string FindWallpaperEnginePath();
bool ChangeWallpaper(const std::string& wallpaperEnginePath, const std::string& wallpaperPath, int monitorIndex =  -1);

#endif