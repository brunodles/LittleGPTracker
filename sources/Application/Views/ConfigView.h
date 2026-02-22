#ifndef _CONFIG_VIEW_H_
#define _CONFIG_VIEW_H_

#include "BaseClasses/FieldView.h"
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

	void OnLoadProject() ;
	void OnSaveAsProject(char * data) ;
	void OnPurgeInstruments(bool removeFromDisk) ;
	void OnQuit() ;

protected:
private:
  Project *project_;
  // Debug
  unsigned long lastTick_;
  unsigned long lastClock_;
  UIField *tempoField_;

  void insertLabel(GUIPoint position, char *name);

} ;
#endif
