#include "UIStringVarField.h"
#include "Application/AppWindow.h"

UIStringVarField::UIStringVarField(GUIPoint &position, Variable &v)
    : UIIntVarField(position, v, "%s", 0, 0, 1, 1) {}

void UIStringVarField::Draw(GUIWindow &w, int offset) {
    GUITextProperties props;
    GUIPoint position = GetPosition();
    position._y += offset;

    if (focus_) {
        ((AppWindow &)w).SetColor(CD_CURSOR);
        props.invert_ = true;
    } else {
        ((AppWindow &)w).SetColor(CD_TEXT_VALUE);
    }

    w.DrawString(src_.GetString(), position, props);
}
