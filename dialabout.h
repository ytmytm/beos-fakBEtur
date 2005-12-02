
#ifndef _DIALABOUT_H
#define _DIALABOUT_H

#include <View.h>
#include <Window.h>

class dialAbout : public BWindow {
	public:
		dialAbout(const char *title);
};

class dialView : public BView {
	public:
		dialView(BRect frame, const char *name, const char *title);
		virtual void Draw(BRect updateRect);
	private:
		const char *appname;
};

#endif
