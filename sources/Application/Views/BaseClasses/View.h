
#ifndef _VIEW_H_
#define _VIEW_H_

#include "Application/Model/Config.h"
#include "Application/Model/Project.h"
#include "Application/Player/Player.h"
#include "Foundation/T_SimpleList.h"
#include "I_Action.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "ViewEvent.h"
#ifdef SDL2
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 30
#define POS_Y_LAST_LINE 29
#define POS_Y_BEFORE_LAST_LINE 28

/** Event maks to be used with Bitwise operations */
enum GUIEventPadButtonMasks {
    EPBM_LEFT = 1,
    EPBM_DOWN = 2,
    EPBM_RIGHT = 4,
    EPBM_UP = 8,
    EPBM_L = 16,
    EPBM_B = 32,
    EPBM_A = 64,
    EPBM_R = 128,
    EPBM_START = 256,
    EPBM_SELECT = 512,
    EPBM_DOUBLE_A = 1024,
    EPBM_DOUBLE_B = 2048
};

enum ViewType {
    /** Main screen, select the order of the chains, play the song, make it live
     */
    VT_SONG,
    /** Select the Phrases of a Chain */
    VT_CHAIN,
    /** Create phrases */
    VT_PHRASE,
    /** Project configuration, changes per propject or affects the project */
    VT_PROJECT,
    /** LGPT configuration, configurations that are global to the application */
    VT_CONFIG,
    /** Customize the instrument */
    VT_INSTRUMENT,
    /** Table screen under phrase */
    VT_TABLE,
    /** Table screen under instrument */
    VT_TABLE2,
    /** Define the Groove */
    VT_GROOVE,
    /** [WIP] Global Mixer, adjust the levels of each instrument and possible
       effects */
    VT_MIXER
};

enum ViewMode {
    VM_NORMAL,
    VM_NEW,
    VM_CLONE,
    VM_SELECTION,
    VM_MUTEON,
    VM_SOLOON
};

enum ColorDefinition {
    /** background color */
    CD_BACKGROUND,
    /** text color: default */
    CD_NORMAL,
    /** color of the border in the start screen / dialogs */
    CD_BORDER,
    /** row count in song screen */
    CD_HILITE1,
    /** cursor color */
    CD_HILITE2,
    /** text color: for console prints */
    CD_CONSOLE,
    /** text color: cursor / selected */
    CD_CURSOR,
    /** play indicator color */
    CD_PLAY,
    /** mute indicator color */
    CD_MUTE,
    /** text color: when content is "FE" */
    CD_SONGVIEWFE,
    /** text color: when content is "00" */
    CD_SONGVIEW00,
    /** text color: when content is empty ("--", "----") */
    CD_BLANKSPACE,
    /** text color: row type 1 */
    CD_ROW,
    /** text color: row type 2 (eg. when index is multiple of 4) */
    CD_ROW2,
    /** color of "--" at row 00,04,08,0c in phrase screen */
    CD_MAJORBEAT
};

enum ViewUpdateDirection { VUD_LEFT = 0, VUD_RIGHT, VUD_UP, VUD_DOWN };

class View;
class ModalView;

typedef void (*ModalViewCallback)(View &v, ModalView &d);

class View : public Observable {
  public:
    View(GUIWindow &w, ViewData *viewData);
    View(View &v);

    void SetFocus(ViewType vt) {
        viewType_ = vt;
        hasFocus_ = true;
        OnFocus();
    };

    void LooseFocus() { hasFocus_ = false; };

    void Clear();

    void ProcessButton(unsigned short mask, bool pressed);

    void Redraw();

    // Override in subclasses

    virtual void DrawView() = 0;
    virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick) = 0;
    virtual void OnFocus() = 0;

    void SetDirty(bool dirty);

    // Primitive locking mechanism

    bool Lock();
    void WaitForObject();
    void Unlock();

    // Char based draw routines

    virtual void SetColor(ColorDefinition cd);
    virtual void ClearRect(int x, int y, int w, int h);
    virtual void DrawString(int x, int y, const char *txt,
                            GUITextProperties &props);

    void DoModal(ModalView *view, ModalViewCallback cb = 0);

    void EnableNotification();
    void SetNotification(const char *notification, int offset = 2);

  protected:
    virtual void ProcessButtonMask(unsigned short mask, bool pressed) = 0;

    // to remove once everything got to viewdata

    inline void updateData(unsigned char *c, int offset, unsigned char limit,
                           bool wrap) {
        int v = *c;
        if (v == 0xFF) { // Uninitiaized data
            v = 0;
        }
        v += offset;
        if (v < 0)
            v = (wrap ? (limit + 1 + v) : 0);
        if (v > limit)
            v = (wrap ? v - (limit + 1) : limit);
        *c = v;
    }

    GUIPoint GetAnchor();
    GUIPoint GetTitlePosition();

    void drawMap();
    void drawNotes();

  public: // temp hack for modl windo constructors
    GUIWindow &w_;
    ViewData *viewData_;

  protected:
    ViewMode viewMode_;
    bool isDirty_; // .Do we need to redraw screeen
    ViewType viewType_;
    bool hasFocus_;

  private:
    unsigned short mask_;
    bool locked_;
    uint32_t notificationTime_;
    uint16_t NOTIFICATION_TIMEOUT;
    std::string displayNotification_;
    int notiDistY_;
    static bool initPrivate_;
    ModalView *modalView_;
    ModalViewCallback modalViewCallback_;

  public:
    static int margin_;
    static int songRowCount_;
    static bool miniLayout_;
    static int altRowNumber_;
    static int majorBeatNumber_;
};

#endif
