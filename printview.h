
#ifndef _PRINTVIEW_H
#define _PRINTVIEW_H

#include "befakprint.h"

#include <View.h>

class BMessage;
class BPrintJob;

class printView : public beFakPrint {

	public:
		printView(int id, sqlite *db, BMessage *pSettings);
//		virtual ~beFakPrint();
		void Go(void);
	private:
		BPrintJob *printJob;
};

class printViewView : public BView {
	public:
		printViewView(BRect frame, const char *name);

		virtual void Draw(BRect updateRect);
//	private:
};

#endif
