
#ifndef _DIALNUMBER_H
#define _DIALNUMBER_H

#include <Window.h>
#include <sqlite3.h>

class BHandler;
class BButton;
class BMessage;
class BTextControl;
class BView;

class dialNumber : public BWindow {
	public:
		dialNumber(const char *title, const char *name, const char *def, uint32 msg, BHandler *hr);
		void MessageReceived(BMessage *Message);

	private:
		BHandler *handler;
		BView *view;
		BButton *but_ok;
		BTextControl *number;
		uint32 mess;
};

#endif
