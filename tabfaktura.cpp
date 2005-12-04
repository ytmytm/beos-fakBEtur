//
// TODO:
// - nazwa nowej faktury: '##/miesiac/rok', ## brac z konf, ale kiedy uaktualniac?
// IDEAS:
// - pewnego słonecznego dnia wrzucić do destruktora listę z delete...
// - pole uwagi nie reaguje na zmiany! (sprawdzac UndoState?)
// - obliczac wartosci i ceny w jednym miejscu
//	 (np. suma[] i data->data w pozfakdata oraz (fut!) wydruk)
// - usunac kopiowanie kodu (execsql, inne kawalki)
// - zamiast usuwać/dodawać wszystkie pozitems - może pamiętać ich id?
//	 generować, uaktualniać, czyścić
//	 (problem: które UPDATE, które INSERT, a które DELETE)
//
// wyrzucić uwagi i zastąpić całym podsumowaniem? podsumowanie na 3 karcie?
// opcja faktury korygującej (jak? trzeba pamiętać co się zmieniło)
//
// obliczanie:
// 1. w.brutto = round(ilość*c.brutto) = round(ilość*round(c.netto*stawka)) = 3.23
// 2. w.brutto = round(stawka*ilość*c.netto) = 3.24 (3.235)
//
// 2. jest poprawny (wtedy w.netto*stawka = w.brutto!)
// kwota vat = wbrutto-wnetto
//
// testy w commitfaktura
// - data/sposób płatności
// - brak adresu odbiorcy
// - kto wystawił?
// - lista towarów pusta
// testy w docommitcurtowardata
// - towar o tej nazwie już jest
// - czy dodać to do bazy towarów?
// - ilość/cena wynosi 0
// - nazwa, pkwiu, jm, stawka - nie wpisane
//

#include "globals.h"
#include "tabfaktura.h"
#include "befakprint.h"

#include <Alert.h>
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
#include <TextView.h>

#include <stdio.h>

const uint32 LIST_INV	= 'TFLI';
const uint32 LIST_SEL	= 'TFLS';
const uint32 BUT_NEW	= 'TFBN';
const uint32 BUT_DEL	= 'TFBD';
const uint32 BUT_RESTORE= 'TFBR';
const uint32 BUT_SAVE	= 'TFBS';
const uint32 BUT_PRINT	= 'TFBP';
const uint32 DC			= 'TFDC';

const uint32 CBUT		= 'TFCB';
const uint32 MENUST		= 'TFMS';
const uint32 MENUFP		= 'TFMF';
const uint32 MENUFSYM	= 'TFMY';
const uint32 TERMCHANGE	= 'TFCT';

const uint32 DCT		= 'TFDD';
const uint32 BUT_PSAVE	= 'TFB0';
const uint32 BUT_PNEW	= 'TFB1';
const uint32 BUT_PIMPORT= 'TFB2';
const uint32 BUT_PDEL	= 'TFB3';
const uint32 MENUTSYM	= 'TFMT';
const uint32 MENUVAT	= 'TFMV';
const uint32 MENUJM		= 'TFMJ';
const uint32 PLIST_INV	= 'TFPI';
const uint32 PLIST_SEL	= 'TFPS';

const char *stransportu[] = { "własny sprzedawcy", "własny odbiorcy", NULL };
const char *fplatnosci[] = { "gotówką", "przelewem", "czekiem", "kartą płatniczą", "kartą kredytową", NULL };
extern const char *jmiary[];
const char *plisthead[] = { "Lp", "Nazwa", "PKWiU", "Ilość", "Jm", "Rabat (%)", "Cena jedn.", "W. netto", "VAT", "W. VAT", "W. brutto", NULL };
const int plistw[] = { 20, 90, 60, 40, 20, 50, 70, 70, 30, 60, 100, -1 };

