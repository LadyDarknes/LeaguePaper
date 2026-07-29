#ifndef UTILIZERS_H
#define UTILIZERS_H

#include <string>
#include <windows.h>
#include <wininet.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>

struct LcuInfo {
    std::string port;
    std::string password;
    bool isValid = false;
};

struct SelectionInfo {
    int championId = 0;
    int championPickIntent = 0;
    int selectedSkinId = 0;
    std::string phase = "None";
};

std::string ReadRegistryString(HKEY hKeyParent, const std::string& subKey, const std::string& valueName);
std::string FindWallpaperEnginePath();
bool ChangeWallpaper(const std::string& wallpaperEnginePath, const std::string& wallpaperPath, int monitorIndex);

LcuInfo GetLcuInfo();
std::string HttpGetLcu(const LcuInfo& lcu, const std::string& endpoint);
bool DownloadFile(const std::string& url, const std::string& destPath);
std::string GetChampionAlias(int champId);
bool FetchAndApplySplashArt(int champId, int skinId, const std::string& destPath);
std::string GetCustomWallpaper(int championId, int skinId);
SelectionInfo GetCurrentSelection(const LcuInfo& lcu);
std::string Base64Encode(const std::string& in);

#endif