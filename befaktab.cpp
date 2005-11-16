
#include "befaktab.h"
#include "globals.h"

#include <Alert.h>
#include <TabView.h>
#include <View.h>
#include <stdio.h>

beFakTab::beFakTab(BTabView *tv, sqlite *db) {

	dbData = db;

	BRect r;

	r = tv->Bounds();
	r.InsetBy(5, 10);
	view = new BView(r, "tab", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	tab = new BTab(view);
	tv->AddTab(view, tab);
}

beFakTab::~beFakTab() {
	// nothing?
}

void beFakTab::MessageReceived(BMessage *Message) {
	// process message
}

// if returns false -> cancel action and resume editing current data
bool beFakTab::CommitCurdata(bool haveCancelButton = true) {
	// ask if commit data from current object into database
	if (!dirty)
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
printf("commiting data\n");
			curdataFromTab();
			DoCommitCurdata();	// this is implemented in inherited class
	};
	return true;
}
