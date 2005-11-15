
#ifndef _BEFAKAPP_H
#define _BEFAKAPP_H

#include <SpLocaleApp.h>

#include "mainwindow.h"

class BeFAKApp : public SpLocaleApp {
	public:
		BeFAKApp();
		~BeFAKApp();
		virtual void ReadyToRun();
	private:
		BeFAKMainWindow *mainWindow;
};

#endif
