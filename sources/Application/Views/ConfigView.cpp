#include "ConfigView.h"
#include "Application/Build.h"
#include "Application/Mixer/MixerService.h"
#include "Application/Model/ProjectDatas.h"
#include "Application/Model/Scale.h"
#include "Application/Persistency/PersistencyService.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/ModalDialogs/NewProjectDialog.h"
#include "Application/Views/ModalDialogs/SelectProjectDialog.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UIField.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "BaseClasses/UITempoField.h"
#include "Services/Midi/MidiService.h"
#include "System/System/System.h"

#define ACTION_PURGE            MAKE_FOURCC('P','U','R','G')
#define ACTION_SAVE             MAKE_FOURCC('S','A','V','E')
#define ACTION_SAVE_AS          MAKE_FOURCC('S','V','A','S')
#define ACTION_LOAD             MAKE_FOURCC('L','O','A','D')
#define ACTION_QUIT             MAKE_FOURCC('Q','U','I','T')
#define ACTION_PURGE_INSTRUMENT MAKE_FOURCC('P','R','G','I')
#define ACTION_TEMPO_CHANGED    MAKE_FOURCC('T','E','M','P')

/** Horizontal position of the label */
#define POS_X_LABEL 8
/** Horizontal position of the values */
#define POS_X_VALUE 18

static void SaveAsProjectCallback(View &v,ModalView &dialog) {

    FileSystemService FSS;
    NewProjectDialog &npd=(NewProjectDialog &)dialog;

    if (dialog.GetReturnCode()>0) {
		std::string str_dstprjdir;
		std::string str_dstsmpdir;

		Path root("root:");
		str_dstprjdir = root.GetName() + "/" + npd.GetName();
		str_dstsmpdir = str_dstprjdir + "/samples/";

		Path path_srcprjdir("Config:");
		Path path_srcsmpdir("Config:samples");
		Path path_dstprjdir = Path(str_dstprjdir);
		Path path_dstsmpdir = Path(str_dstsmpdir);

        Path path_srclgptdatsav = path_srcprjdir.GetPath() + "lgptsav_tmp.dat";
        Path path_dstlgptdatsav = path_dstprjdir.GetPath() + "/lgptsav.dat";

		if (path_dstprjdir.Exists()) {
			Trace::Log("ConfigView", "Dst Dir '%s' Exist == true",
			path_dstprjdir.GetPath().c_str());
		} else {
			if (FileSystem::GetInstance()->MakeDir(path_dstprjdir.GetPath().c_str()).Failed()) {
				Trace::Log("ConfigView", "Failed to create dir '%s'", path_dstprjdir.GetPath().c_str());
				return;
			};

		if (FileSystem::GetInstance()->MakeDir(path_dstsmpdir.GetPath().c_str()).Failed()) {
			Trace::Log("ConfigView", "Failed to create sample dir '%s'", path_dstprjdir.GetPath().c_str());
			return;
		};

        if (FSS.Copy(path_srclgptdatsav, path_dstlgptdatsav) > -1) {
            FSS.Delete(path_srclgptdatsav);
        }

        I_Dir *idir_srcsmpdir =
            FileSystem::GetInstance()->Open(path_srcsmpdir.GetPath().c_str());
        if (idir_srcsmpdir) {
				idir_srcsmpdir->GetContent("*");
				idir_srcsmpdir->Sort();
				IteratorPtr<Path>it(idir_srcsmpdir->GetIterator());
				for (it->Begin();!it->IsDone();it->Next()) {
					Path &current=it->CurrentItem();
					if (current.IsFile()) {
						Path dstfile = Path((str_dstsmpdir+current.GetName()).c_str());
						Path srcfile = Path(current.GetPath());
						FSS.Copy(srcfile.GetPath(),dstfile.GetPath());
					}
				}
			}

		((ConfigView &)v).OnSaveAsProject((char*)str_dstprjdir.c_str());
		}
    }
}

static void LoadCallback(View &v,ModalView &dialog) {
    MixerService::GetInstance()->SetRenderMode(0);
    if (dialog.GetReturnCode()==MBL_YES) {
		((ConfigView &)v).OnLoadProject() ;
	}
} ;

static void QuitCallback(View &v,ModalView &dialog) {
    MixerService::GetInstance()->SetRenderMode(0);
    if (dialog.GetReturnCode()==MBL_YES) {
		((ConfigView &)v).OnQuit() ;
	}
} ;

static void PurgeCallback(View &v,ModalView &dialog) {
	((ConfigView &)v).OnPurgeInstruments(dialog.GetReturnCode()==MBL_YES) ;
} ;

void ConfigView::insertLabel(GUIPoint position, char *name) {
    position._x = POS_X_LABEL;
    UIStaticField *f = new UIStaticField(position, name, CD_SONGVIEW00);
    Insert(f);
}

