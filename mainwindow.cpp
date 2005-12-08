//
// TODO:
// - dialog do edycji stawek vat
// - drukowanie przez printjob
// - usunąć duplikat execSQL() - zrobic jakos globalnie?
// - prawdziwe validateDate
// - menu do sterowania num.uproszczona, # kopii, EOL, wybór pliku szablonu
// - numeracja uproszczona - uzywac? (wtedy bez miesiaca)
// - liczba kopii - uzywac? (ale to u mnie, czy w print/page setup?)
// - pole 'uwagi' w towar/faktura nie reaguje na zmiany
// - dialog kalendarza
// - guzik importu z innej faktury (wybór numeru z listy/ok/anuluj)
// - DECROUND zaimplementować jako hook i userfunction, nie jako patch na sqlite
// - dummy menu ze statystykami
// - trzymanie stanu magazynu, info i podsumowania magazynowe
// - na koniec - usunac printfy z debugiem
// zmiana curtab i przełączanie jest brzydkie, może cały beFakTab powinien
// dziedziczyć z btab?

#include "mainwindow.h"
#include "dialfirma.h"
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
const uint32 MENU_CONFFIRMA	= 'MKOF';
const uint32 MENU_CONFVAT	= 'MKOV';
const uint32 MENU_PRINTT80	= 'MPT8';
const uint32 MENU_PRINTT136 = 'MPT1';
const uint32 MENU_PRINTHTML = 'MPHT';
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
	menu->AddItem(new BMenuItem("O programie", new BMessage(MENU_ABOUT)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Zamknij", new BMessage(B_QUIT_REQUESTED), 'Q'));
	menuBar->AddItem(menu);

	menu = new BMenu("Dokument", B_ITEMS_IN_COLUMN);
	menu->AddItem(pmenuo = new BMenuItem("Oryginał", new BMessage(MENU_PRINTO)));
	menu->AddItem(pmenuc = new BMenuItem("Kopia", new BMessage(MENU_PRINTC)));
	menu->AddItem(pmenud = new BMenuItem("Duplikat", new BMessage(MENU_PRINTD)));
	menuBar->AddItem(menu);

	menu = new BMenu("Opcje", B_ITEMS_IN_COLUMN);
	menu->AddItem(new BMenuItem("Dane firmy", new BMessage(MENU_CONFFIRMA)));
	BMenu *printmenu = new BMenu("Rodzaj wydruku", B_ITEMS_IN_COLUMN);
	menu->AddItem(printmenu);
	menu->AddItem(new BMenuItem("Stawki VAT", new BMessage(MENU_CONFVAT)));
	menuBar->AddItem(menu);

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
	tabView->Select(0);
	curTab = tabs[0];
	updateMenus();
}

BeFAKMainWindow::~BeFAKMainWindow() {
	CloseDatabase();
}

void BeFAKMainWindow::initTabs(BTabView *tv) {
	tabs[0] = new tabFaktura(tv, dbData, this);
	tabs[1] = new tabTowar(tv, dbData, this);
	tabs[2] = new tabFirma(tv, dbData, this);
}

void BeFAKMainWindow::DoConfigFirma(bool cancancel) {
	firmaDialog = new dialFirma(APP_NAME, dbData, cancancel);
}

void BeFAKMainWindow::DoConfigVAT(void) {
	// XXX a new db dialog here
	BAlert *vatalert = new BAlert(APP_NAME, "Tutaj dialog do edycji stawek VAT", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
	vatalert->Go();
}

void BeFAKMainWindow::DoCheckConfig(void) {
	int nRows, nCols;
	char **result;
	BString sql;
	// select NAZWA and all config data
	sql = "SELECT nazwa, wersja, p_mode, p_typ, p_textcols, p_texteol, p_lkopii, f_numprosta FROM konfiguracja WHERE zrobiona = 1";
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
		p_lkopii = toint(result[i++]);
		f_numprosta = toint(result[i++]);
	}
}

void BeFAKMainWindow::updateMenus(void) {
	pmenuo->SetMarked(p_typ == 0);
	pmenuc->SetMarked(p_typ == 1);
	pmenud->SetMarked(p_typ == 2);
	pmenut80->SetMarked( (p_mode==1) && (p_textcols==80) );
	pmenut136->SetMarked( (p_mode==1) && (p_textcols==136) );
	pmenuhtml->SetMarked(p_mode == 2);
	BString sql = "UPDATE konfiguracja SET p_mode = %i, p_typ = %i, p_textcols = %i, "
		"p_texteol = %i, p_lkopii = %i, f_numprosta = %i "
		"WHERE zrobiona = 1";
	sqlite_exec_printf(dbData, sql.String(), 0, 0, &dbErrMsg,
		p_mode, p_typ, p_textcols, p_texteol, p_lkopii, f_numprosta);
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
		case MSG_NAMECHANGE:
			if (Message->FindString("_newtitle", &tmp) == B_OK) {
				title = APP_NAME; title += " : "; title += tmp;
				this->SetTitle(title.String());
			}
			break;
		case MSG_REQFIRMAUP:
		case MSG_REQTOWARUP:
			// XXX tab 0 is faktura, make it defined constant
			tabs[0]->MessageReceived(Message);
			break;
		case MSG_REQTOWARLIST:
			// XXX tab 1 is towar, make it defined constant
			tabs[1]->MessageReceived(Message);
			break;
		case MSG_REQFIRMALIST:
			// XXX tab 2 is firma, make it defined constant
			tabs[2]->MessageReceived(Message);
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
