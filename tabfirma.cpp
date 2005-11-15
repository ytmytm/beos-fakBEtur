
#include "tabfirma.h"

#include <Box.h>
#include <Button.h>
#include <TabView.h>
#include <View.h>
#include <stdio.h>

tabFirma::tabFirma(BTabView *tv) {
	BRect r;

	r = tv->Bounds();
	r.InsetBy(5, 10);
	view = new BView(r, "tabFirma", B_FOLLOW_ALL_SIDES, 0);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	tab = new BTab(view);
	tv->AddTab(view, tab);
	tab->SetLabel("Kontrahenci");

	r = view->Bounds();
}

tabFirma::~tabFirma() {

}
