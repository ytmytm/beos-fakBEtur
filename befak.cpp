
#include "befak.h"
#include "globals.h"
#include "mainwindow.h"

BeFAKApp::BeFAKApp() : BApplication(APP_SIGNATURE) {
	mainWindow = new BeFAKMainWindow(APP_NAME);
	if (mainWindow != NULL) {
		mainWindow->Show();
	} else {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
}

BeFAKApp::~BeFAKApp() {
	if (mainWindow != NULL)
		if (mainWindow->LockWithTimeout(30000000) == B_OK)
			mainWindow->Quit();
}

void BeFAKApp::ReadyToRun() {
	// sth etc. just before running Run()
}
