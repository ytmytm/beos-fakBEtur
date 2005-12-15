
#include <Button.h>
#include <String.h>
#include <TextControl.h>
#include <View.h>

#include "globals.h"
#include "dialnumber.h"
#include <stdio.h>

const uint32 DC			= 'NUDC';
const uint32 BUT_OK		= 'NUOK';

dialNumber::dialNumber(const char *title, const char *name, const char *def, uint32 msg, BHandler *hr) : BWindow(
	BRect(100, 100, 380, 200),
	title,
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ) {

	handler = hr;
	mess = msg;

	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
	view = new BView(this->Bounds(), "numberView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	view->AddChild(number = new BTextControl(BRect(35,20,240,40), "numberSymbol", name, def, new BMessage(DC)));
	number->SetDivider(be_plain_font->StringWidth(number->Label())+5);
	view->AddChild(but_ok = new BButton(BRect(210,55,260,80), "numberButok", "OK", new BMessage(BUT_OK)));
	number->MakeFocus();
	but_ok->MakeDefault(true);
	this->Show();
}

void dialNumber::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case BUT_OK:
			{
				BMessage *msg = new BMessage(mess);
				msg->AddString("_value", number->Text());
				handler->Looper()->PostMessage(msg);
				Quit();
				break;
			}
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}
