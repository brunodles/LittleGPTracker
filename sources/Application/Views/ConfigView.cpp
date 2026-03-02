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

void ConfigView::insertLabel(GUIPoint position, char *name) {
    position._x = POS_X_LABEL;
    UIStaticField *f = new UIStaticField(position, name, CD_SONGVIEW00);
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
    SetColor(CD_NORMAL);
    DrawString(pos._x, pos._y, "Config", props);

    // Draw version
    SetColor(CD_SONGVIEW00);
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
