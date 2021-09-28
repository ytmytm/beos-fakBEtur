
#ifndef _PRINTVIEW_H
#define _PRINTVIEW_H

#include "befakprint.h"

#include <View.h>

class BMessage;
class BPrintJob;
class BWindow;

class printView : public beFakPrint, public BView {

	public:
		printView(int id, sqlite3 *db, int numkopii, BMessage *pSettings);
		virtual ~printView();
		void Go(void);
		virtual void Draw(BRect updateRect);
	private:
		void DrawStr(const BString str);
		void DrawStrCenter(const BString str, float l, float r);
		void DrawStrRight(const BString str, float r);
		void DrawStrLeft(const BString str, float l);

		BWindow	*pWindow;
		BPrintJob *printJob;
};

#endif
