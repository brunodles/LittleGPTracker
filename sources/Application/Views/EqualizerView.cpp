#include "EqualizerView.h"

#include "Application/Instruments/MidiInstrument.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/InstrumentBank.h"
#include "BaseClasses/UIBigHexVarField.h"
#include "BaseClasses/UIStaticField.h"
#include "Foundation/Variables/Variable.h"
#include "System/Console/Trace.h"
#include <stdio.h>
#include <string.h>

namespace {
static const FourCC kEqFreqIds[EqUtils::kEqBandCount] = {
    I_CMD_EQF1, I_CMD_EQF2, I_CMD_EQF3, I_CMD_EQF4, I_CMD_EQF5, I_CMD_EQF6};
static const FourCC kEqGainIds[EqUtils::kEqBandCount] = {
    I_CMD_EQG1, I_CMD_EQG2, I_CMD_EQG3, I_CMD_EQG4, I_CMD_EQG5, I_CMD_EQG6};

static void formatFrequency(unsigned short raw, char *buffer, int size) {
    float hz = EqUtils::DecodeFrequency(raw);
    if (hz >= 1000.0f) {
        snprintf(buffer, size, "%.2f kHz", hz / 1000.0f);
    } else {
        snprintf(buffer, size, "%.1f Hz", hz);
    }
}

static void formatGainQ(unsigned short raw, char *buffer, int size) {
    float gainDb = 0.0f;
    float q = 0.0f;
    EqUtils::DecodeGainQ(raw, gainDb, q);
    snprintf(buffer, size, "Gain %+0.1f dB  Q %0.2f", gainDb, q);
}
} // namespace

EqualizerView::EqualizerView(GUIWindow &w, ViewData *data)
    : FieldView(w, data), project_(data->project_), current_(0), lastFocusID_(0) {
    onInstrumentChange();
}

EqualizerView::~EqualizerView() {}

int EqualizerView::getBandIndex(FourCC id, bool *isGainQ) const {
    for (int i = 0; i < EqUtils::kEqBandCount; ++i) {
        if (id == kEqFreqIds[i]) {
            if (isGainQ) {
                *isGainQ = false;
            }
            return i;
        }
        if (id == kEqGainIds[i]) {
            if (isGainQ) {
                *isGainQ = true;
            }
            return i;
        }
    }
    return -1;
}

void EqualizerView::renderHelpLine(const char *line1, const char *line2) {
    GUITextProperties props;
    GUIPoint pos = GetTitlePosition();
    pos._y += 1;
    SetColor(CD_HILITE1);
    DrawString(pos._x, pos._y, line1, props);
    pos._y += 1;
    SetColor(CD_NORMAL);
    DrawString(pos._x, pos._y, line2, props);
}

void EqualizerView::describeFocus(char *line1, char *line2, int size) {
    line1[0] = 0;
    line2[0] = 0;

    UIIntVarField *field = (UIIntVarField *)GetFocus();
    if (!field) {
        snprintf(line1, size, "Band selection");
        snprintf(line2, size, "Use arrows to move between bands");
        return;
    }

    bool gainQ = false;
    int band = getBandIndex(field->GetVariableID(), &gainQ);
    if (band < 0) {
        snprintf(line1, size, "Equalizer");
        snprintf(line2, size, "Adjust frequency or gain/Q");
        return;
    }

    Variable &var = field->GetVariable();
    if (gainQ) {
        char valueLine[80];
        formatGainQ((unsigned short)var.GetInt(), valueLine, sizeof(valueLine));
        snprintf(line1, size, "Band %d gain/Q", band + 1);
        snprintf(line2, size, "%s", valueLine);
    } else {
        char valueLine[80];
        formatFrequency((unsigned short)var.GetInt(), valueLine, sizeof(valueLine));
        snprintf(line1, size, "Band %d frequency", band + 1);
        snprintf(line2, size, "%s", valueLine);
    }
}

