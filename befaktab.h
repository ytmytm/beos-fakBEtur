
#ifndef _BEFAKTAB_H
#define _BEFAKTAB_H

#include "CLVEasyItem.h"
#include <sqlite3.h>

class BHandler;
class BMessage;
class BTab;
class BTabView;
class BView;

class beFakTab {
	public:
		beFakTab(BTabView *tv, sqlite3 *db, BHandler *hr);
		virtual ~beFakTab();

		virtual void MessageReceived(BMessage *Message);
		virtual bool CommitCurdata(bool haveCancelButton = true);
		virtual void DoCommitCurdata(void) { };	// INSERT/UPDATE
		virtual bool validateTab(void) { return true; };

		virtual void curdataFromTab(void) { };
		virtual void curdataToTab(void) { };
		virtual void updateTab(void) { };

		const char *execSQL(const char *input);
		const char *validateDecimal(const char *input);
		const char *validateDate(const char *input);

		BHandler *handler;
		BView *view;
		BTab *tab;

		sqlite3 *dbData;
		char *dbErrMsg;

	protected:
		bool dirty;
};

class tab2ListItem : public CLVEasyItem {
	public:
		tab2ListItem(int id, const char *col0, const char *col1) : CLVEasyItem(
			0, false, false, 20.0) {
			fId = id;
			SetColumnContent(0,col0);
			SetColumnContent(1,col1);
		};
		int Id(void) { return fId; };
	private:
		int fId;
};

#define MSG_NAMECHANGE	'BFNC'

#endif
