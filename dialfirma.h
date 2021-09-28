
#ifndef _DIALFIRMA_H
#define _DIALFIRMA_H

#include <Window.h>
#include <sqlite3.h>

class BBox;
class BButton;
class BMessage;
class BTextControl;
class BView;

class dialFirma : public BWindow {
	public:
		dialFirma(const char *title, sqlite3 *db, bool cancancel);
		virtual void MessageReceived(BMessage *Message);

	private:
		void commit(void);

		BView *view;
		BBox *box1, *box2;
		BButton *but_ok, *but_cancel;

		BTextControl *data[11];
		BTextControl *wystawil;

		sqlite3 *dbData;
		char *dbErrMsg;

};

#endif
