
#include "dialfile.h"

#include <Directory.h>
#include <Entry.h>
#include <FilePanel.h>
#include <Message.h>
#include <Messenger.h>
#include <Window.h>

sem_id dialog_sem;

BWindow	*dialFile::fSourceWindow = NULL;
BWindow	*dialFile::fmainWindow = NULL;

BEntry *dialFile::OpenDialog(const char *title, const char *path, uint32 node_flavors) {
	dialog_sem = create_sem(0, "dialog_sem");

	BEntry *entry = NULL;
	BEntry *fpentry;
	DialogLooper *dlglooper = new DialogLooper();
	entry_ref ref;
	void *ptr=NULL;

	if (path != NULL) {
		entry = new BEntry(path);
		if (entry->GetRef(&ref) != B_OK) {
			entry->Unset();
			entry->SetTo("/boot/home/");
			entry->GetRef(&ref);
		}
		ptr=&ref;
	}
	BFilePanel *fp = new BFilePanel(B_OPEN_PANEL,
		new BMessenger(dlglooper, dlglooper),
		(entry_ref *)ptr,
		node_flavors,
		false, NULL, NULL, true, true);

	if (title != NULL)
		fp->Window()->SetTitle(title);
	// center window
	if (fmainWindow != NULL) {
		BRect panelRect, mainWRect;
		panelRect=fp->Window()->Frame();
		mainWRect=fmainWindow->Frame();
		fp->Window()->MoveTo(mainWRect.left+(mainWRect.Width()-panelRect.Width())/2,
					mainWRect.top+(mainWRect.Height()-panelRect.Height())/2);
	}
	fp->Show();
	while (acquire_sem_etc(dialog_sem, 1, B_RELATIVE_TIMEOUT, 10000)==B_TIMED_OUT) {
		if (fSourceWindow != NULL)
			fSourceWindow->UpdateIfNeeded();
	}
	fSourceWindow = NULL;
	fpentry = dlglooper->GetEntry();
	dlglooper->Lock();
	dlglooper->Quit();	// don't delete, but quit()
	delete fp;
	delete_sem(dialog_sem);
	return fpentry;
}

BEntry *dialFile::SaveDialog(const char *title, const char *path, const char *filename) {
	dialog_sem=create_sem(0, "dialog_sem");

	BEntry *entry = NULL;
	BEntry *fpentry;
	DialogLooper *dlglooper = new DialogLooper();
	entry_ref ref;
	void *ptr=NULL;

	if (path != NULL) {
		entry = new BEntry(path);
		if (entry->GetRef(&ref) != B_OK) {
			entry->Unset();
			entry->SetTo("/boot/home/");
			entry->GetRef(&ref);
		}
		ptr=&ref;
	}
	BFilePanel	*fp=new BFilePanel(B_SAVE_PANEL,
		new BMessenger(dlglooper, dlglooper),
		(entry_ref *)ptr,
		0,
		false, NULL, NULL, true, true);
	if (title!=NULL)
		fp->Window()->SetTitle(title);
	if (filename)
		fp->SetSaveText(filename);
	// center window
	if (fmainWindow != NULL) {
		BRect panelRect, mainWRect;
		panelRect = fp->Window()->Frame();
		mainWRect = fmainWindow->Frame();
		fp->Window()->MoveTo(mainWRect.left+(mainWRect.Width()-panelRect.Width())/2,
					mainWRect.top+(mainWRect.Height()-panelRect.Height())/2);
	}
	fp->Show();
	while (acquire_sem_etc(dialog_sem, 1, B_RELATIVE_TIMEOUT, 10000)==B_TIMED_OUT) {
		if (fSourceWindow != NULL)
			fSourceWindow->UpdateIfNeeded();
	}
	fSourceWindow = NULL;
	fpentry = dlglooper->GetEntry();
	dlglooper->Lock();
	dlglooper->Quit();	// don't delete, but quit()
	delete fp;
	delete_sem(dialog_sem);
	return fpentry;
}

DialogLooper::DialogLooper() : BLooper(
	"DialogLooper", B_LOW_PRIORITY) {

	fEntry = new BEntry();
	Run();
}

void DialogLooper::MessageReceived(BMessage *msg) {
	switch(msg->what) {
		case B_CANCEL:
			release_sem(dialog_sem);
			break;
		case B_REFS_RECEIVED:
			{	entry_ref ref;
				if (msg->FindRef("refs", 0, &ref) == B_OK)
					fEntry->SetTo(&ref);
				else
					fEntry->Unset();
				break;
			}
		case B_SAVE_REQUESTED:
			{	const char *name;
				entry_ref ref;

				if (msg->FindString("name", &name) != B_OK) {
					fEntry->Unset();
					break;
				}
				if (msg->FindRef("directory", 0, &ref) == B_OK)
					fEntry->SetTo(new BDirectory(&ref), name);
				else {
					fEntry->Unset();
					break;
				}
				break;
			}
		default:
			break;
	}
}

BEntry *DialogLooper::GetEntry() {
	return fEntry;
}
