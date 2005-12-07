
#ifndef _DIALFILE_H
#define _DIALFILE_H

#include <Looper.h>

class BEntry;
class BMessage;
class BWindow;

class dialFile {
	public:
		static BWindow	*fSourceWindow;
		static BWindow	*fmainWindow;
		static BEntry *OpenDialog(const char *title, const char *path=NULL, uint32 node_flavors=B_FILE_NODE);
		static BEntry *SaveDialog(const char *title, const char *path=NULL, const char *filename=NULL);
};

class DialogLooper : public BLooper {
	public:
		DialogLooper();

		virtual void MessageReceived(BMessage *);
		BEntry *GetEntry();

	private:
		BEntry *fEntry;
};

#endif
