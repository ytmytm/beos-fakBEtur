
#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

	#include <SpLocaleApp.h>
	#include <Window.h>
	#include <sqlite.h>
	#include "globals.h"

	class beFakTab;
	class dialAbout;
	class dialFirma;
	class BButton;
	class BCheckBox;
	class BListView;
	class BMenu;
	class BMenuBar;
	class BTabView;
	class BTextControl;

	class BeFAKMainWindow : public BWindow {
		public:
			BeFAKMainWindow(const char *windowTitle);
			~BeFAKMainWindow();
//			virtual void FrameResized(float width, float height);
			virtual void MessageReceived(BMessage *Message);
//			virtual void DispatchMessage(BMessage *message, BHandler *handler);
			virtual bool QuitRequested();
//			virtual void RefsReceived(BMessage *Message);
			void DoAbout(void);
			void DoConfigFirma(bool cancancel = true);
			void DoCheckConfig(void);
		private:
			// database handlers
			int OpenDatabase(void);
			void CloseDatabase(void);
			// tab handlers
			void initTabs(BTabView *tv);

			// main view widgets
			BMenuBar *menuBar;
			BMenu *menu;
			BTabView *tabView;
			// dialogs
			dialAbout *aboutDialog;
			dialFirma *firmaDialog;

			// tabs
			beFakTab *curTab, *tabs[3];

			// database
			sqlite *dbData;
			char *dbErrMsg;
			// configuration
			int liczbakopii;	// # kopii do wydruku
			int ostatni_nr;		// nr ostatniej faktury // XXX a nie w faktab?
			bool num_prosta;	// numeracja uproszczona - bez miesiaca - nr/rok
	};

#endif
