
#include <Button.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <View.h>
#include "ColumnListView.h"

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

tab3ListItem::tab3ListItem(const char *col0, const char *col1, const char *col2) : CLVEasyItem(
	0, false, false, 20.0) {
	SetColumnContent(0,col0);
	SetColumnContent(1,col1,true,true);
	SetColumnContent(2,col2,true,true);
}

dialStat::dialStat(sqlite3 *db, BHandler *hr) : BWindow(
	BRect(100+20, 100+20, 740+20, 580+20),
	NULL,
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ), beFakTab(NULL,db,NULL) {

	handler = hr;
	dbData = db;

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
	view->AddChild(minilosc = new BTextControl(BRect(240,10,500,30), "statRok", "Ukryj towary o sprzedanej ilości <=", NULL, new BMessage(DC)));
	view->AddChild(but_find = new BButton(BRect(560,10,630,20), "statButFind", "Znajdź", new BMessage(BUT_FIND)));
	but_find->ResizeToPreferred();
	view->AddChild(new BStringView(BRect(490,430,550,445), "statSum0d", "Suma netto:"));
	view->AddChild(suma[0] = new BStringView(BRect(560,430,630,445), "statSum0v", NULL));
	view->AddChild(new BStringView(BRect(490,450,550,465), "statSum1d", "Suma VAT:"));
	view->AddChild(suma[1] = new BStringView(BRect(560,450,630,465), "statSum1v", NULL));
	// add column list
	CLVContainerView *containerView;
	list = new ColumnListView(BRect(5,50,625,405), &containerView, NULL, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	list->AddColumn(new CLVColumn("Nazwa", 420, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Ilość", 100, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Wartość netto", 98, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->view->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));
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
	but_find->MakeDefault(true);
	this->SetTitle(title);
	this->Show();
}

void dialStat::DoFind(void) {
	BString sql;
	char omies[11];

	// clear current list
	if (list->CountItems()>0) {
		tab3ListItem *anItem;
		for (int i=0; (anItem=(tab3ListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
	// make dates
	sprintf(omies,"%s-%02i-01",rok->Text(),mies);
	// fetch data into list
	sql = "SELECT p.nazwa, DECROUND(SUM(p.ilosc)) AS sumailosc, DECROUND(SUM(DECROUND(DECROUND(p.netto*(100-p.rabat)/100.0)*p.ilosc))) AS sumanetto ";
	sql += "FROM faktura AS f, pozycjafakt AS p ";
	sql += "WHERE p.fakturaid = f.id AND f.data_sprzedazy BETWEEN '";
	sql += omies; sql += "' AND DATE('"; sql += omies; sql +="', '+1 month', 'start of month', '-1 day') ";
	sql += "GROUP BY p.nazwa ORDER BY sumanetto DESC";
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
	} else {
		int val;
		for (int i=1;i<=nRows;i++) {
			sql = "SELECT "; sql += result[i*nCols+1]; sql+= ">= 0"; sql += minilosc->Text();
			val = toint(execSQL(sql.String()));
			if (val)
				list->AddItem(new tab3ListItem(result[i*nCols+0], result[i*nCols+1], result[i*nCols+2]));
		}
	}
	sqlite3_free_table(result);
	// podsumowanie - suma netto, suma vat
	sql = "SELECT DECROUND(SUM(DECROUND(DECROUND(p.netto*(100-p.rabat)/100.0)*p.ilosc))) AS sumanetto, ";
	sql += "DECROUND(SUM(DECROUND(DECROUND(DECROUND(p.netto*(100-p.rabat)/100.0)*p.ilosc)*s.stawka/100.0))) AS sumavat ";
	sql += "FROM faktura AS f, pozycjafakt AS p, stawka_vat AS s ";
	sql += "WHERE p.fakturaid = f.id AND p.vatid = s.id AND f.data_sprzedazy BETWEEN '";
	sql += omies; sql += "' AND DATE('"; sql += omies; sql +="', '+1 month', 'start of month', '-1 day') ";
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
		suma[0]->SetText("0.00 zł");
		suma[1]->SetText("0.00 zł");
	} else {
		int i = nCols;
		sql = result[i++]; sql += " zł";
		suma[0]->SetText(sql.String());
		sql = result[i++]; sql += " zł";
		suma[1]->SetText(sql.String());
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
			// no idea what should be here...
			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}
