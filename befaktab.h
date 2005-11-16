
#ifndef _BEFAKTAB_H
#define _BEFAKTAB_H

class BMessage;
class BTab;
class BTabView;
class BView;

class beFakTab {
	public:
		beFakTab(BTabView *tv);
		virtual ~beFakTab();

		virtual void MessageReceived(BMessage *Message);
		virtual bool CommitCurdata(bool haveCancelButton = true);
		virtual void DoCommitCurdata(void) { };	// INSERT/UPDATE

		virtual void curdataFromTab(void) { };
		virtual void curdataToTab(void) { };
		virtual void updateTab(void) { };

		BView *view;
		BTab *tab;

	protected:
		bool dirty;
};

#endif