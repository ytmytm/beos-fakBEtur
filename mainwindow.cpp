
#include "mainwindow.h"
#include "tabfirma.h"
#include <View.h>
#include <Alert.h>
//#include <Box.h>
//#include <Button.h>
//#include <CheckBox.h>
#include <ListView.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
//#include <PopUpMenu.h>
//#include <RadioButton.h>
//#include <ScrollView.h>
//#include <StringView.h>
#include <TabView.h>
//#include <TextControl.h>
#include <stdio.h>

const uint32 MENU_DEFMSG 	= 'M000';

BeFAKMainWindow::BeFAKMainWindow(const char *windowTitle) : BWindow(
	BRect(100, 100, 900, 700), windowTitle, B_DOCUMENT_WINDOW, B_OUTLINE_RESIZE, B_CURRENT_WORKSPACE ) {

	// get memory for objects
	idlist = NULL;
	curdata = new fakdat();

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
	menu->AddItem(new BMenuItem("Item", new BMessage(MENU_DEFMSG), 0, 0));
	menu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
	menuBar->AddItem(menu);

	// tabview
	r = mainView->Bounds();
//	r.left = 150; r.top = 20; r.bottom = r.bottom - 50;
	r.top = 20;
	tabView = new BTabView(r, "tabView");
	mainView->AddChild(tabView);

	// initialize widgets
	initTabs(tabView);

	// initialize database
	int ret = OpenDatabase();
	if (ret < 0)
		exit(1);
	// initialize list
//	XXX act like if this one is new or select the first one?
	currentid = -1;
}

BeFAKMainWindow::~BeFAKMainWindow() {
	CloseDatabase();
}

void BeFAKMainWindow::initTabs(BTabView *tv) {
	tabFirma *tab1;
	tab1 = new tabFirma(tv);
}

void BeFAKMainWindow::curdataFromTabs(void) {
}

void BeFAKMainWindow::curdata2Tabs(void) {
}

void BeFAKMainWindow::MessageReceived(BMessage *Message) {
	int i;
	this->DisableUpdates();
	switch (Message->what) {
		case MENU_DEFMSG:
//			RefreshIndexList();
			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
	this->EnableUpdates();
}

void BeFAKMainWindow::ChangedSelection(int newid) {
	if (!(CommitCurdata())) {
		// XXX do nothing if cancel, restore old selection?
		return;
	}
	// fetch and store into new data
//	printf("fetching for index=%i\n",newid);
	FetchCurdata(newid);
//	curdata2Tabs();
	currentid = newid;
}

bool BeFAKMainWindow::QuitRequested() {
//	config->position = this->Frame();
//	config->save();
//	CommitCurdata(false);
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

//--------------------
// database stuff, abstract it?

// if returns false -> cancel action and resume editing current data
bool BeFAKMainWindow::CommitCurdata(bool haveCancelButton = true) {
	if (!(curdata->dirty))
		return true;
	// ask if store
	BAlert *ask;
	if (haveCancelButton)
		ask = new BAlert(APP_NAME, "Zapisać zmiany w aktualnej karcie?", "Tak", "Nie", "Anuluj", B_WIDTH_AS_USUAL, B_IDEA_ALERT);
	else
		ask = new BAlert(APP_NAME, "Zapisać zmiany w aktualnej karcie?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_IDEA_ALERT);
	int ret = ask->Go();
	switch (ret) {
		case 2:
//printf("cancel\n");
			return false;
			break;
		case 1:
//printf("nie\n");
			break;
		case 0:
		default:
//printf("commiting data\n");
			curdataFromTabs();
			DoCommitCurdata();
	};
	return true;
}

// insert/update curdata unconditionally
void BeFAKMainWindow::DoCommitCurdata(void) {

}

// delete current
void BeFAKMainWindow::DoDeleteCurdata(void) {
}

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

void BeFAKMainWindow::RefreshIndexList(void) {
	// clear current list
	if (listView->CountItems()>0) {
		BStringItem *anItem;
		for (int i=0; (anItem=(BStringItem*)listView->ItemAt(i)); i++)
			delete anItem;
		if (!listView->IsEmpty())
			listView->MakeEmpty();
	}
	// clear current idlist
	delete [] idlist;
	// select list from db
}

int BeFAKMainWindow::GenerateId(void) {
	int id = 1;
	int nRows, nCols;
	char **result;
	sqlite_get_table(dbData, "SELECT MAX(id) FROM karta", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows > 0) {
		// there is something in db
		id = toint(result[1]) + 1;
	}
	sqlite_free_table(result);
	return id;
}

void BeFAKMainWindow::FetchCurdata(int id) {
printf("in fetchcurdata with %i\n",id);
	curdata->id = id;
	curdata->dirty = false;
}

//--------------------

void fakdat::dump_all(void) {
	printf("------------\n");
	printf("\n");
}

void fakdat::clear(void) {
	dirty = false;
	id = -1;
}

//---------------------

#include <stdlib.h>

int toint(const char *input) {
	if (input != NULL)
		return strtol(input, NULL, 10);
	else
		return 0;
}
