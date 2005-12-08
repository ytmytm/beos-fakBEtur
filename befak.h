
#ifndef _BEFAKAPP_H
#define _BEFAKAPP_H

#include <Application.h>

class BeFAKMainWindow;

class BeFAKApp : public BApplication {
	public:
		BeFAKApp();
		~BeFAKApp();
		virtual void ReadyToRun();
	private:
		BeFAKMainWindow *mainWindow;
};

#endif
