
#include "befaktab.h"
#include "fakdata.h"
#include "globals.h"

#include <Alert.h>
#include <String.h>
#include <TabView.h>
#include <View.h>
#include <parsedate.h>
#include <stdio.h>

beFakTab::beFakTab(BTabView *tv, sqlite3 *db, BHandler *hr) {

	dbData = db;
	handler = hr;

	if (tv == NULL)
		return;

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

const char *beFakTab::execSQL(const char *input) {
	int nRows, nCols;
	char **result;
	static BString res;
//printf("sql=[%s]\n",sql.String());
	sqlite3_get_table(dbData, input, &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows < 1)
		res = "";
	else
		res = result[1];
	sqlite3_free_table(result);
	return res.String();
}

const char *beFakTab::validateDecimal(const char *input) {
	BString sql, tmp;

	tmp = input;
	if (tmp.Length() == 0)
		tmp = "0";
	tmp.ReplaceAll(",",".");	// XXX more safeguards?

	sql = "SELECT DECROUND(ABS(0"; sql += tmp; sql += "))";
	return execSQL(sql.String());
}

const char *beFakTab::validateDate(const char *input) {
	static BString tmp;
	time_t date = parsedate(input,-1);
	if (date<0) {
		tmp = execSQL("SELECT DATE('now')");
	} else {
		char result[11];
		struct tm t;
		localtime_r(&date,&t);
		strftime(result,sizeof(result),"%F",&t);
		tmp = result;
	}
	return tmp.String();
}

// if returns false -> cancel action and resume editing current data
bool beFakTab::CommitCurdata(bool haveCancelButton) {
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
//printf("commiting data\n");
			curdataFromTab();
			DoCommitCurdata();	// this is implemented in inherited class
	};
	return true;
}
