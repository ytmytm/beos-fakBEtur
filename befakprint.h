
#ifndef _BEFAKPRINT_H
#define _BEFAKPRINT_H

#include <String.h>
#include <sqlite3.h>
#include "fakdata.h"

class beFakPrint {
	public:
		beFakPrint(int id, sqlite3 *db, int numkopii);
		// if destructor override -> call it at the end of own
		virtual ~beFakPrint();
		virtual void Go(void);
		void makeSummary(void);
		void updateSummary(const BString wnetto, const int vatid, const BString wvat, const BString wbrutto);
		const char *slownie(const char *input);
		const char *makeName(void);
		void saveToFile(const char *name, const BString *content);

	private:
		const char *rozbij_tysiac(int val);

	protected:
		// configuration
		int p_typ;
		BString p_writepath;
		int p_textcols;
		int p_texteol;
		BString p_htmltemplate;
		int n_kopii;
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
		sqlite3 *dbData;
		char *dbErrMsg;
};

#endif
