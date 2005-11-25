//
// dopisać:
// domyślnie podana nazwa: '##/miesiac/rok'
//
// wypełnić drugą kartę - lista, towary, podsumowanie? (label z numerem!)
//	lista: tablica klas towardat
// dodać trzecią kartę na notatki?
// opcja faktury korygującej (jak? trzeba pamiętać co się zmieniło)
//

#include "globals.h"
#include "tabfaktura.h"

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ListView.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <TabView.h>
#include <TextControl.h>

#include <stdio.h>

const uint32 LIST_INV	= 'TFLI';
const uint32 LIST_SEL	= 'TFLS';
const uint32 BUT_NEW	= 'TFBN';
const uint32 BUT_DEL	= 'TFBD';
const uint32 BUT_RESTORE= 'TFBR';
const uint32 BUT_SAVE	= 'TFBS';
const uint32 DC			= 'TFDC';

const uint32 CBUT		= 'TFCB';
const uint32 MENUST		= 'TFMS';
const uint32 MENUFP		= 'TFMF';
const uint32 MENUFSYM	= 'TFMY';
const uint32 TERMCHANGE	= 'TFCT';

const char *stransportu[] = { "własny sprzedawcy", "własny odbiorcy", NULL };
const char *fplatnosci[] = { "gotówką", "przelewem", "czekiem", "kartą płatniczą", "kartą kredytową", NULL };