void EqualizerView::fillParameters() {
    if (!current_ || current_->GetType() != IT_SAMPLE) {
        return;
    }

    SampleInstrument *instrument = (SampleInstrument *)current_;
    GUIPoint position(2, 4);

    for (int band = 0; band < EqUtils::kEqBandCount; ++band) {
        static const char *bandNames[EqUtils::kEqBandCount] = {
            "B1", "B2", "B3", "B4", "B5", "B6"};
        UIStaticField *bandLabel = new UIStaticField(position, bandNames[band]);
        T_SimpleList<UIField>::Insert(bandLabel);

        GUIPoint fieldPos = position;
        fieldPos._x += 4;
        Variable *freq = instrument->FindVariable(kEqFreqIds[band]);
        UIBigHexVarField *freqField =
            new UIBigHexVarField(fieldPos, *freq, 4, "freq: %4.4X", 0, 0xFFFF, 16);
        T_SimpleList<UIField>::Insert(freqField);

        fieldPos._x += 11;
        Variable *gainQ = instrument->FindVariable(kEqGainIds[band]);
        UIBigHexVarField *gainField = new UIBigHexVarField(
            fieldPos, *gainQ, 4, "gain/q: %4.4X", 0, 0xFFFF, 16);
        T_SimpleList<UIField>::Insert(gainField);

        position._y += 1;
    }
}

void EqualizerView::onInstrumentChange() {
    ClearFocus();

    I_Instrument *old = current_;
    int i = viewData_->currentInstrument_;
    InstrumentBank *bank = viewData_->project_->GetInstrumentBank();
    current_ = bank->GetInstrument(i);

    if (current_ != old) {
        if (old) {
            old->RemoveObserver(*this);
        }
        T_SimpleList<UIField>::Empty();
        fillParameters();
        SetFocus(T_SimpleList<UIField>::GetFirst());
        if (current_) {
            current_->AddObserver(*this);
        }
    }

    IteratorPtr<UIField> it(T_SimpleList<UIField>::GetIterator());
    for (it->Begin(); !it->IsDone(); it->Next()) {
        UIIntVarField &field = (UIIntVarField &)it->CurrentItem();
        if (field.GetVariableID() == lastFocusID_) {
            SetFocus(&field);
            break;
        }
    }
}

void EqualizerView::ProcessButtonMask(unsigned short mask, bool pressed) {
    if (!pressed) {
        return;
    }

    if (mask & EPBM_R) {
        if (mask & EPBM_UP) {
            ViewType vt = VT_INSTRUMENT;
            ViewEvent ve(VET_SWITCH_VIEW, &vt);
            SetChanged();
            NotifyObservers(&ve);
            return;
        }
        if (mask & EPBM_LEFT) {
            ViewType vt = VT_PHRASE;
            ViewEvent ve(VET_SWITCH_VIEW, &vt);
            SetChanged();
            NotifyObservers(&ve);
            return;
        }
    }

    FieldView::ProcessButtonMask(mask);

    UIIntVarField *field = (UIIntVarField *)GetFocus();
    if (field) {
        lastFocusID_ = field->GetVariableID();
    }
}

void EqualizerView::DrawView() {
    Clear();
    View::EnableNotification();

    GUITextProperties props;
    GUIPoint pos = GetTitlePosition();
    SetColor(CD_NORMAL);

    char title[40];
    const char *instrumentName = "(none)";
    if (current_) {
        instrumentName = current_->GetName();
    }
    snprintf(title, sizeof(title), "Equalizer %2.2X", viewData_->currentInstrument_);
    DrawString(pos._x, pos._y, title, props);

    pos._y += 1;
    DrawString(pos._x, pos._y, instrumentName, props);

    char help1[80];
    char help2[80];
    describeFocus(help1, help2, sizeof(help1));
    pos._y += 1;
    SetColor(CD_HILITE1);
    DrawString(pos._x, pos._y, help1, props);
    pos._y += 1;
    SetColor(CD_NORMAL);
    DrawString(pos._x, pos._y, help2, props);

    if (current_ && current_->GetType() == IT_SAMPLE) {
        FieldView::Redraw();
    } else {
        pos._y += 2;
        UIStaticField noEq(pos, "EQ available for sample instruments only");
        noEq.Draw(w_);
    }

    drawMap();
    drawNotes();
}

void EqualizerView::OnFocus() { onInstrumentChange(); }

void EqualizerView::Update(Observable &o, I_ObservableData *) {
    if (&o == current_) {
        isDirty_ = true;
        return;
    }
    onInstrumentChange();
}
