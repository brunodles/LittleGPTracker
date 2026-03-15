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

inline void defineColor(const char *value, GUIColor *&color) {
    if (!value || !color)
        return;

    const char *hex = (value[0] == '#') ? value + 1 : value;
    if (strlen(hex) < 6)
        return;

    unsigned char r, g, b;
    char2hex(hex, &r);
    char2hex(hex + 2, &g);
    char2hex(hex + 4, &b);

    // Rebind so aliases (e.g. textColorFe/textColorEmpty defaults) can diverge.
    color = new GUIColor(r, g, b);
}

Theme::Theme() {
    // initialize default values

    backgroundColor = new GUIColor(0x00, 0x00, 0x00);
    borderColor = new GUIColor(0xFF, 0x47, 0x77);

    hiColor1 = new GUIColor(0xFF, 0x47, 0x77);
    hiColor2 = new GUIColor(0xE5, 0xD4, 0xC8);
    cursorColor = new GUIColor(0xFF, 0x47, 0x77);

    playColor = new GUIColor(0xFF, 0x00, 0x8C);
    muteColor = new GUIColor(0xF5, 0xEB, 0xFF);

    rowColor1 = new GUIColor(0xE5, 0xD4, 0xC8);
    rowColor2 = new GUIColor(0xFF, 0x47, 0x77);

    majorBeatColor = new GUIColor(0xFF, 0x47, 0x77);

    consoleColor = new GUIColor(0xFF, 0x47, 0x77);

    textColorValue = new GUIColor(0xE5, 0xD4, 0xC8);
    textColorEmpty = new GUIColor(0x36, 0x43, 0x4A);
    textColorFe = new GUIColor(0x36, 0x43, 0x4A);
    textColor00 = new GUIColor(0x36, 0x43, 0x4A);
    textColorInfo = new GUIColor(0x36, 0x43, 0x4A);

    altRowNumber = 4;
    majorBeatNumber = 4;
    showColumnTitles = false;

    // load from Theme file

    Path path("bin:theme.xml");
    Trace::Log("Theme", "Got Theme path=%s", path.GetPath().c_str());
    TiXmlDocument *document = new TiXmlDocument(path.GetPath());
    bool loadOkay = document->LoadFile();

    if (loadOkay) {
        // Check first node is Theme/ GPTheme

        TiXmlNode *rootnode = 0;

        rootnode = document->FirstChild("theme");
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
                        value = element->Attribute("value");
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

    Trace::Log("Theme", "Theme Result:");
    Trace::Log("Theme", "  background=%02X%02X%02X", backgroundColor->_r, backgroundColor->_g, backgroundColor->_b);
    Trace::Log("Theme", "  border=%02X%02X%02X", borderColor->_r, borderColor->_g, borderColor->_b);
    Trace::Log("Theme", "  hiColor1=%02X%02X%02X", hiColor1->_r, hiColor1->_g, hiColor1->_b);
    Trace::Log("Theme", "  hiColor2=%02X%02X%02X", hiColor2->_r, hiColor2->_g, hiColor2->_b);
    Trace::Log("Theme", "  cursorColor=%02X%02X%02X", cursorColor->_r, cursorColor->_g, cursorColor->_b);
    Trace::Log("Theme", "  playColor=%02X%02X%02X", playColor->_r, playColor->_g, playColor->_b);
    Trace::Log("Theme", "  muteColor=%02X%02X%02X", muteColor->_r, muteColor->_g, muteColor->_b);
    Trace::Log("Theme", "  rowColor1=%02X%02X%02X", rowColor1->_r, rowColor1->_g, rowColor1->_b);
    Trace::Log("Theme", "  rowColor2=%02X%02X%02X", rowColor2->_r, rowColor2->_g, rowColor2->_b);
    Trace::Log("Theme", "  majorBeatColor=%02X%02X%02X", majorBeatColor->_r, majorBeatColor->_g, majorBeatColor->_b);
    Trace::Log("Theme", "  consoleColor=%02X%02X%02X", consoleColor->_r, consoleColor->_g, consoleColor->_b);
    Trace::Log("Theme", "  textColorValue=%02X%02X%02X", textColorValue->_r, textColorValue->_g, textColorValue->_b);
    Trace::Log("Theme", "  textColorEmpty=%02X%02X%02X", textColorEmpty->_r, textColorEmpty->_g, textColorEmpty->_b);
    Trace::Log("Theme", "  textColorFe=%02X%02X%02X", textColorFe->_r, textColorFe->_g, textColorFe->_b);
    Trace::Log("Theme", "  textColor00=%02X%02X%02X", textColor00->_r, textColor00->_g, textColor00->_b);
    Trace::Log("Theme", "  textColorInfo=%02X%02X%02X", textColorInfo->_r, textColorInfo->_g, textColorInfo->_b);
    Trace::Log("Theme", "  altRowNumber=%d majorBeatNumber=%d showColumnTitles=%d fontType=%s",
     altRowNumber, majorBeatNumber, showColumnTitles, fontType);
}

bool Theme::proccessKeyValue(const char *key, const char *value) {
    Trace::Log("Theme", "Processing key=%s value=%s", key, value);
    if (isKeyEqualTo(key, "background")) {
        defineColor(value, backgroundColor);
    } else if (isKeyEqualTo(key, "text_color_value") || isKeyEqualTo(key, "foreground")) {
        defineColor(value, textColorValue);
    } else if (isKeyEqualTo(key, "border")) {
        defineColor(value, borderColor);
    } else if (isKeyEqualTo(key, "hicolor1")) {
        defineColor(value, hiColor1);
    } else if (isKeyEqualTo(key, "hicolor2")) {
        defineColor(value, hiColor2);
    } else if (isKeyEqualTo(key, "cursorcolor")) {
        defineColor(value, cursorColor);
    } else if (isKeyEqualTo(key, "playcolor")) {
        defineColor(value, playColor);
    } else if (isKeyEqualTo(key, "mutecolor")) {
        defineColor(value, muteColor);
    } else if (isKeyEqualTo(key, "text_color_fe") || isKeyEqualTo(key, "songview_fe")) {
        defineColor(value, textColorFe);
    } else if (isKeyEqualTo(key, "text_color_00") || isKeyEqualTo(key, "songview_00")) {
        defineColor(value, textColor00);
    } else if (isKeyEqualTo(key, "rowcolor1")) {
        defineColor(value, rowColor1);
    } else if (isKeyEqualTo(key, "rowcolor2")) {
        defineColor(value, rowColor2);
    } else if (isKeyEqualTo(key, "altrownumber")) {
        altRowNumber = strToInt(value);
    } else if (isKeyEqualTo(key, "majorbeatnumber")) {
        majorBeatNumber = strToInt(value);
    } else if (isKeyEqualTo(key, "majorbeat")) {
        defineColor(value, majorBeatColor);
    } else if (isKeyEqualTo(key, "text_color_empty")) {
        defineColor(value, textColorEmpty);
    } else if (isKeyEqualTo(key, "blankspace")) {
        defineColor(value, textColorEmpty);
    } else if (isKeyEqualTo(key, "text_color_info")) {
        defineColor(value, textColorInfo);
    } else if (isKeyEqualTo(key, "show_column_titles")) {
        showColumnTitles = value;
    } else if (isKeyEqualTo(key, "fonttype")) {
        fontType = value;
    } else {
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------

Theme::~Theme() {}

//------------------------------------------------------------------------------
