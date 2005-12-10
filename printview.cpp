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

printView::printView(int id, sqlite *db, BMessage *pSettings) : beFakPrint(id,db) {
	status_t result;
printf("printjob for [%s]\n", fdata->nazwa.String());
	printJob = new BPrintJob(fdata->nazwa.String());
	printJob->SetSettings(new BMessage(*pSettings));
	result = printJob->ConfigJob();
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
	BWindow *pWindow = new BWindow(pageRect, "Podgląd wydruku", B_TITLED_WINDOW, 0);
	printViewView *pView = new printViewView(pageRect, "pView");
	pWindow->AddChild(pView);
	pWindow->Show();
//	return;
	printJob->BeginJob();
	// for all pages...
	printJob->DrawView(pView,BRect(pageRect),BPoint(0.0,0.0));	// cala strona, od (0,0)
	printJob->SpoolPage();
	printJob->CommitJob();
//	if (pWindow->Lock())
//		pWindow->Quit();
}

printViewView::printViewView(BRect frame, const char *name)
		: BView(frame, name, B_FOLLOW_ALL, B_WILL_DRAW) {	
}

void printViewView::Draw(BRect pageRect) {
	printf("rect:[%f,%f,%f,%f]\n",pageRect.left,pageRect.top,pageRect.right,pageRect.bottom);

	BFont font(be_plain_font);
	font.SetFamilyAndStyle("Verdana","Regular");
	font.SetFlags(B_DISABLE_ANTIALIASING);
	BFont fontb(be_bold_font);
	fontb.SetFamilyAndStyle("Arial","Bold");
	fontb.SetFlags(B_DISABLE_ANTIALIASING);

	font.SetSize(14.0);
	SetFont(&fontb);
	DrawString("[ąćęłńóśżź ĄĆĘŁŃÓŚŻŹ] ZUPA!", BPoint(10.0,50.0));
}
