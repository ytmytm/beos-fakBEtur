
#ifndef _BEFAKTAB_H
#define _BEFAKTAB_H

class BMessage;
class BTab;
class BTabView;
class BView;

class beFakTab {
	public:
		beFakTab(BTabView *tv);
		virtual ~beFakTab();

		virtual void MessageReceived(BMessage *Message);

		BView *view;
		BTab *tab;
	//private:
};

#endif