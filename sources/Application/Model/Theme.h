#ifndef _Theme_H_
#define _Theme_H_

#include "Foundation/T_Singleton.h"
#include "System/Console/Trace.h"
#include "Externals/TinyXML/tinyxml.h"
#include "UIFramework/Framework/GUIColor.h"

class Theme{
    public:
        Theme();
        ~Theme();

        bool proccessKeyValue(const char *key, const char *value);
        void save();


        // *key: BACKGROUND
        GUIColor* bgColor;
        // *key: FOREGROUND
        GUIColor* fgColor;
        // *key: BORDER
        GUIColor* borderColor;
        // *key: HICOLOR1
        GUIColor* hiColor1;
        // *key: HICOLOR2
        GUIColor* hiColor2;
        // *key: CURSORCOLOR
        GUIColor* cursorColor;
        // *key: PLAYCOLOR
        GUIColor* playColor;
        // *key: MUTECOLOR
        GUIColor* muteColor;
        // *key: SONGVIEW_FE
        GUIColor* songviewFe;
        // *key: SONGVIEW_00
        GUIColor* songview00;
        // *key: ROWCOLOR1
        GUIColor* rowColor1;
        // *key: ROWCOLOR2
        GUIColor* rowColor2;
        // *key: ALTROWNUMBER
        int altRowNumber;
        // *key: MAJORBEATNUMBER
        int majorBeatNumber;
        // *key: MAJORBEAT
        GUIColor* majorBeatColor;
        // *key: BLANKSPACE
        GUIColor* blankSpaceColor;
        // *key: SHOW_COLUMN_TITLES
        bool showColumnTitles;
        // *key: FONTTYPE
	    const char *fontType;
};

#endif
