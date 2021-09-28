//
// IDEAS:
// - zamiast usuwać/dodawać wszystkie pozitems - może pamiętać ich id?
//	 generować, uaktualniać, czyścić
//	 (problem: które UPDATE, które INSERT, a które DELETE)
//
// wyrzucić uwagi i zastąpić całym podsumowaniem? podsumowanie na 3 karcie?
// opcja faktury korygującej (jak? trzeba pamiętać co się zmieniło)
//

#include "globals.h"
#include "tabfaktura.h"
#include "printtext.h"
#include "printhtml.h"
#include "printview.h"
#include "dialimport.h"
#include "dialsymbol.h"
#include "dialcalendar.h"

#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <PrintJob.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include <TextView.h>
#include "ColumnListView.h"

#include <stdio.h>

const uint32 LIST_INV	= 'TFLI';
const uint32 LIST_SEL	= 'TFLS';
const uint32 BUT_NEW	= 'TFBN';
const uint32 BUT_DEL	= 'TFBD';
const uint32 BUT_RESTORE= 'TFBR';
const uint32 BUT_SAVE	= 'TFBS';
const uint32 BUT_PRINT	= 'TFBP';
const uint32 BUT_PAID	= 'TFBI';
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
extern const char *jmiary[];	// tabtowar.cpp
const char *plisthead[] = { "#", "Nazwa", "PKWiU", "Ilość", "Jm", "Rabat (%)", "Cena jedn.", "W. netto", "VAT", "W. VAT", "W. brutto", NULL };
const int plistw[] = { 15, 90, 60, 40, 20, 50, 70, 70, 30, 60, 70, -1 };

class pozCListItem : public CLVEasyItem {
	public:
		pozCListItem(int col0, const char *col1, const char *col2, const char *col3, const char *col4, const char *col5, const char *col6, const char *col7, const char *col8, const char *col9, const char *col10) : CLVEasyItem(
			0, false, false, 20.0) {
			BString tmp = "";
			tmp << col0;
			SetColumnContent(0,tmp.String(), true, true);	// lp
			SetColumnContent(1,col1);				// nazwa
			SetColumnContent(2,col2);				// pkwiu
			SetColumnContent(3,col3, true, true);	// ilosc
			SetColumnContent(4,col4, true, true);	// jm
			SetColumnContent(5,col5, true, true);	// rabat
			SetColumnContent(6,col6, true, true);	//
			SetColumnContent(7,col7, true, true);	//
			SetColumnContent(8,col8, true, true);	//
			SetColumnContent(9,col9, true, true);	//
			SetColumnContent(10,col10, true, true);	//
		}
};

