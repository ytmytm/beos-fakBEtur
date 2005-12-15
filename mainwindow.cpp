//
// TODO:
// - konfiguracja:
//		- czy ostrzegać o wszystkich błędach?
//		- czy używać numeracji uproszczonej (bez miesiąca: nr kolejny/rok)
//		- txt: znak końca linii
// - przy wydruku (tabfaktura) honorować:
//		- liczba kopii
//		- złożenie oryginał+faktura
// - parametry wydruku przetwarzać w tabfaktura, każdy obiekt print użyć raz
//		- przy zapisie do pliku dołączać typ dokumentu i numer kopii do nazwy
// - dialog kalendarza
// - menu ze statystykami
//		- sprzedaży netto: select nazwa, decround(sum(decround(decround(netto*(100-rabat)/100.0)*ilosc))) as suma from pozycjafakt group by nazwa order by suma desc;
//			- dodać wybór miesiąca
//		- niezapłacone faktury
//		- przeterminowane płatności
// - trzymanie stanu magazynu, info i podsumowania magazynowe
//		- info o stanie magazynowym z dnia XXXX do ustawienia w towary
//			- dodać pole 'magazyn' i 'datamagazyn' do db
//		- podsumowanie zliczać dynamicznie (nie trzeba wtedy uaktualniać stanu
//		  przy zmianie faktury)
//		- nie robić w/w dla tych, które są 'usługa'
// - wydruk - cennik
// - pole 'uwagi' w towar/faktura nie reaguje na zmiany
// - usunąć duplikat execSQL() - zrobic jakos globalnie?
// - na koniec - usunac printfy z debugiem
//
// zmiana curtab i przełączanie jest brzydkie, może cały beFakTab powinien
// dziedziczyć z btab?

#include "mainwindow.h"
#include "dialfirma.h"
#include "dialvat.h"
#include "dialnumber.h"
#include "sqlschema.h"
#include "tabfirma.h"
#include "tabtowar.h"
#include "tabfaktura.h"

#include <Alert.h>
#include <Application.h>
#include <File.h>
#include <View.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <TabView.h>

#include <stdio.h>

const uint32 MENU_PRINTO	= 'MPOR';
const uint32 MENU_PRINTC	= 'MPKO';
const uint32 MENU_PRINTD	= 'MPKD';
const uint32 MENU_PRINTE	= 'MPKE';
const uint32 MENU_CONFFIRMA	= 'MKOF';
const uint32 MENU_CONFVAT	= 'MKOV';
const uint32 MENU_PRINTPS	= 'MPPS';
const uint32 MENU_PRINTT80	= 'MPT8';
const uint32 MENU_PRINTT136 = 'MPT1';
const uint32 MENU_PRINTHTML = 'MPHT';
const uint32 MENU_ABOUT		= 'MABO';
const uint32 MENU_PAYDAY	= 'MPAY';
const uint32 MENU_NUMCOPY	= 'MNCO';

const uint32 MSG_NUMCOPY	= 'mNCO';
const uint32 MSG_PAYDAY		= 'mPAY';

enum { FAKTURATAB = 0, TOWARTAB, FIRMATAB };