ConfigView::ConfigView(GUIWindow &w,ViewData *data):FieldView(w,data) {

    lastClock_ = 0;
    lastTick_ = 0;

    project_ = data->project_;
    Config *config = Config::GetInstance();

    GUIPoint position = GetAnchor();
    position._x = POS_X_VALUE;

    Variable *v = project_->FindVariable(VAR_TEMPO);

    v = project_->FindVariable(VAR_MIDIDEVICE);
    NAssert(v);
    insertLabel(position, "MIDI");
    Insert(new UIIntVarField(position, *v, "%s", 0, MidiService::GetInstance()->Size(), 1, 1));

    position._y += 2;
    v = config->FindVariable("AUTO_LOAD_LAST");
    Trace::Log("CONVI", "Confg View");
    Trace::Log("CONVI", "AUTO_LOAD_LAST.id=%s", v->GetID());
    Trace::Log("CONVI", "AUTO_LOAD_LAST=%s", v ? v->GetString() : "undefined");
    insertLabel(position, "Auto Load");
    Insert(new UIIntVarField(position, *v, "%s", 0, 1, 1, 1));

    position._y += 2;
    insertLabel(position, "Config.xml");
    position._x = POS_X_VALUE;
    UIActionField *a1 = new UIActionField("Save", ACTION_SAVE, position);
    a1->AddObserver(*this);
    Insert(a1);
}

ConfigView::~ConfigView() {
}

void ConfigView::ProcessButtonMask(unsigned short mask,bool pressed) {

    if (!pressed)
        return;

    FieldView::ProcessButtonMask(mask);

    if (mask & EPBM_R) {
        if (mask & EPBM_DOWN) {
			ViewType vt = VT_PROJECT;
			ViewEvent ve(VET_SWITCH_VIEW, &vt);
			SetChanged();
            NotifyObservers(&ve);
        }
    };
} ;

void ConfigView::DrawView() {
    Clear();

    GUITextProperties props;
    GUIPoint pos=GetTitlePosition() ;

    // Draw title
    SetColor(CD_NORMAL);
    DrawString(pos._x, pos._y, "Config", props);

    // Draw version
    SetColor(CD_SONGVIEW00);
    DrawString(SCREEN_WIDTH - strlen(VERSION_STRING), POS_Y_LAST_LINE,
               VERSION_STRING, props);

    FieldView::Redraw();
    drawMap();

    int currentMode = project_->GetRenderMode();
    if ((viewData_->renderMode_ != currentMode) && !MixerService::GetInstance()->IsRendering()) {
        // Mode changed
        if (currentMode > 0 && viewData_->renderMode_ == 0) {
            View::SetNotification("Rendering on, press start");
        } else if (currentMode == 0 && viewData_->renderMode_ > 0) {
            View::SetNotification("Rendering off");
        }
        viewData_->renderMode_ = currentMode;
        MixerService::GetInstance()->SetRenderMode(currentMode);
    }

    View::EnableNotification();
} ;

void ConfigView::Update(Observable &,I_ObservableData *data) {

	if (!hasFocus_) {
		return ;
	}

# ifdef _64BIT
	int fourcc=*((int*)data);
#else
    int fourcc = (unsigned int)data;
#endif

    UIField *focus = GetFocus();
    if (fourcc!= ACTION_TEMPO_CHANGED) {
		focus->ClearFocus() ;
		focus->Draw(w_) ;
		w_.Flush() ;
		focus->SetFocus() ;
	} else {
		focus=tempoField_ ;
	}
    Player *player = Player::GetInstance();
    switch (fourcc) {
		case ACTION_PURGE:
			project_->Purge() ;
			break ;
		case ACTION_PURGE_INSTRUMENT:
		{
			MessageBox *mb=new MessageBox(*this,"Purge unused samples from disk ?",MBBF_YES|MBBF_NO) ;
			DoModal(mb,PurgeCallback) ;
			break ;
		}
        case ACTION_SAVE: {
            MixerService::GetInstance()->SetRenderMode(0);
            PersistencyService *service = PersistencyService::GetInstance();
            service->Save();
            break;
        }
        case ACTION_SAVE_AS: {
            PersistencyService *service = PersistencyService::GetInstance();
            service->Save("Config:lgptsav_tmp.dat");
            NewProjectDialog *mb = new NewProjectDialog(*this, "root:");
            DoModal(mb, SaveAsProjectCallback);
            break;
        }
        case ACTION_LOAD: {
            MessageBox *mb = new MessageBox(
                *this, "Load song and lose changes ?", MBBF_YES | MBBF_NO);
            DoModal(mb, LoadCallback);
            break;
        }
        case ACTION_QUIT: {
            MessageBox *mb = new MessageBox(*this, "Quit and lose faith ?",
                                            MBBF_YES | MBBF_NO);
            DoModal(mb, QuitCallback);
            break;
        }
        case ACTION_TEMPO_CHANGED:
			break ;
		default:
			NInvalid ;
			break ;
	} ;
    focus->Draw(w_) ;
	isDirty_=true ;
} ;

void ConfigView::OnPurgeInstruments(bool removeFromDisk) {
	project_->PurgeInstruments(removeFromDisk) ;
} ;

void ConfigView::OnLoadProject() {
	ViewEvent ve(VET_QUIT_PROJECT) ;
	SetChanged();
	NotifyObservers(&ve) ;
} ;

void ConfigView::OnSaveAsProject(char * data) {
        ViewEvent ve(VET_SAVEAS_PROJECT,data) ;
	SetChanged();
	NotifyObservers(&ve) ;
} ;

void ConfigView::OnQuit() {
	ViewEvent ve(VET_QUIT_APP) ;
	SetChanged();
	NotifyObservers(&ve) ;
} ;
