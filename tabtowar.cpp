//
// trzymac ceny i marze jako decimal(10,2(-4))? mozna wtedy zwalic obliczenia na sqlite
//		select round(34.23*0.22,2) -> 7.53 (7.5306)
// round działa jeżeli choćby jedna liczba ma '.' - np. 25/100.0
//
// trzymac vat jako foreign key (poczytac o tym!), nie kombinowac z wartosciami stawek
//
// problem: ceny i marża mogą być w ułamkach! (zamiana '.' na ',', tylko cyfry)
//
// sprawdzić jak właściwie działa netto/marża/rabat w bizmaster
// sprawdzić co robi 'usługa'
// sprawdzić co jest w zakładce 'ceny' w cf
// kontrola wypełnienia wymaganych pól (cena, vat, itd.) - czy są i czym są

#include "tabtowar.h"
#include "fakdata.h"

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ListView.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include <stdio.h>

const uint32 LIST_INV	= 'TTLI';
const uint32 LIST_SEL	= 'TTLS';
const uint32 BUT_NEW	= 'TTBN';
const uint32 BUT_DEL	= 'TTBD';
const uint32 BUT_RESTORE= 'TTBR';
const uint32 BUT_SAVE	= 'TTBS';
const uint32 DC			= 'TTDC';

const uint32 MENUJM		= 'TTMJ';
const uint32 MENUVAT	= 'TTMV';

const char *jmiary[] = { "szt.", "kg", "kpl.", "m", "mb", "m2", "km", NULL };

