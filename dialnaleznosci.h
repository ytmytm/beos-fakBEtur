
#ifndef _DIALNALEZNOSCI_H
#define _DIALNALEZNOSCI_H

#include <Window.h>
#include <sqlite.h>

class BButton;
class BMessage;
class BStringView;
class BTextControl;
class BView;
class ColumnListView;
class dialNalodb;

class dialNaleznosci : public BWindow {
	public:
		dialNaleznosci(sqlite *db);
		void MessageReceived(BMessage *Message);
//		bool QuitRequested();

	private:
		void DoFind(void);
		void DoPayFor(int item);
		void DoPayForAll(void);
		void DoShowNaleznosciOdb(int item);
		const char *execSQL(const char *input);
		const char *validateDecimal(const char *input);

		BView *view;
		BButton *but_find, *but_whoowes, *but_pay, *but_payall;
		BTextControl *daysago;
		ColumnListView *list;

		dialNalodb *nalodbDialog;

		sqlite *dbData;
		char *dbErrMsg;
};

#endif
