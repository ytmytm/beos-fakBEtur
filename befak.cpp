
#include "befak.h"
#include "dialabout.h"
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

void BeFAKApp::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		default:
			BApplication::MessageReceived(msg);
			break;
	}
}

void BeFAKApp::AboutRequested(void) {
	aboutDialog = new dialAbout("O programie");
	aboutDialog->SetApplicationName(APP_NAME);
	aboutDialog->SetVersionNumber(APP_VERSION);
	aboutDialog->SetCopyrightString(B_UTF8_COPYRIGHT"2006 by Maciej Witkowiak");
	aboutDialog->SetText("Kontakt: <mwitkowiak@gmail.com>, <ytm@elysium.pl>\n"
		"http://members.elysium.pl/ytm/html\n"
		"http://ytm.bossstation.dnsalias.org/html/\n"
		"\nProgram powstał na konkurs zorganizowany\nprzez portal http://www.haiku-os.pl.\n"
		"\t\t\t\t\t\t\t:* E.");
	aboutDialog->Show();
}
