
#ifndef _TABFIRMA_H
#define _TABFIRMA_H

#include "befaktab.h"

class BBox;
class BButton;
class BCheckBox;
class BListView;
class BTextControl;

class tabFirma : public beFakTab {

	public:
		tabFirma(BTabView *tv);
		~tabFirma();

	private:
		BButton *but_new, *but_del, *but_restore, *but_save;
		BListView *list;
		BBox *box1, *box2, *box3, *box4;
		BTextControl *data[11];
		BCheckBox *odbiorca, *dostawca, *aktywny, *zablokowany;
};

#endif
