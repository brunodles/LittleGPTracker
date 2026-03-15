#include "InstrumentView.h"
#include "Application/Instruments/MidiInstrument.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "Application/Model/Config.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIIntVarOffField.h"
#include "BaseClasses/UINoteVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "Foundation/Variables/Variable.h"
#include "ModalDialogs/ImportSampleDialog.h"
#include "ModalDialogs/MessageBox.h"
#include "System/System/System.h"

/** Horizontal position of the label */
#define POS_X_C1 13
#define POS_X_C2 24
#define POS_X_C3 33
/** Horizontal position of the values */
#define POS_X_VALUE 18

InstrumentView::InstrumentView(GUIWindow &w,ViewData *data):FieldView(w,data) {
	project_=data->project_ ;
	lastFocusID_=0 ;
	current_=0 ;
	onInstrumentChange() ;
}

InstrumentView::~InstrumentView() {
}

InstrumentType InstrumentView::getInstrumentType() {
	int i=viewData_->currentInstrument_ ;
	InstrumentBank *bank=viewData_->project_->GetInstrumentBank() ;
	I_Instrument *instrument=bank->GetInstrument(i) ;
    return instrument->GetType() ;
} ;

void InstrumentView::onInstrumentChange() {

	ClearFocus() ;

	I_Instrument *old=current_ ;

	int i=viewData_->currentInstrument_ ;
	InstrumentBank *bank=viewData_->project_->GetInstrumentBank() ;
	current_=bank->GetInstrument(i) ;

	if (current_!=old) {
		current_->RemoveObserver(*this) ;
	} ;
	T_SimpleList<UIField>::Empty() ;

	InstrumentType it=getInstrumentType() ;

    switch (it) {
		case IT_MIDI:
			fillMidiParameters() ;
			break ;
		case IT_SAMPLE:
			fillSampleParameters() ;
			break ;
	} ;

	SetFocus(T_SimpleList<UIField>::GetFirst()) ;
	IteratorPtr<UIField> it2(T_SimpleList<UIField>::GetIterator()) ;
	for (it2->Begin();!it2->IsDone();it2->Next()) {
        UIIntVarField &field=(UIIntVarField &)it2->CurrentItem() ;
        if (field.GetVariableID()==lastFocusID_) {
            SetFocus(&field) ;
            break ;
        }
    } ;
	if (current_!=old) {
		current_->AddObserver(*this) ;
	}
} ;

