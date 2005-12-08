
#ifndef _BEFAKAPP_H
#define _BEFAKAPP_H

#include <Application.h>

class dialAbout;
class BeFAKMainWindow;

class BeFAKApp : public BApplication {
	public:
		BeFAKApp();
		~BeFAKApp();
		virtual void ReadyToRun();
		virtual void MessageReceived(BMessage *message);
		virtual void AboutRequested(void);

	private:
		BeFAKMainWindow *mainWindow;
		dialAbout *aboutDialog;

};

#endif
