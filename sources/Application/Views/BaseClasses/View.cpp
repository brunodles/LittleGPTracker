#include "View.h"
#include "System/Console/Trace.h"
#include "Application/Player/Player.h"
#include "Application/Utils/char.h"
#include "Application/AppWindow.h"
#include "Application/Model/Config.h"
#include "ModalView.h"

bool View::initPrivate_ = false;

int View::margin_ = 0;
int View::songRowCount_; //=21 sets screen height among other things
bool View::miniLayout_ = false;
int View::altRowNumber_ = 4;
int View::majorBeatNumber_ = 4;

View::View(GUIWindow &w,ViewData *viewData):
	w_(w),
	modalView_(0),
	modalViewCallback_(0),
	hasFocus_(false)
{
    if (!initPrivate_) {
        GUIRect rect = w.GetRect();
		miniLayout_= (rect.Width() < 320);
		View::margin_= 0;
		songRowCount_ = miniLayout_ ? 16:22; // 22 is row display count among other things

        altRowNumber_ = Config::GetInstance()->altRowNumber;
        majorBeatNumber_ = Config::GetInstance()->majorBeatNumber;

        initPrivate_ = true;
    }
    mask_ = 0;
	viewMode_ = VM_NORMAL;
	locked_= false;
	viewData_ = viewData;
	NOTIFICATION_TIMEOUT = 1000;
    displayNotification_ = "";
}

GUIPoint View::GetAnchor() {
    return GUIPoint((SCREEN_WIDTH - SONG_CHANNEL_COUNT * 3) / 2 + 2,
                    (SCREEN_HEIGHT - View::songRowCount_) / 2);
}

GUIPoint View::GetTitlePosition() {
#ifndef PLATFORM_CAANOO
	return GUIPoint(0,0) ;
#else
	return GUIPoint(0,1) ;
#endif
} ;

bool View::Lock() {
	if (locked_) return false ;
	locked_=true ;
	return true ;
} ;

void View::WaitForObject() {
	while (locked_) {} ;
}

void View::Unlock() {
	locked_=false ;
}

void View::drawMap() {
    if (!miniLayout_) {
        GUIPoint anchor = GetAnchor();
        GUIPoint pos(View::margin_, anchor._y);
    	GUITextProperties props;

		// Full Map
		// |C   |
		// |P G |
		// |SCPI|
		// |M TT|
		// Detect which column of the map is selected
		int mapColumn = 0;
		switch (viewType_) {	
		case VT_CONFIG:
		case VT_PROJECT:
		case VT_SONG:
		case VT_MIXER:
			mapColumn = 0;
			break;

        case VT_CHAIN:
            mapColumn = 1;
            break;

        case VT_GROOVE:
		case VT_PHRASE:
        case VT_TABLE: // under phrase
            mapColumn = 2;
			break;

        case VT_INSTRUMENT:
        case VT_TABLE2: // under instrument
            mapColumn = 3;
			break;

        default: //??
			mapColumn = 0;
		}

		SetColor(CD_SONGVIEW00);
		// Draw main map in empty color
		DrawString(pos._x, pos._y,     "P G ", props);
//		DrawString(pos._x, pos._y + 1, "SCPI", props); // not neded as it will be draw as hilite1
		DrawString(pos._x, pos._y + 2, "  TT", props);
		

		// Change the color for the hilights
		SetColor(CD_HILITE1);
        // Draw C if is on project
        //		if (viewType_ == VT_PROJECT) {
        //        	DrawString(pos._x, pos._y - 1, "C   ", props);
        //		}

        // First line
        // |P G |
        switch (mapColumn) {
		case 0:
        	DrawString(pos._x, pos._y, "P", props);
			break;
        case 2:
            DrawString(pos._x+2, pos._y, "G", props);
			break;
        default:
            break;
        }

        DrawString(pos._x, pos._y + 1, "SCPI", props);

        // Last line
		// |M TT|
		switch (mapColumn) {
        // case 0:
        //	DrawString(pos._x, pos._y + 2, "M", props);
        //	break;
        case 2:
            DrawString(pos._x + 2, pos._y + 2, "T", props);
			break;
        case 3:
            DrawString(pos._x + 3, pos._y + 2, "T", props);
			break;
        default:
            break;
        }

        // draw current screen on map
        SetColor(CD_HILITE2);
        props.invert_= true ;
		pos._y = anchor._y;
        switch (viewType_) {
        case VT_CHAIN:
            DrawString(pos._x + 1, pos._y + 1, "C", props);
            break;
		case VT_PHRASE:
            DrawString(pos._x + 2, pos._y + 1, "P", props);
            break;
		case VT_PROJECT:
            DrawString(pos._x, pos._y, "P", props);
            break;
		case VT_INSTRUMENT:
            DrawString(pos._x + 3, pos._y + 1, "I", props);
            break;
		case VT_TABLE: //under phrase
            DrawString(pos._x + 2, pos._y + 2, "T", props);
            break;
		case VT_TABLE2: //under instrument
            DrawString(pos._x + 3, pos._y + 2, "T", props);
            break;
		case VT_GROOVE:
            DrawString(pos._x + 2, pos._y, "G", props);
            break;

            //		case VT_CONFIG:
            //	        DrawString(pos._x, pos._y -1, "C", props) ;
            //            break;
            //
            //        case VT_MIXER:
            //            DrawString(pos._x,  pos._y + 2, "M", props) ;
            //            break;

        default: // VT_SONG
            DrawString(pos._x, pos._y + 1, "S", props);
        }

    } //! minilayout
}

