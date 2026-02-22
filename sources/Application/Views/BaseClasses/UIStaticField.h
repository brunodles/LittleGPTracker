#ifndef _UI_STATIC_FIELD_H_
#define _UI_STATIC_FIELD_H_

#include "UIField.h"

class UIStaticField: public UIField {

public:

	UIStaticField(GUIPoint &position,const char *string) ;
	virtual ~UIStaticField() {} ;
	virtual void Draw(GUIWindow &w, int offset=0) ;
	virtual void ProcessArrow(unsigned short mask) ;
	virtual void OnClick() {} ;

	virtual bool IsStatic() ;

    /** Type 'int', but expected to be 'ColorDefinition'.
     * The build was failing for w32. */
    int color = CD_NORMAL;

  protected:
    const char *string_;
} ;

#endif
