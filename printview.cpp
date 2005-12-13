//
// TODO:
//	- info w dokumentacji jak zrobić embedding fontów w pdfwriterze
//	- ustalić listę kilku fontów, które mają polskie literki - próbować po
//		kolei, jak sprawdzać w BFont co zostało ustalone?
//	- 'ż'(ółw) - niedrukowalne, duże też, zastąpić
//
// IDEAS:
//

#include "globals.h"
#include "printview.h"

#include <Font.h>
#include <Message.h>
#include <PrintJob.h>
#include <View.h>
#include <Window.h>
#include <stdio.h>

printView::printView(int id, sqlite *db, BMessage *pSettings) : beFakPrint(id,db),
	 BView(BRect(0,0,100,100), "printView", B_FOLLOW_ALL, B_WILL_DRAW) {
	status_t result = B_OK;
printf("printjob for [%s]\n", fdata->nazwa.String());
	printJob = new BPrintJob(fdata->nazwa.String());
	printJob->SetSettings(new BMessage(*pSettings));
// dla przyspieszenia preview, potem wlaczyc!!!
//XXX	result = printJob->ConfigJob();
	// XXX return, ale jak błąd zgłosić? geterror lub sth?
	if (result != B_OK)
		return;
	if ((printJob->LastPage() - printJob->FirstPage() + 1) <= 0)
		return;
}

void printView::Go(void) {
	printf("go...\n");
	// information from printJob
	BRect pageRect = printJob->PrintableRect();	
//	int32 firstPage = printJob->FirstPage();
//	int32 lastPage = printJob->LastPage();
//	int32 pageCount = printJob.LastPage() - printJob.FirstPage() + 1
//	printf("rect:[%f,%f,%f,%f]\n",pageRect.left,pageRect.top,pageRect.right,pageRect.bottom);
//	printf("page1:%i,lastp:%i\n",firstPage,lastPage);
	// calculate # of own pages (according to rect), clip lastpage against pages in document

	// dla debugu:
	// - przygotowac okno
	// - w oknie view do rysowania, tam w Draw() wszystko wyrysowac
	// - wyswietlic
	BRect r = pageRect;
	r.OffsetBy(20,20);
	pWindow = new BWindow(r, "Podgląd wydruku", B_TITLED_WINDOW, 0);
	pWindow->AddChild(this);
	MoveTo(pageRect.LeftTop());
	ResizeTo(pageRect.Width(),pageRect.Height());
	pWindow->Show();
	return;	// XXX removeme!
	printJob->BeginJob();
	// for all pages...
	printJob->DrawView(this,BRect(pageRect),BPoint(0.0,0.0));	// cala strona, od (0,0)
	printJob->SpoolPage();
	printJob->CommitJob();
}

printView::~printView() {
	if (pWindow->Lock()) {
		RemoveSelf();
		pWindow->Quit();
	}
}

#define LEFT	(pageRect.left+10)
#define TOP		(pageRect.top+font.Size()+10)
#define ELINE	(cur.y+font.Size()+5)
#define ELINEB	(cur.y+fontb.Size()+5)