tabFaktura::tabFaktura(BTabView *tv, sqlite *db, BHandler *hr) : beFakTab(tv, db, hr) {

	idlist = NULL;
	curdata = new fakturadat(db);
	odbiorca = new firmadat(db);
	curtowar = new towardat(db);
	faklista = new pozfaklist(db);
	this->dirty = false;

	curtowarvatid = -1;
	towarmark = -1;
	lasttowarsel = -1;
	towardirty = false;

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
	but_print = new BButton(BRect(405,510,485,534), "tf_but_print", "Drukuj", new BMessage(BUT_PRINT), B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	this->view->AddChild(but_new);
	this->view->AddChild(but_del);
	this->view->AddChild(but_restore);
	this->view->AddChild(but_save);
	this->view->AddChild(but_print);

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

	initTab1();
	initTab2();
	makeNewForm();
	updateTab();
	RefreshIndexList();
}

tabFaktura::~tabFaktura() {

}

void tabFaktura::initTab1(void) {
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
	BRect r;
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
	menusymbol = new BPopUpMenu("[wybierz]");
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
}

void tabFaktura::initTab2(void) {
	BRect r, s;
	BMessage *msg;
	int i;
	// box5
	box5 = new BBox(BRect(10,10,590,350),"tfbox5");
	box5->SetLabel("Pozycje");
	viewpozy->AddChild(box5);
	// box5-stuff
	viewtable = new BView(BRect(10,20,560,160), "tableview", B_FOLLOW_ALL_SIDES, 0);
	r = viewtable->Bounds();
	r.right = plistw[0];
	for (i=0;i<=10;i++) {
		pozcolumn[i] = new BListView(r, NULL);
		pozcolumn[i]->SetInvocationMessage(new BMessage(PLIST_INV));
		pozcolumn[i]->SetSelectionMessage(new BMessage(PLIST_SEL));
		viewtable->AddChild(pozcolumn[i]);
		r.OffsetBy(plistw[i],0);
		r.right = r.left + plistw[i+1];
	}
	box5->AddChild(new BScrollView("ftablescroll", viewtable, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	// box6
	box6 = new BBox(BRect(10,170,570,250), "tfbox6");
	box6->SetLabel("Nowa pozycja");
	box5->AddChild(box6);
	// box6-stuff
	towar[0] = new BTextControl(BRect(10,15,190,35), "tftowar0", "Nazwa", NULL, new BMessage(DCT));
	towar[1] = new BTextControl(BRect(310,15,460,35), "tftowar1", "PKWiU", NULL, new BMessage(DCT));
	towar[2] = new BTextControl(BRect(10,45,190,65), "tftowar2", "Cena netto (zł)", NULL, new BMessage(DCT));
	towar[3] = new BTextControl(BRect(200,45,290,65), "tftowar3", "Rabat (%)", NULL, new BMessage(DCT));
	towar[4] = new BTextControl(BRect(300,45,430,65), "tftowar4", "Ilość", NULL, new BMessage(DCT));
	towar[5] = new BTextControl(BRect(440,45,500,65), "tftowar5", "jm", NULL, new BMessage(DCT));
	box6->AddChild(towar[0]); box6->AddChild(towar[1]);
	box6->AddChild(towar[2]); box6->AddChild(towar[3]);
	box6->AddChild(towar[4]); box6->AddChild(towar[5]);
	// towar-symbole
	tmenusymbol = new BPopUpMenu("[wybierz]");
	int nRows, nCols;
	char **result;
	BString sqlQuery;
	sqlQuery = "SELECT id, symbol FROM towar ORDER BY id";
	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// XXX Panic! empty table
	} else {
		tsymbolMenuItems = new BMenuItem*[nRows];
		tsymbolIds = new int[nRows];
		tsymbolRows = nRows;
		for (int i=1;i<=nRows;i++) {
			msg = new BMessage(MENUTSYM);
			msg->AddInt32("_towarid", toint(result[i*nCols+0]));
			tsymbolIds[i-1] = toint(result[i*nCols+0]);
			tsymbolMenuItems[i-1] = new BMenuItem(result[i*nCols+1], msg);
			tmenusymbol->AddItem(tsymbolMenuItems[i-1]);
		}
	}
	BMenuField *tmenusymbolField = new BMenuField(BRect(200,15,300,35), "tfmtsymbol", "Symb.", tmenusymbol);
	tmenusymbolField->SetDivider(be_plain_font->StringWidth(tmenusymbolField->Label())+15);
	box6->AddChild(tmenusymbolField);
	// vat-menu
	menuvat = new BPopUpMenu("[wybierz]");
	sqlQuery = "SELECT id, nazwa FROM stawka_vat WHERE aktywne = 1 ORDER BY id";
	sqlite_get_table(dbData, sqlQuery.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// XXX Panic! empty vat table
	} else {
		vatMenuItems = new BMenuItem*[nRows];
		vatIds = new int[nRows];
		vatRows = nRows;
		for (int i=1;i<=nRows;i++) {
			msg = new BMessage(MENUVAT);
			msg->AddInt32("_vatid", toint(result[i*nCols+0]));
			vatIds[i-1] = toint(result[i*nCols+0]);
			vatMenuItems[i-1] = new BMenuItem(result[i*nCols+1], msg);
			menuvat->AddItem(vatMenuItems[i-1]);
		}
	}
	BMenuField *menuvatField = new BMenuField(BRect(470,15,555,35), "tfmv", "VAT", menuvat);
	menuvatField->SetDivider(be_plain_font->StringWidth(menuvatField->Label())+15);
	box6->AddChild(menuvatField);
	// jm-menu
	BMenu *menujm = new BMenu("");
	int j=0;
	while (jmiary[j] != NULL) {
		msg = new BMessage(MENUJM); msg->AddString("_jm",jmiary[j]);
		menujm->AddItem(new BMenuItem(jmiary[j], msg));
		j++;
	}
	BMenuField *menujmField = new BMenuField(BRect(505,45,555,65), "tfmf", NULL, menujm);
	box6->AddChild(menujmField);
	// box5-stuff-cont
	but_psave = new BButton(BRect(10,260,70,290), "tf_but_psave", "Zapisz", new BMessage(BUT_PSAVE));
	but_pnew = new BButton(BRect(330,260,390,290), "tf_but_pnew", "Nowy", new BMessage(BUT_PNEW));
	but_pimport = new BButton(BRect(400,260,480,290), "tf_but_pimport", "Import z innej", new BMessage(BUT_PIMPORT));
	but_pdel = new BButton(BRect(490,260,560,290), "tf_but_pdel", "Usuń", new BMessage(BUT_PDEL));
	box5->AddChild(but_psave);
	box5->AddChild(but_pnew);
	box5->AddChild(but_pimport);
	box5->AddChild(but_pdel);

	r.left = 10; r.right = 100; r.top = 290; r.bottom = 310;
	s = r; s.OffsetBy(0,20);
	box5->AddChild(new BStringView(r, "tf_ss0", "Cena jednostkowa")); r.OffsetBy(100,0);
	box5->AddChild(new BStringView(r, "tf_ss1", "Cena brutto")); r.OffsetBy(80,0);
	box5->AddChild(new BStringView(r, "tf_ss2", "Ilość")); r.OffsetBy(80,0);
	box5->AddChild(new BStringView(r, "tf_ss3", "Wartość netto")); r.OffsetBy(80,0);
	box5->AddChild(new BStringView(r, "tf_ss4", "Kwota VAT")); r.OffsetBy(80,0);
	box5->AddChild(new BStringView(r, "tf_ss5", "Wartość brutto"));
	for (j=0;j<=5;j++) {
		suma[j] = new BStringView(s, NULL, "???");
		box5->AddChild(suma[j]);
		if (j==0)
			s.OffsetBy(100,0);
		else
			s.OffsetBy(80,0);
	}
	// box7
	box7 = new BBox(BRect(10,360,590,450), "tfbox7");
	box7->SetLabel("Uwagi");
	viewpozy->AddChild(box7);
	// box7-stuff
	r = box7->Bounds();
	r.InsetBy(10,15);
	s = r; s.OffsetTo(0,0);
	uwagi = new BTextView(r, "tfuwagi", s, B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
	box7->AddChild(uwagi);
	// fix widths
	towar[0]->SetDivider(50);
	towar[1]->SetDivider(50);
	towar[2]->SetDivider(90);
	towar[4]->SetDivider(30);
	RefreshTowarList();
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
	curdata->uwagi = uwagi->Text();
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
	uwagi->SetText(curdata->uwagi.String());
	updateTab();
}

void tabFaktura::updateTab(void) {
	BMessage *msg = new BMessage(MSG_NAMECHANGE);
	msg->AddString("_newtitle", nazwa->Text());
	handler->Looper()->PostMessage(msg);

	bool state = (cbzaplacono->Value() == B_CONTROL_ON);
	ogol[8]->SetEnabled(state);
	ogol[9]->SetEnabled(state);
	cbut[3]->SetEnabled(state);
	ogol[2]->SetText(validateDate(ogol[2]->Text()));
	ogol[3]->SetText(validateDate(ogol[3]->Text()));
	ogol[6]->SetText(validateDate(ogol[6]->Text()));
	ogol[9]->SetText(validateDate(ogol[9]->Text()));
}

void tabFaktura::updateTab2(void) {
	int i;
	for (i=0;i<vatRows;i++) {
		vatMenuItems[i]->SetMarked((vatIds[i] == curtowarvatid));
	}
	if (curtowarvatid < 0) {
		menuvat->Superitem()->SetLabel("[wybierz]");
		return;
	}
	// validate numeric fields
	towar[2]->SetText(validateDecimal(towar[2]->Text()));
	towar[3]->SetText(validateDecimal(towar[3]->Text()));
	towar[4]->SetText(validateDecimal(towar[4]->Text()));
	// calculate data for summary suma[]
	BString sql, cnetto, cbrutto;
	sql = "SELECT 0"; sql += towar[2]->Text();
	sql += "*(100-0"; sql += towar[3]->Text(); sql += ")/100.0";
	cnetto = decround(execSQL(sql.String()));	// cnettojednostkowa
	suma[0]->SetText(cnetto.String());
	// cbrutto = cnettojedn*stawka
	sql = "SELECT 0"; sql += cnetto; sql +="*(100+stawka)/100.0 FROM stawka_vat WHERE id = ";
	sql << curtowarvatid;
	suma[1]->SetText(decround(execSQL(sql.String())));	// cbrutto
	suma[2]->SetText(towar[4]->Text());					// ilosc
	// wnetto = cnettojedn*ilosc
	sql = "SELECT 0"; sql += cnetto; sql += "*0"; sql += towar[4]->Text();
	suma[3]->SetText(decround(execSQL(sql.String())));	// wnetto
	// wbrutto = cnettojedn*ilosc*stawka = wnetto*stawka
	sql = "SELECT 0"; sql += suma[3]->Text();
	sql += "*(100+stawka)/100.0 FROM stawka_vat WHERE id = "; sql << curtowarvatid;
	suma[5]->SetText(decround(execSQL(sql.String())));	// wbrutto
	// wvat = wbrutto - wnetto
	sql = "SELECT 0"; sql += suma[5]->Text(); sql += "-0"; sql += suma[3]->Text();
	suma[4]->SetText(decround(execSQL(sql.String())));	// wvat
}

void tabFaktura::makeNewForm(void) {
	curdata->clear();
	// unselect symbolmenu
	for (int i=0;i<symbolRows;i++) {
		symbolMenuItems[i]->SetMarked(false);
	}
	menusymbol->Superitem()->SetLabel("[wybierz]");
	// XXX refresh symbolmenu
	// XXX prepare new 'nazwa' for faktura
	// XXX notify mainwindow about change
	curdata->ogol[2] = execSQL("SELECT DATE('now')");
	curdata->ogol[3] = execSQL("SELECT DATE('now')");
	// XXX this is already in TERMCHANGE handler
	curdata->ogol[7] = "30";
	BString tmp;
	tmp = "SELECT DATE('now', '0";
	tmp << curdata->ogol[7].String();
	tmp += " days')";
	curdata->ogol[6] = execSQL(tmp.String());
	// 
	uwagi->SetText("");
	cbzaplacono->SetValue(B_CONTROL_OFF);
	curdataToTab();
}

void tabFaktura::makeNewTowar(void) {
	int i;
	// clear symbolmenu
	for (i=0;i<tsymbolRows;i++) {
		tsymbolMenuItems[i]->SetMarked(false);
	}
	tmenusymbol->Superitem()->SetLabel("[wybierz]");
	// clear widgets
	for (i=0;i<=5;i++)
		towar[i]->SetText("");
	for (i=0;i<=5;i++)
		suma[i]->SetText("");
	curtowarvatid = -1;
	towardirty = false;
	// un-mark list
	towarmark = -1;
	for (i=0;i<=10;i++)
		pozcolumn[i]->DeselectAll();
	updateTab2();
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
		case BUT_PRINT:
			curdataFromTab();
			DoCommitCurdata();
			curdataToTab();
			// XXX prepare default params
			printCurrent();
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
		case CBUT:
			{	// XXX find source, get date from dialog, put into src
				BAlert *calendar = new BAlert(APP_NAME, "Tutaj dialog z kalendarzem do wyboru daty", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
				calendar->Go();
				break;
				// XXX if not canceled:
				// this->dirty = true;
			}
// from tab2
		case DCT:
			this->dirty = true;
			towardirty = true;
			updateTab2();
			break;
		case MENUTSYM:
			if (Message->FindInt32("_towarid", &item) == B_OK) {
				curtowar->id = item;
				curtowar->fetch();
				towar[0]->SetText(curtowar->data[0].String());
				towar[1]->SetText(curtowar->data[2].String());
				towar[2]->SetText(curtowar->ceny[0].String());
				towar[3]->SetText(curtowar->ceny[3].String());
				towar[4]->SetText("0");
				towar[5]->SetText(curtowar->data[3].String());
				curtowarvatid = curtowar->vatid;
				updateTab2();
				this->dirty = true;
				towardirty = true;
			}
		case MENUJM:
			if (Message->FindString("_jm", &tmp) == B_OK) {
				towar[5]->SetText(tmp);
				this->dirty = true;
				towardirty = true;
			}
			break;
		case MENUVAT:
			this->dirty = true;
			towardirty = true;
			if (Message->FindInt32("_vatid", &item) == B_OK) {
				curtowarvatid = item;
			}
			updateTab2();
			break;
		case BUT_PSAVE:
			this->dirty = true;
			towardirty = true;
//			printf("saving!\n");
			DoCommitTowardata();
			break;
		case BUT_PNEW:
			if (CommitCurtowar()) {
				makeNewTowar();
			}
			break;
		case BUT_PDEL:
			i = pozcolumn[0]->CurrentSelection(0);
			if (i>0) {
//				printf("delete %i-th\n",i);
				faklista->remove(i);
				makeNewTowar();
				faklista->setlp();
				RefreshTowarList();
			}
			towardirty = true;
			break;
		case PLIST_SEL:
		case PLIST_INV:
			{	int j;
				void *ptr;
				BListView *plist;

				i = -1;
				if (Message->FindPointer("source", &ptr) == B_OK) {
					plist = static_cast<BListView*>(ptr);
					i = plist->CurrentSelection(0);
				}
				if (i != lasttowarsel) {
					lasttowarsel = i;
//					printf("newsel:%i\n",i);
					// zmiana wybranego towaru, uaktualnienie towarmark
					ChangedTowarSelection(lasttowarsel);
					if (towarmark != 0) {
						for (j=0; j<=10; j++) {
							pozcolumn[j]->Select(towarmark);
							pozcolumn[j]->ScrollToSelection();
						}
					} else {
						for (j=0; j<=10; j++) {
							pozcolumn[j]->Select(0);
							pozcolumn[j]->ScrollToSelection();
							pozcolumn[j]->DeselectAll();
						}
					}
				}
				break;
			}
		case MSG_ORDERPRINT:
			// XXX read params from msg
			printCurrent();
			break;
	}
}

void tabFaktura::ChangedSelection(int newid) {
// XXX make return bool so can ignore changesel and do nothing?

	if (!(CommitCurtowar())) {
		return;
	}
	if (!(CommitCurdata())) {
		// XXX do nothing if cancel, restore old selection?
		return;
	}
	// initialize
	makeNewForm();
	makeNewTowar();
	// fetch and store into new data
	curdata->id = newid;
	DoFetchCurdata();
}

void tabFaktura::ChangedTowarSelection(int newid) {

	if (!(CommitCurtowar())) {
		return;
	}
	towarmark = newid;
	pozfakdata *item = faklista->itemat(newid);
	if (item != NULL) {
		// fetch data into widgets
		towar[0]->SetText(item->data[1].String());	// nazwa
		towar[1]->SetText(item->data[2].String());	// pkwiu
		towar[4]->SetText(item->data[3].String());	// ilosc
		towar[5]->SetText(item->data[4].String());	// jm
		towar[3]->SetText(item->data[5].String());	// rabat
		curtowarvatid = item->vatid;				// vatid
		towar[2]->SetText(item->data[11].String());	// cnetto
//		// cena netto z jednostkowej przed rabatem
//		BString sql;
//		sql = "SELECT 0"; sql += item->data[6];
//		sql += "/(1-0"; sql += item->data[5]; sql += "/100.0)";
//		towar[2]->SetText(execSQL(sql.String()));
		updateTab2();
	} else {
//		printf("null item after selchg, happens after all-delete\n");
	}
	towardirty = false;
}

void tabFaktura::DoCommitCurdata(void) {
	// XXX perform all checks against supplied data
	curdata->commit();
	faklista->commit(curdata->id);
	this->dirty = false;
	towardirty = false;
	RefreshIndexList();
}

void tabFaktura::DoDeleteCurdata(void) {
// XXX ask for confimation?
	curdata->del();
	curdataToTab();
	faklista->clear();
	towardirty = false;
	RefreshTowarList();
	RefreshIndexList();
}

void tabFaktura::DoFetchCurdata(void) {
	if (curdata->id >=0) {
		curdata->fetch();
		faklista->fetch(curdata->id);
		this->dirty = false;
		towardirty = false;
		RefreshTowarList();
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
// ret. false -> cancel action and resume editing
bool tabFaktura::CommitCurtowar(void) {
	if (!towardirty)
		return true;
	BAlert *ask = new BAlert(APP_NAME, "Zapisać zmiany w aktualnej pozycji?", "Tak", "Nie", "Anuluj", B_WIDTH_AS_USUAL, B_IDEA_ALERT);
	int ret = ask->Go();
	switch (ret) {
		case 2:
			// XXX po wybraniu tego pyta dwa razy!
			return false;
		case 1:
//			makeNewTowar();
			break;
		case 0:
		default:
			return DoCommitTowardata();	// XXX if returns false - return now
			break;
	}
	return true;
}

bool tabFaktura::DoCommitTowardata(void) {
	pozfakdata *newdata;
	BString sql;

	// XXX tutaj wszystkie niezbędne testy, w razie bledu - return false

	if (towarmark < 0) {
		// nowa pozycja
		newdata = new pozfakdata();
	} else {
		newdata = faklista->itemat(towarmark);
	}
	// zapisz pola danych
	newdata->data[0] = "0";					// lp
	newdata->data[1] = towar[0]->Text();	// nazwa
	newdata->data[2] = towar[1]->Text();	// pkwiu
	newdata->data[3] = towar[4]->Text();	// ilosc
	newdata->data[4] = towar[5]->Text();	// jm
	newdata->data[5] = towar[3]->Text();	// rabat %
	newdata->data[6] = suma[0]->Text();		// cena jedn. (po rabacie)
	newdata->data[7] = suma[3]->Text();		// w.netto
	newdata->vatid = curtowarvatid;			// stawka vat, update data[8]
	newdata->data[9] = suma[4]->Text();		// kwota vat
	newdata->data[10] = suma[5]->Text();	// w.brutto
	newdata->data[11] = towar[2]->Text();	// c.netto
	sql = "SELECT nazwa FROM stawka_vat WHERE id = "; sql << curtowarvatid;
	newdata->data[8] = execSQL(sql.String());
	// dodaj do listy
	if (towarmark < 0)
		faklista->addlast(newdata);
	// nowy towar, wyczyść pola
	makeNewTowar();
	// update listy
	faklista->setlp();
	// update visuala
	RefreshTowarList();
	return true;
}

void tabFaktura::RefreshTowarList(void) {
	// clear current lists
	BListView *list;
	BString tmp;
	int i;

	for (i=0;i<=10;i++) {
		list = pozcolumn[i];
		if (list->CountItems()>0) {
			BStringItem *anItem;
			for (int i=0; (anItem=(BStringItem*)list->ItemAt(i)); i++)
				delete anItem;
			if (!list->IsEmpty())
				list->MakeEmpty();
		}
	}
	// fill with current data
	pozfakitem *cur = faklista->start;
	for (i=0;i<=10;i++) {
		pozcolumn[i]->AddItem(new BStringItem(plisthead[i]));
	}

	while (cur!=NULL) {
//		printf("[%i] - %s\n",cur->lp, cur->data->data[1].String());
		tmp = ""; tmp << cur->lp;
		pozcolumn[0]->AddItem(new BStringItem(tmp.String()));
		for (i=1;i<=10;i++) {
			pozcolumn[i]->AddItem(new BStringItem(cur->data->data[i].String()));
		}
		cur = cur->nxt;
	}
}

void tabFaktura::printCurrent(void) {
	printf("do printing stuff\n");
	beFakPrint *print = new beFakPrint(curdata->id, this->dbData);
	print->Go();
	printf("launched printing job\n");
	delete print;
}
