
#include "befakprint.h"
#include <stdio.h>

// baseclass with stuff, inherit to print/export/whatever

beFakPrint::beFakPrint(int id, sqlite *db) {

	dbData = db;
	fakturaid = id;
	if (id<1) {
		printf("illegal id!\n");
		return;
	}
	// readout stuff
printf("reading stuff\n");
	fdata = new fakturadat(db);
	flist = new pozfaklist(db);

	fdata->id = fakturaid;
	fdata->fetch();
	flist->fetch(fdata->id);
printf("stuff in memory, do sth with it\n");

}

beFakPrint::~beFakPrint() {
printf("at the and call destructor from baseclass\n");
	delete flist;
	delete fdata;
}

void beFakPrint::Go(void) {
	printf("override and do sth with data, then killyourself\n");
}