void InstrumentView::fillSampleParameters() {

	int i=viewData_->currentInstrument_ ;
	InstrumentBank *bank=viewData_->project_->GetInstrumentBank() ;
	I_Instrument *instr=bank->GetInstrument(i) ;
	SampleInstrument *instrument=(SampleInstrument *)instr  ;
	GUIPoint position=GetAnchor() ;

//	position._y+=View::fieldSpaceHeight_;
    insertLabel(POS_X_C1-6, position._y-1, "Sample");
	Variable *v = instrument->FindVariable(SIP_SAMPLE);
	SamplePool *sp = SamplePool::GetInstance();
    position._x = POS_X_C1 - 5;
	UIIntVarField *f1 = new UIIntVarField(position,*v,"%s",0,sp->GetNameListSize()-1,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;
	f1->SetFocus();
    position._x = POS_X_VALUE;

    position._y += 1;
#ifdef FFMPEG_ENABLED
    insertLabel(POS_X_C1 - 2, position._y, "Fx");
    v = instrument->FindVariable(SIP_PRINTFX);
    position._x = POS_X_C1;
    f1 = new UIIntVarField(position, *v, "%s", 0, 3, 1, 2);
    T_SimpleList<UIField>::Insert(f1) ;

    insertLabel(POS_X_C2 - 3, position._y, "Wet");
    position._x = POS_X_C2;
    v = instrument->FindVariable(SIP_IR_WET);
    f1 = new UIIntVarField(position, *v, "%d%%", 0, 100, 1, 10);
    T_SimpleList<UIField>::Insert(f1);

    position._x = POS_X_C3;
    insertLabel(POS_X_C3 - 3, position._y, "Pad");
    v = instrument->FindVariable(SIP_IR_PAD);
    f1 = new UIIntVarField(position, *v, "%dms", 0, 5000, 5, 100);
    T_SimpleList<UIField>::Insert(f1);
#endif
    position._y += 2;
    position._x = POS_X_C1 ;
    insertLabel(POS_X_C1 - 6, position._y, "volume");
    v=instrument->FindVariable(SIP_VOLUME) ;
	f1=new UIIntVarField(position, *v, "%d", 0, 255, 1, 10) ;
	T_SimpleList<UIField>::Insert(f1) ;

    insertLabel(POS_X_C2 - 3, position._y, "Pam");
    position._x = POS_X_C2;
	v=instrument->FindVariable(SIP_PAN) ;
	f1=new UIIntVarField(position,*v,"%2.2X",0,0xFE,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._y += 1;
	position._x = POS_X_C1;
    insertLabel(POS_X_C1 - 4, position._y, "Root");
	v=instrument->FindVariable(SIP_ROOTNOTE) ;
	f1=new UINoteVarField(position,*v,"%s",0,0x7F,1,0x0C) ;
	T_SimpleList<UIField>::Insert(f1) ;

    position._x+=5;
    insertLabel(position._x, position._y, "detune");
	v=instrument->FindVariable(SIP_FINETUNE) ;
    position._x+=6;
	f1=new UIIntVarField(position,*v,"%2.2X",0,255,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

    position._y += 1;
    position._x = POS_X_C1;
    insertLabel(POS_X_C1 - 5, position._y, "crush");
    v=instrument->FindVariable(SIP_CRUSH);
	f1=new UIIntVarField(position,*v,"%d",1,0x10,1,4) ;
	T_SimpleList<UIField>::Insert(f1) ;

    insertLabel(POS_X_C2 - 5, position._y, "drive");
    position._x = POS_X_C2;
    v = instrument->FindVariable(SIP_CRUSHVOL);
    f1=new UIIntVarField(position, *v, "%2.2X",0,0xFF,1,0x10);
	T_SimpleList<UIField>::Insert(f1);

    position._y += 1;
    insertLabel(POS_X_C1 - 10, position._y, "downsample");
    position._x  = POS_X_C1;
	v=instrument->FindVariable(SIP_DOWNSMPL) ;
	f1=new UIIntVarField(position,*v,"%d",0,8,1,4) ;
	T_SimpleList<UIField>::Insert(f1) ;

    position._y += 2;
    insertLabel(POS_X_C1 - 7, position._y, "flt cut");
    position._x = POS_X_C1;
	v=instrument->FindVariable(SIP_FILTCUTOFF) ;
	f1=new UIIntVarField(position,*v,"%2.2X",0,0xFF,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

    insertLabel(POS_X_C2 - 3, position._y, "res");
	position._x = POS_X_C2;
	v=instrument->FindVariable(SIP_FILTRESO) ;
	f1=new UIIntVarField(position,*v,"%2.2X",0,0xFF,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._y += 1;
    insertLabel(POS_X_C1 - 4, position._y, "type");
	position._x = POS_X_C1;
	v=instrument->FindVariable(SIP_FILTMIX) ;
	f1=new UIIntVarField(position,*v,"%2.2X",0,0xFF,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._x = POS_X_C2;
	v=instrument->FindVariable(SIP_FILTMODE) ;
    insertLabel(POS_X_C2 - 4, position._y, "Mode");
	f1=new UIIntVarField(position,*v,"%s",0,2,1,1) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._y += 1;
    position._x = POS_X_C1;
	v=instrument->FindVariable(SIP_ATTENUATE) ;
    insertLabel(POS_X_C1 - 9, position._y, "attenuate");
	f1=new UIIntVarField(position,*v,"%2.2X",1,0xFF,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

    position._y += 1;
    insertLabel(POS_X_C1 - 7, position._y, "fb tune");

	v=instrument->FindVariable(SIP_FBTUNE) ;
	f1=new UIIntVarField(position,*v,"%2.2X",0,0xFF,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._x = POS_X_C2;
    insertLabel(POS_X_C2 - 3, position._y, "mix");
	v=instrument->FindVariable(SIP_FBMIX) ;
	f1=new UIIntVarField(position,*v,"%2.2X",0,0xFF,1,0X10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._y += 2;
    position._x = POS_X_C1;
	v=instrument->FindVariable(SIP_INTERPOLATION) ;
    insertLabel(POS_X_C1 - 13, position._y, "interpolation");
	f1=new UIIntVarField(position,*v,"%s",0,1,1,1) ;
	T_SimpleList<UIField>::Insert(f1) ;

    position._y += 1;
	v=instrument->FindVariable(SIP_LOOPMODE) ;
    insertLabel(POS_X_C1 - 9, position._y, "loop mode");
	f1=new UIIntVarField(position,*v,"%s",0,SILM_LAST-1,1,1) ;
	T_SimpleList<UIField>::Insert(f1) ;
	position._y+=1 ;

	v=instrument->FindVariable(SIP_SLICES) ;
    insertLabel(POS_X_C1 - 6, position._y, "slices");
    position._x = POS_X_C1;
	f1=new UIIntVarField(position,*v,"%2.2X",1,0xFF,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._y += 1;
    position._x = POS_X_C1;
	v=instrument->FindVariable(SIP_START) ;
    insertLabel(POS_X_C1 - 5, position._y, "start");
	f1=new UIBigHexVarField(position,*v,7,"%7.7X",0,instrument->GetSampleSize()-1,16) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._y += 1;
	v=instrument->FindVariable(SIP_LOOPSTART) ;
    insertLabel(POS_X_C1 - 10, position._y, "loop start");
	f1=new UIBigHexVarField(position,*v,7,"%7.7X",0,instrument->GetSampleSize()-1,16) ;
	T_SimpleList<UIField>::Insert(f1) ;

	v=instrument->FindVariable(SIP_END) ;
    insertLabel(POS_X_C2 - 3, position._y, "end");
    position._x = POS_X_C2;
	f1=new UIBigHexVarField(position,*v,7,"%7.7X",0,instrument->GetSampleSize()-1,16) ;
	T_SimpleList<UIField>::Insert(f1) ;

	v=instrument->FindVariable(SIP_TABLEAUTO) ;
	position._y += 2 ;
    position._x = POS_X_C1;
    insertLabel(POS_X_C1 - 10, position._y, "automation");
	UIIntVarField *f2=new UIIntVarField(position,*v,"%s",0,1,1,1) ;
	T_SimpleList<UIField>::Insert(f2) ;

	position._y+=1 ;
	v=instrument->FindVariable(SIP_TABLE) ;
    insertLabel(POS_X_C1 - 5, position._y, "table");
	f1=new UIIntVarOffField(position,*v,"%2.2X",0x00,0x7F,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

} ;

void InstrumentView::fillMidiParameters() {

	int i=viewData_->currentInstrument_ ;
	InstrumentBank *bank=viewData_->project_->GetInstrumentBank() ;
	I_Instrument *instr=bank->GetInstrument(i) ;
	MidiInstrument *instrument=(MidiInstrument *)instr  ;
	GUIPoint position=GetAnchor() ;

	Variable *v=instrument->FindVariable(MIP_CHANNEL) ;
	UIIntVarField* f1=new UIIntVarField(position,*v,"channel: %2.2d",0,0x0F,1,0x04,1) ;
	T_SimpleList<UIField>::Insert(f1) ;
	f1->SetFocus() ;

	position._y+=1;
	v=instrument->FindVariable(MIP_VOLUME) ;
	f1=new UIIntVarField(position,*v,"volume: %2.2X",0,0xFF,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	position._y+=1;
	v=instrument->FindVariable(MIP_NOTELENGTH) ;
	f1=new UIIntVarField(position,*v,"length: %2.2X",0,0xFF,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	v=instrument->FindVariable(MIP_TABLEAUTO) ;
	position._y+=2 ;
	UIIntVarField *f2=new UIIntVarField(position,*v,"automation: %s",0,1,1,1) ;
	T_SimpleList<UIField>::Insert(f2) ;

	position._y+=1;
	v=instrument->FindVariable(MIP_TABLE) ;
	f1=new UIIntVarOffField(position,*v,"table: %2.2X",0,0x7F,1,0x10) ;
	T_SimpleList<UIField>::Insert(f1) ;

} ;


void InstrumentView::warpToNext(int offset) {
	int instrument=viewData_->currentInstrument_+offset ;
	if (instrument>=MAX_INSTRUMENT_COUNT) {
		instrument=instrument-MAX_INSTRUMENT_COUNT ;
	} ;
	if (instrument<0) {
		instrument=MAX_INSTRUMENT_COUNT+instrument ;
	} ;
	viewData_->currentInstrument_=instrument ;
	onInstrumentChange() ;
	isDirty_=true ;
} ;

void InstrumentView::ProcessButtonMask(unsigned short mask,bool pressed) {

	if (!pressed) return ;

	isDirty_=false ;

	if (viewMode_==VM_NEW) {
		if (mask==EPBM_A) {
			UIIntVarField *field=(UIIntVarField *)GetFocus() ;
			Variable &v=field->GetVariable() ;
			switch(v.GetID()) {
				case SIP_SAMPLE:
				 {
                    // First check if the samplelib exists

					 Path sampleLib(SamplePool::GetInstance()->GetSampleLib()) ;
					 if (FileSystem::GetInstance()->GetFileType(sampleLib.GetPath().c_str())!=FT_DIR) {
						 MessageBox *mb=new MessageBox(*this,"Can't access the samplelib",MBBF_OK) ;
						 DoModal(mb) ;
					 } else { ;
						// Go to import sample

						 ImportSampleDialog *isd=new ImportSampleDialog(*this) ;
						 DoModal(isd) ;
					}
					break ;
				 }
				case SIP_TABLE:
				 {
					int next=TableHolder::GetInstance()->GetNext() ;
					if (next!=NO_MORE_TABLE) {
						v.SetInt(next) ;
						isDirty_=true ;
					}
					break ;
                }
                case SIP_PRINTFX: {
                    FxPrinter printer(viewData_);
                    isDirty_ = printer.Run();
                    View::SetNotification(printer.GetNotification());
                    break;
                }
                default:
                    break ;
			}
			mask&=(0xFFFF-EPBM_A) ;
		}
	}

	if (viewMode_==VM_CLONE) {
        if ((mask&EPBM_A)&&(mask&EPBM_L)) {
			UIIntVarField *field=(UIIntVarField *)GetFocus() ;
			mask&=(0xFFFF-EPBM_A) ;
			Variable &v=field->GetVariable() ;
			int current=v.GetInt() ;
			if (current==-1) return ;

			int next=TableHolder::GetInstance()->Clone(current) ;
			if (next!=NO_MORE_TABLE) {
				v.SetInt(next) ;
				isDirty_=true ;
			}
		}
		mask&=(0xFFFF-(EPBM_A|EPBM_L)) ;
	} ;

	if (viewMode_==VM_SELECTION) {
	} else {
		viewMode_=VM_NORMAL ;
	}

	FieldView::ProcessButtonMask(mask) ;

    Player *player=Player::GetInstance() ;
	// B Modifier

    if (mask & EPBM_B) {
        if (mask&EPBM_LEFT) warpToNext(-1) ;
		if (mask&EPBM_RIGHT) warpToNext(+1);
		if (mask&EPBM_DOWN) warpToNext(-16) ;
		if (mask&EPBM_UP) warpToNext(+16);
		if (mask&EPBM_A) { // Allow cut instrument
		   if (getInstrumentType()==IT_SAMPLE) {
                if (GetFocus()==T_SimpleList<UIField>::GetFirst()) {
	               int i=viewData_->currentInstrument_ ;
	               InstrumentBank *bank=viewData_->project_->GetInstrumentBank() ;
	               I_Instrument *instr=bank->GetInstrument(i) ;
					instr->Purge() ;
//                   Variable *v=instr->FindVariable(SIP_SAMPLE) ;
//                   v->SetInt(-1) ;
                   isDirty_=true ;
                }
           }

		   // Check if on table
		   if (GetFocus()==T_SimpleList<UIField>::GetLast()) {
	            int i=viewData_->currentInstrument_ ;
	            InstrumentBank *bank=viewData_->project_->GetInstrumentBank() ;
	            I_Instrument *instr=bank->GetInstrument(i) ;
                Variable *v=instr->FindVariable(SIP_TABLE) ;
                v->SetInt(-1) ;
                isDirty_=true ;
		   } ;
        }
        if (mask&EPBM_L) {
            viewMode_=VM_CLONE ;
        } ;
    } else {

        // A modifier

        if (mask == EPBM_A) {
            FourCC varID = ((UIIntVarField *)GetFocus())->GetVariableID();
            if ((varID == SIP_TABLE) || (varID == MIP_TABLE) ||
                (varID == SIP_SAMPLE) || (varID == SIP_PRINTFX)) {
                viewMode_ = VM_NEW;
			}
        } else {

            // R Modifier

            if (mask & EPBM_R) {
                if (mask & EPBM_LEFT) {
                    ViewType vt = VT_PHRASE;
                    ViewEvent ve(VET_SWITCH_VIEW, &vt);
                    SetChanged();
                    NotifyObservers(&ve);
                }

                if (mask & EPBM_DOWN) {

                    // Go to table view

                    ViewType vt = VT_TABLE2;

                    int i = viewData_->currentInstrument_;
                    InstrumentBank *bank =
                        viewData_->project_->GetInstrumentBank();
                    I_Instrument *instr = bank->GetInstrument(i);
                    int table = instr->GetTable();
                    if (table != VAR_OFF) {
                        viewData_->currentTable_ = table;
                    }
                    ViewEvent ve(VET_SWITCH_VIEW, &vt);
                    SetChanged();
                    NotifyObservers(&ve);
                }

                // if (mask&EPBM_RIGHT) {

                //	// Go to import sample

                //		ViewType vt=VT_IMPORT ;
                //		ViewEvent ve(VET_SWITCH_VIEW,&vt) ;
                //		SetChanged();
                //		NotifyObservers(&ve) ;
                //}

                if (mask & EPBM_START) {
                    player->OnStartButton(PM_PHRASE, viewData_->songX_, true,
                                          viewData_->chainRow_);
                }
            } else {
                // No modifier
                if (mask & EPBM_START) {
                    player->OnStartButton(PM_PHRASE, viewData_->songX_, false,
                                          viewData_->chainRow_);
                }
            }
        }
    }

    UIIntVarField *field = (UIIntVarField *)GetFocus();
    if (field) {
	   lastFocusID_=field->GetVariableID() ;
    }

} ;

void InstrumentView::DrawView() {

	Clear() ;
    View::EnableNotification();

    GUITextProperties props;
    GUIPoint pos = GetTitlePosition();

    // Draw title

    char title[20];
    SetColor(CD_TEXT_VALUE);
    sprintf(title, "Instrument %2.2X", viewData_->currentInstrument_);
    DrawString(pos._x, pos._y, title, props);

    // Draw fields

    FieldView::Redraw();
    drawMap() ;
} ;

void InstrumentView::OnFocus() { onInstrumentChange(); }

void InstrumentView::Update(Observable &o,I_ObservableData *d) {
	onInstrumentChange() ;
}
