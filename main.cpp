
#include "befak.h"

int AppReturnValue(0);

int main (void) {
	AppReturnValue = B_ERROR;
	BeFAKApp myApp;
	myApp.Run();
	return AppReturnValue;
}
