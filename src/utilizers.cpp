#include "utilizers.hpp"

namespace fs = std::filesystem;

std::string ReadRegistryString(HKEY hKeyParent, const std::string& subKey, const std::string& valueName) {
    HKEY hKey;
    if (RegOpenKeyExA(hKeyParent, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return "";
    }

    char buffer[MAX_PATH];
    DWORD dataSize = sizeof(buffer);
    DWORD type = REG_SZ;

    LONG result = RegQueryValueExA(hKey, valueName.c_str(), NULL, &type, (LPBYTE)buffer, &dataSize);
    RegCloseKey(hKey);

    if (result == ERROR_SUCCESS) {
        return std::string(buffer);
    }
    return "";
}

std::string FindWallpaperEnginePath() {
    std::string regPath = ReadRegistryString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 431960", "InstallLocation");

    if (!regPath.empty()) 
    {
        std::string exePath = regPath + "\\wallpaper64.exe";
        if (fs::exists(exePath)) return exePath;
    }

    regPath = ReadRegistryString(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 431960", "InstallLocation");

    if (!regPath.empty()) {
        std::string exePath = regPath + "\\wallpaper64.exe";
        if (fs::exists(exePath)) return exePath;
    }

    const std::string defaultPaths[] = {
        "C:\\Program Files (x86)\\Steam\\steamapps\\common\\wallpaper_engine\\wallpaper64.exe",
        "C:\\Program Files\\Steam\\steamapps\\common\\wallpaper_engine\\wallpaper64.exe",
        "D:\\SteamLibrary\\steamapps\\common\\wallpaper_engine\\wallpaper64.exe",
        "E:\\SteamLibrary\\steamapps\\common\\wallpaper_engine\\wallpaper64.exe"
    };

    for (const auto& path : defaultPaths) {
        if (fs::exists(path)) {
            return path;
        }
    }

    return "";
}

bool ChangeWallpaper(const std::string& wallpaperEnginePath, const std::string& wallpaperPath, int monitorIndex) {
    std::string command = "\"" + wallpaperEnginePath + "\" -control openWallpaper -file \"" + wallpaperPath + "\"";
    if (monitorIndex >= 0) {
        command += " -monitor " + std::to_string(monitorIndex);
    }

    char* cmdarg = &command[0];
    // CreateProcessA shits, I dont even know what the fuck happening here...
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));
    // end of CreateProcessA shits
    CreateProcessA(NULL, cmdarg, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}