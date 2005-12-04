
#ifndef _BEFAKPRINT_H
#define _BEFAKPRINT_H

#include <String.h>
#include <sqlite.h>
#include "fakdata.h"

class beFakPrint {
	public:
		beFakPrint(int id, sqlite *db);
		// if destructor override -> call it at the end of own
		virtual ~beFakPrint();
		virtual void Go(void);
		void makeSummary(void);
		void updateSummary(const BString wnetto, const int vatid, const BString wvat, const BString wbrutto);
		const char *slownie(const char *input);

		int typ;	// 0 - oryginal, 1 - kopia, 2 - duplikat

	private:
		const char *rozbij_tysiac(int val);

	protected:
		// data holders
		fakturadat *fdata;
		pozfaklist *flist;

		struct summary {
			BString summa[4];
		};

		summary *fsumma;
		summary razem;
		int fsummarows;
		BString typfaktury;
		//
		int fakturaid;
		BString own[11];	// informacje o wlasnej firmie
		// db stuff
		sqlite *dbData;
		char *dbErrMsg;
};

#endif
