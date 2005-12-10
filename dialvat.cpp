
#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <ListView.h>
#include <ScrollView.h>
#include <String.h>
#include <TextControl.h>
#include <View.h>

#include "globals.h"
#include "dialvat.h"
#include "fakdata.h"	// toint...
#include <stdio.h>

const uint32 BUT_NEW	= 'DVBN';
const uint32 BUT_DEL	= 'DVBD';
const uint32 BUT_SAVE	= 'DVBS';
const uint32 DC			= 'DVDC';
const uint32 LIST_INV	= 'DVLI';
const uint32 LIST_SEL	= 'DVLS';

dialVat::dialVat(sqlite *db, BHandler *hr) : BWindow(
	BRect(120, 120, 120+380, 120+250),
	"Stawki VAT",
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ), beFakTab(NULL, db, hr) {

	idlist = NULL;

	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);

	view = new BView(this->Bounds(), "vatView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	list = new BListView(BRect(10,10,110,180), "dVListView");
	this->view->AddChild(new BScrollView("dVScrollView", list, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));

	box1 = new BBox(BRect(150,45,350,130), "dVBox1");
	box1->SetLabel("Dane stawki podatku");
	view->AddChild(box1);

	nazwa  = new BTextControl(BRect(10,20,160,40), "dVNazwa", "Nazwa", NULL, new BMessage(DC));
	stawka = new BTextControl(BRect(10,50,160,70), "dvStawka", "Stawka (%)", NULL, new BMessage(DC));
	box1->AddChild(nazwa);
	box1->AddChild(stawka);

	but_new = new BButton(BRect(160,200,240,230), "dv_but_new", "Nowa", new BMessage(BUT_NEW), B_FOLLOW_BOTTOM);
	but_del = new BButton(BRect(30,200,110,230), "dv_but_del", "Usuń", new BMessage(BUT_DEL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_save = new BButton(BRect(290,200,370,230), "dv_but_save", "Zapisz", new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);

	view->AddChild(but_new);
	view->AddChild(but_del);
	view->AddChild(but_save);

	// fix widths
	float d = max(be_plain_font->StringWidth(nazwa->Label())+5,be_plain_font->StringWidth(stawka->Label())+5);
	nazwa->SetDivider(d); stawka->SetDivider(d);

	makeNewStawka();
	RefreshIndexList();
	Show();
}

dialVat::~dialVat() {
	delete [] idlist;
}

bool dialVat::QuitRequested(void) {
	// refresh vat menus everywhere
	BMessage *msg;
	msg = new BMessage(MSG_REQVATUP);
	handler->Looper()->PostMessage(msg);
	return true;
}

void dialVat::makeNewStawka(void) {
	id = -1;
	this->dirty = false;
	list->DeselectAll();
	nazwa->SetText(""); stawka->SetText("");
	updateTab();
}

void dialVat::updateTab(void) {
	bool state = (id<0);
	stawka->SetText(validateDecimal(stawka->Text()));
	stawka->SetEnabled(state);
}

void dialVat::MessageReceived(BMessage *Message) {
	BString sql;
	int ret;

	switch (Message->what) {
		case DC:
			this->dirty = true;
			break;
		case BUT_NEW:
			makeNewStawka();
			break;
		case BUT_SAVE:
			if (id>=0) {
				sql = "UPDATE stawka_vat SET nazwa = %Q WHERE aktywne = 1 AND id = "; sql << id;
				ret = sqlite_exec_printf(dbData, sql.String(), 0, 0, &dbErrMsg, nazwa->Text());
			} else {
				// dopisujemy nowy, ostrzec ze nieodwracalnie???
				if (strlen(nazwa->Text())>0) {
					stawka->SetText(validateDecimal(stawka->Text()));
					ret = sqlite_exec_printf(dbData,
					"INSERT INTO stawka_vat (nazwa,stawka,aktywne) VALUES ( %Q, %Q, 1 )",
					0, 0, &dbErrMsg, nazwa->Text(), stawka->Text());
				}
			}
			RefreshIndexList();
			break;
		case BUT_DEL:
			if (id>=0) {
				sql = "UPDATE stawka_vat SET aktywne = 0 WHERE id = "; sql << id;
				ret = sqlite_exec_printf(dbData, sql.String(), 0, 0, &dbErrMsg);
			}
			makeNewStawka();
			RefreshIndexList();
			break;
		case LIST_SEL:
		case LIST_INV:
			{
				int i = list->CurrentSelection(0);
				if (i>=0) {
					printf("sel:%i,id=%i\n",i,idlist[i]);
					ChangedSelection(idlist[i]);
				}
				break;
			}
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}

void dialVat::ChangedSelection(int newid) {
	// fetch and store new data
	id = newid;
	if (id >=0) {
		int nRows, nCols;
		char **result;
		BString sql = "SELECT nazwa, stawka FROM stawka_vat WHERE id = "; sql << id;
		sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
		if (nRows < 1) {
			// no entries
		} else {
			nazwa->SetText(result[nCols]);
			stawka->SetText(validateDecimal(result[nCols+1]));
		}
		sqlite_free_table(result);
		this->dirty = false;
		updateTab();
	}
}

void dialVat::RefreshIndexList(void) {
	// clear current list
	if (list->CountItems()>0) {
		BStringItem *anItem;
		for (int i=0; (anItem=(BStringItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
	// clear current idlist
	if (idlist!=NULL) {
		delete [] idlist;
		idlist = NULL;
	}
	// select list from db
	int nRows, nCols;
	char **result;
	sqlite_get_table(dbData, "SELECT id, nazwa, stawka FROM stawka_vat WHERE aktywne = 1 ORDER BY id", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
	} else {
		BString tmp;
		idlist = new int[nRows];
		for (int i=1;i<=nRows;i++) {
			idlist[i-1] = toint(result[i*nCols+0]);
			tmp = result[i*nCols+1];
			tmp << ", " << result[i*nCols+2];
			list->AddItem(new BStringItem(tmp.String()));
		}
	}
	sqlite_free_table(result);
}
