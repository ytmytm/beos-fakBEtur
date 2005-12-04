
#ifndef _PRINTHTML_H
#define _PRINTHTML_H

#include "befakprint.h"

class printHTML : public beFakPrint {

	public:
		printHTML(int id, sqlite *db);
//		virtual ~beFakPrint();
		void Go(void);
};

#endif
