
#include <Button.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <View.h>

#include "globals.h"
#include "dialsymbol.h"
#include "fakdata.h"
#include <stdio.h>

const uint32 DC			= 'SYDC';
const uint32 BUT_OK		= 'SYOK';
const uint32 BUT_CANCEL	= 'SYCA';

dialSymbol::dialSymbol(sqlite3 *db, bool towar, dbdat *data, BHandler *hr) : BWindow(
	BRect(100, 100, 420, 260),
	"Wpisz symbol nowych danych",
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ) {

	handler = hr;
	istowar = towar;
	curdata = data;
	dbData = db;

	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
	view = new BView(this->Bounds(), "symbolView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	view->AddChild(new BStringView(BRect(15,20,80,35), "symbolNazwaLabel", "Nazwa:"));
	view->AddChild(symbol = new BTextControl(BRect(15,50,260,75), "symbolSymbol", "Symbol:", NULL, new BMessage(DC)));
	symbol->SetDivider(be_plain_font->StringWidth(symbol->Label())+5);
	BRect r(0,20,240,35);
	r.left = symbol->Divider()+40;
	view->AddChild(nazwa = new BStringView(r, "symbolNazwa", NULL));
	view->AddChild(error = new BStringView(BRect(70,80,240,100), "symbolError", NULL));
	view->AddChild(but_ok = new BButton(BRect(240,110,290,140), "symbolButok", "OK", new BMessage(BUT_OK)));
	view->AddChild(but_cancel = new BButton(BRect(15,110,70,140), "symbolButCancel", "Anuluj", new BMessage(BUT_CANCEL)));
	but_ok->ResizeToPreferred();
	but_cancel->ResizeToPreferred();

	if (istowar) {
		nazwa->SetText(((towardat*)curdata)->data[0].String());
		symbol->SetText(((towardat*)curdata)->data[0].String());
	} else {
		nazwa->SetText(((firmadat*)curdata)->data[0].String());
		symbol->SetText(((firmadat*)curdata)->data[0].String());
	}
	but_ok->MakeDefault(true);
	symbol->MakeFocus();
}

bool dialSymbol::commit(void) {
	BString table;
	BString sql, tmp;
	int nRows, nCols;
	char **result;

	if (strlen(symbol->Text()) == 0) {
		error->SetText("SYMBOL NIE MOŻE BYĆ PUSTY");
		return false;
	}

	table = (istowar) ? "towar" : "firma";
	tmp = symbol->Text(); tmp.ReplaceAll("'","''");	// sql quote
	sql = "SELECT id FROM "; sql += table; sql += " WHERE symbol = '"; sql += tmp; sql += "'";
	// unique?
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
	sqlite3_free_table(result);
	if (nRows > 0) {
		error->SetText("TAKI SYMBOL JUŻ JEST NA LIŚCIE");
		return false;
	}
	if (istowar) {
		((towardat*)curdata)->data[1] = symbol->Text();
	} else {
		((firmadat*)curdata)->data[1] = symbol->Text();
	}
	// commit new data
	curdata->commit();
	// notify
	BMessage *msg;
	if (istowar)
		msg = new BMessage(MSG_REQTOWARLIST);
	else
		msg = new BMessage(MSG_REQFIRMALIST);
	handler->Looper()->PostMessage(msg);

	return true;
}

bool dialSymbol::QuitRequested() {
	delete curdata;
	return true;
}

void dialSymbol::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case BUT_OK:
			if (!(commit())) {	// commit ok?
				break;
			}	// else fall through to exit
		case B_CANCEL:
		case BUT_CANCEL:
			delete curdata;
			curdata = NULL;
			Quit();
			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}
