
#ifndef _BEFAKPRINT_H
#define _BEFAKPRINT_H

#include <String.h>
#include <sqlite.h>
#include "fakdata.h"

class beFakPrint {
	public:
		beFakPrint(int id, sqlite *db);
		virtual ~beFakPrint();
		virtual void Go(void);

		int typ;	// 0 - oryginal, 1 - kopia, 2 - duplikat
		// tylko dla tekstowego
		bool wide;	// tryb szerokiego druku (136) vs (80) kolumn
		int ncols;	// liczba kolumn

	private:
		// data holders
		fakturadat *fdata;
		pozfaklist *flist;
		//
		int fakturaid;
		BString own[11];
		// db stuff
		sqlite *dbData;
		char *dbErrMsg;
		// tylko dla tekstowego
		const char *rightAlign(const BString line, const BString right);
		const char *centerAlign(const BString line);
		const char *halfAlign(const BString line, const BString right);
		const char *fitAlignR(const BString line, int len, bool space = false);
		const char *fitAlignL(const BString line, int len, bool space = false);
};

#endif
