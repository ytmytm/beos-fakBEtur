
#include "dialabout.h"

#include <AppKit.h>
#include <Bitmap.h>
#include <GraphicsDefs.h>
#include <Screen.h>
#include <StorageKit.h>
#include <StringView.h>
#include <TextView.h>

AboutView::AboutView(BRect frame, const char *name, BBitmap *icon)
		: BView(frame, name, B_FOLLOW_ALL, B_WILL_DRAW) {	
	SetViewColor(216,216,216);
	bIcon=new BBitmap(BRect(0,0,31,31), B_CMAP8);
}

void AboutView::SetIcon(BBitmap *icon) {
	bIcon->SetBits(icon->Bits(), icon->BitsLength(), 0, icon->ColorSpace());
	Invalidate();
}

void AboutView::Draw(BRect updateRect) {
	if (BRect(0,0,21+33,Bounds().Height()).Intersects(updateRect)) {
		SetDrawingMode(B_OP_OVER);
		SetHighColor(184,184,184,255);
		FillRect(BRect(0,0,10+23,Bounds().Height()));
		SetHighColor(0,0,0,255);
		DrawBitmap(bIcon, BPoint(21,10));
	}
}

dialAbout::dialAbout(const char *title) : BWindow(
		BRect(100,100,400,280),
		title,
		B_FLOATING_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) {

	vAbout = new AboutView(Bounds(), "AboutView", NULL);	
	AddChild(vAbout);
	BRect frame = Bounds();

	applicationNameSV = new BStringView(BRect(60,1, frame.Width()-60, 27), "applicationNameSV", "");
	applicationNameSV->SetFont(be_bold_font);
	vAbout->AddChild(applicationNameSV);

	versionNumberSV = new BStringView(BRect(applicationNameSV->StringWidth(applicationNameSV->Text())+65, 1, frame.Width()-5, 27), "versionNumberSV", "");
	versionNumberSV->SetFontSize(10);
	vAbout->AddChild(versionNumberSV);

	copyrightStringSV = new BStringView(BRect(60,27, frame.Width()-60, 41), "copyrightStringSV", "");
	copyrightStringSV->SetFontSize(10);
	vAbout->AddChild(copyrightStringSV);

	textTV = new BTextView(BRect(60,55, frame.right-10, frame.bottom-8), "textTV", BRect(1,1,frame.Width()-72,400), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
	textTV->SetViewColor(216,216,216,255);
	textTV->MakeEditable(false);
	textTV->MakeSelectable(false);
	textTV->SetWordWrap(true);
	BFont *font = new BFont(be_plain_font);
	font->SetSize(10);
	textTV->SetFontAndColor(font);
	vAbout->AddChild(textTV);
	delete font;

	BScreen	*screen = new BScreen();
	MoveTo((int)((screen->Frame().Width()-Bounds().Width())/2), (int)((screen->Frame().Height()-Bounds().Height())/2));
	delete screen;

	app_info info;
	if (be_app->GetAppInfo(&info) == B_OK) {
		BBitmap bmp(BRect(0,0,31,31), B_CMAP8);
		if (BNodeInfo::GetTrackerIcon(&info.ref, &bmp, B_LARGE_ICON)==B_OK)
			SetIcon(&bmp);
	}
}

void dialAbout::SetApplicationName(const char *name) {
	font_height	boldheight, plainheight;

	applicationNameSV->GetFontHeight(&boldheight);
	versionNumberSV->GetFontHeight(&plainheight);

	applicationNameSV->SetText(name);
	versionNumberSV->ResizeTo((Bounds().Width()-5)-(applicationNameSV->StringWidth(applicationNameSV->Text())+65), 26);
	versionNumberSV->MoveTo(applicationNameSV->StringWidth(applicationNameSV->Text())+65, applicationNameSV->Frame().top-(boldheight.descent-plainheight.descent));
}

void dialAbout::SetVersionNumber(const char *version) {
	versionNumberSV->SetText(version);
}

void dialAbout::SetIcon(BBitmap *icon) {
	vAbout->SetIcon(icon);
}

void dialAbout::SetCopyrightString(const char *copyright) {
	copyrightStringSV->SetText(copyright);
}

void dialAbout::SetText(const char *text) {
	textTV->SetText(text);
}

bool dialAbout::QuitRequested() {
	Quit();
	return false;
}
