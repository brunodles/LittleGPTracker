#include "NullView.h"
#include "Application/Build.h"

NullView::NullView(GUIWindow &w,ViewData *viewData):View(w,viewData) {
}

NullView::~NullView() {
} 

void NullView::ProcessButtonMask(unsigned short mask,bool pressed) {

} ;

void NullView::DrawView() {

	Clear() ;


	GUITextProperties props;
	SetColor(CD_HILITE2) ;

    GUIPoint pos;
    pos._y = POS_Y_BEFORE_LAST_LINE;
    pos._x=(SCREEN_WIDTH-strlen(VERSION_STRING))/2 ;
    DrawString(pos._x, pos._y, VERSION_STRING, props);
} ;

void NullView::OnPlayerUpdate(PlayerEventType ,unsigned int tick) {

} ;

void NullView::OnFocus() {
} ;
