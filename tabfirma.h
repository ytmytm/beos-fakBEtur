
#ifndef _TABFIRMA_H
#define _TABFIRMA_H

#include "befaktab.h"
#include "fakdata.h"

class BBox;
class BButton;
class BCheckBox;
class BListView;
class BMessage;
class BTextControl;

class tabFirma : public beFakTab {

	public:
		tabFirma(BTabView *tv);
		~tabFirma();
		void MessageReceived(BMessage *Message);
		//
		void DoCommitCurdata(void);
		//
		// update - enable/disable widgets, parse msg (combo items)
		// curdataTo - read data from curtab to widgets, call update
		// curdataFrom - read from widgets to curtab
		void curdataFromTab(void);
		void curdataToTab(void);
		void updateTab(void);

	private:
		BButton *but_new, *but_del, *but_restore, *but_save;
		BListView *list;
		BBox *box1, *box2, *box3, *box4;
		BTextControl *data[11];
		BCheckBox *odbiorca, *dostawca, *aktywny, *zablokowany;
		firmadat *curdata;
};

#endif
