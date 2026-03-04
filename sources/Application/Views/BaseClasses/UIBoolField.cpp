#include "UIBoolField.h"
#include "Application/AppWindow.h"

UIBoolField::UIBoolField(GUIPoint &position, bool initialValue)
    : UIField(position), value_(initialValue) {
}

UIBoolField::~UIBoolField() {
}

void UIBoolField::Draw(GUIWindow &w, int offset) {
    GUITextProperties props;
    GUIPoint pos = GetPosition();
    pos._y += offset;

    if (focus_) {
        ((AppWindow &)w).SetColor(CD_HILITE2);
        props.invert_ = true;
    } else {
        ((AppWindow &)w).SetColor(CD_NORMAL);
        props.invert_ = false;
    }

    const char *text = value_ ? "Yes" : "No";
    w.DrawString(text, pos, props);
}

void UIBoolField::OnClick() {}

void UIBoolField::ProcessArrow(unsigned short mask) {
    switch (mask) {
        case EPBM_UP:
            SetValue(true);
            break;
        case EPBM_DOWN:
            SetValue(false);
            break;
        case EPBM_LEFT:
        case EPBM_RIGHT:
            SetValue(!value_);
            break;
        default:
            break;
    }
}

bool UIBoolField::GetValue() const {
    return value_;
}

void UIBoolField::SetValue(bool v) {
    if (v != value_) {
        value_ = v;
        NotifyObservers(this);
    }
}
