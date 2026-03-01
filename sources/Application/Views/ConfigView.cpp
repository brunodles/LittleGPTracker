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

#define ACTION_SAVE MAKE_FOURCC('S', 'A', 'V', 'E')

/** Horizontal position of the label */
#define POS_X_LABEL 8
/** Horizontal position of the values */
#define POS_X_VALUE 18

static void SaveAsProjectCallback(View &v, ModalView &dialog) {

    FileSystemService FSS;
    NewProjectDialog &npd = (NewProjectDialog &)dialog;

    if (dialog.GetReturnCode() > 0) {
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
            if (FileSystem::GetInstance()
                    ->MakeDir(path_dstprjdir.GetPath().c_str())
                    .Failed()) {
                Trace::Log("ConfigView", "Failed to create dir '%s'",
                           path_dstprjdir.GetPath().c_str());
                return;
            };

            if (FileSystem::GetInstance()
                    ->MakeDir(path_dstsmpdir.GetPath().c_str())
                    .Failed()) {
                Trace::Log("ConfigView", "Failed to create sample dir '%s'",
                           path_dstprjdir.GetPath().c_str());
                return;
            };

            if (FSS.Copy(path_srclgptdatsav, path_dstlgptdatsav) > -1) {
                FSS.Delete(path_srclgptdatsav);
            }

            I_Dir *idir_srcsmpdir = FileSystem::GetInstance()->Open(
                path_srcsmpdir.GetPath().c_str());
            if (idir_srcsmpdir) {
                idir_srcsmpdir->GetContent("*");
                idir_srcsmpdir->Sort();
                IteratorPtr<Path> it(idir_srcsmpdir->GetIterator());
                for (it->Begin(); !it->IsDone(); it->Next()) {
                    Path &current = it->CurrentItem();
                    if (current.IsFile()) {
                        Path dstfile =
                            Path((str_dstsmpdir + current.GetName()).c_str());
                        Path srcfile = Path(current.GetPath());
                        FSS.Copy(srcfile.GetPath(), dstfile.GetPath());
                    }
                }
            }

            ((ConfigView &)v).OnSaveAsProject((char *)str_dstprjdir.c_str());
        }
    }
}

static void LoadCallback(View &v, ModalView &dialog) {
    MixerService::GetInstance()->SetRenderMode(0);
    if (dialog.GetReturnCode() == MBL_YES) {
        ((ConfigView &)v).OnLoadProject();
    }
};

static void QuitCallback(View &v, ModalView &dialog) {
    MixerService::GetInstance()->SetRenderMode(0);
    if (dialog.GetReturnCode() == MBL_YES) {
        ((ConfigView &)v).OnQuit();
    }
};

static void PurgeCallback(View &v, ModalView &dialog) {
    ((ConfigView &)v).OnPurgeInstruments(dialog.GetReturnCode() == MBL_YES);
};

void ConfigView::insertLabel(GUIPoint position, char *name) {
    position._x = POS_X_LABEL;
    UIStaticField *f = new UIStaticField(position, name, CD_TEXT_INFO);
    Insert(f);
}

ConfigView::ConfigView(GUIWindow &w, ViewData *data) : FieldView(w, data) {

    lastClock_ = 0;
    lastTick_ = 0;

    project_ = data->project_;
    Config *config = Config::GetInstance();

    GUIPoint position = GetAnchor();
    position._x = POS_X_VALUE;

    Variable *v = project_->FindVariable(VAR_MIDIDEVICE);
    NAssert(v);
    insertLabel(position, "MIDI");
    Insert(new UIIntVarField(position, *v, "%s", 0,
                             MidiService::GetInstance()->Size(), 1, 1));

    position._y += 2;
    insertLabel(position, "Auto Load");
    autoLoadField = new UIBoolField(position, config->projectAutoLoadEnabled);
    //autoLoadField->AddObserver(*this);
    Insert(autoLoadField);

    position._y += 2;
    insertLabel(position, "Config.xml");
    position._x = POS_X_VALUE;
    UIActionField *a1 = new UIActionField("Save", ACTION_SAVE, position);
    a1->AddObserver(*this);
    Insert(a1);
}

ConfigView::~ConfigView() {}

void ConfigView::ProcessButtonMask(unsigned short mask, bool pressed) {

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
};

void ConfigView::DrawView() {
    Clear();

    GUITextProperties props;
    GUIPoint pos = GetTitlePosition();

    // Draw title
    SetColor(CD_TEXT_TITLE);
    DrawString(pos._x, pos._y, "Config", props);

    // Draw version
    SetColor(CD_TEXT_INFO);
    DrawString(SCREEN_WIDTH - strlen(VERSION_STRING), POS_Y_LAST_LINE,
               VERSION_STRING, props);

    FieldView::Redraw();
    drawMap();

    int currentMode = project_->GetRenderMode();
    if ((viewData_->renderMode_ != currentMode) &&
        !MixerService::GetInstance()->IsRendering()) {
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
};

void ConfigView::Update(Observable &o, I_ObservableData *data) {
    if (!hasFocus_) {
        return;
    }

#ifdef _64BIT
    int fourcc = *((int *)data);
#else
    int fourcc = (unsigned int)data;
#endif

    UIField *focus = GetFocus();
    Player *player = Player::GetInstance();
    switch (fourcc) {
    case ACTION_SAVE: {
        View::SetNotification("Saving...");
        Config *config = Config::GetInstance();
        config->projectAutoLoadEnabled = autoLoadField->GetValue();
        config->Save();
        View::SetNotification("Saved!");
        break;
    }
    default:
        NInvalid;
        break;
    };
    focus->Draw(w_);
    isDirty_ = true;
};

void ConfigView::OnPurgeInstruments(bool removeFromDisk) {
    project_->PurgeInstruments(removeFromDisk);
};

void ConfigView::OnLoadProject() {
    ViewEvent ve(VET_QUIT_PROJECT);
    SetChanged();
    NotifyObservers(&ve);
};

void ConfigView::OnSaveAsProject(char *data) {
    ViewEvent ve(VET_SAVEAS_PROJECT, data);
    SetChanged();
    NotifyObservers(&ve);
};

void ConfigView::OnQuit() {
    ViewEvent ve(VET_QUIT_APP);
    SetChanged();
    NotifyObservers(&ve);
};
