
#include "dialabout.h"

dialView::dialView(BRect rect, const char *name, const char *title) : BView(
	rect, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW) {

	appname = title;
}

void dialView::Draw(BRect updateRect) {
	SetFontSize(24);
	MovePenTo(BPoint(10,40));
	DrawString(appname);
	MovePenTo(BPoint(10,80));
	DrawString("(C) 2005 Maciej Witkowiak");
	MovePenTo(BPoint(10,120));
	DrawString("<ytm@elysium.pl>");
	SetFontSize(9);
	MovePenTo(BPoint(320,140));
	DrawString(":* E.");
}

dialAbout::dialAbout(const char *title) : BWindow(
	BRect(62, 100, 62+370, 260),
	"O programie",
	B_FLOATING_WINDOW,
	B_NOT_RESIZABLE ) {

	AddChild(new dialView(Bounds(), "aboutView", title));
	Show();
}
