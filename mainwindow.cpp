//
// TODO:
// - obsluga drukowania/eksportu 
//		- commit current faktura
//		- przekazanie parametrow:
//			fakturaid, kopia/orig/dupl, # kopii, wydruk/eksport HTML/text
//		- w zaleznosci od typu wydruku nowy obiekt (dziedziczy z tego samego?)
// - dialog konfiguracji wydruku (liczba kopii, tekst/grafika/?)
// - dummy menu ze statystykami
// - na koniec - usunac printfy z debugiem
// zmiana curtab i przełączanie jest brzydkie, może cały beFakTab powinien
// dziedziczyć z btab?
// - daleka przyszlosc: przepisac ROUND() w sqlite tak, by dzialalo prawidlowo
// wydruk:
//	- jedna bazowa klasa do drukowania, przyjmuje parametry (k/o/d, #kopii), oblicza brakujace
//	  dane itp. pozfakdata+faktura+konfiguracja powinny wystarczyc
//	  klasy pochodne - generowanie naglowka, tabelki i stopki
//	  	html - wypelnienie szablonu, zapis do pliku
//		print - wyrysowanie bview, pochodna z bview, printjob
//		txt - wypisanie i sformatowanie tekstu na 80/130(?) kolumn, zapis do pliku lub na lp

#include "mainwindow.h"
#include "dialabout.h"
#include "dialfirma.h"
#include "sqlschema.h"
#include "tabfirma.h"
#include "tabtowar.h"
#include "tabfaktura.h"

#include <View.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <TabView.h>

#include <stdio.h>

const uint32 MENU_PRINTO	= 'MPOR';
const uint32 MENU_PRINTC	= 'MPKO';
const uint32 MENU_PRINTD	= 'MPKD';
const uint32 MENU_CONFFIRMA	= 'MKOF';
const uint32 MENU_CONFPRINT	= 'MKOP';
const uint32 MENU_ABOUT		= 'MABO';

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
	((SpLocaleApp*)be_app)->AddToFileMenu(menu,false,false,false);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Zamknij", new BMessage(B_QUIT_REQUESTED), 'Q'));
	menuBar->AddItem(menu);

	menu = new BMenu("Wydruk", B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem("Oryginał", new BMessage(MENU_PRINTO)));
	menu->AddItem(new BMenuItem("Kopia", new BMessage(MENU_PRINTC)));
	menu->AddItem(new BMenuItem("Duplikat", new BMessage(MENU_PRINTD)));
	menuBar->AddItem(menu);

	menu = new BMenu("Opcje", B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem("Dane firmy", new BMessage(MENU_CONFFIRMA)));
	menu->AddItem(new BMenuItem("Wydruki", new BMessage(MENU_CONFPRINT)));
	menuBar->AddItem(menu);

	menu = new BMenu("Pomoc", B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem("O programie", new BMessage(MENU_ABOUT)));
	menuBar->AddItem(menu);

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
	tabView->Select(0);
}

BeFAKMainWindow::~BeFAKMainWindow() {
	CloseDatabase();
}

void BeFAKMainWindow::initTabs(BTabView *tv) {
	tabs[0] = new tabFaktura(tv, dbData, this);
	tabs[1] = new tabTowar(tv, dbData, this);
	tabs[2] = new tabFirma(tv, dbData, this);
}

void BeFAKMainWindow::DoAbout(void) {
	aboutDialog = new dialAbout(APP_NAME "  " APP_VERSION);
}

void BeFAKMainWindow::DoConfigFirma(bool cancancel) {
	firmaDialog = new dialFirma(APP_NAME, dbData, cancancel);
}

void BeFAKMainWindow::DoCheckConfig(void) {
	int nRows, nCols;
	char **result;
	BString sql;
	// select NAZWA and all config data
	sql = "SELECT nazwa, liczbakopii, ostatni_nr, num_prosta FROM konfiguracja WHERE zrobiona = 1";
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
		liczbakopii = toint(result[i++]);
		ostatni_nr = toint(result[i++]);
//		printf("ostatninr = %i\n",ostatni_nr);
		num_prosta = toint(result[i++]);
	}
}

void BeFAKMainWindow::MessageReceived(BMessage *Message) {
	BString title;
	const char *tmp;

	this->DisableUpdates();
	switch (Message->what) {
		case MENU_PRINTO:
		case MENU_PRINTC:
		case MENU_PRINTD:
			{	BMessage *msg;
				msg = new BMessage(MSG_ORDERPRINT);
				// XXX fit msg with additional print params
				curTab->MessageReceived(msg);
				break;
			}
		case MENU_ABOUT:
			DoAbout();
			break;
		case MENU_CONFFIRMA:
			DoConfigFirma();
			break;
		case MSG_NAMECHANGE:
			if (Message->FindString("_newtitle", &tmp) == B_OK) {
				title = APP_NAME; title += " : "; title += tmp;
				this->SetTitle(title.String());
			}
			break;
		default:
			curTab = tabs[tabView->Selection()];
			curTab->MessageReceived(Message);
			BWindow::MessageReceived(Message);
			break;
	}
	this->EnableUpdates();
}

bool BeFAKMainWindow::QuitRequested() {
//	config->position = this->Frame();
//	config->save();
	// XXX cannot commit upon exit! object doesn't exist here!!!
//	curTab->CommitCurdata(false);
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
}
