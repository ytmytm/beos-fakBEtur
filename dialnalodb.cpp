
#include <Box.h>
#include <Button.h>
#include <String.h>
#include <StringView.h>
#include <View.h>
#include "ColumnListView.h"
#include "CLVEasyItem.h"

#include "globals.h"
#include "dialnalodb.h"
#include "fakdata.h"
#include <stdio.h>

const uint32 BUT_CLOSE	= 'DNOC';

class tab4ListItem : public CLVEasyItem {
	public:
		tab4ListItem(int id, const char *col0, const char *col1, const char *col2, const char *col3) : CLVEasyItem(
			0, false, false, 20.0) {
			fId = id;
			SetColumnContent(0,col0);
			SetColumnContent(1,col1);
			SetColumnContent(2,col2,true,true);
			SetColumnContent(3,col3,true,true);
		};
		int Id(void) { return fId; };
	private:
		int fId;
};

dialNalodb::dialNalodb(sqlite *db, const char *odb) : BWindow(
	BRect(100, 100, 740, 580),
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
	this->Show();

	BString tmp = "SELECT id FROM firma WHERE nazwa = '";
	tmp += odb; tmp += "'";
	id = toint(execSQL(tmp.String()));
	if (id<=0)
		return;
	tmp = "Wykaz należności: "; tmp += odb;
	this->SetTitle(tmp.String());

	// fill left list (4)
	// fill right list (2)
	// fill summaries
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
