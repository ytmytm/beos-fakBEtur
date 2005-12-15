
#include <Button.h>
#include <ListView.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <View.h>

#include "globals.h"
#include "fakdata.h"	// toint...
#include "dialstat.h"
#include <stdio.h>

const uint32 DC			= 'STDC';
const uint32 BUT_FIND	= 'STFI';
const uint32 MENU_MIES	= 'STMI';
const uint32 LIST_INV	= 'STLI';
const uint32 LIST_SEL	= 'STLS';

const char *miesiace[] = { "styczeń", "luty", "marzec", "kwiecień", "maj", "czerwiec", "lipiec", "sierpień", "wrzesień", "październik", "listopad", "grudzień", NULL };
const char *title = "Miesięczna statystyka sprzedaży";

dialStat::dialStat(sqlite *db, BHandler *hr) : BWindow(
	BRect(100, 100, 740, 580),
	NULL,
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ) {

	handler = hr;
	dbData = db;

	lastsel = -1;

	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
	view = new BView(this->Bounds(), "statView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	menu = new BPopUpMenu("[wybierz]");
	BMessage *msg;
	for (int i=0;i<=11;i++) {
		msg = new BMessage(MENU_MIES);
		msg->AddInt32("_miesid", i+1);
		menu->AddItem(new BMenuItem(miesiace[i], msg));
	}
	view->AddChild(new BMenuField(BRect(10,10,150,30), "statMenuf", "Dane za", menu));
	view->AddChild(rok = new BTextControl(BRect(160,10,230,30), "statRok", "Rok", NULL, new BMessage(DC)));
	view->AddChild(minilosc = new BTextControl(BRect(240,10,500,30), "statRok", "Nie pokazuj towarów o sprzedanej ilości <=", NULL, new BMessage(DC)));
	view->AddChild(but_find = new BButton(BRect(560,10,630,20), "statButFind", "Znajdź", new BMessage(BUT_FIND)));
	view->AddChild(new BStringView(BRect(490,430,550,445), "statSum0d", "Suma netto:"));
	view->AddChild(suma[0] = new BStringView(BRect(560,430,630,445), "statSum0v", NULL));
	view->AddChild(new BStringView(BRect(490,450,550,465), "statSum1d", "Suma VAT:"));
	view->AddChild(suma[1] = new BStringView(BRect(560,450,630,465), "statSum1v", NULL));
	// add 3 column list
	viewtable = new BView(BRect(5,50,625,410), "statTableview", B_FOLLOW_ALL_SIDES, 0);
	BRect r = viewtable->Bounds();
	r.right = r.right - 70 - 70;
	listcol[0] = new BListView(r, NULL); viewtable->AddChild(listcol[0]);
	r.left = r.right; r.right += 70;
	listcol[1] = new BListView(r, NULL); viewtable->AddChild(listcol[1]);
	r.left = r.right; r.right += 70;
	listcol[2] = new BListView(r, NULL); viewtable->AddChild(listcol[2]);
	view->AddChild(new BScrollView("statScroll", viewtable, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	for (int i=0;i<=2;i++) {
		listcol[i]->SetInvocationMessage(new BMessage(LIST_INV));
		listcol[i]->SetSelectionMessage(new BMessage(LIST_SEL));
	}
	// fix width
	rok->SetDivider(be_plain_font->StringWidth(rok->Label())+5);
	minilosc->SetDivider(be_plain_font->StringWidth(minilosc->Label())+5);
	// defaults into rok/minilosc - current
	BString data = execSQL("SELECT DATE('now')");
	if (data.Length()==0)
		data = "2006-01-01";
	BString dmies = data;
	data.Remove(4,data.Length()-4);
	rok->SetText(data.String());
	dmies.Remove(0,5);
	dmies.Remove(2,dmies.Length()-2);
	mies = toint(dmies.String());
	menu->ItemAt(mies-1)->SetMarked(true);
	minilosc->SetText("0");
	makeListHeaders();
	but_find->MakeDefault(true);
	this->SetTitle(title);
	this->Show();
}

void dialStat::makeListHeaders(void) {
	// clear current lists
	BListView *list;
	int i;

	for (i=0;i<=2;i++) {
		list = listcol[i];
		if (list->CountItems()>0) {
			BStringItem *anItem;
			for (int i=0; (anItem=(BStringItem*)list->ItemAt(i)); i++)
				delete anItem;
			if (!list->IsEmpty())
				list->MakeEmpty();
		}
	}
	// insert headers
	listcol[0]->AddItem(new BStringItem("Nazwa"));
	listcol[1]->AddItem(new BStringItem("Ilość"));
	listcol[2]->AddItem(new BStringItem("Wartość netto"));
}

void dialStat::DoFind(void) {
	BString sql;
	char omies[11];
	// clear list
	makeListHeaders();
	// make dates
	sprintf(omies,"%s-%02i-01",rok->Text(),mies);
	// fetch data into list
	sql = "SELECT p.nazwa, DECROUND(SUM(p.ilosc)) AS sumailosc, DECROUND(SUM(DECROUND(DECROUND(p.netto*(100-p.rabat)/100.0)*p.ilosc))) AS sumanetto ";
	sql += "FROM faktura AS f, pozycjafakt AS p ";
	sql += "WHERE p.fakturaid = f.id AND f.data_sprzedazy BETWEEN '";
	sql += omies; sql += "' AND DATE('"; sql += omies; sql +="', '+1 month', 'start of month', '-1 day') ";
	sql += "GROUP BY p.nazwa ORDER BY sumanetto DESC";
	int val;
	int nRows, nCols;
	char **result;
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
	} else {
		for (int i=1;i<=nRows;i++) {
			sql = "SELECT "; sql += result[i*nCols+1]; sql+= ">= 0"; sql += minilosc->Text();
			val = toint(execSQL(sql.String()));
			if (val)
				for (int j=0;j<=2;j++) {
					listcol[j]->AddItem(new BStringItem(result[i*nCols+j]));
				}
		}
	}
	sqlite_free_table(result);
	// podsumowanie - suma netto, suma vat
	sql = "SELECT DECROUND(SUM(DECROUND(DECROUND(p.netto*(100-p.rabat)/100.0)*p.ilosc))) AS sumanetto, ";
	sql += "DECROUND(SUM(DECROUND(DECROUND(DECROUND(p.netto*(100-p.rabat)/100.0)*p.ilosc)*s.stawka/100.0))) AS sumavat ";
	sql += "FROM faktura AS f, pozycjafakt AS p, stawka_vat AS s ";
	sql += "WHERE p.fakturaid = f.id AND p.vatid = s.id AND f.data_sprzedazy BETWEEN '";
	sql += omies; sql += "' AND DATE('"; sql += omies; sql +="', '+1 month', 'start of month', '-1 day') ";
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
		suma[0]->SetText("0");
		suma[1]->SetText("0");
	} else {
		int i = nCols;
		suma[0]->SetText(result[i++]);
		suma[1]->SetText(result[i++]);
	}
	// update window title
	sql = title; sql += " za "; sql += miesiace[mies-1]; sql += " "; sql += rok->Text();
	this->SetTitle(sql.String());
}

bool dialStat::QuitRequested() {
	return true;
}

void dialStat::MessageReceived(BMessage *Message) {
	BString tmp;
	int32 item;
	switch (Message->what) {
		case BUT_FIND:
			DoFind();
			break;
		case DC:
			// validate rok/ilosc as integers
			tmp = "SELECT ROUND(0"; tmp += rok->Text(); tmp += ")";
			rok->SetText(execSQL(tmp.String()));
			tmp = "SELECT ROUND(0"; tmp += minilosc->Text(); tmp += ")";
			minilosc->SetText(execSQL(tmp.String()));
			break;
		case MENU_MIES:
			if (Message->FindInt32("_miesid", &item) == B_OK) {
				mies = item;
			}
			break;
		case LIST_INV:
		case LIST_SEL:
			{	int i = -1;
				void *ptr;
				BListView *plist;
				if (Message->FindPointer("source", &ptr) == B_OK) {
					plist = static_cast<BListView*>(ptr);
					i = plist->CurrentSelection(0);
				}
				if (i != lastsel) {
					lastsel = i;
					if (lastsel != 0) {
						for (int j=0;j<=2;j++) {
							listcol[j]->Select(i);
							listcol[j]->ScrollToSelection();
						}
					} else {
						for (int j=0;j<=2;j++) {
							listcol[j]->Select(0);
							listcol[j]->ScrollToSelection();
							listcol[j]->DeselectAll();
						}
					}
				}
				break;
			}
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}

// XXX this is 3rd duplicated in befaktab, fakdata!
const char *dialStat::execSQL(const char *input) {
	int nRows, nCols;
	char **result;
	static BString res;
//printf("sql=[%s]\n",sql.String());
	sqlite_get_table(dbData, input, &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows < 1)
		res = "";
	else
		res = result[1];
	sqlite_free_table(result);
	return res.String();
}