void printView::Draw(BRect pageRect) {
	printf("rect:[%f,%f,%f,%f]\n",pageRect.left,pageRect.top,pageRect.right,pageRect.bottom);

	// fonty: font - do napisów zywkłych, fontb - tytuły
	BFont font(be_plain_font);
	font.SetFamilyAndStyle("Verdana","Regular");
	font.SetFlags(B_DISABLE_ANTIALIASING);
	BFont fontb(be_bold_font);
	fontb.SetFamilyAndStyle("Arial","Bold");
	fontb.SetFlags(B_DISABLE_ANTIALIASING);
	font.SetSize(10.0);
	fontb.SetSize(10.0);
	// line1 - nazwa sprzedawcy, miejsce wyst,datawyst
	// header
	BString tmp;
	BPoint cur;
	// miejsce, data
	SetFont(&font);
	tmp = fdata->ogol[0]; tmp += ", "; tmp += fdata->ogol[2];
	cur = PenLocation();
	MovePenTo(pageRect.right-font.StringWidth(tmp.String())-10, TOP);
	DrawStr(tmp);
	// nazwasprzedawcy
	SetFont(&fontb);
	tmp = own[0];
	cur = PenLocation(); MovePenTo(LEFT, TOP);
	DrawStr(tmp);
	// dane sprzedawcy: kod miejsce, adres
	SetFont(&font);
	tmp = own[3]; tmp += " "; tmp += own[4]; tmp += ", "; tmp += own[2];
	cur = PenLocation(); MovePenTo(LEFT, ELINE);
	DrawStr(tmp);
	// telefon, email
	tmp = "tel. "; tmp += own[5]; tmp += ", "; tmp += own[6];
	cur = PenLocation(); MovePenTo(LEFT, ELINE);
	DrawStr(tmp);
	// bank, konto
	tmp = own[9]; tmp += " "; tmp += own[10];
	cur = PenLocation(); MovePenTo(LEFT, ELINE);
	DrawStr(tmp);
	// regon, nip
	tmp = "";
	if (own[8].Length()>0) { tmp += "REGON: "; tmp += own[8].String(); }
	if (own[7].Length()>0) { if (tmp.Length()>0) tmp += ", ";
		tmp += "NIP: ", tmp += own[7];
	}
	if (tmp.Length()>0) {
		cur = PenLocation(); MovePenTo(LEFT,ELINE); DrawStr(tmp);
	}
	// tytuł dokumentu
	fontb.SetSize(18.0);
	SetFont(&fontb);
	cur = PenLocation();
	tmp = "Faktura VAT nr "; tmp += fdata->nazwa;
	cur.x = pageRect.left+(pageRect.Width()-fontb.StringWidth(tmp.String()))/2;
	cur.y += fontb.Size()+fontb.Size()+10;
	MovePenTo(cur);
	DrawStr(tmp);
	// rodzaj dokumentu
	cur = PenLocation();
	tmp = typfaktury;
	cur.y += fontb.Size()+5;
	fontb.SetSize(16.0);
	SetFont(&fontb);
	cur.x = pageRect.left+(pageRect.Width()-fontb.StringWidth(tmp.String()))/2;
	MovePenTo(cur);
	DrawStr(tmp);
	// nabywca...
	tmp = "Nabywca: ";
	cur = PenLocation();
	cur.y += fontb.Size() + fontb.Size();
	font.SetSize(10.0);
	SetFont(&font);
	cur.x = pageRect.left+(pageRect.Width()/3)-font.StringWidth(tmp.String());
	MovePenTo(cur);
	DrawStr(tmp);
	fontb.SetSize(10.0);
	SetFont(&fontb);
	DrawStr(fdata->odata[0]);
	SetFont(&font);
	tmp = "Adres: ";
	cur = PenLocation();
	cur.x = pageRect.left+(pageRect.Width()/3)-font.StringWidth(tmp.String());
	cur.y = ELINE;
	MovePenTo(cur);
	DrawStr(tmp);
	cur = PenLocation();
	tmp = fdata->odata[2]; tmp += ", "; tmp += fdata->odata[3]; tmp += " "; tmp += fdata->odata[4];
	DrawStr(tmp);
	cur.x = pageRect.left+(pageRect.Width()/3);
	cur.y = ELINE;
	tmp = "tel. "; tmp += fdata->odata[5]; tmp += ", "; tmp += fdata->odata[6];
	MovePenTo(cur);
	DrawStr(tmp);
	cur.y = ELINE;
	tmp = "";
	if (fdata->odata[8].Length()>0) { tmp += "REGON: "; tmp += fdata->odata[8]; }
	if (fdata->odata[7].Length()>0) { if (tmp.Length()>0) tmp += ", ";
		tmp += "NIP: "; tmp += fdata->odata[7];
	}
	if (tmp.Length()>0) {
		MovePenTo(cur);
		DrawStr(tmp);
	}
	// sposob zaplaty, data zaplaty, termin zaplaty, srodek transportu
	cur = PenLocation();
	cur.y = ELINE; cur.y = ELINE;
	tmp = "Sposób zapłaty: ";
	cur.x = pageRect.left+(pageRect.Width()/4)-font.StringWidth(tmp.String());
	MovePenTo(cur);
	DrawStr(tmp);
	tmp = fdata->ogol[5];
	DrawStr(tmp);
	tmp = "Termin zapłaty: ";
	cur.x = pageRect.left+(3*pageRect.Width()/4)-font.StringWidth(tmp.String());
	MovePenTo(cur);
	DrawStr(tmp);
	SetFont(&fontb);
	tmp = fdata->ogol[6];
	DrawStr(tmp);
	SetFont(&font);
	cur.y = ELINE;
	tmp = "Data sprzedazy: ";	/// 'ż'!
	cur.x = pageRect.left+(pageRect.Width()/4)-font.StringWidth(tmp.String());
	MovePenTo(cur);
	DrawStr(tmp);
	tmp = fdata->ogol[3];
	DrawStr(tmp);
	if (fdata->ogol[4].Length()>0) {
		tmp = "Środek transportu: ";
		cur.x = pageRect.left+(3*pageRect.Width()/4)-font.StringWidth(tmp.String());
		MovePenTo(cur);
		DrawStr(tmp);
		tmp = fdata->ogol[4];
		DrawStr(tmp);
	}
	// tabela header
	// tabela
	// tabela podsumowanie
	// do zaplaty
	// do zaplaty slownie
	// wystawil [wystawil] odebral
	// --------            -------
	font.SetSize(10.0);
	SetFont(&font);
	tmp = "wystawił: ";
	cur.x = pageRect.left + 1*(pageRect.Width()/8) - font.StringWidth(tmp.String());
	cur.y = pageRect.bottom - 5 * font.Size();
	MovePenTo(cur);
	DrawStr(tmp);
	cur = PenLocation();
	tmp = fdata->ogol[1];
	DrawStr(tmp);
	cur = PenLocation();
	cur.x = pageRect.left + 1*(pageRect.Width()/8);
	StrokeLine(BPoint(cur.x,cur.y+5),BPoint(cur.x+pageRect.Width()/4, cur.y+5));
	tmp = "odebrał: ";
	cur.x = pageRect.left + 5*(pageRect.Width()/8) - font.StringWidth(tmp.String());
	cur.y = pageRect.bottom - 5 * font.Size();
	MovePenTo(cur);
	DrawStr(tmp);
	cur = PenLocation();
	cur.x = pageRect.left + 5*(pageRect.Width()/8);
	StrokeLine(BPoint(cur.x,cur.y+5),BPoint(cur.x+pageRect.Width()/4, cur.y+5));
}

void printView::DrawStr(const BString str) {
	DrawString(str.String());
}
