
#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

	#include <SpLocaleApp.h>
	#include <Window.h>
	#include <sqlite.h>
	#include "globals.h"

// helper
int toint(const char *input);

// fakt data storage
#include <String.h>
class fakdat {
	public:
	fakdat(void) { clear(); };
	~fakdat() { };
	void dump_all(void);
	void clear(void);
	// data holders
	bool dirty;
	int id;
	// data
};

	class beFakTab;
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
		private:
			// database handlers
			int OpenDatabase(void);
			void CloseDatabase(void);
			void RefreshIndexList(void);
			void FetchCurdata(int id);
			bool CommitCurdata(bool haveCancelButton = true);	// action?
			void DoCommitCurdata(void);
			void DoDeleteCurdata(void);
			int GenerateId(void);
			// tab handlers
			void initTabs(BTabView *tv);
			void curdataFromTabs(void);
			void curdata2Tabs(void);
			// init - setup widgets, call update
			// update - enable/disable widgets, parse msg (combo items)
			// curdata2 - read data from curtab to widgets, call update
			// curdatafrom - read from widgets to curtab
			// tab1
			void initTab1(BTabView *tv);
			void updateTab1(BMessage *msg = NULL);
			void curdataFromTab1(void);
			void curdata2Tab1(void);

			// action
			void ChangedSelection(int newid);

			// main view widgets
			BMenuBar *menuBar;
			BMenu *menu;
			BListView *listView;
			BTabView *tabView;

			// bottom navigation/control
//			BButton *but_new, *but_clear, *but_restore, *but_delete, *but_save;
			// tab1 controls
//			BTextControl *t1miejsc, *t1nazwalokalna, *t1gmina, *t1powiat, *t1wojewodztwo;
//			BTextControl *t1nrobszaru, *t1nrinwent, *t1x, *t1y, *t1stanmiejsc, *t1stanobszar;
//			BCheckBox *t1cz, *t1ct, *t1cl, *t1cw, *t1ca, *t1cp, *t1cr;
			beFakTab *curTab, *tab1;

			// database
			sqlite *dbData;
			char *dbErrMsg;
			// data holders
			fakdat *curdata;	// must exist BEFORE tabs are created
			int currentid;	// id of currently edited data, <0 -> INSERT >=0 -> UPDATE
			int *idlist;	// ids of listView data
	};

#endif
