
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
	private:
		fakturadat *fdata;
		pozfaklist *flist;
		//
		int fakturaid;
		BString own[11];
		// db stuff
		sqlite *dbData;
		char *dbErrMsg;
};

#endif
