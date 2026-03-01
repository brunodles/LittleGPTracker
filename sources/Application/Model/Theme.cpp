#include "Theme.h"
#include "Application/Utils/char.h"

/**
 * Compares two strings for equality.
 * @param key The first string to compare.
 * @param value The second string to compare.
 * @param caseSensitive Whether the comparison should be case-sensitive
 * (default: false for case-insensitive).
 * @return true if both strings are equal, false otherwise. Returns false if
 * either string is null.
 */
inline bool isKeyEqualTo(const char *key, const char *value,
                         bool caseSensitive = false) {
    if (!key || !value)
        return false;
    if (caseSensitive) {
        return strcmp(key, value) == 0;
    }
    return strcasecmp(key, value) == 0;
}

/**
 * Converts a string to a boolean value.
 * @param value The string to convert. Can be null or empty.
 * @return true if value starts with 'Y', 'y', 'T', or 't', or equals "YES" or
 * "TRUE" (case-insensitive). false otherwise.
 */
inline bool strToBool(const char *value) {
    if (!value)
        return false;
    char firstChar = tolower(value[0]);
    if (firstChar == '1' || firstChar == 'y' || firstChar == 't')
        return true;
    if (strcasecmp(value, "yes") == 0)
        return true;
    if (strcasecmp(value, "true") == 0)
        return true;
    return false;
}

/**
 * Converts a string to an integer value.
 * @param value The string to convert. Can be null or empty.
 * @return The integer value parsed from the string using atoi().
 *         Returns 0 if the string is null, empty, or not a valid integer.
 */
inline int strToInt(const char *value) { return atoi(value); }

inline GUIColor strToColor(const char *value) {
    if (value) {
        unsigned char r;
        char2hex(value, &r);
        unsigned char g;
        char2hex(value + 2, &g);
        unsigned char b;
        char2hex(value + 4, &b);
        return GUIColor(r, g, b);
    }
    return GUIColor(0, 0, 0);
}

inline void defineColor(const char *value, GUIColor* color) {
    if (value) {
        unsigned char r, g, b;
        char2hex(value, &r);
        char2hex(value + 2, &g);
        char2hex(value + 4, &b);
        color->_r = r;
        color->_g = g;
        color->_b = b;
        //color = new GUIColor(r, g, b);
    }
}

Theme::Theme() {
    // initialize default values

    // GUIColor AppWindow::consoleColor_(0x00, 0xFF, 0x00);
    bgColor = new GUIColor(0x1D, 0x0A, 0x1F);
    fgColor = new GUIColor(0xF5, 0xEB, 0xFF);
    borderColor = new GUIColor(0xFF, 0x00, 0x8C);
    hiColor1 = new GUIColor(0xB7, 0x50, 0xD1);
    hiColor2 = new GUIColor(0xDB, 0x33, 0xDB);
    cursorColor = new GUIColor(0xFF, 0x00, 0x8C);
    playColor = new GUIColor(0xFF, 0x00, 0x8C);
    muteColor = new GUIColor(0xF5, 0xEB, 0xFF);
    songviewFe = new GUIColor(0xA5, 0x5B, 0x8F);
    songview00 = new GUIColor(0x85, 0x3B, 0x6F);
    rowColor1 = new GUIColor(0xBA, 0x28, 0xF9);
    rowColor2 = new GUIColor(0xFF, 0x00, 0xFF);
    altRowNumber = 4;
    majorBeatNumber = 4;
    majorBeatColor = new GUIColor(0xBA, 0x28, 0xF9);
    blankSpaceColor = new GUIColor(0xF5, 0xEB, 0xFF);
    showColumnTitles = true;
    fontType;

    // Theme
    altRowNumber = 4;
    majorBeatNumber = 4;
    showColumnTitles = false;

    // load from Theme file

    Path path("bin:Theme.xml");
    Trace::Log("Theme", "Got Theme path=%s", path.GetPath().c_str());
    TiXmlDocument *document = new TiXmlDocument(path.GetPath());
    bool loadOkay = document->LoadFile();

    if (loadOkay) {
        // Check first node is Theme/ GPTheme

        TiXmlNode *rootnode = 0;

        rootnode = document->FirstChild("Theme");
        if (!rootnode) {
            rootnode = document->FirstChild("GPTheme");
        }

        if (rootnode) {
            TiXmlElement *rootelement = rootnode->ToElement();
            TiXmlNode *node = rootelement->FirstChildElement();

            // Loop on all children

            if (node) {
                TiXmlElement *element = node->ToElement();
                while (element) {
                    const char *key = element->Value();
                    const char *value = element->Attribute("value");
                    if (!value) {
                        value = element->Attribute("VALUE");
                    }
                    if (key && value) {
                        if (proccessKeyValue(key, value)) {
                            // do nothing
                        }
                    }
                    element = element->NextSiblingElement();
                }
            }
        }
    } else {
        Trace::Log("Theme", "No (bad?) Theme.xml");
    }
    delete (document);
}

bool Theme::proccessKeyValue(const char *key, const char *value) {
    if (isKeyEqualTo(key, "BACKGROUND")) {
        defineColor(value, bgColor);
    } else if (isKeyEqualTo(key, "FOREGROUND")) {
        defineColor(value, fgColor);
    } else if (isKeyEqualTo(key, "BORDER")) {
        defineColor(value, borderColor);
    } else if (isKeyEqualTo(key, "HICOLOR1")) {
        defineColor(value, hiColor1);
    } else if (isKeyEqualTo(key, "HICOLOR2")) {
        defineColor(value, hiColor2);
    } else if (isKeyEqualTo(key, "CURSORCOLOR")) {
        defineColor(value, cursorColor);
    } else if (isKeyEqualTo(key, "PLAYCOLOR")) {
        defineColor(value, playColor);
    } else if (isKeyEqualTo(key, "MUTECOLOR")) {
        defineColor(value, muteColor);
    } else if (isKeyEqualTo(key, "SONGVIEW_FE")) {
        defineColor(value, songviewFe);
    } else if (isKeyEqualTo(key, "SONGVIEW_00")) {
        defineColor(value, songview00);
    } else if (isKeyEqualTo(key, "ROWCOLOR1")) {
        defineColor(value, rowColor1);
    } else if (isKeyEqualTo(key, "ROWCOLOR2")) {
        defineColor(value, rowColor2);
    } else if (isKeyEqualTo(key, "ALTROWNUMBER")) {
        altRowNumber = strToInt(value);
    } else if (isKeyEqualTo(key, "MAJORBEATNUMBER")) {
        majorBeatNumber = strToInt(value);
    } else if (isKeyEqualTo(key, "MAJORBEAT")) {
        defineColor(value, majorBeatColor);
    } else if (isKeyEqualTo(key, "BLANKSPACE")) {
        defineColor(value, blankSpaceColor);
    } else if (isKeyEqualTo(key, "SHOW_COLUMN_TITLES")) {
        showColumnTitles = value;
    } else if (isKeyEqualTo(key, "FONTTYPE")) {
        fontType = value;
    } else {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------

Theme::~Theme() {}

//------------------------------------------------------------------------------
