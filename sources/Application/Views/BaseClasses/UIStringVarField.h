#ifndef _UI_STRING_VAR_FIELD_H_
#define _UI_STRING_VAR_FIELD_H_

#include "UIIntVarField.h"

// Read-only field bound to a STRING variable: draws the variable's current
// string value and ignores arrow keys (no editing). Used by the note
// detection IN/OUT readouts.
//
// Inherits from UIIntVarField (not UIField) so the existing casts in
// InstrumentView::onInstrumentChange() and ProcessButtonMask() keep working:
// GetVariableID()/GetVariable() come from the base class.

class UIStringVarField: public UIIntVarField {

public:
    UIStringVarField(GUIPoint &position, Variable &v);

    virtual ~UIStringVarField() {}

    virtual void Draw(GUIWindow &w, int offset = 0);
    virtual void ProcessArrow(unsigned short mask) {}
    virtual void OnClick() {}
};

#endif // _UI_STRING_VAR_FIELD_H_
