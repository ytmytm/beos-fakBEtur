
// todo:
// curdata to/from, msg hangling, db functions: commit,fetch,delete
// fetch vats into menu, (keep local?)
// update: calculate brutto, show dodano
// change/commit: check data (numerical!)
//
// problem: ceny i marża mogą być w ułamkach! (zamiana '.' na ',', tylko cyfry)
//
// sprawdzić jak właściwie działa netto/marża/rabat w bizmaster
// sprawdzić co jest w zakładce 'ceny' w cf

#include "tabtowar.h"
#include "fakdata.h"

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ListView.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include <stdio.h>

const uint32 LIST_INV	= 'TTLI';
const uint32 LIST_SEL	= 'TTLS';
const uint32 BUT_NEW	= 'TTBN';
const uint32 BUT_DEL	= 'TTBD';
const uint32 BUT_RESTORE= 'TTBR';
const uint32 BUT_SAVE	= 'TTBS';
const uint32 DC			= 'TTDC';

const uint32 MENUJM		= 'TTMJ';

const char *jmiary[] = { "szt.", "kg", "kpl.", "m", "mb", "m2", "km", NULL };

tabTowar::tabTowar(BTabView *tv, sqlite *db) : beFakTab(tv, db) {

	idlist = NULL;
	curdata = new towardat(db);
	this->dirty = false;

	this->tab->SetLabel("Towary");
	BRect r;
	r = this->view->Bounds();

	// listview in scrollview
	r.left = 5; r.right = 160; r.top = 30; r.bottom = 500;
	list = new BListView(r, "tTListView");
	this->view->AddChild(new BScrollView("tTScrollView", list, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));
	// buttons
	but_new = new BButton(BRect(40,0,130,24), "tt_but_new", "Nowy towar", new BMessage(BUT_NEW), B_FOLLOW_LEFT|B_FOLLOW_TOP);
	but_del = new BButton(BRect(40,510,130,534), "tt_but_del", "Usuń zaznaczone", new BMessage(BUT_DEL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_restore = new BButton(BRect(235,510,325,534), "tt_but_restore", "Przywróć", new BMessage(BUT_RESTORE), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_save = new BButton(BRect(580,510,670,534), "tt_but_save", "Zapisz", new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
	this->view->AddChild(but_new);
	this->view->AddChild(but_del);
	this->view->AddChild(but_restore);
	this->view->AddChild(but_save);
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
	dodano = new BStringView(BRect(280,80,340,95), "tts0", "Dodano");
	box1->AddChild(dodano);
	dodano = new BStringView(BRect(350,80,420,95), "ttsd", "XX-YY-ZZZZ");
	box1->AddChild(dodano);
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
	box2 = new BBox(BRect(230,150,710,230), "tt_box2");
	box2->SetLabel("Ceny");
	this->view->AddChild(box2);
	// box2-stuff
	ceny[0] = new BTextControl(BRect(10,15,190,35), "ttc0", "Cena netto (zł)", NULL, new BMessage(DC));
	ceny[1] = new BTextControl(BRect(10,50,190,65), "ttc1", "Netto zakupu (zł)", NULL, new BMessage(DC));
	ceny[2] = new BTextControl(BRect(200,50,340,65), "ttc2", "Marża (%)", NULL, new BMessage(DC));
	ceny[3] = new BTextControl(BRect(350,50,470,65), "ttc3", "Rabat (%)", NULL, new BMessage(DC));
	box2->AddChild(ceny[0]);
	box2->AddChild(ceny[1]);
	box2->AddChild(ceny[2]);
	box2->AddChild(ceny[3]);
	brutto = new BStringView(BRect(330,15,390,35), "ttb0", "Cena brutto:");
	box2->AddChild(brutto);
	brutto = new BStringView(BRect(400,15,475,35), "ttbr", "XXXX,YY zł");
	box2->AddChild(brutto);
	// box2-menu
	BPopUpMenu *menuvat = new BPopUpMenu("");
//XXX fetch from db!
	menuvat->AddItem(new BMenuItem("zw.", new BMessage('abcd')));
	BMenuField *menuvatField = new BMenuField(BRect(200,15,330,35), "ttmv", "VAT", menuvat);
	menuvatField->SetDivider(be_plain_font->StringWidth(menuvatField->Label())+15);
	box2->AddChild(menuvatField);
	// box3
	box3 = new BBox(BRect(230,240,710,320), "tt_box3");
	box3->SetLabel("Notatki");
	this->view->AddChild(box3);
	// box3-stuff
	r = box3->Bounds();
	r.InsetBy(10,15);
	BRect s = r; s.OffsetTo(0,0);
	notatki = new BTextView(r, "ttno", s, B_FOLLOW_LEFT|B_FOLLOW_TOP, B_WILL_DRAW);
	box3->AddChild(notatki);
	// fix widths
	int i;
	for (i=0;i<=3;i++) {
		data[i]->SetDivider(be_plain_font->StringWidth(data[i]->Label())+5);
		ceny[i]->SetDivider(be_plain_font->StringWidth(ceny[i]->Label())+5);
	}
	data[0]->SetDivider(50); data[2]->SetDivider(50);
	ceny[0]->SetDivider(90); ceny[1]->SetDivider(90);
//XXX	updateTab();
//XXX	RefreshIndexList();
}

tabTowar::~tabTowar() {

}

void tabTowar::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case DC:
			this->dirty = true;
//XXX			updateTab();
			break;
	}
}
