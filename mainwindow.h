
#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

	#include <Window.h>
	#include <sqlite3.h>
	#include "globals.h"

	class beFakTab;
	class dialFirma;
	class dialNaleznosci;
	class dialNumber;
	class dialStat;
	class dialVat;
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
			virtual void DispatchMessage(BMessage *message, BHandler *handler);
			virtual bool QuitRequested();
//			virtual void RefsReceived(BMessage *Message);
			void DoAbout(void);
			void DoConfigFirma(bool cancancel = true);
			void DoConfigVAT(void);
			void DoConfigCopies(void);
			void DoConfigCopiesAfter(BMessage *msg);
			void DoConfigPayday(void);
			void DoConfigPaydayAfter(BMessage *msg);
			void DoStatMies(void);
			void DoStatNaleznosci(void);
			void DoCheckConfig(void);
		private:
			// database handlers
			int OpenDatabase(void);
			void CloseDatabase(void);
			void InitDatabase(void);
			// other
			void updateMenus(void);

			// main view widgets
			BMenuBar *menuBar;
			BMenu *menu;
			BTabView *tabView;
			// dialogs
			dialFirma *firmaDialog;
			dialVat *vatDialog;
			dialNumber *numDialog;
			dialStat *statDialog;
			dialNaleznosci *nalezDialog;
			// print menus, options
			BMenuItem *pmenuo, *pmenuc, *pmenud, *pmenue;
			BMenuItem *pmenups, *pmenut80, *pmenut136, *pmenuhtml;
			BMenuItem *fmenunum;
			// print settings
			int p_mode;			// 0 - print_server, 1 - text, 2 - html
			int p_typ;			// 0 - orig, 1 - kopie, 2 - dupl, 3 - or+kopie
			int p_textcols;		// 80/136
			int p_texteol;		// 0 - CR+LF, 1 - LF, 2 - CR
			// faktura settings
			bool f_numprosta;	// true - numeracja bez miesiąca

			// tabs
			beFakTab *curTab, *tabs[3];

			// database
			sqlite3 *dbData;
			char *dbErrMsg;
	};

#endif
