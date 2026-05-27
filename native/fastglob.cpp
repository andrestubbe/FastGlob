#include "fastglob.h"
#include <windows.h>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <sstream>

// ============================================================================
// DLL Entry Point
// ============================================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

// Helper to convert jstring to std::wstring
std::wstring jstringToWString(JNIEnv* env, jstring jstr) {
    if (!jstr) return L"";
    const jchar* jchars = env->GetStringChars(jstr, nullptr);
    jsize len = env->GetStringLength(jstr);
    std::wstring wstr(reinterpret_cast<const wchar_t*>(jchars), len);
    env->ReleaseStringChars(jstr, jchars);
    return wstr;
}

// Convert std::wstring to jstring
jstring wstringToJString(JNIEnv* env, const std::wstring& wstr) {
    return env->NewString(reinterpret_cast<const jchar*>(wstr.data()), static_cast<jsize>(wstr.length()));
}

// Check if character is path separator
inline bool isSlash(wchar_t c) {
    return c == L'/' || c == L'\\';
}

// Simple wildcard match function
// Supports:
// - `*` matches any sequence of characters within a path component (doesn't cross `/` or `\`)
// - `?` matches a single character (doesn't cross slash)
// - `**` matches any sequence of characters including path separators
bool matchPatternPart(const wchar_t* pat, const wchar_t* str) {
    if (!*pat && !*str) return true;
    
    // ** matches anything recursively
    if (pat[0] == L'*' && pat[1] == L'*') {
        const wchar_t* nextPat = pat + 2;
        if (isSlash(*nextPat)) nextPat++; // skip separator
        if (!*nextPat) return true; // trailing ** matches everything
        
        // Try matching nextPat at any position of str
        const wchar_t* s = str;
        while (*s) {
            if (matchPatternPart(nextPat, s)) return true;
            s++;
        }
        return false;
    }
    
    if (*pat == L'*') {
        // * matches any characters EXCEPT path separators
        while (*str && !isSlash(*str)) {
            if (matchPatternPart(pat + 1, str)) return true;
            str++;
        }
        return matchPatternPart(pat + 1, str);
    }
    
    if (*pat == L'?') {
        if (!*str || isSlash(*str)) return false;
        return matchPatternPart(pat + 1, str + 1);
    }
    
    if (isSlash(*pat) && isSlash(*str)) {
        return matchPatternPart(pat + 1, str + 1);
    }
    
    if (*pat == *str) {
        if (!*pat) return true;
        return matchPatternPart(pat + 1, str + 1);
    }
    
    return false;
}

// Expand brace patterns like "src/**/*.{cpp,h}" into ["src/**/*.cpp", "src/**/*.h"]
std::vector<std::wstring> expandBraces(const std::wstring& pattern) {
    std::vector<std::wstring> results;
    size_t openBrace = pattern.find(L'{');
    if (openBrace == std::wstring::npos) {
        results.push_back(pattern);
        return results;
    }
    
    size_t closeBrace = pattern.find(L'}', openBrace);
    if (closeBrace == std::wstring::npos) {
        results.push_back(pattern);
        return results;
    }
    
    std::wstring prefix = pattern.substr(0, openBrace);
    std::wstring suffix = pattern.substr(closeBrace + 1);
    std::wstring inner = pattern.substr(openBrace + 1, closeBrace - openBrace - 1);
    
    // Split by comma
    std::vector<std::wstring> parts;
    std::wstring part;
    std::wistringstream ss(inner);
    while (std::getline(ss, part, L',')) {
        parts.push_back(part);
    }
    
    for (const auto& p : parts) {
        std::wstring expanded = prefix + p + suffix;
        // Recursively expand if there are more braces
        auto subExpanded = expandBraces(expanded);
        results.insert(results.end(), subExpanded.begin(), subExpanded.end());
    }
    
    return results;
}

// Normalize path separators to forward slash
std::wstring normalizePath(const std::wstring& path) {
    std::wstring normalized = path;
    for (auto& c : normalized) {
        if (c == L'\\') c = L'/';
    }
    return normalized;
}

struct GlobConfig {
    std::vector<std::wstring> includePatterns;
    std::vector<std::wstring> excludePatterns;
};

// Check if a normalized relative path matches the glob config
bool isMatch(const std::wstring& relPath, const GlobConfig& config) {
    std::wstring normPath = normalizePath(relPath);
    
    // Check exclude patterns first
    for (const auto& pat : config.excludePatterns) {
        if (matchPatternPart(pat.c_str(), normPath.c_str())) {
            return false;
        }
    }
    
    // Check include patterns
    for (const auto& pat : config.includePatterns) {
        if (matchPatternPart(pat.c_str(), normPath.c_str())) {
            return true;
        }
    }
    
    return false;
}

// Recursive directory traversal using Win32 API
void traverseDirectory(const std::wstring& baseDir, const std::wstring& currentSubDir, const GlobConfig& config, std::vector<std::wstring>& matches) {
    std::wstring searchPath = baseDir;
    if (!currentSubDir.empty()) {
        searchPath += L"\\" + currentSubDir;
    }
    searchPath += L"\\*";
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    do {
        std::wstring name = findData.cFileName;
        if (name == L"." || name == L"..") {
            continue;
        }
        
        std::wstring relativePath = currentSubDir.empty() ? name : currentSubDir + L"\\" + name;
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recurse into directories
            traverseDirectory(baseDir, relativePath, config, matches);
        } else {
            // Match file
            if (isMatch(relativePath, config)) {
                matches.push_back(relativePath);
            }
        }
    } while (FindNextFileW(hFind, &findData));
    
    FindClose(hFind);
}

// ============================================================================
// JNI Implementations
// ============================================================================

JNIEXPORT jobjectArray JNICALL Java_fastglob_FastGLOB_glob(JNIEnv* env, jclass clazz, jstring baseDir, jstring pattern) {
    std::wstring wBaseDir = jstringToWString(env, baseDir);
    std::wstring wPattern = jstringToWString(env, pattern);
    
    // Parse pattern
    GlobConfig config;
    
    // Expand braces
    std::vector<std::wstring> expanded = expandBraces(wPattern);
    
    for (const auto& pat : expanded) {
        if (pat.empty()) continue;
        
        std::wstring normPat = normalizePath(pat);
        if (normPat[0] == L'!') {
            config.excludePatterns.push_back(normPat.substr(1));
        } else {
            config.includePatterns.push_back(normPat);
        }
    }
    
    std::vector<std::wstring> matches;
    traverseDirectory(wBaseDir, L"", config, matches);
    
    // Create Java String array
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray resultArray = env->NewObjectArray(static_cast<jsize>(matches.size()), stringClass, nullptr);
    
    for (size_t i = 0; i < matches.size(); ++i) {
        jstring js = wstringToJString(env, normalizePath(matches[i]));
        env->SetObjectArrayElement(resultArray, static_cast<jsize>(i), js);
        env->DeleteLocalRef(js);
    }
    
    return resultArray;
}
