
#ifndef _PRINTTEXT_H
#define _PRINTTEXT_H

#include "befakprint.h"

class printText : public beFakPrint {

	public:
		printText(int id, sqlite *db);
//		virtual ~beFakPrint();
		void Go(void);
		// tylko dla tekstowego
		bool wide;	// tryb szerokiego druku (136) vs (80) kolumn
		int ncols;	// liczba kolumn

#define ELINE	"\n"	// znak końca linii

	private:
		// text formatting helpers
		const char *rightAlign(const BString line, const BString right);
		const char *centerAlign(const BString line);
		const char *halfAlign(const BString line, const BString right);
		const char *fitAlignR(const BString line, int len, bool space = false);
		const char *fitAlignL(const BString line, int len, bool space = false);
		const char *leftFill(const BString line, int spaces);

};

#endif
