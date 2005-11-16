
#ifndef _FAKDATA_H
#define _FAKDATA_H

#include <String.h>

	class firmadat {
		public:
			firmadat(void) { clear(); };
			~firmadat() { };
			void dump(void);
			void clear(void);
			// data holders
			int id;
			// data itself
			BString data[11];
			bool odbiorca, dostawca, aktywny, zablokowany;
	};

#endif
