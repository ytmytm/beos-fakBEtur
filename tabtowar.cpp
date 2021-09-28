//
// obliczenia via sqlite, dzialaja ladnie jesli argumenty maja '.' np. 25/100.0
// zaokraglanie do dwoch cyfr wlasne, zgodne z rozporzadzeniem mf (0.005->0.01)
// pozostaje miec nadzieje, ze jest liczone poprawnie
// pola ceny[] ida przez sqlite - mozna wpisywac wyrazenia arytmetyczne
//
// pole 'usługa' - związane tylko z magazynem
// TODO:
// blokada: DoCommitCurdata powinno zwracać wartość do Commit, aby móc anulować
// zmianę pozycji (cur-dirty, zmiana na liście)

#include "globals.h"
#include "tabtowar.h"

#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include "ColumnListView.h"
#include <stdio.h>

const uint32 LIST_INV	= 'TTLI';
const uint32 LIST_SEL	= 'TTLS';
const uint32 BUT_NEW	= 'TTBN';
const uint32 BUT_DEL	= 'TTBD';
const uint32 BUT_RESTORE= 'TTBR';
const uint32 BUT_SAVE	= 'TTBS';
const uint32 DC			= 'TTDC';
const uint32 BUT_SELL	= 'TTBL';
const uint32 BUT_IMPORT	= 'TTBI';
const uint32 BUT_MARZA	= 'TTBM';

const uint32 IGNORE		= 'IGNO';
const uint32 MENUJM		= 'TTMJ';
const uint32 MENUVAT	= 'TTMV';

const char *jmiary[] = { "szt.", "kg", "kpl.", "m", "mb", "m2", "km", "l", NULL };

