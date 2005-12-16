
#ifndef _DIALCALENDAR_H
#define _DIALCALENDAR_H

#include <Window.h>

class BButton;
class BHandler;
class BMessage;
class BStringView;
class BTextControl;
class BView;

class dialCalendar : public BWindow {
	public:
		dialCalendar(const char *inidate, BTextControl *ptr, int32 msg, BHandler *hr);
		void MessageReceived(BMessage *Message);

	private:
		void RefreshCalendar(void);
		const char *makeDateString(void);

		BTextControl *dateField;
		BHandler *handler;
		BView *view;
		BStringView *monthyear;
		BButton *but_prevy, *but_prevm, *but_nextm, *but_nexty;
		BButton *caltab[7][6];
		int caldaytab[7][6];
		BButton *but_ok;

		int32 msgdc;
		int year, month, day;
};

#endif
