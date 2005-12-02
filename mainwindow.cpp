//
// TODO:
// - jesli nie ma pliku bazy danych - stworzyc z wbudowanego schema
// - konfiguracja - osobna tabela w bazie
// - przy pierwszym uruchomieniu sprawdzic istnienie konfiguracji
//	 - nie ma -> wywolac dialog
// - menu z dialogiem konfiguracji (moja firma, inne opcje)
// - menu z aboutprogram (balert)
// - menu z drukowaniem (oryginal/kopia), wydruk, eksport html etc.
// - nazwa towaru/faktury/kontrahenta w pasku tytułu (przyjąć msg od child?)
//
// zmiana curtab i przełączanie jest brzydkie, może cały beFakTab powinien
// dziedziczyć z btab?
//

#include "mainwindow.h"
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
//	menu->AddItem(new BMenuItem("Item", new BMessage(MENU_DEFMSG), 0, 0));
	menu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
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
//	r.left = 150; r.top = 20; r.bottom = r.bottom - 50;
	r.top = 20;
	tabView = new BTabView(r, "tabView");
	mainView->AddChild(tabView);

	// initialize database
	int ret = OpenDatabase();
	if (ret < 0)
		exit(1);

	// initialize datawidgets
	initTabs(tabView);
	tabView->Select(0);
}

BeFAKMainWindow::~BeFAKMainWindow() {
	CloseDatabase();
}

void BeFAKMainWindow::initTabs(BTabView *tv) {
	tabs[0] = new tabFaktura(tv, dbData);
	tabs[1] = new tabTowar(tv, dbData);
	tabs[2] = new tabFirma(tv, dbData);
}

void BeFAKMainWindow::MessageReceived(BMessage *Message) {
	this->DisableUpdates();
	switch (Message->what) {
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
	curTab->CommitCurdata(false);
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

//--------------------
// database stuff, abstract it?

int BeFAKMainWindow::OpenDatabase(void) {
	// XXX port BSAP solution for non-existing db
	dbData = sqlite_open("faktury.sq2", 0666, &dbErrMsg);
	if ((dbData==0)||(dbErrMsg!=0)) {
		printf("database not found\n");	// XXX alert!
		return -1;
	}
	sqlite_exec(dbData, "VACUUM", 0, 0, &dbErrMsg);
	return 0;
}

void BeFAKMainWindow::CloseDatabase(void) {
	sqlite_exec(dbData, "VACUUM", 0, 0, &dbErrMsg);
	sqlite_close(dbData);
}
