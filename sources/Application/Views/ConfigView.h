#ifndef _CONFIG_VIEW_H_
#define _CONFIG_VIEW_H_

// Disable ConfigView for Win32 and Apple.
#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__MACH__)
#define CONFIG_VIEW_ENABLED

#include "BaseClasses/FieldView.h"
#include "BaseClasses/UIBoolField.h"
#include "Foundation/Observable.h"
#include "System/Console/Trace.h"
#include "ViewData.h"

class ConfigView: public FieldView,public I_Observer {
public:
	ConfigView(GUIWindow &w,ViewData *data) ;
	virtual ~ConfigView() ;

	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType,unsigned int) {} ;
	virtual void OnFocus() {} ;

    // Observer for action callback
    void Update(Observable &,I_ObservableData *) ;

protected:
private:
  Project *project_;
  // Debug
  unsigned long lastTick_;
  unsigned long lastClock_;

  UIBoolField *autoLoadField;
  UIField *themeField_;

  void insertLabel(GUIPoint position, char *name);

} ;

#endif
#endif
