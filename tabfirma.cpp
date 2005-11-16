//
// skopiować logikę z kesy (mainwindow) - obsługa buttonów i zmiany fokusu
// na liście, wypełnianie listy itd.
//
// część bazodanową przenieść do klasy danych (tylko handle na db, otwieranie
// z mainwindow, alerty tamże)
//


#include "tabfirma.h"
#include "fakdata.h"

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ListView.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <stdio.h>

const uint32 LIST_INV	= 'TFLI';
const uint32 LIST_SEL	= 'TFLS';
const uint32 BUT_NEW	= 'TFBN';
const uint32 BUT_DEL	= 'TFBD';
const uint32 BUT_RESTORE= 'TFBR';
const uint32 BUT_SAVE	= 'TFBS';
const uint32 DC			= 'TFDC';

tabFirma::tabFirma(BTabView *tv) : beFakTab(tv) {

	curdata = new firmadat();

	BRect r;
	r = this->view->Bounds();

	// listview in scrollview
	r.left = 5; r.right = 160; r.top = 30; r.bottom = 500;
	list = new BListView(r, "tFListView");
	this->view->AddChild(new BScrollView("tFScrollView", list, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, false, true));
	list->SetInvocationMessage(new BMessage(LIST_INV));
	list->SetSelectionMessage(new BMessage(LIST_SEL));
	// buttons
	but_new = new BButton(BRect(40,0,130,24), "tf_but_new", "Nowy kontrahent", new BMessage(BUT_NEW), B_FOLLOW_LEFT|B_FOLLOW_TOP);
	but_del = new BButton(BRect(40,510,130,534), "tf_but_del", "Usuń zaznaczone", new BMessage(BUT_DEL), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_restore = new BButton(BRect(235,510,325,534), "tf_but_restore", "Przywróć", new BMessage(BUT_RESTORE), B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	but_save = new BButton(BRect(580,510,670,534), "tf_but_save", "Zapisz", new BMessage(BUT_SAVE), B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
	this->view->AddChild(but_new);
	this->view->AddChild(but_del);
	this->view->AddChild(but_restore);
	this->view->AddChild(but_save);
	// box1
	box1 = new BBox(BRect(230,30,670,180), "tf_box1");
	box1->SetLabel("Dane adresowe");
	this->view->AddChild(box1);
	// box1-stuff
	data[0] = new BTextControl(BRect(10,15,270,35), "tfd0", "Nazwa", NULL, new BMessage(DC));
	data[1] = new BTextControl(BRect(280,15,420,35), "tfd1", "Symbol", NULL, new BMessage(DC));
	data[2] = new BTextControl(BRect(10,50,420,65), "tfd2", "Adres", NULL, new BMessage(DC));
	data[3] = new BTextControl(BRect(10,80,150,95), "tfd3", "Kod", NULL, new BMessage(DC));
	data[4] = new BTextControl(BRect(160,80,420,95), "tfd4", "Miejscowość", NULL, new BMessage(DC));
	data[5] = new BTextControl(BRect(10,110,200,125), "tfd5", "Tel.", NULL, new BMessage(DC));
	data[6] = new BTextControl(BRect(210,110,420,125), "tfd6", "Email", NULL, new BMessage(DC));
	box1->AddChild(data[0]); box1->AddChild(data[1]);
	box1->AddChild(data[2]);
	box1->AddChild(data[3]); box1->AddChild(data[4]);
	box1->AddChild(data[5]); box1->AddChild(data[6]);
	// box2
	box2 = new BBox(BRect(230,190,670,340), "tf_box2");
	box2->SetLabel("Dane firmy");
	this->view->AddChild(box2);
	// box2-stuff
	r.left = 10; r.top = 20; r.right = 420; r.bottom = 35;
	data[7] = new BTextControl(r, "tfd7", "NIP", NULL, new BMessage(DC)); r.OffsetBy(0, 30);
	data[8] = new BTextControl(r, "tfd8", "REGON", NULL, new BMessage(DC)); r.OffsetBy(0, 30);
	data[9] = new BTextControl(r, "tfd9", "Bank", NULL, new BMessage(DC)); r.OffsetBy(0, 30);
	data[10] = new BTextControl(r, "tfd10", "Nr konta", NULL, new BMessage(DC));
	box2->AddChild(data[7]); box2->AddChild(data[8]);
	box2->AddChild(data[9]); box2->AddChild(data[10]);
	// box3
	box3 = new BBox(BRect(230,360,380,440), "tf_box3");
	this->view->AddChild(box3);
	// box3-stuff
	odbiorca = new BCheckBox(BRect(10,15,100,35), "tf_co", "Odbiorca", new BMessage(DC));
	dostawca = new BCheckBox(BRect(10,50,100,65), "tf_cd", "Dostawca", new BMessage(DC));
	box3->AddChild(odbiorca); box3->AddChild(dostawca);
	// box4
	box4 = new BBox(BRect(520,360,670,440), "tf_box4");
	this->view->AddChild(box4);
	// box4-stuff
	aktywny = new BCheckBox(BRect(10,15,100,35), "tfca", "Aktywny", new BMessage(DC));
	zablokowany = new BCheckBox(BRect(10,50,100,65), "tf_cz", "Zablokowany", new BMessage(DC));
	box4->AddChild(aktywny); box4->AddChild(zablokowany);	
	// fix widths
	int i;
	for (i=0;i<=6;i++) {
		data[i]->SetDivider(be_plain_font->StringWidth(data[i]->Label())+5);
	}
	data[0]->SetDivider(50); data[2]->SetDivider(50);
	data[3]->SetDivider(50); data[5]->SetDivider(50);
	data[7]->SetDivider(50); data[8]->SetDivider(50);
	data[9]->SetDivider(50); data[10]->SetDivider(50);
	updateTab();
}

tabFirma::~tabFirma() {

}

void tabFirma::curdataFromTab(void) {
	int i;
	for (i=0;i<=10;i++) {
		curdata->data[i] = data[i]->Text();
	}
	curdata->odbiorca = (odbiorca->Value() == B_CONTROL_ON);
	curdata->dostawca = (dostawca->Value() == B_CONTROL_ON);
	curdata->aktywny = (aktywny->Value() == B_CONTROL_ON);
	curdata->zablokowany = (zablokowany->Value() == B_CONTROL_ON);
}

void tabFirma::curdataToTab(void) {
	int i;
	for (i=0;i<=10;i++) {
		data[i]->SetText(curdata->data[i].String());
	}
	odbiorca->SetValue(curdata->odbiorca ? B_CONTROL_ON : B_CONTROL_OFF);
	dostawca->SetValue(curdata->dostawca ? B_CONTROL_ON : B_CONTROL_OFF);
	aktywny->SetValue(curdata->aktywny ? B_CONTROL_ON : B_CONTROL_OFF);
	zablokowany->SetValue(curdata->zablokowany ? B_CONTROL_ON : B_CONTROL_OFF);
	updateTab();
}

void tabFirma::updateTab(void) {
	bool state = (zablokowany->Value() != B_CONTROL_ON);

	int i;
	for (i=0;i<=10;i++) {
		data[i]->SetEnabled(state);
	}
	odbiorca->SetEnabled(state);
	dostawca->SetEnabled(state);
	aktywny->SetEnabled(state);	
}

void tabFirma::MessageReceived(BMessage *Message) {
	switch (Message->what) {
		case DC:
			printf("change!\n");
			curdata->dirty = true;
			updateTab();
			break;
	}
}
