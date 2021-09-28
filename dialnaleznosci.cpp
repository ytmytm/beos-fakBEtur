
#include <Button.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <View.h>
#include "ColumnListView.h"
#include "CLVEasyItem.h"

#include "globals.h"
#include "fakdata.h"	// toint...
#include "dialnaleznosci.h"
#include "dialnalodb.h"
#include <stdio.h>

const uint32 DC			= 'NADC';
const uint32 BUT_FIND	= 'NAFI';
const uint32 BUT_WHO	= 'NAWO';
const uint32 BUT_PAY	= 'NAPY';
const uint32 BUT_PAYALL	= 'NAPA';
const uint32 LIST_INV	= 'NALI';

dialNaleznosci::dialNaleznosci(sqlite3 *db) : BWindow(
	BRect(100+20, 100+20, 740+20, 580+20),
	"Należności",
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ), beFakTab(NULL, db, NULL) {

	dbData = db;

	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
	view = new BView(this->Bounds(), "nalezView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	view->AddChild(daysago = new BTextControl(BRect(10,10,430,30), "nalezDago", "Wykaz niezapłaconych faktur, których termin płatności minął więcej niż: ", "30", new BMessage(DC)));
	daysago->SetDivider(be_plain_font->StringWidth(daysago->Label())+5);
	view->AddChild(new BStringView(BRect(435,10,510,30), "nalezDagos", "dni temu."));
	view->AddChild(but_find = new BButton(BRect(550,10,630,30), "nalezButFind", "Znajdź", new BMessage(BUT_FIND)));
	// lista
	CLVContainerView *containerView;
	list = new ColumnListView(BRect(10,45,620,420), &containerView, NULL, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	list->AddColumn(new CLVColumn("Numer", 100, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Nazwa odbiorcy", 240, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Dni", 70, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Zapłacono", 100, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Pozostało", 96, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->view->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(LIST_INV));
//	list->SetSelectionMessage(new BMessage(LIST_SEL));

	view->AddChild(but_whoowes = new BButton(BRect(10,440,110,460), "nalezButWho", "Należności odbiorcy", new BMessage(BUT_WHO)));
	view->AddChild(but_pay = new BButton(BRect(240,440,360,460), "nalezButPay", "Zapłacono za wybraną", new BMessage(BUT_PAY)));
	view->AddChild(but_payall = new BButton(BRect(500,440,620,460), "nalezButPayAll", "Zapłacono za wszystkie", new BMessage(BUT_PAYALL)));
	but_whoowes->ResizeToPreferred();
	but_pay->ResizeToPreferred();
	but_payall->ResizeToPreferred();

	but_find->MakeDefault(true);
	daysago->MakeFocus();
	this->Show();
}

void dialNaleznosci::DoFind(void) {
	// clear current list
	if (list->CountItems()>0) {
		tab5ListItem *anItem;
		for (int i=0; (anItem=(tab5ListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
	// construct sql query
	BString sql = "SELECT id, nazwa, onazwa, termin_zaplaty, zapl_kwota FROM faktura WHERE ";
	sql += "termin_zaplaty <= '";
	sql += daysagostring(toint(daysago->Text()));
	sql += "' ORDER BY termin_zaplaty";
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
	} else {
		int nRows2, nCols2;
		char **result2;
		for (int i=1;i<=nRows;i++) {
			// brutto 'do zaplaty'
			sql = "SELECT DECROUND(SUM(DECROUND(DECROUND(DECROUND(p.netto*(100-p.rabat)/100.0)*p.ilosc)*(100+s.stawka)/100.0))) AS sumabrutto ";
			sql += "FROM faktura AS f, pozycjafakt AS p, stawka_vat AS s ";
			sql += "WHERE p.fakturaid = f.id AND p.vatid = s.id AND f.id = ";
			sql << result[i*nCols+0];
			sqlite3_get_table(dbData, sql.String(), &result2, &nRows2, &nCols2, &dbErrMsg);
			// pozostalo - czy zaplacona_kwota < brutto?
			sql = "SELECT 0"; sql += result[i*nCols+4]; sql += "<0"; sql += result2[nCols2];
			if (toint(execSQL(sql.String()))) {
				BString dnizaleg;
				BString reszta;
				sql = "SELECT DECROUND(0"; sql += result2[nCols2]; sql += "-0"; sql += result[i*nCols+4]; sql += ")";
				reszta = validateDecimal(execSQL(sql.String()));
				dnizaleg << calcdaysago(result[i*nCols+3]);
				list->AddItem(new tab5ListItem(toint(result[i*nCols+0]), result[i*nCols+1], result[i*nCols+2], dnizaleg.String(), validateDecimal(result[i*nCols+4]), reszta.String()));
			}
			sqlite3_free_table(result2);
		}
	}
	sqlite3_free_table(result);
}

void dialNaleznosci::DoPayForAll(void) {
	int items = list->CountItems();

	for (int i=0;i<items;i++)
		DoPayFor(i);
}

void dialNaleznosci::DoPayFor(int item) {
	tab5ListItem *it = ((tab5ListItem*)list->ItemAt(item));
	if (it->Id() <= 0)
		return;
	BString sql = "UPDATE faktura SET zapl_kwota = DECROUND(0";
	sql += it->GetColumnContentText(3);
	sql += "+0";
	sql += it->GetColumnContentText(4);
	sql += "), zapl_dnia = DATE('now') WHERE id = ";
	sql << it->Id();
	execSQL(sql.String());
}

void dialNaleznosci::DoShowNaleznosciOdb(int item) {
	tab5ListItem *it = ((tab5ListItem*)list->ItemAt(item));
	nalodbDialog = new dialNalodb(dbData, it->Odbiorca());
}

void dialNaleznosci::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case DC:
			{ 	BString tmp = "SELECT ABS(ROUND(0";
				tmp += daysago->Text(); tmp += "))";
				daysago->SetText(execSQL(tmp.String()));
				break;
			}
		case BUT_FIND:
			DoFind();
			break;
		case BUT_PAY:
			{	int i = list->CurrentSelection(0);
				if (i>=0)
					DoPayFor(i);
				DoFind();
				break;
			}
			break;
		case BUT_PAYALL:
			DoPayForAll();
			DoFind();
			break;
		case BUT_WHO:
		case LIST_INV:
			{	int i = list->CurrentSelection(0);
				if (i>=0)
					DoShowNaleznosciOdb(i);
				break;
			}
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}
