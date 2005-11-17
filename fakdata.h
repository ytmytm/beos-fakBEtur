
#ifndef _FAKDATA_H
#define _FAKDATA_H

#include <String.h>
#include <sqlite.h>

	class firmadat {
		public:
			firmadat(sqlite *db);
			~firmadat() { };
			void dump(void);
			void clear(void);
			// data management
			int generate_id(void);
			void commit(void);
			void fetch(void);
			void del(void);
			// data holders
			int id;
			// data itself
			BString data[11];
			bool odbiorca, dostawca, aktywny, zablokowany;
		private:
			sqlite *dbData;
			char *dbErrMsg;
	};

	class towardat {
		public:
			towardat(sqlite *db);
			~towardat() { };
			void dump(void);
			void clear(void);
			// data management
			int generate_id(void);
			void commit(void);
			void fetch(void);
			void del(void);
			// data holders
			int id;
			// data itself
			BString data[4], notatki, dodany;
			bool usluga;
			int netto, zakupu, marza, rabat, vat;
		private:
			sqlite *dbData;
			char *dbErrMsg;
	};

	// helper
	int toint(const char *input);

#endif
