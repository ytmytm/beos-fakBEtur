
#include <Box.h>
#include <Button.h>
#include <String.h>
#include <TextControl.h>
#include <View.h>

#include "globals.h"
#include "dialfirma.h"
#include <stdio.h>

const uint32 DC			= 'FIDC';
const uint32 BUT_OK		= 'FIOK';
const uint32 BUT_CANCEL	= 'FICA';

dialFirma::dialFirma(const char *title, sqlite3 *db, bool cancancel) : BWindow(
	BRect(120, 120, 120+460, 120+330+30+40),
	"Informacje o Twojej firmie",
	B_TITLED_WINDOW,
	B_NOT_RESIZABLE ) {

	dbData = db;

	this->SetFeel(B_FLOATING_APP_WINDOW_FEEL);

	view = new BView(this->Bounds(), "firmaView", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	this->AddChild(view);
	// powtorz boxy z tabfirma
	// box1
	box1 = new BBox(BRect(10,20,450,170), "firma_box1");
	box1->SetLabel("Dane adresowe");
	view->AddChild(box1);
	// box2
	box2 = new BBox(BRect(10,180,450,330), "firma_box2");
	box2->SetLabel("Dane firmy");
	view->AddChild(box2);
	// XXX this is ripped from tabfirma!
	// box1-stuff
	data[0] = new BTextControl(BRect(10,15,270,35), "tfd0", "Nazwa", NULL, new BMessage(DC));
//	data[1] = new BTextControl(BRect(280,15,420,35), "tfd1", "Symbol", NULL, new BMessage(DC));
	data[2] = new BTextControl(BRect(10,50,420,65), "tfd2", "Adres", NULL, new BMessage(DC));
	data[3] = new BTextControl(BRect(10,80,150,95), "tfd3", "Kod", NULL, new BMessage(DC));
	data[4] = new BTextControl(BRect(160,80,420,95), "tfd4", "Miejscowość", NULL, new BMessage(DC));
	data[5] = new BTextControl(BRect(10,110,200,125), "tfd5", "Tel.", NULL, new BMessage(DC));
	data[6] = new BTextControl(BRect(210,110,420,125), "tfd6", "Email", NULL, new BMessage(DC));
	box1->AddChild(data[0]);
	//box1->AddChild(data[1]);
	box1->AddChild(data[2]);
	box1->AddChild(data[3]); box1->AddChild(data[4]);
	box1->AddChild(data[5]); box1->AddChild(data[6]);
	BRect r;
	r.left = 10; r.top = 20; r.right = 420; r.bottom = 35;
	data[7] = new BTextControl(r, "tfd7", "NIP", NULL, new BMessage(DC)); r.OffsetBy(0, 30);
	data[8] = new BTextControl(r, "tfd8", "REGON", NULL, new BMessage(DC)); r.OffsetBy(0, 30);
	data[9] = new BTextControl(r, "tfd9", "Bank", NULL, new BMessage(DC)); r.OffsetBy(0, 30);
	data[10] = new BTextControl(r, "tfd10", "Nr konta", NULL, new BMessage(DC));
	box2->AddChild(data[7]); box2->AddChild(data[8]);
	box2->AddChild(data[9]); box2->AddChild(data[10]);
	// fix widths
	// first set them to be enough
	for (int i=0;i<=10;i++) {
		if (i!=1)
			data[i]->SetDivider(be_plain_font->StringWidth(data[i]->Label())+5);
	}
	// align in columns
	float d;
	d = MAX(data[0]->Divider(), data[2]->Divider());
	d = MAX(data[3]->Divider(), d);
	d = MAX(data[5]->Divider(), d);
	data[0]->SetDivider(d); data[2]->SetDivider(d);
	data[3]->SetDivider(d); data[5]->SetDivider(d);
	d = MAX(data[7]->Divider(), data[8]->Divider());
	d = MAX(data[9]->Divider(), d);
	d = MAX(data[10]->Divider(), d);
	data[7]->SetDivider(d); data[8]->SetDivider(d);
	data[9]->SetDivider(d); data[10]->SetDivider(d);
	// XXX end of ripped stuff
	wystawil = new BTextControl(BRect(10,335,300,350), "tfWystawia", "Wystawiający dokumenty", NULL, new BMessage(DC));
	wystawil->SetDivider(be_plain_font->StringWidth(wystawil->Label())+5);
	view->AddChild(wystawil);
	// buttons - OK, CANCEL
	but_ok = new BButton(BRect(370,360,420,380), "firma_butok", "OK", new BMessage(BUT_OK));
	if (cancancel) {
		but_cancel = new BButton(BRect(10,360,60,380), "firma_butcancel", "Anuluj", new BMessage(BUT_CANCEL));
		view->AddChild(but_cancel);
		but_cancel->ResizeToPreferred();
	}
	view->AddChild(but_ok);
	but_ok->ResizeToPreferred();
	but_ok->MakeDefault(true);
	// fetch i wypelnij boxy danymi
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;
	sql = "SELECT ";
	sql += "nazwa, adres, kod, miejscowosc, telefon, email";
	sql += ", nip, regon, bank, konto, f_wystawil";
	sql += " FROM konfiguracja WHERE zrobiona = 1";
//printf("sql:%s\n",sql.String());
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i\n", nRows, nCols);
	if (nRows < 1) {
		// WILL NEVER HAPPEN!
//		printf("initial\n");
	} else {
//		printf("not initial\n");
		// readout data
		i = nCols;
		data[0]->SetText(result[i++]);
		for (j=2;j<=10;j++) {
			data[j]->SetText(result[i++]);
		}
		wystawil->SetText(result[i++]);
	}
	Show();
}

void dialFirma::commit(void) {
	BString sql;
	int ret;
//printf("commit");
	sql = "UPDATE konfiguracja SET ";
	sql += "nazwa = %Q, adres = %Q, kod = %Q, miejscowosc = %Q, telefon = %Q, email = %Q";
	sql += ", nip = %Q, regon = %Q, bank = %Q, konto = %Q, f_wystawil = %Q";
	sql += " WHERE zrobiona = 1";
//printf("sql:[%s]\n",sql.String());
	char *query = sqlite3_mprintf(sql.String(),
		data[0]->Text(), data[2]->Text(), data[3]->Text(),
		data[4]->Text(), data[5]->Text(), data[6]->Text(), data[7]->Text(),
		data[8]->Text(), data[9]->Text(), data[10]->Text(),
		wystawil->Text());
	ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
	sqlite3_free(query);
//printf("result: %i, %s;\n", ret, dbErrMsg);
}

void dialFirma::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case BUT_OK:
//			printf("but_ok!\n");
			commit();
			Quit();
			break;
		case BUT_CANCEL:
//			printf("but_cancel!\n");
			Quit();
			break;
		default:
			BWindow::MessageReceived(Message);
			break;
	}
}
