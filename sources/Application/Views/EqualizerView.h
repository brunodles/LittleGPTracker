#ifndef _EQUALIZER_VIEW_H_
#define _EQUALIZER_VIEW_H_

#include "Application/Instruments/CommandList.h"
#include "BaseClasses/FieldView.h"
#include "Foundation/Observable.h"
#include "ViewData.h"

class Project;
class I_Instrument;

class EqualizerView : public FieldView, public I_Observer {
public:
    EqualizerView(GUIWindow &w, ViewData *data);
    virtual ~EqualizerView();

    virtual void ProcessButtonMask(unsigned short mask, bool pressed);
    virtual void DrawView();
    virtual void OnPlayerUpdate(PlayerEventType, unsigned int) {}
    virtual void OnFocus();
    virtual void Update(Observable &o, I_ObservableData *d);

private:
    void onInstrumentChange();
    void fillParameters();
    void renderHelpLine(const char *line1, const char *line2);
    void describeFocus(char *line1, char *line2, int size);
    int getBandIndex(FourCC id, bool *isGainQ = 0) const;
    void savePresetSlot();
    void applyPresetSlot();

private:
    Project *project_;
    I_Instrument *current_;
    FourCC lastFocusID_;
    int presetSlot_;
};

#endif
