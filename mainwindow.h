
#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

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
	class BMenuItem;
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
			void InitDatabase(void);
			// tab handlers
			void initTabs(BTabView *tv);
			// other
			void updateMenus(void);

			// main view widgets
			BMenuBar *menuBar;
			BMenu *menu;
			BTabView *tabView;
			// dialogs
			dialAbout *aboutDialog;
			dialFirma *firmaDialog;
			// print menus, options
			BMenuItem *pmenuo, *pmenuc, *pmenud;
			BMenuItem *pmenut80, *pmenut136, *pmenuhtml;
			// print settings (send them with MSG_PRINTCONF)
			int ptyp;			// 0 - orig, 1 - copy, 2 - dupl
			int pmode;			// 0 - print_server, 1 - text, 2 - html
			int pwide;			// for text: 0 - 80, 1 - 136

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
