#include "UIStaticField.h"
#include "Application/AppWindow.h"

UIStaticField::UIStaticField(GUIPoint &position, const char *string)
    : UIField(position) {
    string_ = string ;
};

UIStaticField::UIStaticField(GUIPoint &position, const char *string, const ColorDefinition color):UIField(position) {
    string_ = string;
    colorDefinitionIndex_ = static_cast<int>(color);
} ;

void UIStaticField::Draw(GUIWindow &w,int offset) {

	GUITextProperties props ;
	GUIPoint position=GetPosition() ;
	position._y+=offset ;

    ((AppWindow &)w)
        .SetColor(static_cast<ColorDefinition>(colorDefinitionIndex_));
    w.DrawString(string_,position,props) ;	
} ;

void UIStaticField::ProcessArrow(unsigned short mask){
} ;

bool UIStaticField::IsStatic() {
	return true ;
} ;
