
#ifndef _DIALIMPORT_H
#define _DIALIMPORT_H

#include <Window.h>
#include <sqlite3.h>

class pozfaklist;
class BButton;
class BHandler;
class BMessage;
class BView;
class ColumnListView;

class dialImport : public BWindow {
	public:
		dialImport(sqlite3 *db, int aktualna, pozfaklist *faklista, BHandler *hr);
		virtual ~dialImport();
		virtual void MessageReceived(BMessage *Message);

	private:
		bool commit(void);

		BHandler *handler;
		BView *view;
		BButton *but_import;
		ColumnListView *list;

		pozfaklist *flist;

		sqlite3 *dbData;
		char *dbErrMsg;
};

#endif