tabFaktura::tabFaktura(BTabView *tv, sqlite3 *db, BHandler *hr) : beFakTab(tv, db, hr) {

	printSettings = NULL;
	curdata = new fakturadat(db);
	odbiorca = new firmadat(db);
	curtowar = new towardat(db);
	faklista = new pozfaklist(db);
	this->dirty = false;

	curtowarvatid = -1;
	towarmark = -1;
	towardirty = false;

	this->tab->SetLabel("Faktury [F2]");
	BRect r;
	r = this->view->Bounds();

	// columnlistview
	r.left = 5; r.right = 160; r.top = 30; r.bottom = 490;
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	list->AddColumn(new CLVColumn("Numer", 60, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Data sprzedaży", 94, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->view->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));
	// buttons
	but_new = new BButton(BRect(30,0,140,24), "tf_but_new", "Nowa faktura [F5]", new BMessage(BUT_NEW), B_FOLLOW_LEFT|B_FOLLOW_TOP);
	but_del = new BButton(BRect(30,510,140,534), "tf_but_del", "Usuń zaznaczone [F8]", new BMessage(BUT_DEL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_restore = new BButton(BRect(235,510,325,534), "tf_but_restore", "Przywróć [F6]", new BMessage(BUT_RESTORE), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_save = new BButton(BRect(580,510,670,534), "tf_but_save", "Zapisz", new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
	but_print = new BButton(BRect(405,510,485,534), "tf_but_print", "Drukuj [F9]", new BMessage(BUT_PRINT), B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
	this->view->AddChild(but_new);
	this->view->AddChild(but_del);
	this->view->AddChild(but_restore);
	this->view->AddChild(but_save);
	this->view->AddChild(but_print);
	but_new->ResizeToPreferred();
	but_del->ResizeToPreferred();
	but_restore->ResizeToPreferred();
	but_save->ResizeToPreferred();
	but_print->ResizeToPreferred();

	tbv2 = new BTabView(BRect(180,0,790,500), "tftbv2");
	this->view->AddChild(tbv2);
	r = tbv2->Bounds();
	r.InsetBy(5,5);
	viewogol = new BView(r, "tfviewogol", B_FOLLOW_ALL_SIDES, 0);
	viewogol->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	tabogol = new BTab(viewogol);
	tbv2->AddTab(viewogol, tabogol);
	tabogol->SetLabel("Dane ogólne [F10]");
	viewpozy = new BView(r, "tfviewpozy", B_FOLLOW_ALL_SIDES, 0);
	viewpozy->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	tabpozy = new BTab(viewpozy);
	tbv2->AddTab(viewpozy, tabpozy);
	tabpozy->SetLabel("Pozycje [F11]");

	initTab1();
	initTab2();
	makeNewForm();
	updateTab();
	RefreshIndexList();
}

tabFaktura::~tabFaktura() {
	delete curtowar;
	delete curdata;
	delete faklista;
	delete [] symbolIds;
	delete [] tsymbolIds;
	delete [] vatIds;
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
	msg->AddPointer("_datefield", ogol[2]);
	cbut[0] = new BButton(BRect(260,70,280,90), "tfcbut0", "+", msg);
	msg = new BMessage(CBUT);
	msg->AddPointer("_datefield", ogol[3]);
	cbut[1] = new BButton(BRect(260,100,280,120), "tfcbut1", "+", msg);
	box1->AddChild(cbut[0]);
	box1->AddChild(cbut[1]);
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
	ogol[5] = new BTextControl(BRect(10,20,150,40), "tfd5", "Forma", NULL, new BMessage(DC));
	ogol[6] = new BTextControl(BRect(10,50,150,70), "tfd6", "Termin", NULL, new BMessage(DC));
	ogol[7] = new BTextControl(BRect(200,50,270,70), "tfd7", "Dni", NULL, new BMessage(TERMCHANGE));
	box2->AddChild(ogol[5]);
	box2->AddChild(ogol[6]);
	box2->AddChild(ogol[7]);
	// box2-menu
	BMenu *menufp = new BMenu("");
	j=0;
	while (fplatnosci[j] != NULL) {
		msg = new BMessage(MENUFP); msg->AddString("_fp",fplatnosci[j]);
		menufp->AddItem(new BMenuItem(fplatnosci[j], msg));
		j++;
	}
	BMenuField *menufpField = new BMenuField(BRect(160,20,180,40), "tfmffp", NULL, menufp);
	box2->AddChild(menufpField);
	msg = new BMessage(CBUT);
	msg->AddPointer("_datefield", ogol[6]);
	cbut[2] = new BButton(BRect(160,50,180,70), "tfcbut2", "+", msg);
	box2->AddChild(cbut[2]);
	// box3
	box3 = new BBox(BRect(310,130,590,210),"tf1box3");
	box3->SetLabel("Zaliczka");
	viewogol->AddChild(box3);
	// box3-stuff
	ogol[8] = new BTextControl(BRect(10,20,170,40), "tfd8", "Kwota (zł)", NULL, new BMessage(DC));
	ogol[9] = new BTextControl(BRect(10,50,170,70), "tfd9", "Data", NULL, new BMessage(DC));
	box3->AddChild(ogol[8]);
	box3->AddChild(ogol[9]);
	msg = new BMessage(CBUT);
	msg->AddPointer("_datefield", ogol[9]);
	cbut[3] = new BButton(BRect(180,50,200,70), "tfcbut3", "+", msg);
	box3->AddChild(cbut[3]);
	but_paid = new BButton(BRect(180,20,230,40), "tfbutpaid", "Zapłacono", new BMessage(BUT_PAID));
	box3->AddChild(but_paid);
	but_paid->ResizeToPreferred();
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
	// firma-symbole
	menusymbol = new BPopUpMenu("[wybierz]");
	symbolRows = 0; symbolIds = NULL;
	RefreshFirmaSymbols();
	BMenuField *menusymbolField = new BMenuField(BRect(280,15,420,35), "tfmsymbol", "Symbol", menusymbol);
	menusymbolField->SetDivider(be_plain_font->StringWidth(menusymbolField->Label())+15);
	box4->AddChild(menusymbolField);
	// fix widths
	nazwa->SetDivider(be_plain_font->StringWidth(nazwa->Label())+5);
	int i;
	for (i=0;i<=9;i++)
		ogol[i]->SetDivider(be_plain_font->StringWidth(ogol[i]->Label())+5);
	// kolumna ogol: 0-4
	float d;
	d = 0;
	for (i=0;i<=4;i++)
		d = MAX(ogol[i]->Divider(), d);
	for (i=0;i<=4;i++)
		ogol[i]->SetDivider(d);
	// platnosc
	d = MAX(ogol[5]->Divider(), ogol[6]->Divider());
	ogol[5]->SetDivider(d); ogol[6]->SetDivider(d);
	// zaliczka
	d = MAX(ogol[8]->Divider(), ogol[9]->Divider());
	ogol[8]->SetDivider(d); ogol[9]->SetDivider(d);

	for (i=0;i<=10;i++) {
		if (i!=1)
			data[i]->SetDivider(be_plain_font->StringWidth(data[i]->Label())+5);
	}
	d = 0;
	for (i=0;i<=10;i++) {
		if ((i!=1)&&(i!=4)&&(i!=6))
			d = MAX(data[i]->Divider(), d);
	}
	for (i=0;i<=10;i++) {
		if ((i!=1)&&(i!=4)&&(i!=6))
			data[i]->SetDivider(d);
	}
	//
	but_save->MakeDefault(true);
}

void tabFaktura::initTab2(void) {
	BRect r, s;
	BMessage *msg;
	int i;
	// box5
	box5 = new BBox(BRect(10,10,590,350),"tfbox5", B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	box5->SetLabel("Pozycje");
	viewpozy->AddChild(box5);
	// box5-stuff
	CLVContainerView *containerView;
	pozlist = new ColumnListView(BRect(10,20,560,150), &containerView, NULL, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	for (i=0;i<=10;i++) {
		pozlist->AddColumn(new CLVColumn(plisthead[i], plistw[i], CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE));
	}
	pozlist->SetSortFunction(CLVEasyItem::CompareItems);
	box5->AddChild(containerView);
	pozlist->SetInvocationMessage(new BMessage(PLIST_INV));
	pozlist->SetSelectionMessage(new BMessage(PLIST_SEL));
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
	tsymbolRows = 0; tsymbolIds = NULL;
	RefreshTowarSymbols();
	BMenuField *tmenusymbolField = new BMenuField(BRect(200,15,300,35), "tfmtsymbol", "Symb.", tmenusymbol);
	tmenusymbolField->SetDivider(be_plain_font->StringWidth(tmenusymbolField->Label())+15);
	box6->AddChild(tmenusymbolField);
	// vat-symbole
	menuvat = new BPopUpMenu("[wybierz]");
	vatRows = 0; vatIds = NULL;
	RefreshVatSymbols();
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
	but_pnew = new BButton(BRect(310,260,370,290), "tf_but_pnew", "Nowy", new BMessage(BUT_PNEW));
	but_pimport = new BButton(BRect(390,260,480,290), "tf_but_pimport", "Import z innej", new BMessage(BUT_PIMPORT));
	but_pdel = new BButton(BRect(490,260,560,290), "tf_but_pdel", "Usuń", new BMessage(BUT_PDEL));
	box5->AddChild(but_psave);
	box5->AddChild(but_pnew);
	box5->AddChild(but_pimport);
	box5->AddChild(but_pdel);
	but_psave->ResizeToPreferred();
	but_pnew->ResizeToPreferred();
	but_pimport->ResizeToPreferred();
	but_pdel->ResizeToPreferred();

	r.left = 10; r.right = 100; r.top = 290; r.bottom = 310;
	s = r; s.OffsetBy(0,20);
	box5->AddChild(new BStringView(r, "tf_ss0", "Cena jednost.")); r.OffsetBy(100,0);
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
	box7 = new BBox(BRect(10,360,590,435), "tfbox7");
	box7->SetLabel("Uwagi");
	viewpozy->AddChild(box7);
	// box7-stuff
	r = box7->Bounds();
	r.InsetBy(10,15);
	s = r; s.OffsetTo(0,0);
	uwagi = new BTextView(r, "tfuwagi", s, B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
	box7->AddChild(uwagi);
	// payment
	sumasuma = new BStringView(BRect(10,440,90,455), "tfssd", "Do zapłaty:");
	viewpozy->AddChild(sumasuma);
	BFont fontb(be_bold_font);
	sumasuma->SetFont(&fontb);
	sumasuma->SetAlignment(B_ALIGN_RIGHT);
	sumasuma = new BStringView(BRect(100,440,400,455), "tfssv", NULL);
	viewpozy->AddChild(sumasuma);
	sumasuma->SetFont(&fontb);
	// magazyn
	box5->AddChild(magazyn = new BStringView(BRect(100,255,180,280), "tfmagd", "Magazyn:"));
	magazyn->SetAlignment(B_ALIGN_RIGHT);
	magazyn->SetFont(&fontb);
	box5->AddChild(magazyn = new BStringView(BRect(180,255,300,280), "tfmag", NULL));
	magazyn->SetFont(&fontb);
	// fix widths
	for (i=0;i<=5;i++)
		towar[i]->SetDivider(be_plain_font->StringWidth(towar[i]->Label())+5);
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
	uwagi->SetText(curdata->uwagi.String());
	updateTab();
}

void tabFaktura::updateTab(void) {
	BMessage *msg = new BMessage(MSG_NAMECHANGE);
	msg->AddString("_newtitle", nazwa->Text());
	handler->Looper()->PostMessage(msg);

	ogol[2]->SetText(validateDate(ogol[2]->Text()));
	ogol[3]->SetText(validateDate(ogol[3]->Text()));
	ogol[6]->SetText(validateDate(ogol[6]->Text()));
	ogol[8]->SetText(validateDecimal(ogol[8]->Text()));
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
	int nCols;
	char **result = faklista->calcBrutto(towar[2]->Text(), towar[3]->Text(), towar[4]->Text(), curtowarvatid, &nCols);
	if (nCols<1) {
		for (int j=0;j<6;j++)
			suma[j]->SetText("0.00");
	} else {
		for (int j=0;j<6;j++)
			suma[j]->SetText(result[nCols+j]);
	}
	faklista->calcBruttoFin(result);
	// retr magazyn state
	BString sql, nazwa;
	nazwa = towar[0]->Text(); nazwa.ReplaceAll("'","''"); nazwa.Prepend("'"); nazwa.Append("'");
	sql = "SELECT usluga FROM towar WHERE nazwa = "; sql += nazwa;
	if (!toint(execSQL(sql.String()))) {
		sql = "SELECT magazyn FROM towar WHERE nazwa = "; sql += nazwa;
		sql = execSQL(sql.String());
		sql += " ";
		sql += towar[5]->Text();
	}
	else
		sql = "[usługa]";
	magazyn->SetText(sql.String());
}

void tabFaktura::updatePayment(void) {
	BString tmp = faklista->calcSumPayment();
	tmp += " zł";
	sumasuma->SetText(tmp.String());
}

void tabFaktura::updateTermin(void) {
	BString tmp;

	tmp = "SELECT DATE('";
	tmp += ogol[3]->Text();
	tmp += "', '0";
	tmp += ogol[7]->Text();
	tmp += " days')";
	ogol[6]->SetText(execSQL(tmp.String()));
}

void tabFaktura::makeNewForm(void) {
	BString tmp;
	int i, curmax;
	BString rok, mies;
	int nRows, nCols;
	char **result;

	curdata->clear();
	faklista->clear();
	this->dirty = false;
	// unselect symbolmenu
	for (int i=0;i<symbolRows;i++)
		symbolMenuItems[i]->SetMarked(false);
	menusymbol->Superitem()->SetLabel("[wybierz]");
	// data sprzedaży, wystawienia
	tmp = execSQL("SELECT DATE('now')");
	curdata->ogol[2] = tmp;
	curdata->ogol[3] = tmp;
	// data płatności = dziś+paydays
	curdata->ogol[7] = execSQL("SELECT paydays FROM konfiguracja WHERE zrobiona = 1");
	ogol[3]->SetText(curdata->ogol[3].String());
	ogol[7]->SetText(curdata->ogol[7].String());
	updateTermin();
	curdata->ogol[6] = ogol[6]->Text();
	// 
	uwagi->SetText("");
	// miejsce wystawienia - weź z danych firmy
	curdata->ogol[0] = execSQL("SELECT miejscowosc FROM konfiguracja WHERE zrobiona = 1");
	// wystawiający - z konfiguracji
	curdata->ogol[1] = execSQL("SELECT f_wystawil FROM konfiguracja WHERE zrobiona = 1");
	// wygenerowanie nowej nazwy
	bool prosta = toint(execSQL("SELECT f_numprosta FROM konfiguracja WHERE zrobiona = 1"));
	// identyfikator z daty
	rok = tmp;
	rok.Remove(4,rok.Length()-4);
	if (prosta) {
		mies = "01";
	} else {
		mies = tmp;
		mies.Remove(0,5);
		mies.Remove(2,mies.Length()-2);
	}
	// max z konfiguracji? i tak działa nieźle
	curmax = 0;
	// nazwy wszystkich faktur z aktualnym rokiem i miesiacem
	tmp = "SELECT nazwa FROM faktura WHERE data_wystawienia > '";
	tmp += rok; tmp += "-"; tmp += mies; tmp += "-01' ORDER BY data_wystawienia";
	sqlite3_get_table(dbData, tmp.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
//		printf("database is empty\n");
	} else {
		for (int j=1;j<=nRows;j++) {
			tmp = result[j*nCols+0];
			tmp.ReplaceAll("-","/");
			i = tmp.FindFirst('/');
			tmp.Remove(i,tmp.Length()-i);
			i = toint(tmp.String());
			curmax = (i>curmax) ? i : curmax;
		}
	}
	sqlite3_free_table(result);
	// dodaj 1
	tmp = ""; tmp << curmax+1; tmp << "/";
	if (!(prosta)) {
		tmp << mies; tmp << "/";
	}
	tmp << rok;
	// wygeneruj string, zapisz
	curdata->nazwa = tmp;
	curdataToTab();
	RefreshTowarList();
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
	magazyn->SetText("");
	curtowarvatid = -1;
	towardirty = false;
	// un-mark list
	towarmark = -1;
	pozlist->DeselectAll();
	updateTab2();
}

bool tabFaktura::validateTab(void) {
	BAlert *error;
	BString sql, tmp;
	int i;

	// numer/nazwa - niepuste
	if (strlen(nazwa->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano numeru faktury!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		nazwa->MakeFocus();
		return false;
	}
	// numer/nazwa - unikalne
	tmp = nazwa->Text(); tmp.ReplaceAll("'","''");	// sql quote
	sql = "SELECT id FROM faktura WHERE nazwa = '"; sql += tmp; sql += "'";
	i = toint(execSQL(sql.String()));
	if (((curdata->id < 0) && ( i!= 0 )) || ((curdata->id > 0) && (i != 0) && (i != curdata->id))) {
		error = new BAlert(APP_NAME, "Numer faktury nie jest unikalny!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		nazwa->MakeFocus();
		return false;
	}
	// miejsce - niepuste
	if (strlen(ogol[0]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano miejsca wystawienia faktury!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		ogol[0]->MakeFocus();
		return false;
	}
	// wystawil - niepuste
	if (strlen(ogol[1]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano wystawiającego fakturę.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			ogol[1]->MakeFocus();
			return false;
		}
	}
	// data wystawienia - niepuste
	if (strlen(ogol[2]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano daty wystawienia faktury!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		ogol[2]->MakeFocus();
		return false;
	}
	// data sprzedazy - niepuste
	if (strlen(ogol[3]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano daty sprzedaży!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		ogol[3]->MakeFocus();
		return false;
	}
	// forma platnosci - niepusta
	if (strlen(ogol[5]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano formy płatności!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		ogol[5]->MakeFocus();
		return false;
	}
	// termin zaplaty - niepusty
	if (strlen(ogol[6]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano terminu zapłaty!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		ogol[6]->MakeFocus();
		return false;
	}
	// lista towarów niepusta
	if ((faklista->start == faklista->end) && (faklista->start == NULL)) {
		error = new BAlert(APP_NAME, "Na fakturze nie ma żadnej pozycji!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		tbv2->Select(1);
		return false;	
	}
	// test odbiorcy
	// adres - wszystkie dane
	if ((strlen(data[2]->Text())==0) || (strlen(data[3]->Text())==0) || (strlen(data[4]->Text())==0)) {
		error = new BAlert(APP_NAME, "Adres kontrahenta jest niepełny.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			data[2]->MakeFocus();
			return false;
		}
	}
	// NIP - niepusty,poprawny
	if (strlen(data[7]->Text())==0) {
		error = new BAlert(APP_NAME, "Nie wpisano numeru NIP kontrahenta.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			data[7]->MakeFocus();
			return false;
		}
	}
	// REGON - niepusty,poprawny
	if (strlen(data[8]->Text())==0) {
		error = new BAlert(APP_NAME, "Nie wpisano numeru REGON kontrahenta.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			data[8]->MakeFocus();
			return false;
		}
	}
	// nr konta - niepusty,poprawny
	if (strlen(data[10]->Text())==0) {
		error = new BAlert(APP_NAME, "Nie wpisanu numeru konta kontrahenta.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			data[10]->MakeFocus();
			return false;
		}
	}
	// nazwa - niepusta
	if (strlen(data[0]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano nazwy kontrahenta!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		data[0]->MakeFocus();
		return false;
	}
	// nazwa - unikalna
	tmp = data[0]->Text(); tmp.ReplaceAll("'","''");	// sql quote
	sql = "SELECT id FROM firma WHERE nazwa = '"; sql += tmp; sql += "'";
	i = toint(execSQL(sql.String()));
	if (i == 0) {
		firmadat *newfirma = new firmadat(dbData);
		newfirma->id = -1;
		for (i=0;i<=10;i++)
			if (i!=1)
				newfirma->data[i] = data[i]->Text();
		dialSymbol *tow = new dialSymbol(dbData, false, (dbdat*)newfirma, handler);
		tow->Show();
	} else {
		// taka nazwa jest w bazie...
		// porównać dane z tym z id
		firmadat *oldfirma = new firmadat(dbData);
		int j = 0;
		oldfirma->id = i;
		oldfirma->fetch();
		for (i=0;i<=10;i++) {
			if (i!=1) {
				if (strcmp(data[i]->Text(),oldfirma->data[i].String()))
					j++;
			}
		}
		// jeśli dane są różne - męczyć usera
		if (j!=0) {
			error = new BAlert(APP_NAME, "Dane kontrahenta z bazy różnią się z tymi wpisanymi na fakturze\nCo robić?.", "Uaktualnij bazę", "Uaktualnij fakturę", "Nic nie rób", B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			int ret = error->Go();
			switch(ret) {
				case 0:	// fak->baza
					for (i=0;i<=10;i++) {
						if (i!=1)
							oldfirma->data[i] = data[i]->Text();
					}
					oldfirma->commit();
					break;
				case 1: // baza->fak
					for (i=0;i<=10;i++) {
						if (i!=1)
							curdata->odata[i] = oldfirma->data[i];
					}
					break;
				case 2:	// nothing
				default:
					break;
			}
		}
		delete oldfirma;
	}
	return true;
}

bool tabFaktura::validateTowar(void) {
	BAlert *error;
	BString sql, tmp;
	int i;
	// nazwa niepusta?
	if (strlen(towar[0]->Text()) == 0) {
		// pusta, ale jesli nie brudne dane, to znaczy ze nowy towar!
		if (!towardirty)
			return true;
		error = new BAlert(APP_NAME, "Nie wpisano nazwy towaru!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		towar[0]->MakeFocus();
		return false;
	}
	// nazwa - unikalna na fakturze
	pozfakitem *cur = faklista->start;
	i = 0;
	while (cur!=NULL) {
		if (!strcmp(cur->data->data[1].String(), towar[0]->Text()))
			i++;
		cur = cur->nxt;
	}
	// jest na fakturze, ale czy to edycja, czy dodanie nowego?
	if ((i > 0) && (towarmark<1)) {
		error = new BAlert(APP_NAME, "Towar o tej nazwie już jest na fakturze!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		return false;
	}
	// pkwiu - ostrzeżenie że pusty
	if (strlen(towar[1]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano kodu PKWiU towaru.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			towar[1]->MakeFocus();
			return false;
		}
	}
	// cena niezerowa
	sql = "SELECT 100*0"; sql += towar[2]->Text();
	i = toint(execSQL(sql.String()));
	if (i == 0) {
		error = new BAlert(APP_NAME, "Cena towaru jest równa zero.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			towar[2]->MakeFocus();
			return false;
		}
	}
	// ilosc niezerowa
	sql = "SELECT 100*0"; sql += towar[4]->Text();
	i = toint(execSQL(sql.String()));
	if (i == 0) {
		error = new BAlert(APP_NAME, "Sprzedawana ilość towaru jest równa zero!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		towar[4]->MakeFocus();
		return false;
	}
	// jm - ostrzeżenie że pusty
	if (strlen(towar[5]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wybrano jednostki miary.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			towar[5]->MakeFocus();
			return false;
		}
	}
	// stawka vat
	if (curtowarvatid < 0) {
		error = new BAlert(APP_NAME, "Nie wybrano stawki VAT!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		return false;
	}
	// nazwa - nowa w bazie?
	tmp = towar[0]->Text(); tmp.ReplaceAll("'","''");	// sql quote
	sql = "SELECT id FROM towar WHERE nazwa = '"; sql += tmp; sql += "'";
	i = toint(execSQL(sql.String()));
	if (i == 0) {
		towardat *newtowar = new towardat(dbData);
		newtowar->id = -1;
		newtowar->data[0] = towar[0]->Text();	// nazwa
		newtowar->data[2] = towar[1]->Text();	// pkwiu
		newtowar->data[3] = towar[5]->Text();	// jm
		newtowar->ceny[0] = towar[2]->Text();	// cena netto
		newtowar->vatid = curtowarvatid;		// vatid
		dialSymbol *sym = new dialSymbol(dbData, true, (dbdat*)newtowar, handler);
		sym->Show();
	} else {
		// taka nazwa jest w bazie...
		// porównać dane z tym z id
		towardat *oldtowar = new towardat(dbData);
		int j = 0;
		oldtowar->id = i;
		oldtowar->fetch();
		if (strcmp(towar[1]->Text(),oldtowar->data[2].String())) j++; // pkwiu
		if (strcmp(towar[5]->Text(),oldtowar->data[3].String())) j++; // jm
		if (strcmp(towar[2]->Text(),oldtowar->ceny[0].String())) j++; // netto
		if (curtowarvatid != oldtowar->vatid) j++;					  // vatid
		// jeśli dane są różne - męczyć usera
		if (j!=0) {
			error = new BAlert(APP_NAME, "Dane towaru z bazy różnią się z tymi wpisanymi na fakturze\nCo robić?.", "Uaktualnij bazę", "Uaktualnij fakturę", "Nic nie rób", B_WIDTH_AS_USUAL, B_WARNING_ALERT);
			int ret = error->Go();
			switch(ret) {
				case 0:	// fak->baza
					oldtowar->data[2] = towar[1]->Text();
					oldtowar->data[3] = towar[5]->Text();
					oldtowar->ceny[0] = towar[2]->Text();
					oldtowar->vatid = curtowarvatid;
					oldtowar->commit();
					break;
				case 1: // baza->fak
					towar[1]->SetText(oldtowar->data[2].String());
					towar[2]->SetText(oldtowar->ceny[0].String());
					towar[5]->SetText(oldtowar->data[3].String());
					curtowarvatid = oldtowar->vatid;
					break;
				case 2:	// nothing
				default:
					break;
			}
		}
		// sprawdzić stan magazynu, o ile to nie usługa
		// i nie edytujemy starej faktury (data sprzedazy musi byc >= ostatnia zmiana mag)
		if (!(oldtowar->usluga)) {
			// if (data_sprzedazy)>=(data_ostatniej zmiany)
			sql = "SELECT '"; sql += ogol[3]->Text(); sql += "' >= '"; sql += oldtowar->magzmiana; sql += "'";
			if (toint(execSQL(sql.String()))) {
				if (curdata->id>0) {
					// magazyn+starafaktura < nowa?
					sql = "SELECT (0"; sql += oldtowar->magazyn; sql += "+ilosc < 0"; sql += towar[4]->Text();
					sql += ") FROM pozycjafakt WHERE fakturaid = "; sql << curdata->id;
					sql += " AND nazwa = '"; sql += oldtowar->data[0]; sql += "'";
				} else {
					// magazyn < nowa?
					sql = "SELECT 0"; sql += oldtowar->magazyn; sql += "< 0"; sql += towar[4]->Text();
				}
				if (toint(execSQL(sql.String()))) {
					error = new BAlert(APP_NAME, "Sprzedawana ilość jest większa od tej w magazynie.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
					if (error->Go() == 1) {
						towar[4]->MakeFocus();
						return false;
					}
				}
			}
		}
		delete oldtowar;
	}
	updateTab2();		// in case that data was fetched from db
	return true;
}

void tabFaktura::MessageReceived(BMessage *Message) {
	int i;
	int32 item;
	const char *tmp;
	BString result,sql;

	switch (Message->what) {
		case B_F10_KEY:
			tbv2->Select(0);
			break;
		case B_F11_KEY:
			tbv2->Select(1);
			break;
		case DC:
			this->dirty = true;
			updateTab();
			break;
		case B_F5_KEY:
		case BUT_NEW:
			if (CommitCurdata()) {
				list->DeselectAll();
				makeNewForm();
			}
			break;
		case B_F6_KEY:
		case BUT_RESTORE:
			DoFetchCurdata();
			break;
		case B_F8_KEY:
		case BUT_DEL:
			DoDeleteCurdata();
			break;
		case BUT_SAVE:
			updateTab();		// ensure that data is validated, DC might not be delivered
			updateTab2();		// ensure that data is validated, DC might not be delivered
			curdataFromTab();
			DoCommitCurdata();
			curdataToTab();
			break;
		case B_F9_KEY:
		case BUT_PRINT:
			updateTab();		// ensure that data is validated, DC might not be delivered
			updateTab2();		// ensure that data is validated, DC might not be delivered
			curdataFromTab();
			DoCommitCurdata();
			curdataToTab();
			printCurrent();	// XXX check if data was commited, id>0 etc
			break;
		case BUT_PAID:
			if (curdata->id>0) {
				updatePayment();
				ogol[8]->SetText(faklista->calcSumPayment());
				ogol[9]->SetText(execSQL("SELECT DATE('now')"));
				updateTab();
				updateTab2();
				curdataFromTab();
				DoCommitCurdata();
				curdataToTab();
			}
		case LIST_SEL:
		case LIST_INV:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					i = ((tab2ListItem*)list->ItemAt(list->CurrentSelection(0)))->Id();
					if (i>=0)
						ChangedSelection(i);
				} else {
					// deselection, what to do?
				}
				break;
			}
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
			updateTermin();
			this->dirty = true;
			break;
		case CBUT:
			{	void *ptr;
				BTextControl *dateField;
				BString curDate;
				if (Message->FindPointer("_datefield", &ptr) == B_OK) {
					dateField = static_cast<BTextControl*>(ptr);
					curDate = validateDate(dateField->Text());
					dialCalendar *calendarDialog = new dialCalendar(curDate.String(), dateField, DC, handler);
					calendarDialog->Show();
				}
				break;
			}
		case BUT_PIMPORT:
			{
				dialImport *di = new dialImport(dbData, curdata->id, faklista, handler);
				di->Show();	// go? block here? XXX lock faktura change/new/refresh?
				break;
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
				towar[4]->MakeFocus();
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
//			printf("saving!\n");
			DoCommitTowardata();
			this->dirty = true;
			towardirty = false;
			towarmark = -1;
			break;
		case BUT_PNEW:
			if (CommitCurtowar()) {
				makeNewTowar();
			}
			break;
		case BUT_PDEL:
			i = pozlist->CurrentSelection(0)+1;
			if (i>0) {
//				printf("delete %i-th\n",i);
				faklista->remove(i);
				makeNewTowar();
				faklista->setlp();
				this->dirty = true;
				RefreshTowarList();
			}
			break;
		case PLIST_SEL:
		case PLIST_INV:
			{	int i = pozlist->CurrentSelection(0);
				if (i>=0) {
					ChangedTowarSelection(i+1);
				} else {
					// deselection, what to do?				
				}
				break;
			}
		case MSG_REQVATUP:
			RefreshVatSymbols();
			break;
		case MSG_REQFIRMAUP:
			RefreshFirmaSymbols();
			break;
		case MSG_REQTOWARUP:
			RefreshTowarSymbols();
			break;
		case MSG_REQFAKPOZLIST:
			RefreshTowarList();
			this->dirty = true;
			break;
		case MENU_PAGESETUP:
			PageSetup(nazwa->Text());
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

	if ((newid<1) || (newid==towarmark))
		return;
	if (!(CommitCurtowar()))
		return;

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
		updateTab2();
	} else {
//printf("null item after selchg, happens after all-delete\n");
	}
	towardirty = false;
}

void tabFaktura::DoCommitCurdata(void) {
	if (!(validateTab()))
		return;
	if (!(DoCommitTowardata()))
		return;
//	update magazyn if needed
	faklista->updateStorage(curdata->id);
//	commit data
	curdata->commit();
	faklista->commit(curdata->id);
//	clear flags, refresh
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
		tab2ListItem *anItem;
		for (int i=0; (anItem=(tab2ListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
	// select list from db
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, "SELECT id, nazwa, data_sprzedazy FROM faktura ORDER BY data_sprzedazy, nazwa", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
	} else {
		for (int i=1;i<=nRows;i++)
			list->AddItem(new tab2ListItem(toint(result[i*nCols+0]), result[i*nCols+1], result[i*nCols+2]));
	}
	sqlite3_free_table(result);
}

// ret. false -> cancel action and resume editing
bool tabFaktura::CommitCurtowar(void) {
	if (!towardirty)
		return true;
	BAlert *ask = new BAlert(APP_NAME, "Zapisać zmiany w aktualnej pozycji?", "Tak", "Nie", "Anuluj", B_WIDTH_AS_USUAL, B_IDEA_ALERT);
	int ret = ask->Go();
	switch (ret) {
		case 2:
			return false;
		case 1:
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

	if (!(validateTowar()))
		return false;
	if (strlen(towar[0]->Text()) == 0)		// against adding empty after 'new'
		return true;
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
	// clear current list
	if (pozlist->CountItems()>0) {
		pozCListItem *anItem;
		for (int i=0; (anItem=(pozCListItem*)pozlist->ItemAt(i)); i++)
			delete anItem;
		if (!pozlist->IsEmpty())
			pozlist->MakeEmpty();
	}
	// fill with current data
	pozfakitem *cur = faklista->start;
	while (cur!=NULL) {
		pozlist->AddItem(new pozCListItem(cur->lp,
			cur->data->data[1].String(), cur->data->data[2].String(), cur->data->data[3].String(), cur->data->data[4].String(), cur->data->data[5].String(),
			cur->data->data[6].String(), cur->data->data[7].String(), cur->data->data[8].String(), cur->data->data[9].String(), cur->data->data[10].String()
		));
		cur = cur->nxt;
	}
	// refresh summary
	updatePayment();
}

void tabFaktura::RefreshFirmaSymbols(void) {
	int i = symbolRows;
	while (i>=0) {
		delete menusymbol->RemoveItem(i--);
	}

	delete [] symbolIds;

	int nRows, nCols;
	char **result;
	BMessage *msg;

	sqlite3_get_table(dbData, "SELECT id, symbol FROM firma WHERE aktywny = 1 ORDER BY id", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// empty table
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
	sqlite3_free_table(result);
}

void tabFaktura::RefreshTowarSymbols(void) {
	int i = tsymbolRows;
	while (i>=0) {
		delete tmenusymbol->RemoveItem(i--);
	}

	delete [] tsymbolIds;

	int nRows, nCols;
	char **result;
	BMessage *msg;

	sqlite3_get_table(dbData, "SELECT id, symbol FROM towar ORDER BY id", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// empty table
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
	sqlite3_free_table(result);
}

void tabFaktura::RefreshVatSymbols(void) {
	int i = vatRows;
	while (i>=0) {
		delete menuvat->RemoveItem(i--);
	}

	delete [] vatIds;

	int nRows, nCols;
	char **result;
	BMessage *msg;

	sqlite3_get_table(dbData, "SELECT id, nazwa FROM stawka_vat WHERE aktywne = 1 ORDER BY id", &result, &nRows, &nCols, &dbErrMsg);
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
	sqlite3_free_table(result);
}

void tabFaktura::printCurrent(void) {
	int p_typ = toint(execSQL("SELECT p_typ FROM konfiguracja WHERE zrobiona = 1"));
	int oldp_typ = p_typ;
	if (p_typ == 3) {
		// set typ orig, print
		execSQL("UPDATE konfiguracja SET p_typ = 0");
		printAPage();
		// set typ to copy, fall through
		execSQL("UPDATE konfiguracja SET p_typ = 1");
		p_typ = 1;
	}
	// print copies?
	if (p_typ == 1) {
		int p_lkopii = toint(execSQL("SELECT p_lkopii FROM konfiguracja WHERE zrobiona = 1"));
		for (int i=1;i<=p_lkopii;i++)
			printAPage(i);
	} else {
		// print dupe or orig
		printAPage();
	}
	// restore original if needed
	if (p_typ != oldp_typ) {
		char *query = sqlite3_mprintf("UPDATE konfiguracja SET p_typ = %i", oldp_typ);
		sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
		sqlite3_free(query);
	}
}

void tabFaktura::printAPage(int numkopii) {
	if (curdata->id<0)
		return;
	beFakPrint *print;
	int p_mode = toint(execSQL("SELECT p_mode FROM konfiguracja WHERE zrobiona = 1"));
	switch(p_mode) {
		case 2:
			print = new printHTML(curdata->id, this->dbData, numkopii);
			break;
		case 1:
			print = new printText(curdata->id, this->dbData, numkopii);
			break;
		case 0:
		default:
			{	if (printSettings == NULL)
					if (PageSetup(nazwa->Text()) != B_OK)
						return;
				print = new printView(curdata->id, this->dbData, numkopii, printSettings);
				break;
			}
	}
	print->Go();
	delete print;
}

status_t tabFaktura::PageSetup(const char *documentname) {
	status_t result = B_OK;

	BPrintJob printJob(documentname);

	if (printSettings != NULL)
		printJob.SetSettings(new BMessage(*printSettings));

	result = printJob.ConfigPage();

	if (result == B_NO_ERROR) {
		delete printSettings;
		printSettings = printJob.Settings();
	}

	return result;
}
