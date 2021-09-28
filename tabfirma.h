
#ifndef _TABFIRMA_H
#define _TABFIRMA_H

#include "befaktab.h"
#include "fakdata.h"

class BBox;
class BButton;
class BCheckBox;
class BHandler;
class BMessage;
class BTextControl;
class ColumnListView;

class tabFirma : public beFakTab {

	public:
		tabFirma(BTabView *tv, sqlite3 *db, BHandler *hr);
		~tabFirma();
		void MessageReceived(BMessage *Message);
		void ChangedSelection(int newid);
		//
		void DoCommitCurdata(void);
		void DoDeleteCurdata(void);
		void DoFetchCurdata(void);
		//
		void RefreshIndexList(void);
		//
		// update - enable/disable widgets, parse msg (combo items)
		// curdataTo - read data from curtab to widgets, call update
		// curdataFrom - read from widgets to curtab
		void curdataFromTab(void);
		void curdataToTab(void);
		void updateTab(void);
		bool validateTab(void);

	private:
		BButton *but_new, *but_del, *but_restore, *but_save, *but_payments;
		ColumnListView *list;
		BBox *box1, *box2, *box3, *box4;
		BTextControl *data[11];
		BCheckBox *odbiorca, *dostawca, *aktywny, *zablokowany;
		firmadat *curdata;
};

#endif
