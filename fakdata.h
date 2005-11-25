
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
			BString data[4], ceny[6], notatki, dodany;
			bool usluga;
			int vatid;
		private:
			sqlite *dbData;
			char *dbErrMsg;
	};

	class fakturadat {
		public:
			fakturadat(sqlite *db);
			~fakturadat() { };
			void dump(void);
			void clear(void);
			// data management
			int generate_id(void);
			int generate_pozid(void);
			void del(void);
			// data holders
			int id;
			// data itself
			BString ogol[10], odata[11];
			bool zaplacono;
		private:
			// data itself
			sqlite *dbData;
			char *dbErrMsg;
	};

	// helpers
	int toint(const char *input);
	const char *decround(const char *input);

#endif