BeFAKMainWindow::BeFAKMainWindow(const char *windowTitle) : BWindow(
	BRect(100, 100, 900, 700), windowTitle, B_DOCUMENT_WINDOW, B_OUTLINE_RESIZE, B_CURRENT_WORKSPACE ) {

	BView *mainView = new BView(this->Bounds(), "mainView", B_FOLLOW_ALL_SIDES, 0);

	if (mainView == NULL) {
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}

	mainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(mainView);

	BRect r;
	r = mainView->Bounds();
	r.bottom = 20;
	menuBar = new BMenuBar(r, "menuBar");
	mainView->AddChild(menuBar);

	menu = new BMenu("Plik", B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem("O programie", new BMessage(MENU_ABOUT)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Ustawienia strony", new BMessage(MENU_PAGESETUP)));
	menu->AddItem(new BMenuItem("Zamknij", new BMessage(B_QUIT_REQUESTED), 'Q'));
	menuBar->AddItem(menu);

	menu = new BMenu("Dokument", B_ITEMS_IN_COLUMN);
	menu->AddItem(pmenuo = new BMenuItem("Oryginał", new BMessage(MENU_PRINTO)));
	menu->AddItem(pmenuc = new BMenuItem("Kopia", new BMessage(MENU_PRINTC)));
	menu->AddItem(pmenud = new BMenuItem("Duplikat", new BMessage(MENU_PRINTD)));
	menu->AddItem(pmenue = new BMenuItem("Oryginał+kopie", new BMessage(MENU_PRINTE)));
	menuBar->AddItem(menu);

	menu = new BMenu("Opcje", B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem("Dane firmy", new BMessage(MENU_CONFFIRMA)));
	menu->AddItem(new BMenuItem("Stawki VAT", new BMessage(MENU_CONFVAT)));
	menu->AddItem(new BMenuItem("Termin płatności", new BMessage(MENU_PAYDAY)));
	menu->AddSeparatorItem();
	BMenu *printmenu = new BMenu("Rodzaj wydruku", B_ITEMS_IN_COLUMN);
	menu->AddItem(printmenu);
	menu->AddItem(new BMenuItem("Liczba kopii", new BMessage(MENU_NUMCOPY)));
	menuBar->AddItem(menu);

	printmenu->AddItem(pmenups   = new BMenuItem("Drukarka", new BMessage(MENU_PRINTPS)));
	printmenu->AddItem(pmenut80  = new BMenuItem("Tekst (80 kol.)", new BMessage(MENU_PRINTT80)));
	printmenu->AddItem(pmenut136 = new BMenuItem("Tekst (136 kol.)", new BMessage(MENU_PRINTT136)));
	printmenu->AddItem(pmenuhtml = new BMenuItem("HTML", new BMessage(MENU_PRINTHTML)));

	// tabview
	r = mainView->Bounds();
	r.top = 20;
	tabView = new BTabView(r, "tabView");
	mainView->AddChild(tabView);

	// initialize database
	int ret = OpenDatabase();
	if (ret < 0)
		exit(1);

	// check if configuration is there
	DoCheckConfig();
	// initialize datawidgets
	initTabs(tabView);
	curTab = tabs[FAKTURATAB];
	tabView->Select(FAKTURATAB);
	updateMenus();
}

BeFAKMainWindow::~BeFAKMainWindow() {
	CloseDatabase();
}

void BeFAKMainWindow::initTabs(BTabView *tv) {
	tabs[FAKTURATAB] = new tabFaktura(tv, dbData, this);
	tabs[TOWARTAB] = new tabTowar(tv, dbData, this);
	tabs[FIRMATAB] = new tabFirma(tv, dbData, this);
}

void BeFAKMainWindow::DoConfigFirma(bool cancancel) {
	firmaDialog = new dialFirma(APP_NAME, dbData, cancancel);
}

void BeFAKMainWindow::DoConfigVAT(void) {
	vatDialog = new dialVat(dbData, this);
}

void BeFAKMainWindow::DoConfigCopies(void) {
	BString def;
	def = tabs[FAKTURATAB]->execSQL("SELECT p_lkopii FROM konfiguracja WHERE zrobiona = 1");
	numDialog = new dialNumber("Liczba drukowanych kopii", "Liczba kopii", def.String(), MSG_NUMCOPY, this);
}

void BeFAKMainWindow::DoConfigCopiesAfter(BMessage *msg) {
	const char *tmp;
	BString sql;
	int p_lkopii = 0;
	if (msg->FindString("_value", &tmp) == B_OK) {
		sql = "SELECT ABS('0"; sql += tmp; sql += "')";
		p_lkopii = toint(tabs[FAKTURATAB]->execSQL(sql.String()));
		sqlite_exec_printf(dbData, "UPDATE konfiguracja SET p_lkopii = %i WHERE zrobiona = 1", 0, 0, &dbErrMsg,
			p_lkopii);
	}
}

void BeFAKMainWindow::DoConfigPayday(void) {
	BString def;
	def = tabs[FAKTURATAB]->execSQL("SELECT paydays FROM konfiguracja WHERE zrobiona = 1");
	numDialog = new dialNumber("Domyślny termin płatności", "Liczba dni", def.String(), MSG_PAYDAY, this);
}

void BeFAKMainWindow::DoConfigPaydayAfter(BMessage *msg) {
	const char *tmp;
	BString sql;
	int paydays = 0;
	if (msg->FindString("_value", &tmp) == B_OK) {
		sql = "SELECT ABS('0"; sql += tmp; sql += "')";
		paydays = toint(tabs[FAKTURATAB]->execSQL(sql.String()));
		sqlite_exec_printf(dbData, "UPDATE konfiguracja SET paydays = %i WHERE zrobiona = 1", 0, 0, &dbErrMsg,
			paydays);
	}
}

void BeFAKMainWindow::DoCheckConfig(void) {
	int nRows, nCols;
	char **result;
	BString sql;
	// select NAZWA and all config data
	sql = "SELECT nazwa, wersja, p_mode, p_typ, p_textcols, p_texteol, f_numprosta FROM konfiguracja WHERE zrobiona = 1";
//printf("sql:%s\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i\n", nRows, nCols);
	if (nRows < 1) {
		DoConfigFirma(false);	// should never happen
	} else {
		int i = nCols;
		sql = result[i++];	// test for 'nazwa' not ""
		if (sql.Length() == 0)
			DoConfigFirma(false);
		// read other config data from result
		if (strcmp(result[i++], APP_DBVERSION)) {
			sql = "Plik:\n"; sql += DATABASE_PATHNAME;
			sql += "\nzawiera nieprawidłową (starszą) wersję bazy danych,\n";
			sql += "proszę go usunąć i wprowadzić dane ponownie.\n";
			sql += "Do czasu rozstrzygnięcia konkursu format bazy\n";
			sql += "może jeszcze ulec zmianie!\n";
			BAlert *error = new BAlert(APP_NAME, sql.String(), "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
			error->Go();
			exit(2);
		}
		p_mode = toint(result[i++]);
		p_typ = toint(result[i++]);
		p_textcols = toint(result[i++]);
		p_texteol = toint(result[i++]);
		f_numprosta = toint(result[i++]);
	}
}

void BeFAKMainWindow::updateMenus(void) {
	pmenuo->SetMarked(p_typ == 0);
	pmenuc->SetMarked(p_typ == 1);
	pmenud->SetMarked(p_typ == 2);
	pmenue->SetMarked(p_typ == 3);
	pmenups->SetMarked(p_mode == 0);
	pmenut80->SetMarked( (p_mode==1) && (p_textcols==80) );
	pmenut136->SetMarked( (p_mode==1) && (p_textcols==136) );
	pmenuhtml->SetMarked(p_mode == 2);
	BString sql = "UPDATE konfiguracja SET p_mode = %i, p_typ = %i, p_textcols = %i, "
		"p_texteol = %i, f_numprosta = %i "
		"WHERE zrobiona = 1";
	sqlite_exec_printf(dbData, sql.String(), 0, 0, &dbErrMsg,
		p_mode, p_typ, p_textcols, p_texteol, f_numprosta);
//printf("result:%s\n",dbErrMsg);
}

void BeFAKMainWindow::MessageReceived(BMessage *Message) {
	BString title;
	const char *tmp;

	this->DisableUpdates();
	switch (Message->what) {
		case MENU_PRINTO:
			p_typ = 0;
			updateMenus();
			break;
		case MENU_PRINTC:
			p_typ = 1;
			updateMenus();
			break;
		case MENU_PRINTD:
			p_typ = 2;
			updateMenus();
			break;
		case MENU_PRINTE:
			p_typ = 3;
			updateMenus();
		case MENU_PRINTPS:
			p_mode = 0;
			updateMenus();
			break;
		case MENU_PRINTT80:
			p_mode = 1;
			p_textcols = 80;
			updateMenus();
			break;
		case MENU_PRINTT136:
			p_mode = 1;
			p_textcols = 136;
			updateMenus();
			break;
		case MENU_PRINTHTML:
			p_mode = 2;
			updateMenus();
			break;
		case MENU_ABOUT:
			// really should go with B_ABOUT_REQUESTED, but it doesn't work...
			be_app->AboutRequested();
			break;
		case MENU_CONFFIRMA:
			DoConfigFirma();
			break;
		case MENU_CONFVAT:
			DoConfigVAT();
			break;
		case MENU_NUMCOPY:
			DoConfigCopies();
			break;
		case MSG_NUMCOPY:
			DoConfigCopiesAfter(Message);
			break;
		case MENU_PAYDAY:
			DoConfigPayday();
			break;
		case MSG_PAYDAY:
			DoConfigPaydayAfter(Message);
			break;
		case MSG_NAMECHANGE:
			if (Message->FindString("_newtitle", &tmp) == B_OK) {
				title = APP_NAME; title += " : "; title += tmp;
				this->SetTitle(title.String());
			}
			break;
		case MENU_PAGESETUP:
		case MSG_REQFIRMAUP:
		case MSG_REQTOWARUP:
			tabs[FAKTURATAB]->MessageReceived(Message);
			break;
		case MSG_REQTOWARLIST:
			tabs[TOWARTAB]->MessageReceived(Message);
			break;
		case MSG_REQFIRMALIST:
			tabs[FIRMATAB]->MessageReceived(Message);
			break;
		case MSG_REQVATUP:
			tabs[FAKTURATAB]->MessageReceived(Message);
			tabs[TOWARTAB]->MessageReceived(Message);
			break;
		default:
			curTab = tabs[tabView->Selection()];
			curTab->MessageReceived(Message);
			break;
	}
	BWindow::MessageReceived(Message);
	this->EnableUpdates();
}

bool BeFAKMainWindow::QuitRequested() {
//	config->position = this->Frame();
//	config->save();
	curTab->CommitCurdata(false);
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

//--------------------
// database stuff, abstract it?

int BeFAKMainWindow::OpenDatabase(void) {
	// BFile test wouldn't be necessary if sqlite_open retured file not found
	// error as advertised
	BFile fData;
	int fResult = fData.SetTo(DATABASE_PATHNAME, B_READ_ONLY);
	// open database anyway - new or existing
	dbData = sqlite_open(DATABASE_PATHNAME, 0666, &dbErrMsg);
	if ((dbData==0)||(dbErrMsg!=0)) {
		// due to sqlite problems - this code is never reached; pity
//		printf("database not found\n");
		return -1;
	}
	if (fResult != B_OK) {
		// file wasn't there - new database has been created
		// populate it with schema and default configuration
		InitDatabase();
	}
	// if VACUUM fails on DB there is another error, we can't handle it
	return sqlite_exec(dbData, "VACUUM", 0, 0, &dbErrMsg);
}

void BeFAKMainWindow::CloseDatabase(void) {
	sqlite_exec(dbData, "VACUUM", 0, 0, &dbErrMsg);
	sqlite_close(dbData);
}

void BeFAKMainWindow::InitDatabase(void) {
//	printf("new database, fill with schema\n");
	sqlite_exec(dbData, sql_schema, 0, 0, &dbErrMsg);
//printf("init result:[%s]\n", dbErrMsg);
}
