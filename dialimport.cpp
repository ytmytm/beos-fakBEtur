
#include <Alert.h>
#include <Button.h>
#include <String.h>
#include <View.h>
#include "ColumnListView.h"

#include "globals.h"
#include "dialimport.h"
#include "befaktab.h"	// tab2list
#include "fakdata.h"
#include <stdio.h>

const uint32 LIST_INV	=	'DILI';
const uint32 BUT_IMPORT =	'DIIM';

dialImport::dialImport(sqlite3 *db, int aktualna, pozfaklist *faklista, BHandler *hr) : BWindow(
	BRect(100, 100, 290, 480),
	"Dokument do importu",
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ) {

	handler = hr;
	flist = faklista;
	dbData = db;

	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
	view = new BView(this->Bounds(), "importView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	view->AddChild(but_import = new BButton(BRect(40,330,140,360), "importButImport", "Import", new BMessage(BUT_IMPORT)));
	but_import->ResizeToPreferred();
	but_import->MakeDefault(true);

	// columnlistview
	CLVContainerView *containerView;
	list = new ColumnListView(BRect(10,10,170,310), &containerView, NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	list->AddColumn(new CLVColumn("Numer", 80, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Data sprzedaży", 79, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->view->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(LIST_INV));
	// refresh list (mostly XXX dupe from tabfaktura)
	// select list from db
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, "SELECT id, nazwa, data_sprzedazy FROM faktura ORDER BY data_sprzedazy, nazwa", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
	} else {
		for (int i=1;i<=nRows;i++)
			if (toint(result[i*nCols+0]) != aktualna)
				list->AddItem(new tab2ListItem(toint(result[i*nCols+0]), result[i*nCols+1], result[i*nCols+2]));
	}
	sqlite3_free_table(result);
	list->MakeFocus();
}

dialImport::~dialImport() {
}

bool dialImport::commit(void) {
	int i, j;
	BString tmp;

	// nie wybrano niczego? wyjść
	i = list->CurrentSelection(0);
	if (i<0)
		return false;
	// fetch list from selected faktura
	fakturadat *olddata = new fakturadat(dbData);
	pozfaklist *olista = new pozfaklist(dbData);
	pozfakdata *curdata;
	// find id of old one
	i = ((tab2ListItem*)list->ItemAt(list->CurrentSelection(0)))->Id();
	olddata->id = i;
	olddata->fetch();
	olista->fetch(i);
	// dla wszystkich towarów na starej:
	pozfakitem *cur = olista->start;
	pozfakitem *ncur;
	while (cur!=NULL) {
		// czy jest na nowej liscie?
		ncur = flist->start;
		j = 0;
		while (ncur!=NULL) {
			if (ncur->data->data[1] == cur->data->data[1]) {
				j++;
				break;
			}
			ncur = ncur->nxt;
		}
		// tak - spytac: pomin, zastap, (zsumuj)
		if (j>0) {
			// tak - spytaj: pomin, zastap
			tmp = "Pozycja o nazwie: "; tmp += cur->data->data[1];
			tmp += "\njuż jest na fakturze.\nIlość na importowanej: ";
			tmp += cur->data->data[3];
			tmp += "\nIlość na aktualnej: "; tmp += ncur->data->data[3];
			BAlert *ask = new BAlert(APP_NAME, tmp.String(), "Zsumuj", "Pomiń", "Zastąp", B_WIDTH_AS_USUAL, B_INFO_ALERT);
			switch (ask->Go()) {
				case 2:	// zastap
					ncur->data->data[3] = cur->data->data[3];
					break;
				case 0:	// suma
					tmp = "SELECT DECROUND(0"; tmp += cur->data->data[3];
					tmp += "+0"; tmp += ncur->data->data[3]; tmp+= ")";
					tmp = flist->execSQL(tmp.String());
					ncur->data->data[3] = tmp;
					break;
				case 1:	// pomin
				default:
					break;
			}
		} else {
			// dodaj nowy do listy
			curdata = new pozfakdata();
			for (i=0; i<=11; i++) {
				curdata->data[i] = cur->data->data[i];
			}
			curdata->vatid = cur->data->vatid;
			flist->addlast(curdata);
		}
		cur = cur->nxt;
	}
	flist->setlp();
	delete olista;
	delete olddata;
	// notify, refresh, make dirty
	BMessage *msg = new BMessage(MSG_REQFAKPOZLIST);
	handler->Looper()->PostMessage(msg);
	return true;
}

void dialImport::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case LIST_INV:
		case BUT_IMPORT:
			if (!(commit())) {	// commit ok?
				// no...
				break;
			}	// else fall through to exit
		case B_CANCEL:
			Quit();
			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}
