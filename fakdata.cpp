
#include "fakdata.h"

#include <stdio.h>

void firmadat::dump( void ) {
	int i;
	for (i=0;i<=10;i++) {
		printf("%i:[%s] ",i,this->data[i].String());
	}
	printf("\n,odb:%i,dos:%i,akt:%i,zab:%i\n-----\n",odbiorca,dostawca,aktywny,zablokowany);
}

void firmadat::clear( void ) {
	int i;
	for (i=0;i<=10;i++) {
		data[i] = "";
	}
	odbiorca = dostawca = aktywny = zablokowany = false;
}
