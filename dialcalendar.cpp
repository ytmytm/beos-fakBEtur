
#include <Button.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <Message.h>
#include <View.h>

#include "globals.h"
#include "dialcalendar.h"
#include "fakdata.h"	// toint...
#include <stdio.h>

const uint32 BUT_OK		= 'CBOK';
const uint32 BUT_CAL	= 'CBCA';
const uint32 BUT_PREVY	= 'CBPY';
const uint32 BUT_PREVM	= 'CBPM';
const uint32 BUT_NEXTM	= 'CBNM';
const uint32 BUT_NEXTY	= 'CBNY';

const char *shortweekdays[] = { "ni", "po", "wt", "śr", "cz", "pi", "so", NULL };

dialCalendar::dialCalendar(const char *inidate, BTextControl *ptr, int32 msg, BHandler *hr) : BWindow(
	BRect(100+20, 100+20, 295+20, 320+20+25),
	NULL,
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ) {

	msgdc = msg;
	dateField = ptr;
	handler = hr;

	// split inidate into y/m/d
	BString tmp;
	tmp = inidate;
	tmp.Remove(4,tmp.Length()-4);
	year = toint(tmp.String());
	tmp = inidate;
	tmp.Remove(0,5);
	tmp.Remove(2,tmp.Length()-2);
	month = toint(tmp.String());
	tmp = inidate;
	tmp.Remove(0,8);
	day = toint(tmp.String());

	this->SetTitle("Wybierz datę");
	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
	view = new BView(this->Bounds(), "calendarView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);
	view->AddChild(monthyear = new BStringView(BRect(60,5,135,25), "calendarMonthYear", NULL));
	monthyear->SetAlignment(B_ALIGN_CENTER);
	view->AddChild(but_prevy = new BButton(BRect(10,5,30,25), "calendarButPrevy", "<<", new BMessage(BUT_PREVY)));
	view->AddChild(but_prevm = new BButton(BRect(35,5,55,25), "calendarButPrevm", "<", new BMessage(BUT_PREVM)));
	view->AddChild(but_nextm = new BButton(BRect(140,5,160,25), "calendarButNextm", ">", new BMessage(BUT_NEXTM)));
	view->AddChild(but_nexty = new BButton(BRect(165,5,185,25), "calendarButNexty", ">>", new BMessage(BUT_NEXTY)));

	int i,j;
	for (i=0;i<=6;i++) {
		view->AddChild(new BStringView(BRect(10+i*25,30,30+i*25,50), NULL, shortweekdays[i]));
	}
	BMessage *mesg;
	for (j=0;j<=5;j++) {
		for (i=0;i<=6;i++) {
			mesg = new BMessage(BUT_CAL);
			mesg->AddInt32("_x", i);
			mesg->AddInt32("_y", j);
			view->AddChild(caltab[i][j] = new BButton(BRect(10+i*25,55+j*25,30+i*25,75+j*25), NULL, "33", mesg));
		}
	}
	view->AddChild(but_ok = new BButton(BRect(120,210,165,230), "calendarButOk", "OK", new BMessage(BUT_OK)));
	but_ok->MakeDefault(true);
	but_ok->ResizeToPreferred();
	RefreshCalendar();
}

bool isLeap (int y) {
	int newYear = y;

	if (newYear < 100)
		newYear += 1900;

	return (((((newYear)%4) == 0 && (((newYear)%100)!=0)) || ((newYear)%400)==0));
}

const int yeardays[]= {0,31,59,90,120,151,181,212,243,273,304,334};
int getdayofweek (int y, int m, int d) {
	int k = y-1;

	d += 365*k+(k/4)-(k/100)+(k/400)+yeardays[m-1];
    if (isLeap(y) && m>2) d++;
		return d%7;
}

const int monthdays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
int daysinmonth (int y, int m) {
    if ((m==2)&& isLeap(y))
        return 29;
    else
        return monthdays[m-1];
}

const char *dialCalendar::makeDateString(void) {
	static char date[11];
	sprintf(date,"%04i-%02i-%02i",year,month,day);
	return date;
}

void dialCalendar::RefreshCalendar(void) {
	BString tmp;
	int d = getdayofweek(year,month,1);
	int m = daysinmonth(year,month);
	int i = 0;
	int j = 0;
	int k = 1;
	// 1st line
	while (i<d) {
		caldaytab[i][j] = -1;
		caltab[i][j]->SetLabel("");
		caltab[i][j]->SetEnabled(false);
		i++;
	};
	// next lines
	while (k<=m) {
		tmp = ""; tmp << k;
		caldaytab[i][j] = k;
		caltab[i][j]->SetLabel(tmp.String());
		caltab[i][j]->SetEnabled(true);
		i++; k++;
		if (i>=7) {
			i=0; j++;
		}
	}
	// finish last lines
	while (j<6) {
		caldaytab[i][j] = -1;
		caltab[i][j]->SetLabel("");
		caltab[i][j]->SetEnabled(false);
		i++;
		if (i>=7) {
			i=0; j++;
		}
	}
	monthyear->SetText(makeDateString());
	this->UpdateIfNeeded();
}

void dialCalendar::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case BUT_PREVY:
			year--;
			RefreshCalendar();
			break;
		case BUT_PREVM:
			month--;
			if (month==0) {
				month = 12; year--;
			}
			RefreshCalendar();
			break;
		case BUT_NEXTM:
			month++;
			if (month==13) {
				month = 1; year++;
			}
			RefreshCalendar();
			break;
		case BUT_NEXTY:
			year++;
			RefreshCalendar();
			break;
		case BUT_CAL:
			{	int32 x,y;
				if (Message->FindInt32("_x", &x) == B_OK) {
					if (Message->FindInt32("_y", &y) == B_OK) {
						day = caldaytab[x][y];
						RefreshCalendar();
					}
				}
				break;
			}
		case BUT_OK:
			{	// set new value
				dateField->Looper()->Lock();
				dateField->SetText(makeDateString());
				dateField->Looper()->Unlock();
				// notify owner
				handler->Looper()->PostMessage(new BMessage(msgdc));
				Quit();
				break;
			}
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}
