#include "UIThemeField.h"
#include "Application/Commands/ApplicationCommandDispatcher.h"
#include "Application/Player/Player.h"
#include "System/System/System.h"

UIThemeField::UIThemeField(FourCC action,GUIPoint &position,Variable &v,const char *format,int min,int max,int xOffset,int yOffset):UIIntVarField(position,v,format,min,max,xOffset,yOffset) {
	action_=action ;
	((WatchedVariable &)v).AddObserver(*this) ;
} ;

void UIThemeField::OnBClick() {
	ApplicationCommandDispatcher::GetInstance()->OnTempoTap() ;
} ;

void UIThemeField::Update(Observable &,I_ObservableData *data) {
	SetChanged() ;
#ifdef _64BIT
	NotifyObservers((I_ObservableData *) &action_);
#else
	NotifyObservers((I_ObservableData *)action_) ;
#endif
}

void UIThemeField::ProcessArrow(unsigned short mask) {
	int value=src_.GetInt() ;

	switch(mask) {
		case EPBM_UP:
			value+=yOffset_ ;
			break ;
		case EPBM_DOWN:
			value-=yOffset_ ;
			break ;
		case EPBM_LEFT:
			value-=xOffset_ ;
			break ;
  		case EPBM_RIGHT:
			value+=xOffset_ ;
			break ;
	} ;
	if (value<min_) {
		value=min_ ;
	} ;
	if (value>max_) {
		value=max_ ;
	}
	
	src_.SetInt(value) ;
} ;

void UIThemeField::ProcessBArrow(unsigned short mask) {

	ApplicationCommandDispatcher *dispatcher=ApplicationCommandDispatcher::GetInstance() ;
	switch(mask) {
		case EPBM_UP:
			break ;
		case EPBM_DOWN:
			break ;
		case EPBM_LEFT:
			dispatcher->OnNudgeDown() ;
			break ;
  		case EPBM_RIGHT:
			dispatcher->OnNudgeUp() ;
			break ;
	} ;
} ;
