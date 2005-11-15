
#ifndef _BEFAKTAB_H
#define _BEFAKTAB_H

class BTab;
class BTabView;
class BView;

class beFakTab {
	public:
		beFakTab(BTabView *tv);
		~beFakTab();

		BView *view;
		BTab *tab;
	//private:
};

#endif