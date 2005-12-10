
#ifndef _PRINTVIEW_H
#define _PRINTVIEW_H

#include "befakprint.h"

#include <View.h>

class BMessage;
class BPrintJob;
class BWindow;

class printView : public beFakPrint, public BView {

	public:
		printView(int id, sqlite *db, BMessage *pSettings);
		virtual ~printView();
		void Go(void);
//		virtual void Draw(BRect updateRect);
	private:
		BWindow	*pWindow;
		BPrintJob *printJob;
};

class printViewView : public BView {
	public:
		printViewView(BRect frame, const char *name);

		virtual void Draw(BRect updateRect);
	private:
};

#endif
