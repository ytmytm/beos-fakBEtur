
#ifndef _DIALNALODB_H
#define _DIALNALODB_H

#include <Window.h>
#include <sqlite3.h>
#include "befaktab.h"

class BBox;
class BButton;
class BMessage;
class BStringView;
class BView;
class ColumnListView;

class dialNalodb : public BWindow, public beFakTab {
	public:
		dialNalodb(sqlite3 *db, const char *odb);
		void MessageReceived(BMessage *Message);

	private:
		BView *view;
		BButton *but_close;
		BBox *boxl, *boxr;
		BStringView *razemnold, *razemn, *razemp;
		ColumnListView *listl, *listr;

		int id;
		BString odbiorca;
};

#endif