tabTowar::tabTowar(BTabView *tv, sqlite *db) : beFakTab(tv, db) {

	idlist = NULL;
	curdata = new towardat(db);
	this->dirty = false;

	this->tab->SetLabel("Towary");
	BRect r;
	r = this->view->Bounds();

	// listview in scrollview
	r.left = 5; r.right = 160; r.top = 30; r.bottom = 500;
	list = new BListView(r, "tTListView");
	this->view->AddChild(new BScrollView("tTScrollView", list, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));
	// buttons
	but_new = new BButton(BRect(40,0,130,24), "tt_but_new", "Nowy towar", new BMessage(BUT_NEW), B_FOLLOW_LEFT|B_FOLLOW_TOP);
	but_del = new BButton(BRect(40,510,130,534), "tt_but_del", "Usuń zaznaczone", new BMessage(BUT_DEL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_restore = new BButton(BRect(235,510,325,534), "tt_but_restore", "Przywróć", new BMessage(BUT_RESTORE), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_save = new BButton(BRect(580,510,670,534), "tt_but_save", "Zapisz", new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
	this->view->AddChild(but_new);
	this->view->AddChild(but_del);
	this->view->AddChild(but_restore);
	this->view->AddChild(but_save);
	// box1
	box1 = new BBox(BRect(230,30,710,140), "tt_box1");
	box1->SetLabel("Dane towaru");
	this->view->AddChild(box1);
	// box1-stuff
	data[0] = new BTextControl(BRect(10,15,270,35), "ttd0", "Nazwa", NULL, new BMessage(DC));
	data[1] = new BTextControl(BRect(280,15,420,35), "ttd1", "Symbol", NULL, new BMessage(DC));
	data[2] = new BTextControl(BRect(10,50,150,65), "ttd2", "PKWiU", NULL, new BMessage(DC));
	data[3] = new BTextControl(BRect(160,50,270,65), "ttd3", "j.m.", NULL, new BMessage(DC));
	box1->AddChild(data[0]); box1->AddChild(data[1]);
	box1->AddChild(data[2]); box1->AddChild(data[3]);
	usluga = new BCheckBox(BRect(350,50,420,65), "ttdo", "Usługa", new BMessage(DC));
	box1->AddChild(usluga);
	dodany = new BStringView(BRect(280,80,340,95), "tts0", "Dodano");
	box1->AddChild(dodany);
	dodany = new BStringView(BRect(350,80,420,95), "ttsd", "XX-YY-ZZZZ");
	box1->AddChild(dodany);
	// box1-menu
	BMenu *menujm = new BMenu("");
	BMessage *msg;
	int j=0;
	while (jmiary[j] != NULL) {
		msg = new BMessage(MENUJM); msg->AddString("_jm",jmiary[j]);
		menujm->AddItem(new BMenuItem(jmiary[j], msg));
		j++;
	}
	BMenuField *menujmField = new BMenuField(BRect(280,50,340,65), "ttmf", NULL, menujm);
	box1->AddChild(menujmField);
	// box2
	box2 = new BBox(BRect(230,150,710,230), "tt_box2");
	box2->SetLabel("Ceny");
	this->view->AddChild(box2);
	// box2-stuff
	ceny[0] = new BTextControl(BRect(10,15,190,35), "ttc0", "Cena netto (zł)", NULL, new BMessage(DC));
	ceny[1] = new BTextControl(BRect(10,50,190,65), "ttc1", "Netto zakupu (zł)", NULL, new BMessage(DC));
	ceny[2] = new BTextControl(BRect(200,50,340,65), "ttc2", "Marża (%)", NULL, new BMessage(DC));
	ceny[3] = new BTextControl(BRect(350,50,470,65), "ttc3", "Rabat (%)", NULL, new BMessage(DC));
	box2->AddChild(ceny[0]);
	box2->AddChild(ceny[1]);
	box2->AddChild(ceny[2]);
	box2->AddChild(ceny[3]);
	brutto = new BStringView(BRect(330,15,390,35), "ttb0", "Cena brutto:");
	box2->AddChild(brutto);
	brutto = new BStringView(BRect(400,15,475,35), "ttbr", "XXXX,YY zł");
	box2->AddChild(brutto);
	// box2-menu
	BPopUpMenu *menuvat = new BPopUpMenu("");
	int nRows, nCols;
	char **result;
	BString sqlQuery;
	sqlQuery = "SELECT nazwa, stawka FROM stawka_vat ORDER BY id";
	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// XXX Panic! empty vat table
	} else {
		vatItems = new BMenuItem*[nRows];
		for (int i=1;i<=nRows;i++) {
			msg = new BMessage(MENUVAT);
			msg->AddString("_vat", result[i*nCols+1]);
			msg->AddInt32("_item", i-1);
			vatItems[i-1] = new BMenuItem(result[i*nCols+0], msg);
			menuvat->AddItem(vatItems[i-1]);
		}
	}
	BMenuField *menuvatField = new BMenuField(BRect(200,15,330,35), "ttmv", "VAT", menuvat);
	menuvatField->SetDivider(be_plain_font->StringWidth(menuvatField->Label())+15);
	box2->AddChild(menuvatField);
	// box3
	box3 = new BBox(BRect(230,240,710,320), "tt_box3");
	box3->SetLabel("Notatki");
	this->view->AddChild(box3);
	// box3-stuff
	r = box3->Bounds();
	r.InsetBy(10,15);
	BRect s = r; s.OffsetTo(0,0);
	notatki = new BTextView(r, "ttno", s, B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
	box3->AddChild(notatki);
	// fix widths
	int i;
	for (i=0;i<=3;i++) {
		data[i]->SetDivider(be_plain_font->StringWidth(data[i]->Label())+5);
		ceny[i]->SetDivider(be_plain_font->StringWidth(ceny[i]->Label())+5);
	}
	data[0]->SetDivider(50); data[2]->SetDivider(50);
	ceny[0]->SetDivider(90); ceny[1]->SetDivider(90);
	updateTab();
	RefreshIndexList();
}

tabTowar::~tabTowar() {

}

const char *tabTowar::validateDecimal(const char *input) {
	static BString tmp;
	int i, l = strlen(input);
	bool wasDot = false;
//printf("input=[%s]\n",input);
	tmp = "";
	for (i=0;i<l;i++) {
		if ((input[i] >= '0') && (input[i] <= '9')) {
			tmp += input[i];
		} else {
			if (((input[i]=='.')||(input[i]==',')) && (!wasDot)) {
				tmp += '.';
				wasDot = true;
				// truncate to 2 decimal places
				if (i+3<l)
					l = i+3;
			}
		}
//printf("in[%i]=%c, tmp=[%s]\n",i,input[i],tmp.String());
	}
	return tmp.String();
}

void tabTowar::curdataFromTab(void) {
	int i;
	for (i=0;i<=3;i++) {
		curdata->data[i] = data[i]->Text();
	}
	curdata->usluga = (usluga->Value() == B_CONTROL_ON);
	curdata->notatki = notatki->Text();
	curdata->netto = 100*toint(validateDecimal(ceny[0]->Text()));
	curdata->zakupu = 100*toint(validateDecimal(ceny[1]->Text()));
	curdata->marza = 100*toint(validateDecimal(ceny[2]->Text()));
	curdata->rabat = 100*toint(validateDecimal(ceny[3]->Text()));
}

void tabTowar::curdataToTab(void) {
	int i;
	for (i=0;i<=3;i++) {
		data[i]->SetText(curdata->data[i].String());
	}
	usluga->SetValue(curdata->usluga ? B_CONTROL_ON : B_CONTROL_OFF);
	notatki->SetText(curdata->notatki.String());
	BString tmp;
	tmp << (curdata->netto / 100); ceny[0]->SetText(tmp.String()); tmp = "";
	tmp << (curdata->zakupu / 100); ceny[1]->SetText(tmp.String()); tmp = "";
	tmp << (curdata->marza / 100); ceny[2]->SetText(tmp.String()); tmp = "";
	tmp << (curdata->rabat / 100); ceny[3]->SetText(tmp.String());
	dodany->SetText(curdata->dodany.String());
	vatItems[curdata->vatitem]->SetMarked(true);
	updateTab();
}

void tabTowar::updateTab(void) {
	int i;
	for (i=0;i<=3;i++) {
		ceny[i]->SetText(validateDecimal(ceny[i]->Text()));
	}
	// XXX usluga, to cos wylaczyc/wyzerowac???
	int nRows, nCols;
	char **result;
	BString sql;	
// XXX brać tu pod uwagę rabat/marżę???
	sql = "SELECT ROUND(0";
	sql += ceny[0]->Text();
	sql += "*(100+";
	sql << curdata->vat;
	sql += ")/100.0,2)";
//printf("sql:%s\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i\n", nRows, nCols);
	// readout data
	brutto->SetText(result[1]);
}

void tabTowar::MessageReceived(BMessage *Message) {
	int i;
	int32 item;
	const char *tmp;

	switch (Message->what) {
		case DC:
			this->dirty = true;
			updateTab();
			break;
		case BUT_NEW:
			if (CommitCurdata()) {
				// clear curdata
				curdata->clear();
				// refresh tabs
				curdataToTab();
			}
			break;
		case BUT_RESTORE:
			DoFetchCurdata();
			break;
		case BUT_DEL:
			DoDeleteCurdata();
			break;
		case BUT_SAVE:
			curdataFromTab();
			DoCommitCurdata();
			curdataToTab();
			break;
		case LIST_SEL:
		case LIST_INV:
//			printf("list selection/invoc\n");
			i = list->CurrentSelection(0);
//			printf("got:%i\n",i);
			if (i>=0) {
//				printf("sel:%i,id=%i\n",i,idlist[i]);
				ChangedSelection(idlist[i]);
			} else {
				// XXX deselection? what to do???
			}
			break;
		case MENUJM:
			if (Message->FindString("_jm", &tmp) == B_OK) {
				data[3]->SetText(tmp);
				this->dirty = true;
			}
		case MENUVAT:
			if (Message->FindString("_vat", &tmp) == B_OK) {
				curdata->vat = toint(tmp);
				this->dirty = true;
				if (Message->FindInt32("_item", &item) == B_OK) {
					curdata->vatitem = item;
				}
				updateTab();
			}
	}
}

void tabTowar::ChangedSelection(int newid) {
	if (!(CommitCurdata())) {
		// XXX do nothing if cancel, restore old selection?
		return;
	}
	// fetch and store into new data
	curdata->id = newid;
	DoFetchCurdata();
}

void tabTowar::DoCommitCurdata(void) {
	curdata->commit();
	this->dirty = false;
	RefreshIndexList();
}

void tabTowar::DoDeleteCurdata(void) {
// XXX ask for confimation?
	curdata->del();
	curdataToTab();
	RefreshIndexList();
}

void tabTowar::DoFetchCurdata(void) {
	if (curdata->id >=0) {
		curdata->fetch();
		this->dirty = false;
		curdataToTab();
	}
}

void tabTowar::RefreshIndexList(void) {
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
	char *dbErrMsg;
	BString sqlQuery;
	sqlQuery = "SELECT id, symbol, nazwa FROM towar ORDER BY id";
	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// XXX database is empty, do sth about it?
		printf("database is empty\n");
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
