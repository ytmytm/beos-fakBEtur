
#ifndef _BEFAKPRINT_H
#define _BEFAKPRINT_H

#include <sqlite.h>
#include "fakdata.h"


class beFakPrint {
	public:
		beFakPrint(int id, sqlite *db);
		virtual ~beFakPrint();
		virtual void Go(void);
	private:
		fakturadat *fdata;
		pozfaklist *flist;
		//
		int fakturaid;
		// db stuff
		sqlite *dbData;
		char *dbErrMsg;
};

#endif
