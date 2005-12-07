
#ifndef _DIALABOUT_H
#define _DIALABOUT_H

#include <View.h>
#include <Window.h>

class BBitmap;
class BStringView;
class BTextView;

class AboutView : public BView {
public:
	AboutView(BRect frame, const char *name, BBitmap *icon);
//	~AboutView();
	
	virtual void 	Draw(BRect updateRect);
	void			SetIcon(BBitmap *icon);
private:
	BBitmap	*bIcon;
};

class dialAbout : public BWindow {
public:
	dialAbout(const char *title);
//	~dialAbout();
	
	void SetApplicationName(const char *);
	void SetVersionNumber(const char *);
	void SetIcon(BBitmap *);
	void SetCopyrightString(const char *);
	void SetText(const char *);
	
	virtual bool QuitRequested();
	
private:
	AboutView	*vAbout;
	BStringView	*applicationNameSV,
			*versionNumberSV,
			*copyrightStringSV;
	BTextView	*textTV;
};

#endif
