
//
// XXX mostly most of db stuff is duped from dialnaleznosci
//

#include <Box.h>
#include <Button.h>
#include <String.h>
#include <StringView.h>
#include <View.h>
#include "ColumnListView.h"
#include "CLVEasyItem.h"

#include "globals.h"
#include "dialnaleznosci.h"
#include "dialnalodb.h"
#include "fakdata.h"
#include <stdio.h>

const uint32 BUT_CLOSE	= 'DNOC';

enum { F_ZALEGLA = 1, F_NIEZAPL, F_ZAPLACONA };

dialNalodb::dialNalodb(sqlite3 *db, const char *odb) : BWindow(
	BRect(100+40, 100+40, 740+40, 580+40),
	NULL,
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ), beFakTab(NULL,db,NULL) {

	odbiorca = odb;

	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
	view = new BView(this->Bounds(), "nalodbView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);

	view->AddChild(boxl = new BBox(BRect(10,10,325,430), "nalodbBoxl"));
	boxl->SetLabel("Faktury niezapłacone");
	view->AddChild(boxr = new BBox(BRect(335,10,630,430), "nalodbBoxr"));
	boxr->SetLabel("Faktury zapłacone");

	boxl->AddChild(razemnold = new BStringView(BRect(10,370,220,390), "nalrazemnOldn", "Razem po terminie zapłaty:"));
	razemnold->SetAlignment(B_ALIGN_RIGHT);
	boxl->AddChild(razemnold = new BStringView(BRect(230,370,300,390), "nalrazemOld", NULL));
	boxl->AddChild(razemn = new BStringView(BRect(10,395,220,410), "nalrazemnn", "Wszystkie niezapłacone:"));
	razemn->SetAlignment(B_ALIGN_RIGHT);
	boxl->AddChild(razemn = new BStringView(BRect(230,395,300,410), "nalrazemn", NULL));

	boxr->AddChild(razemp = new BStringView(BRect(10,370,190,390), "nalrazempn", "Razem:"));
	razemp->SetAlignment(B_ALIGN_RIGHT);
	boxr->AddChild(razemp = new BStringView(BRect(200,370,280,390), "nalrazemp", NULL));

	// lista
	CLVContainerView *containerViewL;
	listl = new ColumnListView(BRect(10,20,290,340), &containerViewL, NULL, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	listl->AddColumn(new CLVColumn("Numer", 67, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	listl->AddColumn(new CLVColumn("Nazwa odbiorcy", 240, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE|CLV_HIDDEN));
	listl->AddColumn(new CLVColumn("Dni", 60, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	listl->AddColumn(new CLVColumn("Zapłacono", 75, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	listl->AddColumn(new CLVColumn("Pozostało", 75, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	listl->SetSortFunction(CLVEasyItem::CompareItems);
	boxl->AddChild(containerViewL);
	// lista
	CLVContainerView *containerViewR;
	listr = new ColumnListView(BRect(10,20,270,340), &containerViewR, NULL, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	listr->AddColumn(new CLVColumn("Numer", 130, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	listr->AddColumn(new CLVColumn("Zapłacono", 129, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	listr->SetSortFunction(CLVEasyItem::CompareItems);
	boxr->AddChild(containerViewR);

	view->AddChild(but_close = new BButton(BRect(540,440,620,470), "nalodbButClose", "Zamknij", new BMessage(BUT_CLOSE)));
	but_close->ResizeToPreferred();
	but_close->MakeDefault(true);

	BString tmp = "SELECT id FROM firma WHERE nazwa = '";
	tmp += odb; tmp += "'";
	id = toint(execSQL(tmp.String()));
	if (id<=0)
		return;
	tmp = "Wykaz należności: "; tmp += odb;
	this->SetTitle(tmp.String());

	BString sql;
	int nRows, nCols;
	char **result;
	int typ;

	// prepare temporary for stats
	execSQL("CREATE TEMPORARY TABLE nalodbsuma ( kwota DECIMAL(12,2), typ INTEGER )");

	sql = "SELECT f.id, f.nazwa, f.onazwa, f.termin_zaplaty, f.zapl_kwota";
	sql += " FROM faktura AS f, firma AS k WHERE k.nazwa = f.onazwa ORDER BY f.data_sprzedazy";
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
				// niezapłacona
				BString dnizaleg;
				BString reszta;
				sql = "SELECT DECROUND(0"; sql += result2[nCols2]; sql += "-0"; sql += result[i*nCols+4]; sql += ")";
				reszta = validateDecimal(execSQL(sql.String()));
				dnizaleg << calcdaysago(result[i*nCols+3]);
				listl->AddItem(new tab5ListItem(toint(result[i*nCols+0]), result[i*nCols+1], result[i*nCols+2], dnizaleg.String(), validateDecimal(result[i*nCols+4]), reszta.String()));
				// update summary
				if (toint(dnizaleg.String()) > 0)
					typ = F_ZALEGLA;
				else
					typ = F_NIEZAPL;
				
				char *query = sqlite3_mprintf("INSERT INTO nalodbsuma VALUES ( %Q, %i )", reszta.String(), typ);
				sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
				sqlite3_free(query);
				
			} else {
				// zapłacona
				listr->AddItem(new tab2ListItem(toint(result[i*nCols+0]), result[i*nCols+1], result2[nCols2]));
				typ = F_ZAPLACONA;
				char *query = sqlite3_mprintf("INSERT INTO nalodbsuma VALUES ( %Q, %i )", result2[nCols2], typ);
				sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
				sqlite3_free(query);
			}
			sqlite3_free_table(result2);
		}
	}
	sqlite3_free_table(result);
	// fill summaries
	sql = "SELECT DECROUND(SUM(kwota)) FROM nalodbsuma WHERE typ = ";
	tmp = sql; tmp << F_ZALEGLA;
	tmp = execSQL(tmp.String()); tmp += " zł";
	razemnold->SetText(tmp.String());
	tmp = sql; tmp << F_ZALEGLA; tmp += " OR typ = "; tmp << F_NIEZAPL;
	tmp = execSQL(tmp.String()); tmp += " zł";
	razemn->SetText(tmp.String());
	tmp = sql; tmp << F_ZAPLACONA;
	tmp = execSQL(tmp.String()); tmp += " zł";
	razemp->SetText(tmp.String());
	execSQL("DROP TABLE nalodbsuma");
	this->Show();
}

void dialNalodb::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case BUT_CLOSE:
			Quit();
			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}
