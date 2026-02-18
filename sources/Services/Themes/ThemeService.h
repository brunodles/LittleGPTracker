#ifndef _THEME_SERVICE_H_
#define _THEME_SERVICE_H_

#include "Foundation/T_Factory.h"
#include "Foundation/Variables/Variable.h"
#include <vector>
#include <string>

/**
 * @class ThemeService
 * @brief Service for discovering and managing theme directories.
 *
 * Scans common theme locations and populates the theme variable list
 * with available theme names.
 */
class ThemeService : public T_Factory<ThemeService> {
public:
    ThemeService();
    virtual ~ThemeService();

    /**
     * Scans theme directories and populates the given Variable with theme names.
     * Searches in order: root:themes, root:resources/themes, bin:themes, bin:resources/themes.
     * Stops at the first directory containing themes.
     *
     * @param themeVar Pointer to the Variable to populate with theme names.
     * @return true if themes were found and loaded, false otherwise.
     */
    bool LoadThemes(Variable *themeVar);

private:
    /**
     * Discovers theme names from a single directory path.
     * Filters out hidden files (starting with dot).
     *
     * @param dirPath Directory path to scan.
     * @return Vector of discovered theme names, empty if directory not found or empty.
     */
    std::vector<std::string> DiscoverThemesInPath(const char *dirPath);
};

#endif