void View::drawNotes() {

    if (!miniLayout_) {

		GUIPoint anchor=GetAnchor() ;
		int initialX = View::margin_+10 ;
		int initialY = anchor._y+23 ;
		GUIPoint pos(initialX,initialY) ;
		GUITextProperties props ;

        Player *player=Player::GetInstance() ;
		
		//column banger refactor
		props.invert_= true;
        for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
			if (i==viewData_->songX_) {
				SetColor(CD_HILITE2) ;
			} else {
				SetColor(CD_HILITE1) ;
			}
			if (player->IsRunning() && viewData_->playMode_ != PM_AUDITION) {
				DrawString(pos._x,pos._y,player->GetPlayedNote(i),props) ; //row for the note values
				pos._y++ ;
				DrawString(pos._x,pos._y,player->GetPlayedOctive(i),props) ; //row for the octive values
				pos._y++ ;
				DrawString(pos._x,pos._y,player->GetPlayedInstrument(i),props) ; //draw instrument number
			} else {
				DrawString(pos._x,pos._y,"  ",props) ; //row for the note values
				pos._y++ ;
				DrawString(pos._x,pos._y,"  ",props) ; //row for the octive values
				pos._y++ ;
				DrawString(pos._x,pos._y,"  ",props) ; //draw instrument number
			}
			pos._y = initialY ;
			pos._x+= 3;
		}
     }
}

void View::DoModal(ModalView *view,ModalViewCallback cb) {
	modalView_=view ;
	modalView_->OnFocus() ;
	modalViewCallback_=cb ;
	isDirty_=true ;
} ;

void View::Redraw() {
	if (modalView_) {
		if (isDirty_) {
			DrawView() ;
		}
		modalView_->Redraw() ;
	} else {
		DrawView() ;
	}
	isDirty_=false ;
} ;

void View::SetDirty(bool isDirty) {
	isDirty_=true ;
} ;

void View::ProcessButton(unsigned short mask, bool pressed) {
	isDirty_=false ;
	if (modalView_) {
		modalView_->ProcessButton(mask,pressed);
		modalView_->isDirty_;
		if (modalView_->IsFinished()) {
			// process callback sending the modal dialog
			if (modalViewCallback_) {
				modalViewCallback_(*this,*modalView_) ;
			}
			SAFE_DELETE(modalView_) ;
			isDirty_=true ;
		}
	} else {
		ProcessButtonMask(mask,pressed);
	}
	if (isDirty_) ((AppWindow &)w_).SetDirty() ;
} ;

void View::Clear() {
	((AppWindow &)w_).Clear() ;
}

void View::SetColor(ColorDefinition cd) {
	((AppWindow &)w_).SetColor(cd) ;
} ;

void View::ClearRect(int x,int y,int w,int h) {
	GUIRect rect(x,y,(x+w),(y+h)) ;
	w_.ClearRect(rect) ;
} ;

void View::DrawString(int x,int y,const char *txt,GUITextProperties &props) {
	GUIPoint pos(x,y) ;
	w_.DrawString(txt,pos,props) ;
} ;

/*
	Displays the saved notification for 1 second
*/
void View::EnableNotification() {
	if ((SDL_GetTicks() - notificationTime_) <= NOTIFICATION_TIMEOUT) {
		SetColor(CD_NORMAL);
		GUITextProperties props;
        int xOffset = 4;
        DrawString(xOffset, notiDistY_, displayNotification_.c_str(), props);
    } else {
		displayNotification_ = "";
	}
}

/*
    Set displayed notification
    Saves the current time
    Optionally set display y offset if not in a project (default == 2)
    Allows negative offsets, use with care!
*/
void View::SetNotification(const char *notification, int offset) {
    notificationTime_ = SDL_GetTicks();
    displayNotification_ = notification;
    notiDistY_ = offset;
    isDirty_ = true;
}
