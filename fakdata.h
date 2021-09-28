
#ifndef _FAKDATA_H
#define _FAKDATA_H

#include <String.h>
#include <sqlite3.h>

	class dbdat {
		public:
			dbdat(sqlite3 *db) { dbData = db; clear(); };
			virtual ~dbdat() { };
			virtual void clear(void) { id = -1; };
			// data management
			virtual void commit(void) { };
			virtual void fetch(void) { };
			// data itself
			int id;
		protected:
			sqlite3 *dbData;
			char *dbErrMsg;
	};

	class firmadat : public dbdat {
		public:
			firmadat(sqlite3 *db);
			~firmadat() { };
			void clear(void);
			// data management
			int generate_id(void);
			void commit(void);
			void fetch(void);
			void del(void);
			// data itself
			BString data[11];
			bool odbiorca, dostawca, aktywny, zablokowany;
	};

	class towardat : public dbdat {
		public:
			towardat(sqlite3 *db);
			~towardat() { };
			void clear(void);
			// data management
			int generate_id(void);
			void commit(void);
			void fetch(void);
			void del(void);
			// data itself
			BString data[4], ceny[6], notatki, dodany, magazyn, magzmiana;
			bool usluga;
			int vatid;
	};

	class fakturadat : public dbdat {
		public:
			fakturadat(sqlite3 *db);
			~fakturadat() { };
			void clear(void);
			// data management
			int generate_id(void);
			void commit(void);
			void fetch(void);
			void del(void);
			// data itself
			BString nazwa, uwagi;
			BString ogol[10], odata[11];
	};

	class pozfakdata {
		public:
			// data holder
			pozfakdata() { };
			~pozfakdata() { };
	
			BString data[12];
			// lp, nazwa, pkwiu, ilosc, jm, rabat, cena jednostkowa, w.netto, vat,
			// w.vat, w.brutto, c.netto
			int vatid;	// odp. stawce vat, być może zamiast w/w
	};

	class pozfakitem {
		public:
			pozfakitem(pozfakdata *curdata, pozfakitem *prev = NULL, pozfakitem *next = NULL);
			~pozfakitem() { delete data; };
	
			pozfakitem *nxt, *prv;	// list pointers
			int lp;				//XXX liczba porzadkowa - tu, czy w data?
	
			// data fields
			pozfakdata *data;	// real data holder
	};

	class pozfaklist {	// list object
		public:
			pozfaklist(sqlite3 *db);
			~pozfaklist();
	
			void clear(void);
			void setlp(void);
	
			void addlast(pozfakdata *data);
			void addfirst(pozfakdata *data);
			void addafter(pozfakdata *data, pozfakitem *afterme);
			void addafter(pozfakdata *data, int offset);
	
			pozfakdata *itemat(int offset);
	
			void remove(int offset);
	
			int generate_id(void);
			void commit(int fakturaid);
			void commititem(int fakturaid, pozfakitem *data);
			void fetch(int fakturaid);
			// db, calculations
			const char *execSQL(const char *input);	// XXX dupe from befaktab!
			char **calcBrutto(const char *cnetto, const char *rabat, const char *ilosc, const int vatid, int *retcols);
			void calcBruttoFin(char **result);
			const char *calcSumPayment(void);
			void updateStorage(int faktudaid = -1);

			pozfakitem *start;
			pozfakitem *end;
		private:
			sqlite3 *dbData;
			char *dbErrMsg;
	};

	// helpers
	int toint(const char *input);
	int calcdaysago(const char *olddate);
	const char *daysagostring(int days);

#endif
