#include "ThemeService.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"
#include <cstdlib>
#include <cstring>

ThemeService::ThemeService() {
}

ThemeService::~ThemeService() {
}

std::vector<std::string> ThemeService::DiscoverThemesInPath(const char *dirPath) {
    std::vector<std::string> themes;
    
    Path path(dirPath);
    I_Dir *idir = FileSystem::GetInstance()->Open(path.GetPath().c_str());
    if (!idir) {
        return themes;
    }

    idir->GetContent((char *)"*");
    idir->Sort();
    IteratorPtr<Path> it(idir->GetIterator());
    for (it->Begin(); !it->IsDone(); it->Next()) {
        Path &current = it->CurrentItem();
        std::string name = current.GetName();

        // Skip empty names and hidden files
        if (name.empty() || name[0] == '.')
            continue;

        // Remove trailing .xml (case-insensitive)
        if (name.size() > 4) {
            std::string lower = name;
            for (size_t i = 0; i < lower.size(); ++i)
                lower[i] = (char)tolower((unsigned char)lower[i]);
            if (lower.compare(lower.size() - 4, 4, ".xml") == 0) {
                name = name.substr(0, name.size() - 4);
            }
        }

        themes.push_back(name);
    }
    delete idir;
    
    return themes;
}

bool ThemeService::LoadThemes(Variable *themeVar) {
    if (!themeVar) {
        Trace::Log("ThemeService", "Invalid theme variable pointer");
        return false;
    }

    // Try common theme directory locations
    const char *candidates[] = {
        "root:themes",
        "root:resources/themes",
        "bin:themes",
        "bin:resources/themes",
        NULL
    };

    std::vector<std::string> themeNames;
    for (int i = 0; candidates[i]; ++i) {
        themeNames = DiscoverThemesInPath(candidates[i]);
        if (!themeNames.empty()) {
            Trace::Log("ThemeService", "Found %d themes in %s", 
                      (int)themeNames.size(), candidates[i]);
            break;
        }
    }

    if (themeNames.empty()) {
        Trace::Log("ThemeService", "No themes found in any candidate directory");
        return false;
    }

    // Build char** list for Variable
    int count = (int)themeNames.size();
    char **list = (char **)malloc(sizeof(char *) * count);
    for (int i = 0; i < count; ++i) {
        list[i] = (char *)malloc(themeNames[i].size() + 1);
        strcpy(list[i], themeNames[i].c_str());
    }

    // Preserve current index if valid, otherwise reset to 0
    int curIndex = themeVar->GetInt();
    if (curIndex < 0 || curIndex >= count)
        curIndex = 0;

    // Create temp variable with the new list and copy to existing variable
    Variable tempVar("theme", themeVar->GetID(), list, count, curIndex);
    themeVar->CopyFrom(tempVar);
    
    // Note: ownership of allocated list is intentionally left to the 
    // variable lifetime for persistence

    Trace::Log("ThemeService", "Successfully loaded %d themes", count);
    return true;
}
