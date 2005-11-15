
#ifndef _TABFIRMA_H
#define _TABFIRMA_H

class BTab;
class BTabView;
class BView;

class tabFirma {

	public:
		tabFirma(BTabView *tv);
		~tabFirma();

		BView *view;
		BTab *tab;

//	private:

};

#endif