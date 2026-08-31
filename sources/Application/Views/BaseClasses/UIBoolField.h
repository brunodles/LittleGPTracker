#ifndef _UI_BOOL_FIELD_H_
#define _UI_BOOL_FIELD_H_

#include "UIField.h"
#include "Foundation/Observable.h"

/**
 * A simple boolean field that renders "Yes"/"No" and notifies observers when
 * its value changes.  Designed for forms/settings where a binary choice is
 * required.
 */
class UIBoolField : public UIField, public Observable, public I_ObservableData {
public:
    /**
     * @param position screen coordinates of the field
     * @param initialValue starting boolean value
     */
    UIBoolField(GUIPoint &position, bool initialValue);
    virtual ~UIBoolField();

    /// Draw the field (Yes/No)
    virtual void Draw(GUIWindow &w,int offset=0) override;

    /// Toggle the value when clicked
    virtual void OnClick() override;

    /// Allow arrows to flip the value as well
    virtual void ProcessArrow(unsigned short mask) override;

    /// Current value
    bool GetValue() const;

    /// Explicit setter (notifies observers on change)
    void SetValue(bool v);

protected:
    bool value_;
};

#endif
