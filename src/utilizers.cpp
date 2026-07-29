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

    if (!regPath.empty()) {
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
        "E:\\SteamLibrary\\steamapps\\common\\wallpaper_engine\\wallpaper64.exe",
        "E:\\steam\\steamapps\\common\\wallpaper_engine\\wallpaper64.exe"
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

    std::vector<char> cmdBuf(command.begin(), command.end());
    cmdBuf.push_back('\0');

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcessA(NULL, cmdBuf.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

std::string Base64Encode(const std::string& in) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

LcuInfo GetLcuInfo() {
    LcuInfo info;
    const std::string lockfilePaths[] = {
        "C:\\Riot Games\\League of Legends\\lockfile",
        "D:\\Riot Games\\League of Legends\\lockfile",
        "E:\\Riot Games\\League of Legends\\lockfile",
        "C:\\Program Files\\Riot Games\\League of Legends\\lockfile"
    };

    std::string foundPath = "";
    for (const auto& path : lockfilePaths) {
        if (fs::exists(path)) {
            foundPath = path;
            break;
        }
    }

    if (foundPath.empty()) return info;

    std::ifstream file(foundPath);
    if (!file.is_open()) return info;

    std::string content;
    std::getline(file, content);
    file.close();

    std::vector<std::string> parts;
    std::string token;
    std::stringstream ss(content);
    while (std::getline(ss, token, ':')) {
        parts.push_back(token);
    }

    if (parts.size() >= 4) {
        info.port = parts[2];
        info.password = parts[3];
        info.isValid = true;
    }

    return info;
}

std::string HttpGetLcu(const LcuInfo& lcu, const std::string& endpoint) {
    if (!lcu.isValid) return "";

    HINTERNET hInternet = InternetOpenA("LeaguePaper", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hConnect = InternetConnectA(hInternet, "127.0.0.1", (INTERNET_PORT)std::stoi(lcu.port), NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return "";
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", endpoint.c_str(), NULL, NULL, NULL,
        INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, 0);
    
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "";
    }

    DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                    SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                    SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                    SECURITY_FLAG_IGNORE_REVOCATION;
    InternetSetOptionA(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

    std::string authHeader = "Authorization: Basic " + Base64Encode("riot:" + lcu.password) + "\r\n";
    HttpAddRequestHeadersA(hRequest, authHeader.c_str(), (DWORD)authHeader.length(), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);

    BOOL bSent = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);
    std::string response = "";

    if (bSent) {
        char buffer[4096];
        DWORD bytesRead = 0;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
}

bool DownloadFile(const std::string& url, const std::string& destPath) {
    HINTERNET hInternet = InternetOpenA("LeaguePaper", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hFile = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (!hFile) {
        InternetCloseHandle(hInternet);
        return false;
    }

    fs::path p(destPath);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }

    std::ofstream out(destPath, std::ios::binary);
    if (!out.is_open()) {
        InternetCloseHandle(hFile);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[8192];
    DWORD bytesRead = 0;
    while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        out.write(buffer, bytesRead);
    }

    out.close();
    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return true;
}

std::string GetChampionAlias(int champId) {
    static std::unordered_map<int, std::string> aliasMap;
    if (aliasMap.empty()) {
        fs::path cacheDir = fs::current_path() / "cache";
        fs::create_directories(cacheDir);
        fs::path summaryPath = cacheDir / "champion-summary.json";

        if (!fs::exists(summaryPath)) {
            DownloadFile("https://raw.communitydragon.org/latest/plugins/rcp-be-lol-game-data/global/default/v1/champion-summary.json", summaryPath.string());
        }

        std::ifstream f(summaryPath);
        if (f.is_open()) {
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            size_t pos = 0;
            while ((pos = content.find("\"id\":", pos)) != std::string::npos) {
                size_t idStart = pos + 5;
                int id = std::stoi(content.substr(idStart));
                size_t aliasPos = content.find("\"alias\":\"", pos);
                if (aliasPos != std::string::npos && aliasPos < pos + 400) {
                    size_t strStart = aliasPos + 9;
                    size_t strEnd = content.find("\"", strStart);
                    std::string alias = content.substr(strStart, strEnd - strStart);
                    aliasMap[id] = alias;
                }
                pos = idStart;
            }
        }
    }

    auto it = aliasMap.find(champId);
    if (it != aliasMap.end()) {
        return it->second;
    }
    return "";
}

bool FetchAndApplySplashArt(int champId, int skinId, const std::string& destPath) {
    std::string alias = GetChampionAlias(champId);
    if (alias.empty()) return false;

    int skinIndex = skinId > 0 ? (skinId % 1000) : 0;

    // 1. Try Data Dragon Skin Splash
    std::string ddragonUrl = "https://ddragon.leagueoflegends.com/cdn/img/champion/splash/" + alias + "_" + std::to_string(skinIndex) + ".jpg";
    std::cout << "[WALLPAPER] Trying Data Dragon splash: " << ddragonUrl << std::endl;
    if (DownloadFile(ddragonUrl, destPath)) {
        if (fs::exists(destPath) && fs::file_size(destPath) > 5000) {
            return true;
        }
    }

    // 2. Try Data Dragon Base Splash (fallback)
    if (skinIndex != 0) {
        std::string ddragonBaseUrl = "https://ddragon.leagueoflegends.com/cdn/img/champion/splash/" + alias + "_0.jpg";
        std::cout << "[WALLPAPER] Skin splash failed, trying Base splash: " << ddragonBaseUrl << std::endl;
        if (DownloadFile(ddragonBaseUrl, destPath)) {
            if (fs::exists(destPath) && fs::file_size(destPath) > 5000) {
                return true;
            }
        }
    }

    return false;
}

static int ExtractJsonInt(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return 0;
    size_t colon = json.find(":", pos);
    if (colon == std::string::npos) return 0;
    size_t start = json.find_first_of("0123456789-", colon);
    if (start == std::string::npos) return 0;
    return std::stoi(json.substr(start));
}

SelectionInfo GetCurrentSelection(const LcuInfo& lcu) {
    SelectionInfo sel;
    if (!lcu.isValid) return sel;

    std::string phaseRes = HttpGetLcu(lcu, "/lol-gameflow/v1/gameflow-phase");
    if (!phaseRes.empty()) {
        size_t first = phaseRes.find_first_not_of("\" \r\n");
        size_t last = phaseRes.find_last_not_of("\" \r\n");
        if (first != std::string::npos && last != std::string::npos) {
            sel.phase = phaseRes.substr(first, last - first + 1);
        }
    }

    if (sel.phase == "ChampSelect") {
        std::string selRes = HttpGetLcu(lcu, "/lol-champ-select/v1/session/my-selection");
        if (!selRes.empty()) {
            sel.championId = ExtractJsonInt(selRes, "championId");
            sel.championPickIntent = ExtractJsonInt(selRes, "championPickIntent");
            sel.selectedSkinId = ExtractJsonInt(selRes, "selectedSkinId");
        }
    }

    return sel;
}

std::string GetCustomWallpaper(int championId, int skinId) {
    fs::path customDir = fs::current_path() / "custom";
    if (!fs::exists(customDir)) return "";

    const std::string exts[] = { ".mp4", ".pkg", ".jpg", ".png" };

    if (skinId > 0) {
        for (const auto& ext : exts) {
            fs::path p1 = customDir / (std::to_string(skinId) + ext);
            if (fs::exists(p1)) return fs::absolute(p1).string();

            fs::path p2 = customDir / "skins" / (std::to_string(skinId) + ext);
            if (fs::exists(p2)) return fs::absolute(p2).string();
        }
    }

    if (championId > 0) {
        for (const auto& ext : exts) {
            fs::path p3 = customDir / (std::to_string(championId) + ext);
            if (fs::exists(p3)) return fs::absolute(p3).string();

            fs::path p4 = customDir / "champions" / (std::to_string(championId) + ext);
            if (fs::exists(p4)) return fs::absolute(p4).string();
        }
    }

    return "";
}