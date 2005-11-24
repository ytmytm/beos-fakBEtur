
#include "globals.h"
#include "tabfaktura.h"

#include <Button.h>
#include <ListView.h>
#include <ScrollView.h>
#include <TabView.h>

#include <stdio.h>

const uint32 LIST_INV	= 'TFLI';
const uint32 LIST_SEL	= 'TFLS';
const uint32 BUT_NEW	= 'TFBN';
const uint32 BUT_DEL	= 'TFBD';
const uint32 BUT_RESTORE= 'TFBR';
const uint32 BUT_SAVE	= 'TFBS';
const uint32 DC			= 'TFDC';


tabFaktura::tabFaktura(BTabView *tv, sqlite *db) : beFakTab(tv, db) {

	idlist = NULL;
	curdata = new fakturadat(db);
	this->dirty = false;

	this->tab->SetLabel("Faktury");
	BRect r;
	r = this->view->Bounds();

	// listview in scrollview
	r.left = 5; r.right = 160; r.top = 30; r.bottom = 500;
	list = new BListView(r, "fTListView");
	this->view->AddChild(new BScrollView("fTScrollView", list, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));
	// buttons
	but_new = new BButton(BRect(40,0,130,24), "tf_but_new", "Nowa faktura", new BMessage(BUT_NEW), B_FOLLOW_LEFT|B_FOLLOW_TOP);
	but_del = new BButton(BRect(40,510,130,534), "tf_but_del", "Usuń zaznaczone", new BMessage(BUT_DEL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_restore = new BButton(BRect(235,510,325,534), "tf_but_restore", "Przywróć", new BMessage(BUT_RESTORE), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_save = new BButton(BRect(580,510,670,534), "tf_but_save", "Zapisz", new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
	this->view->AddChild(but_new);
	this->view->AddChild(but_del);
	this->view->AddChild(but_restore);
	this->view->AddChild(but_save);

	BTabView *tbv2 = new BTabView(BRect(180,0,790,500), "tftbv2");
	this->view->AddChild(tbv2);
	r = tbv2->Bounds();
	r.InsetBy(5,5);
	viewogol = new BView(r, "tfviewogol", B_FOLLOW_ALL_SIDES, 0);
	viewogol->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	tabogol = new BTab(viewogol);
	tbv2->AddTab(viewogol, tabogol);
	tabogol->SetLabel("Dane ogólne");
	viewpozy = new BView(r, "tfviewpozy", B_FOLLOW_ALL_SIDES, 0);
	viewpozy->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	tabpozy = new BTab(viewpozy);
	tbv2->AddTab(viewpozy, tabpozy);
	tabpozy->SetLabel("Pozycje");
//	views: 0,0,490,600
}

tabFaktura::~tabFaktura() {

}