tabFaktura::tabFaktura(BTabView *tv, sqlite *db) : beFakTab(tv, db) {

	idlist = NULL;
	curdata = new fakturadat(db);
	odbiorca = new firmadat(db);
	this->dirty = false;

	this->tab->SetLabel("Faktury");
	BRect r;
	r = this->view->Bounds();

	// listview in scrollview
	r.left = 5; r.right = 160; r.top = 30; r.bottom = 500;
	list = new BListView(r, "fTListView");
	this->view->AddChild(new BScrollView("fTScrollView", list, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));
	// buttons
	but_new = new BButton(BRect(40,0,130,24), "tf_but_new", "Nowa faktura", new BMessage(BUT_NEW), B_FOLLOW_LEFT|B_FOLLOW_TOP);
	but_del = new BButton(BRect(40,510,130,534), "tf_but_del", "Usuń zaznaczone", new BMessage(BUT_DEL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_restore = new BButton(BRect(235,510,325,534), "tf_but_restore", "Przywróć", new BMessage(BUT_RESTORE), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_save = new BButton(BRect(580,510,670,534), "tf_but_save", "Zapisz", new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
	this->view->AddChild(but_new);
	this->view->AddChild(but_del);
	this->view->AddChild(but_restore);
	this->view->AddChild(but_save);

	BTabView *tbv2 = new BTabView(BRect(180,0,790,500), "tftbv2");
	this->view->AddChild(tbv2);
	r = tbv2->Bounds();
	r.InsetBy(5,5);
	viewogol = new BView(r, "tfviewogol", B_FOLLOW_ALL_SIDES, 0);
	viewogol->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	tabogol = new BTab(viewogol);
	tbv2->AddTab(viewogol, tabogol);
	tabogol->SetLabel("Dane ogólne");
	viewpozy = new BView(r, "tfviewpozy", B_FOLLOW_ALL_SIDES, 0);
	viewpozy->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	tabpozy = new BTab(viewpozy);
	tbv2->AddTab(viewpozy, tabpozy);
	tabpozy->SetLabel("Pozycje");
//	views: 0,0,490,600
	nazwa = new BTextControl(BRect(10,10,300,30), "tfna", "Nr faktury", NULL, new BMessage(DC));
	viewogol->AddChild(nazwa);
	// box1
	box1 = new BBox(BRect(10,40,300,210),"tf1box1");
	box1->SetLabel("");
	viewogol->AddChild(box1);
	// box1-stuff
	ogol[0] = new BTextControl(BRect(10,10,280,30), "tfd0", "Miejsce wyst.", NULL, new BMessage(DC));
	ogol[1] = new BTextControl(BRect(10,40,280,60), "tfd1", "Wystawił", NULL, new BMessage(DC));
	ogol[2] = new BTextControl(BRect(10,70,250,90), "tfd2", "Data wyst.", NULL, new BMessage(DC));
	ogol[3] = new BTextControl(BRect(10,100,250,120), "tfd3", "Data sprzedaży", NULL, new BMessage(DC));
	ogol[4] = new BTextControl(BRect(10,130,250,150), "tfd4", "Środek transp.", NULL, new BMessage(DC));
	box1->AddChild(ogol[0]);
	box1->AddChild(ogol[1]);
	box1->AddChild(ogol[2]);
	box1->AddChild(ogol[3]);
	box1->AddChild(ogol[4]);
	BMessage *msg;
	msg = new BMessage(CBUT);
	msg->AddInt32("_butnum", 0);
	cbut[0] = new BButton(BRect(260,70,280,90), "tfcbut0", "+", msg);
	msg = new BMessage(CBUT);
	msg->AddInt32("_butnum", 1);
	cbut[1] = new BButton(BRect(260,100,280,120), "tfcbut1", "+", msg);
	box1->AddChild(cbut[0]);
	box1->AddChild(cbut[1]);
	int i;
	for (i=0;i<=4;i++)
		ogol[i]->SetDivider(70);
	// box1-menu
	BMenu *menust = new BMenu("");
	int j=0;
	while (stransportu[j] != NULL) {
		msg = new BMessage(MENUST); msg->AddString("_st",stransportu[j]);
		menust->AddItem(new BMenuItem(stransportu[j], msg));
		j++;
	}
	BMenuField *menustField = new BMenuField(BRect(260,130,280,150), "tfmfst", NULL, menust);
	box1->AddChild(menustField);
	// box2
	box2 = new BBox(BRect(310,40,590,120),"tf1box2");
	box2->SetLabel("Płatność");
	viewogol->AddChild(box2);
	// box2-stuff
	ogol[5] = new BTextControl(BRect(10,10,150,30), "tfd5", "Forma", NULL, new BMessage(DC));
	ogol[6] = new BTextControl(BRect(10,40,150,60), "tfd6", "Termin", NULL, new BMessage(DC));
	ogol[7] = new BTextControl(BRect(200,40,270,60), "tfd7", "Dni", "30", new BMessage(TERMCHANGE));
	box2->AddChild(ogol[5]);
	box2->AddChild(ogol[6]);
	box2->AddChild(ogol[7]);
	ogol[5]->SetDivider(50); ogol[6]->SetDivider(50); ogol[7]->SetDivider(20);
	// box2-menu
	BMenu *menufp = new BMenu("");
	j=0;
	while (fplatnosci[j] != NULL) {
		msg = new BMessage(MENUFP); msg->AddString("_fp",fplatnosci[j]);
		menufp->AddItem(new BMenuItem(fplatnosci[j], msg));
		j++;
	}
	BMenuField *menufpField = new BMenuField(BRect(160,10,180,30), "tfmffp", NULL, menufp);
	box2->AddChild(menufpField);
	msg = new BMessage(CBUT);
	msg->AddInt32("_butnum", 2);
	cbut[2] = new BButton(BRect(160,40,180,60), "tfcbut2", "+", msg);
	box2->AddChild(cbut[2]);
	// box3
	box3 = new BBox(BRect(310,130,590,210),"tf1box3");
	box3->SetLabel("Zaliczka");
	viewogol->AddChild(box3);
	// box3-stuff
	cbzaplacono = new BCheckBox(BRect(10,25,80,40), "tfzap", "Zapłacono", new BMessage(DC));
	box3->AddChild(cbzaplacono);
	ogol[8] = new BTextControl(BRect(90,20,230,40), "tfd8", "Kwota (zł)", NULL, new BMessage(DC));
	ogol[9] = new BTextControl(BRect(10,50,170,70), "tfd9", "Data", NULL, new BMessage(DC));
	box3->AddChild(ogol[8]);
	box3->AddChild(ogol[9]);
	ogol[8]->SetDivider(50); ogol[9]->SetDivider(50);
	msg = new BMessage(CBUT);
	msg->AddInt32("_butnum", 3);
	cbut[3] = new BButton(BRect(180,50,200,70), "tfcbut3", "+", msg);
	box3->AddChild(cbut[3]);
	// box4
	box4 = new BBox(BRect(10,220,590,460),"tfbox4");
	box4->SetLabel("Odbiorca");
	viewogol->AddChild(box4);
	// box1-stuff
	data[0] = new BTextControl(BRect(10,15,270,35), "tfd0", "Nazwa", NULL, new BMessage(DC));
	data[2] = new BTextControl(BRect(10,50,420,65), "tfd2", "Adres", NULL, new BMessage(DC));
	data[3] = new BTextControl(BRect(10,80,150,95), "tfd3", "Kod", NULL, new BMessage(DC));
	data[4] = new BTextControl(BRect(160,80,420,95), "tfd4", "Miejscowość", NULL, new BMessage(DC));
	data[5] = new BTextControl(BRect(10,110,200,125), "tfd5", "Tel.", NULL, new BMessage(DC));
	data[6] = new BTextControl(BRect(210,110,420,125), "tfd6", "Email", NULL, new BMessage(DC));
	box4->AddChild(data[0]);
	box4->AddChild(data[2]);
	box4->AddChild(data[3]); box4->AddChild(data[4]);
	box4->AddChild(data[5]); box4->AddChild(data[6]);
	r.top = 140; r.bottom = 155; r.left = 10, r.right = 420;
	data[7] = new BTextControl(r, "tfd7", "NIP", NULL, new BMessage(DC)); r.OffsetBy(0, 25);
	data[8] = new BTextControl(r, "tfd8", "REGON", NULL, new BMessage(DC)); r.OffsetBy(0, 25);
	data[9] = new BTextControl(r, "tfd9", "Bank", NULL, new BMessage(DC)); r.OffsetBy(0, 25);
	data[10] = new BTextControl(r, "tfd10", "Nr konta", NULL, new BMessage(DC));
	box4->AddChild(data[7]); box4->AddChild(data[8]);
	box4->AddChild(data[9]); box4->AddChild(data[10]);
	// fix widths
	for (i=0;i<=6;i++) {
		if (i!=1)
		data[i]->SetDivider(be_plain_font->StringWidth(data[i]->Label())+5);
	}
	data[0]->SetDivider(50); data[2]->SetDivider(50);
	data[3]->SetDivider(50); data[5]->SetDivider(50);
	data[7]->SetDivider(50); data[8]->SetDivider(50);
	data[9]->SetDivider(50); data[10]->SetDivider(50);
	// firma-symbole
	BPopUpMenu *menusymbol = new BPopUpMenu("[wybierz]");
	int nRows, nCols;
	char **result;
	BString sqlQuery;
	sqlQuery = "SELECT id, symbol FROM firma WHERE aktywny = 1 ORDER BY id";
	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// XXX Panic! empty table
	} else {
		symbolMenuItems = new BMenuItem*[nRows];
		symbolIds = new int[nRows];
		symbolRows = nRows;
		for (int i=1;i<=nRows;i++) {
			msg = new BMessage(MENUFSYM);
			msg->AddInt32("_firmaid", toint(result[i*nCols+0]));
			symbolIds[i-1] = toint(result[i*nCols+0]);
			symbolMenuItems[i-1] = new BMenuItem(result[i*nCols+1], msg);
			menusymbol->AddItem(symbolMenuItems[i-1]);
		}
	}
	BMenuField *menusymbolField = new BMenuField(BRect(280,15,420,35), "tfmsymbol", "Symbol", menusymbol);
	menusymbolField->SetDivider(be_plain_font->StringWidth(menusymbolField->Label())+15);
	box4->AddChild(menusymbolField);
	makeNewForm();
	updateTab();
	RefreshIndexList();
}

tabFaktura::~tabFaktura() {

}

void tabFaktura::curdataFromTab(void) {
	int i;
	curdata->nazwa = nazwa->Text();
	for (i=0;i<=9;i++)
		curdata->ogol[i]=ogol[i]->Text();
	for (i=0;i<=10;i++)
		if (i!=1)
			curdata->odata[i]=data[i]->Text();
	curdata->zaplacono = (cbzaplacono->Value() == B_CONTROL_ON);
}

void tabFaktura::curdataToTab(void) {
	int i;
	nazwa->SetText(curdata->nazwa.String());
	for (i=0;i<=9;i++)
		ogol[i]->SetText(curdata->ogol[i].String());
	for (i=0;i<=10;i++)
		if (i!=1)
			data[i]->SetText(curdata->odata[i].String());
	cbzaplacono->SetValue(curdata->zaplacono ? B_CONTROL_ON : B_CONTROL_OFF);
	updateTab();
}

void tabFaktura::updateTab(void) {
	bool state = (cbzaplacono->Value() == B_CONTROL_ON);
	ogol[8]->SetEnabled(state);
	ogol[9]->SetEnabled(state);
	cbut[3]->SetEnabled(state);
	ogol[2]->SetText(validateDate(ogol[2]->Text()));
	ogol[3]->SetText(validateDate(ogol[3]->Text()));
	ogol[6]->SetText(validateDate(ogol[6]->Text()));
	ogol[9]->SetText(validateDate(ogol[9]->Text()));
}

void tabFaktura::makeNewForm(void) {
	curdata->clear();
	// XXX refresh symbolmenu
	// XXX prepare new 'nazwa' for faktura
	curdata->ogol[2] = execSQL("SELECT DATE('now')");
	curdata->ogol[3] = execSQL("SELECT DATE('now')");
	// XXX this is in TERMCHANGE handler
	curdata->ogol[7] = "30";
	BString tmp;
	tmp = "SELECT DATE('now', '0";
	tmp << curdata->ogol[7].String();
	tmp += " days')";
	curdata->ogol[6] = execSQL(tmp.String());
	curdataToTab();
}

void tabFaktura::MessageReceived(BMessage *Message) {
	int i;
	int32 item;
	const char *tmp;
	BString result,sql;

	switch (Message->what) {
		case DC:
			this->dirty = true;
			updateTab();
			break;
		case BUT_NEW:
			if (CommitCurdata()) {
				makeNewForm();
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
		case MENUST:
			if (Message->FindString("_st", &tmp) == B_OK) {
				ogol[4]->SetText(tmp);
				this->dirty = true;
			}
			break;
		case MENUFP:
			if (Message->FindString("_fp", &tmp) == B_OK) {
				ogol[5]->SetText(tmp);
				this->dirty = true;
			}
			break;
		case MENUFSYM:
			if (Message->FindInt32("_firmaid", &item) == B_OK) {
				odbiorca->id = item;
				odbiorca->fetch();
				for (i=0;i<=10;i++) {
					if (i!=1)
						data[i]->SetText(odbiorca->data[i].String());
				}
				this->dirty = true;
			}
			break;
		case TERMCHANGE:
			sql = "SELECT DATE('now', '0";
			sql << ogol[7]->Text();
			sql += " days')";
			ogol[6]->SetText(execSQL(sql.String()));
			break;
	}
}

void tabFaktura::ChangedSelection(int newid) {
	if (!(CommitCurdata())) {
		// XXX do nothing if cancel, restore old selection?
		return;
	}
	// fetch and store into new data
	curdata->id = newid;
	DoFetchCurdata();
}

void tabFaktura::DoCommitCurdata(void) {
	// XXX perform all checks against supplied data
	curdata->commit();
	this->dirty = false;
	RefreshIndexList();
}

void tabFaktura::DoDeleteCurdata(void) {
// XXX ask for confimation?
	curdata->del();
	curdataToTab();
	RefreshIndexList();
}

void tabFaktura::DoFetchCurdata(void) {
	if (curdata->id >=0) {
		curdata->fetch();
		this->dirty = false;
		curdataToTab();
	}
}

void tabFaktura::RefreshIndexList(void) {
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
	sqlQuery = "SELECT id, nazwa FROM faktura ORDER BY id";
	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// XXX database is empty, do sth about it?
		printf("database is empty\n");
	} else {
		idlist = new int[nRows];
		for (int i=1;i<=nRows;i++) {
			idlist[i-1] = toint(result[i*nCols+0]);
			list->AddItem(new BStringItem(result[i*nCols+1]));
		}
	}
	sqlite_free_table(result);
}
