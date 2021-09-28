//
// - niedrukowalne 'ż'/'Ż'(ółw) zastępowane z/Z
//
// TODO:
//	- info w dokumentacji jak zrobić embedding fontów w pdfwriterze
//	- ustalić listę kilku fontów, które mają polskie literki - próbować po
//		kolei, jak sprawdzać w BFont co zostało ustalone?
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

const char *tabhline1[] = { "Lp.", "Nazwa towaru/usługi", "PKWiU", "Ilość", "J.m.", "Rabat", "Cena", "Wartość", "VAT", "Wartość", "Wartość", NULL };
const char *tabhline2[] = { "",    "",                    "",      "",      "",     "(%)",   "z rabatem", "netto", "", "VAT", "brutto", NULL };
const char *tabhline3[] = { "88",  "MMMMMMMMMMMMMMMMMM", "MM.MM.MM.MM", "88888.88", "MMMMM", "88.88", "88888.88", "888888.88", "88.88%", "888888.88", "888888.88", NULL };

printView::printView(int id, sqlite3 *db, int numkopii, BMessage *pSettings) : beFakPrint(id,db,numkopii),
	 BView(BRect(0,0,100,100), "printView", B_FOLLOW_ALL, B_WILL_DRAW) {
	status_t result = B_OK;
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
//	pWindow->Show();
//	return;	// XXX removeme!
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

#define ALIGN_MARGIN	2
#define LEFT	(pageRect.left+10)
#define TOP		(pageRect.top+font.Size()+10)
#define ELINE	(cur.y+font.Size()+5)
#define ELINEB	(cur.y+fontb.Size()+5)

float tabl[13];

void printView::Draw(BRect pageRect) {
	BString tmp;
	BPoint cur;
	int i, j;
	// fonty: font - do napisów zywkłych, fontb - tytuły
	BFont font(be_plain_font);
	font.SetFamilyAndStyle("Arial","Regular");
	font.SetFlags(B_DISABLE_ANTIALIASING);
	BFont fontb(be_bold_font);
	fontb.SetFamilyAndStyle("Arial","Bold");
	fontb.SetFlags(B_DISABLE_ANTIALIASING);
	font.SetSize(10.0);
	fontb.SetSize(10.0);
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
	tmp = "Data sprzedaży: ";
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
	cur.y = ELINE;
	// tabela header
	// wygenerować szerokości
	tabl[0] = 0;
	float d;
	for (i=0;i<=10;i++) {
		d = MAX(font.StringWidth(tabhline1[i]), font.StringWidth(tabhline2[i]));
		d = MAX(font.StringWidth(tabhline3[i]), d);
		tabl[i+1] = tabl[i]+d;
	}
	float off = pageRect.Width()-tabl[11];
	if (off>0)
		for (i=0;i<=11;i++)
			tabl[i] += off/2;
//	XXX don't know how to handle it yet...
//	else
//		for (i=2;i<=11;i++)
//			tabl[i] += off;
	// finally offset everything to left margin
	for (i=0;i<=11;i++)
		tabl[i] += pageRect.left;
	// paint headers...
	MovePenTo(cur);
	fontb.SetSize(7.0);
	SetFont(&fontb);
	cur = PenLocation();
	cur.x = tabl[0];
	cur.y = ELINE;
	float tabhl0 = cur.y, tabhl1 = cur.y+fontb.Size()+1, tabhy1 = cur.y+fontb.Size()+4, tabhy0 = cur.y-(fontb.Size()+4);

	for (i=0;i<=10;i++) {
		cur.y = tabhl0;
		MovePenTo(cur);
		tmp = tabhline1[i];
		fontb.TruncateString(&tmp, B_TRUNCATE_END, tabl[i+1]-tabl[i]);
		DrawStrCenter(tmp, tabl[i], tabl[i+1]);
		cur.y = tabhl1;
		MovePenTo(cur);
		tmp = tabhline2[i];
		fontb.TruncateString(&tmp, B_TRUNCATE_END, tabl[i+1]-tabl[i]);
		DrawStrCenter(tmp, tabl[i], tabl[i+1]);
	}
	for (i=0;i<=10;i++)
		StrokeRect(BRect(tabl[i],tabhy0,tabl[i+1],tabhy1));

	// tabela...
	font.SetSize(8.0);
	SetFont(&font);
	cur.x = tabl[0]; cur.y = tabhl1; cur.y = ELINE; MovePenTo(cur);
	// iteruj po towarach
	pozfakitem *item = flist->start;
	while (item!=NULL) {
		tmp = ""; tmp << item->lp; font.TruncateString(&tmp, B_TRUNCATE_END, tabl[1]-tabl[0]-ALIGN_MARGIN);
		// lp
		DrawStrLeft(tmp,tabl[0]);
		tmp = item->data->data[1]; font.TruncateString(&tmp, B_TRUNCATE_END, tabl[2]-tabl[1]-ALIGN_MARGIN);
		DrawStrLeft(tmp, tabl[1]);
		for (i=2;i<=10;i++) {
			tmp = item->data->data[i];
			font.TruncateString(&tmp, B_TRUNCATE_END, tabl[i+1]-tabl[i]-ALIGN_MARGIN);
			DrawStrRight(tmp, tabl[i+1]);
		}
		updateSummary(item->data->data[7].String(), item->data->vatid, item->data->data[9].String(), item->data->data[10].String());
		item = item->nxt;
 		cur.y = ELINE; MovePenTo(cur);
	}
	cur = PenLocation();
	float tabsumay0 = cur.y - font.Size() - 2;
	StrokeRect(BRect(tabl[0],tabhy1,tabl[11],tabsumay0));
	// podsumuj
	makeSummary();
	// tabela podsumowanie
	for (i=0;i<fsummarows;i++) {
		for (j=0;j<=3;j++) {
			tmp = fsumma[i].summa[j]; font.TruncateString(&tmp, B_TRUNCATE_END, tabl[8+j]-tabl[7+j]-ALIGN_MARGIN);
			DrawStrRight(tmp, tabl[8+j]);
		}
		cur.y = ELINE; MovePenTo(cur);
	}
	StrokeLine(BPoint(tabl[7],cur.y-font.Size()-2),BPoint(tabl[11],cur.y-font.Size()-2));
	MovePenTo(cur);
	// razem
	for (j=0;j<=3;j++) {
		tmp = razem.summa[j]; font.TruncateString(&tmp, B_TRUNCATE_END, tabl[8+j]-tabl[7+j]-ALIGN_MARGIN);
		DrawStrRight(tmp, tabl[8+j]);
	}
	// RAZEM (slowo)
	tmp = "RAZEM:";
	DrawStrRight(tmp, tabl[7]);
	float tabsumay1 = cur.y+2;
	StrokeRect(BRect(tabl[7], tabsumay0, tabl[11], tabsumay1));
	// ramki
	BeginLineArray(15);
	for (i=1;i<=10;i++)
		AddLine(BPoint(tabl[i],tabhy1), BPoint(tabl[i],tabsumay0), HighColor());
	for (i=7;i<=10;i++)
		AddLine(BPoint(tabl[i],tabsumay0), BPoint(tabl[i],tabsumay1), HighColor());
	EndLineArray();
	// do zaplaty
	cur.x = tabl[1]+(tabl[2]-tabl[1])/2;
	MovePenTo(cur);
	font.SetSize(10.0);
	tmp = "Do zapłaty zł: ";
	DrawStrRight(tmp, cur.x);
	tmp = razem.summa[3];
	DrawStr(tmp);
	// do zaplaty slownie
	cur.y = ELINE;
	MovePenTo(cur);
	tmp = "Słownie: ";
	DrawStrRight(tmp, cur.x);
	tmp = slownie(razem.summa[3].String());
	DrawStr(tmp);
	// wystawil [wystawil] odebral
	// --------            -------
	font.SetSize(10.0);
	SetFont(&font);
	tmp = "wystawił: ";
	cur.x = pageRect.left + 1*(pageRect.Width()/8) - font.StringWidth(tmp.String());
	cur.y = pageRect.bottom - 7 * font.Size();
	MovePenTo(cur);
	DrawStr(tmp);
	cur = PenLocation();
	tmp = fdata->ogol[1];
	font.SetSize(12.0);
	SetFont(&font);
	DrawStr(tmp);

	cur = PenLocation();
	cur.x = pageRect.left + 1*(pageRect.Width()/8);
	StrokeLine(BPoint(cur.x,cur.y+5),BPoint(cur.x+pageRect.Width()/4, cur.y+5));
	cur.y = ELINE;
	MovePenTo(cur);
	font.SetSize(5.0);
	SetFont(&font);
	tmp = "podpis osoby upow.";
	DrawStrCenter(tmp, cur.x, cur.x+pageRect.Width()/4);
	
	font.SetSize(10.0);
	SetFont(&font);
	tmp = "odebrał: ";
	cur.x = pageRect.left + 5*(pageRect.Width()/8) - font.StringWidth(tmp.String());
	cur.y = pageRect.bottom - 7 * font.Size();
	MovePenTo(cur);
	DrawStr(tmp);
	cur = PenLocation();
	cur.x = pageRect.left + 5*(pageRect.Width()/8);
	StrokeLine(BPoint(cur.x,cur.y+5),BPoint(cur.x+pageRect.Width()/4, cur.y+5));
	cur.y = ELINE;
	MovePenTo(cur);
	font.SetSize(5.0);
	SetFont(&font);
	tmp = "podpis osoby upow.";
	DrawStrCenter(tmp, cur.x, cur.x+pageRect.Width()/4);
}

void printView::DrawStr(const BString str) {
	BString tmp = str;
	tmp.ReplaceAll("ż","z");
	tmp.ReplaceAll("Ż","Z");
	DrawString(tmp.String());
}

void printView::DrawStrCenter(const BString str, float l, float r) {
	BPoint cur = PenLocation();
	cur.x = l+(r-l)/2-StringWidth(str.String())/2;
	MovePenTo(cur);
	DrawStr(str);
}

void printView::DrawStrLeft(const BString str, float l) {
	BPoint cur = PenLocation();
	cur.x = l+ALIGN_MARGIN;
	MovePenTo(cur);
	DrawStr(str);
}

void printView::DrawStrRight(const BString str, float r) {
	BPoint cur = PenLocation();
	cur.x = r-StringWidth(str.String())-ALIGN_MARGIN;
	MovePenTo(cur);
	DrawStr(str);
}