tabTowar::tabTowar(BTabView *tv, sqlite3 *db, BHandler *hr) : beFakTab(tv, db, hr) {

	curdata = new towardat(db);
	this->dirty = false;

	this->tab->SetLabel("Towary [F3]");
	BRect r;
	r = this->view->Bounds();

	// columnlistview
	r.left = 5; r.right = 160; r.top = 30; r.bottom = 490;
	CLVContainerView *containerView;
	list = new ColumnListView(r, &containerView, NULL, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM,
		B_WILL_DRAW|B_FRAME_EVENTS|B_NAVIGABLE, B_SINGLE_SELECTION_LIST, false, true, true, true,
		B_FANCY_BORDER);
	list->AddColumn(new CLVColumn("Symbol", 54, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->AddColumn(new CLVColumn("Nazwa", 100, CLV_TELL_ITEMS_WIDTH|CLV_HEADER_TRUNCATE|CLV_SORT_KEYABLE));
	list->SetSortFunction(CLVEasyItem::CompareItems);
	this->view->AddChild(containerView);
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));
	// buttons
	but_new = new BButton(BRect(30,0,140,24), "tt_but_new", "Nowy towar [F5]", new BMessage(BUT_NEW), B_FOLLOW_LEFT|B_FOLLOW_TOP);
	but_del = new BButton(BRect(30,510,140,534), "tt_but_del", "Usuń zaznaczone [F8]", new BMessage(BUT_DEL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_restore = new BButton(BRect(235,510,325,534), "tt_but_restore", "Przywróć [F6]", new BMessage(BUT_RESTORE), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_save = new BButton(BRect(580,510,670,534), "tt_but_save", "Zapisz", new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
	this->view->AddChild(but_new);
	this->view->AddChild(but_del);
	this->view->AddChild(but_restore);
	this->view->AddChild(but_save);
	but_new->ResizeToPreferred();
	but_del->ResizeToPreferred();
	but_restore->ResizeToPreferred();
	but_save->ResizeToPreferred();
	// box1
	box1 = new BBox(BRect(230,30,710,140), "tt_box1");
	box1->SetLabel("Dane towaru");
	this->view->AddChild(box1);
	// box1-stuff
	data[0] = new BTextControl(BRect(10,15,270,35), "ttd0", "Nazwa", NULL, new BMessage(DC));
	data[1] = new BTextControl(BRect(280,15,420,35), "ttd1", "Symbol", NULL, new BMessage(DC));
	data[2] = new BTextControl(BRect(10,50,150,65), "ttd2", "PKWiU", NULL, new BMessage(DC));
	data[3] = new BTextControl(BRect(160,50,270,65), "ttd3", "j.m.", NULL, new BMessage(DC));
	box1->AddChild(data[0]); box1->AddChild(data[1]);
	box1->AddChild(data[2]); box1->AddChild(data[3]);
	usluga = new BCheckBox(BRect(350,50,420,65), "ttdo", "Usługa", new BMessage(DC));
	box1->AddChild(usluga);
	dodany = new BStringView(BRect(280,80,340,95), "tts0", "Dodano:");
	dodany->SetAlignment(B_ALIGN_RIGHT);
	box1->AddChild(dodany);
	dodany = new BStringView(BRect(350,80,420,95), "ttsd", "");
	box1->AddChild(dodany);
	// box1-menu
	BMenu *menujm = new BMenu("");
	BMessage *msg;
	int j=0;
	while (jmiary[j] != NULL) {
		msg = new BMessage(MENUJM); msg->AddString("_jm",jmiary[j]);
		menujm->AddItem(new BMenuItem(jmiary[j], msg));
		j++;
	}
	BMenuField *menujmField = new BMenuField(BRect(280,50,340,65), "ttmf", NULL, menujm);
	box1->AddChild(menujmField);
	// box2
	box2 = new BBox(BRect(230,150,710,300), "tt_box2");
	box2->SetLabel("Ceny");
	this->view->AddChild(box2);
	// box2-stuff
	ceny[0] = new BTextControl(BRect(10,15,190,35), "ttc0", "Cena netto (zł)", NULL, new BMessage(DC));
	ceny[1] = new BTextControl(BRect(10,50,190,65), "ttc1", "Netto zakupu", NULL, new BMessage(DC));
	ceny[2] = new BTextControl(BRect(10,80,190,95), "ttc2", "Marża (%)", NULL, new BMessage(DC));
	ceny[3] = new BTextControl(BRect(200,50,310,65), "ttc3", "Rabat (%)", NULL, new BMessage(DC));
	ceny[4] = new BTextControl(BRect(10,110,190,125), "ttc4", "Kurs waluty", "1", new BMessage(DC));
	ceny[5] = new BTextControl(BRect(200,110,310,125), "ttc5", "Cło (%)", NULL, new BMessage(DC));
	box2->AddChild(ceny[0]);
	box2->AddChild(ceny[1]);
	box2->AddChild(ceny[2]);
	box2->AddChild(ceny[3]);
	box2->AddChild(ceny[4]);
	box2->AddChild(ceny[5]);
	but_sell = new BButton(BRect(340,50,420,65), "tt_but_sell", "Cena sprzedaży", new BMessage(BUT_SELL));
	but_marza = new BButton(BRect(340,80,420,95), "tt_but_marza", "Marża", new BMessage(BUT_MARZA));
	but_import = new BButton(BRect(340,110,420,125), "tt_but_import", "Import", new BMessage(BUT_IMPORT));
	box2->AddChild(but_sell);
	box2->AddChild(but_marza);
	box2->AddChild(but_import);
	but_sell->ResizeToPreferred();
	but_marza->ResizeToPreferred();
	but_import->ResizeToPreferred();
	brutto = new BStringView(BRect(330,15,390,35), "ttb0", "Cena brutto:");
	box2->AddChild(brutto);
	brutto = new BStringView(BRect(400,15,475,35), "ttbr", NULL);
	box2->AddChild(brutto);
	// box2-menu
	menuvat = new BPopUpMenu("[wybierz]");
	vatRows = 0; vatIds = NULL;
	RefreshVatSymbols();
	BMenuField *menuvatField = new BMenuField(BRect(200,15,330,35), "ttmv", "VAT", menuvat);
	menuvatField->SetDivider(be_plain_font->StringWidth(menuvatField->Label())+15);
	box2->AddChild(menuvatField);
	// box3
	box3 = new BBox(BRect(230,310,710,390), "tt_box3");
	box3->SetLabel("Magazyn");
	this->view->AddChild(box3);
	// box3-stuff
	magazyn = new BTextControl(BRect(10,30,250,40), "ttm0", "Stan:", NULL, new BMessage(DC));
	magzmiana = new BStringView(BRect(230,30,370,45), "ttm1", "Ostatnia zmiana:");
	magzmiana->SetAlignment(B_ALIGN_RIGHT);
	box3->AddChild(magzmiana);
	magzmiana = new BStringView(BRect(380,35,470,45), "ttm2", NULL);
	box3->AddChild(magzmiana);
	box3->AddChild(magazyn);
	// box4
	box4 = new BBox(BRect(230,400,710,480), "tt_box4");
	box4->SetLabel("Notatki");
	this->view->AddChild(box4);
	// box4-stuff
	r = box4->Bounds();
	r.InsetBy(10,15);
	BRect s = r; s.OffsetTo(0,0);
	notatki = new BTextView(r, "ttno", s, B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
	box4->AddChild(notatki);
	// fix widths
	int i;
	// first set them to be enough
	for (i=0;i<=3;i++) {
		data[i]->SetDivider(be_plain_font->StringWidth(data[i]->Label())+5);
	}
	for (i=0;i<=5;i++) {
		ceny[i]->SetDivider(be_plain_font->StringWidth(ceny[i]->Label())+5);
	}
	magazyn->SetDivider(be_plain_font->StringWidth(magazyn->Label())+5);
	// align in columns
	float d;
	d = MAX(data[0]->Divider(), data[2]->Divider());
	data[0]->SetDivider(d); data[2]->SetDivider(d);
	d = MAX(ceny[0]->Divider(), ceny[1]->Divider());
	d = MAX(ceny[2]->Divider(), d);
	d = MAX(ceny[4]->Divider(), d);
	ceny[0]->SetDivider(d); ceny[1]->SetDivider(d);
	ceny[2]->SetDivider(d); ceny[4]->SetDivider(d);
	//
	but_save->MakeDefault(true);
	updateTab();
	RefreshIndexList();
}

tabTowar::~tabTowar() {
	delete curdata;
	delete [] vatIds;
}

void tabTowar::curdataFromTab(void) {
	int i;
	for (i=0;i<=3;i++) {
		curdata->data[i] = data[i]->Text();
	}
	for (i=0;i<=5;i++) {
		curdata->ceny[i] = validateDecimal(ceny[i]->Text());
	}
	curdata->usluga = (usluga->Value() == B_CONTROL_ON);
	curdata->notatki = notatki->Text();
	curdata->magazyn = validateDecimal(magazyn->Text());
}

void tabTowar::curdataToTab(void) {
	int i;
	for (i=0;i<=3;i++) {
		data[i]->SetText(curdata->data[i].String());
	}
	for (i=0;i<=5;i++) {
		ceny[i]->SetText(curdata->ceny[i].String());
	}
	usluga->SetValue(curdata->usluga ? B_CONTROL_ON : B_CONTROL_OFF);
	notatki->SetText(curdata->notatki.String());
	dodany->SetText(curdata->dodany.String());
	magazyn->SetText(curdata->magazyn.String());
	magzmiana->SetText(curdata->magzmiana.String());
	updateTab();
}

void tabTowar::updateTab(void) {
	BMessage *msg = new BMessage(MSG_NAMECHANGE);
	msg->AddString("_newtitle", data[0]->Text());
	handler->Looper()->PostMessage(msg);
	int i;
	for (i=0;i<=5;i++) {
		ceny[i]->SetText(validateDecimal(ceny[i]->Text()));
	}
	for (i=0;i<vatRows;i++) {
		vatMenuItems[i]->SetMarked((vatIds[i] == curdata->vatid));
	}
	if (curdata->vatid < 0) {
		menuvat->Superitem()->SetLabel("[wybierz]");
		brutto->SetText("???");
		return;
	}
	// usluga?
	magazyn->SetEnabled(usluga->Value() == B_CONTROL_OFF);

	BString sql;
// brać tu pod uwagę rabat/marżę - NIE: robi to but_sell
	sql = "SELECT DECROUND(0"; sql += ceny[0]->Text();
	sql += "*(100+stawka)/100.0) FROM stawka_vat WHERE id = ";
	sql << curdata->vatid;
	brutto->SetText(execSQL(sql.String()));
}

// perform checks against supplied data
bool tabTowar::validateTab(void) {
	BAlert *error;
	BString sql, tmp;
	int i;
	// nazwa - niepusta
	if (strlen(data[0]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano nazwy towaru!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		data[0]->MakeFocus();
		return false;
	}
	// nazwa - unikalna
	tmp = data[0]->Text(); tmp.ReplaceAll("'","''");	// sql quote
	sql = "SELECT id FROM towar WHERE nazwa = '"; sql += tmp; sql += "'";
	i = toint(execSQL(sql.String()));
	if (((curdata->id < 0) && ( i!= 0 )) || ((curdata->id > 0) && (i != 0) && (i != curdata->id))) {
		error = new BAlert(APP_NAME, "Nazwa towaru nie jest unikalna!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		data[0]->MakeFocus();
		return false;
	}
	// symbol - niepusty
	if (strlen(data[1]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano symbolu towaru!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		data[1]->MakeFocus();
		return false;
	}
	// symbol - unikalny
	tmp = data[1]->Text(); tmp.ReplaceAll("'","''");	// sql quote
	sql = "SELECT id FROM towar WHERE symbol = '"; sql += tmp; sql += "'";
	i = toint(execSQL(sql.String()));
	if (((curdata->id < 0) && ( i!= 0 )) || ((curdata->id > 0) && (i != 0) && (i != curdata->id))) {
		error = new BAlert(APP_NAME, "Symbol towaru nie jest unikalny!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		data[1]->MakeFocus();
		return false;
	}
	// pkwiu - ostrzeżenie że pusty
	if (strlen(data[2]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wpisano kodu PKWiU towaru.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			data[2]->MakeFocus();
			return false;
		}
	}
	// jm - ostrzeżenie że pusty
	if (strlen(data[3]->Text()) == 0) {
		error = new BAlert(APP_NAME, "Nie wybrano jednostki miary.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			data[3]->MakeFocus();
			return false;
		}
	}
	// stawka vat
	if (curdata->vatid < 0) {
		error = new BAlert(APP_NAME, "Nie wybrano stawki VAT!", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		error->Go();
		return false;
	}
	// cena niezerowa
	sql = "SELECT 100*0"; sql += ceny[0]->Text();
	i = toint(execSQL(sql.String()));
	if (i == 0) {
		error = new BAlert(APP_NAME, "Cena towaru jest równa zero.\nKontynuować?", "Tak", "Nie", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (error->Go() == 1) {
			ceny[0]->MakeFocus();
			return false;
		}
	}
	return true;
}

void tabTowar::MessageReceived(BMessage *Message) {
	int32 item;
	const char *tmp;
	BString result,sql;

	switch (Message->what) {
		case DC:
			this->dirty = true;
			updateTab();
			break;
		case B_F5_KEY:
		case BUT_NEW:
			if (CommitCurdata()) {
				list->DeselectAll();
				// clear curdata
				curdata->clear();
				// refresh tabs
				curdataToTab();
				data[0]->MakeFocus();
			}
			break;
		case B_F6_KEY:
		case BUT_RESTORE:
			DoFetchCurdata();
			break;
		case B_F8_KEY:
		case BUT_DEL:
			DoDeleteCurdata();
			break;
		case BUT_SAVE:
			curdataFromTab();
			DoCommitCurdata();
			curdataToTab();
			break;
		case BUT_SELL:
//			printf("calc sell\n");
			sql = "SELECT DECROUND(0";
			sql += validateDecimal(ceny[1]->Text());
			sql += "*(1.0+";
			sql += validateDecimal(ceny[2]->Text());
			sql += "/100.0)";
			sql += "*(1.0-";
			sql += validateDecimal(ceny[3]->Text());
			sql += "/100.0))";
			result = execSQL(sql.String());
//			printf("sql:[%s]\nres[%s]\n",sql.String(),result.String());
			ceny[0]->SetText(result.String());
			updateTab();
			break;
		case BUT_MARZA:
//			printf("calc marza\n");
			sql = "SELECT (0+";
			sql += validateDecimal(ceny[1]->Text());
			sql += ")>0";
			if (toint(execSQL(sql.String()))) {
				sql = "SELECT DECROUND((0";
				sql += validateDecimal(ceny[0]->Text());
				sql += "/(0";
				sql += validateDecimal(ceny[1]->Text());
				sql += "*(1.0-";
				sql += validateDecimal(ceny[3]->Text());
				sql += "/100.0)*";
				sql += validateDecimal(ceny[4]->Text());
				sql += "*(1+";
				sql += validateDecimal(ceny[5]->Text());
				sql += "/100.0))-1)*100)";
				result = execSQL(sql.String());
//				printf("sql:[%s]\nres[%s]\n",sql.String(),result.String());
				ceny[2]->SetText(result.String());
				updateTab();
			}
			break;
		case BUT_IMPORT:
//			printf("calc import\n");
			sql = "SELECT DECROUND(0";
			sql += validateDecimal(ceny[1]->Text());
			sql += "*(1.0+";
			sql += validateDecimal(ceny[2]->Text());
			sql += "/100.0)*";
			sql += validateDecimal(ceny[4]->Text());
			sql += "*(1.0+";
			sql += validateDecimal(ceny[5]->Text());
			sql += "/100.0)*(1.0-";
			sql += validateDecimal(ceny[3]->Text());
			sql += "/100.0))";
			result = execSQL(sql.String());
//			printf("sql:[%s]\nres[%s]\n",sql.String(),result.String());
			ceny[0]->SetText(result.String());
			updateTab();
			break;
		case LIST_SEL:
		case LIST_INV:
			{	int i = list->CurrentSelection(0);
				if (i>=0) {
					i = ((tab2ListItem*)list->ItemAt(list->CurrentSelection(0)))->Id();
					if (i>=0)
						ChangedSelection(i);
				} else {
					// deselection, what to do?				
				}
				break;
			}
		case MENUJM:
			if (Message->FindString("_jm", &tmp) == B_OK) {
				data[3]->SetText(tmp);
				this->dirty = true;
			}
			break;
		case MENUVAT:
			this->dirty = true;
			if (Message->FindInt32("_vatid", &item) == B_OK) {
				curdata->vatid = item;
			}
			updateTab();
			break;
		case MSG_REQVATUP:
			RefreshVatSymbols();
			break;
		case MSG_REQTOWARLIST:
			{
				RefreshIndexList();
				BMessage *msg = new BMessage(MSG_REQTOWARUP);
				handler->Looper()->PostMessage(msg);
				break;
			}
		default:
			break;
	}
}

void tabTowar::ChangedSelection(int newid) {
	if (!(CommitCurdata())) {
		// XXX do nothing if cancel, restore old selection?
		return;
	}
	// fetch and store into new data
	curdata->id = newid;
	DoFetchCurdata();
}

void tabTowar::DoCommitCurdata(void) {
	if (!(validateTab()))
		return;
	curdata->commit();
	this->dirty = false;
	BMessage *msg = new BMessage(MSG_REQTOWARUP);
	handler->Looper()->PostMessage(msg);
	RefreshIndexList();
}

void tabTowar::DoDeleteCurdata(void) {
// XXX ask for confimation?
	curdata->del();
	curdataToTab();
	BMessage *msg = new BMessage(MSG_REQTOWARUP);
	handler->Looper()->PostMessage(msg);
	RefreshIndexList();
}

void tabTowar::DoFetchCurdata(void) {
	if (curdata->id >=0) {
		curdata->fetch();
		this->dirty = false;
		curdataToTab();
	}
}

void tabTowar::RefreshIndexList(void) {
	// clear current list
	if (list->CountItems()>0) {
		tab2ListItem *anItem;
		for (int i=0; (anItem=(tab2ListItem*)list->ItemAt(i)); i++)
			delete anItem;
		if (!list->IsEmpty())
			list->MakeEmpty();
	}
	// select list from db
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, "SELECT id, symbol, nazwa FROM towar ORDER BY id", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// no entries
	} else {
		for (int i=1;i<=nRows;i++)
			list->AddItem(new tab2ListItem(toint(result[i*nCols+0]), result[i*nCols+1], result[i*nCols+2]));
	}
	sqlite3_free_table(result);
}

void tabTowar::RefreshVatSymbols(void) {
	int i = vatRows;
	while (i>=0) {
		delete menuvat->RemoveItem(i--);
	}

	delete [] vatIds;

	int nRows, nCols;
	char **result;
	BMessage *msg;

	sqlite3_get_table(dbData, "SELECT id, nazwa FROM stawka_vat WHERE aktywne = 1 ORDER BY id", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows < 1) {
		// XXX Panic! empty vat table
	} else {
		vatMenuItems = new BMenuItem*[nRows];
		vatIds = new int[nRows];
		vatRows = nRows;
		for (int i=1;i<=nRows;i++) {
			msg = new BMessage(MENUVAT);
			msg->AddInt32("_vatid", toint(result[i*nCols+0]));
			vatIds[i-1] = toint(result[i*nCols+0]);
			vatMenuItems[i-1] = new BMenuItem(result[i*nCols+1], msg);
			menuvat->AddItem(vatMenuItems[i-1]);
		}
	}
	sqlite3_free_table(result);
}
