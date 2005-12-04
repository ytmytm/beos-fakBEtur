
#include "befakprint.h"

#include <stdio.h>

// baseclass with stuff, inherit to print/export/whatever

beFakPrint::beFakPrint(int id, sqlite *db) {

	if (id<1) {
		printf("illegal id!\n");
		return;
	}

	dbData = db;
	fakturaid = id;
	typ = 0;		// XXX parametr!

	switch (typ) {
		case 2:
			typfaktury = "Duplikat";
			break;
		case 1:
			typfaktury = "Kopia";
			break;
		case 0:
		default:
			typfaktury = "Oryginał";
			break;
	}

	// readout stuff
//printf("reading stuff for id=%i\n",fakturaid);
	fdata = new fakturadat(db);
	flist = new pozfaklist(db);

	fdata->id = fakturaid;
	fdata->fetch();
	flist->fetch(fdata->id);
	fsumma = NULL;

	// XXX duplicated in dialfirma constructor!!! separate!!!
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;
	sql = "SELECT ";
	sql += "nazwa, adres, kod, miejscowosc, telefon, email";
	sql += ", nip, regon, bank, konto";
	sql += " FROM konfiguracja WHERE zrobiona = 1";
//printf("sql:%s\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i\n", nRows, nCols);
	if (nRows < 1) {
		// XXX brak informacji o własnej firmie, co robić?
	} else {
//		printf("not initial\n");
		// readout data
		i = nCols;
		own[0] = result[i++];
		for (j=2;j<=10;j++) {
			own[j] = result[i++];
		}
	}
	// tabela pomocnicza do podsumowania
	flist->execSQL("CREATE TEMPORARY TABLE sumawydruk ( wnetto DECIMAL(12,2), vatid INTEGER, wvat DECIMAL(12,2), wbrutto DECIMAL(12,2) )");

//	printf("stuff in memory, do sth with it\n");
}

void beFakPrint::updateSummary(const BString wnetto, const int vatid, const BString wvat, const BString wbrutto) {
	int ret;

	ret = sqlite_exec_printf(dbData, "INSERT INTO sumawydruk (wnetto,vatid,wvat,wbrutto) VALUES ( %Q, %i, %Q, %Q )", 0, 0, &dbErrMsg,
		wnetto.String(), vatid, wvat.String(), wbrutto.String() );
//		printf("result: %i, %s\n", ret, dbErrMsg);

}

void beFakPrint::makeSummary(void) {
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;

	// suma z rozbiciem na stawki
	sql = "SELECT SUM(s.wnetto), v.nazwa, SUM(s.wvat), SUM(s.wbrutto) FROM sumawydruk AS s, stawka_vat AS v WHERE v.id = s.vatid GROUP BY s.vatid ORDER BY v.nazwa";
//printf("sql:[%s]\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows < 1) {
		// nothin'?
	} else {
		i = nCols;
		j = 1;
		fsummarows = nRows;
		fsumma = new summary[nRows];
		while (j <= nRows) {
			fsumma[j-1].summa[0] = decround(result[i++]);
			fsumma[j-1].summa[1] = result[i++];
			fsumma[j-1].summa[2] = decround(result[i++]);
			fsumma[j-1].summa[3] = decround(result[i++]);
			j++;
		}
	}
	sqlite_free_table(result);
	// obliczyc RAZEM
	sql = "SELECT SUM(s.wnetto), v.nazwa, SUM(s.wvat), SUM(s.wbrutto) FROM sumawydruk AS s, stawka_vat AS v WHERE v.id = s.vatid";
//printf("sql:[%s]\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows < 1) {
		// nothin'?
	} else {
		i = nCols;
		razem.summa[0] = decround(result[i++]);
		razem.summa[1] = result[i++];
		razem.summa[2] = decround(result[i++]);
		razem.summa[3] = decround(result[i++]);
	}
	sqlite_free_table(result);
}

beFakPrint::~beFakPrint() {
//printf("at the end call destructor from baseclass\n");
	flist->execSQL("DROP TABLE sumawydruk");
	delete [] fsumma;

	delete flist;
	delete fdata;
}

const char *setki[] = { "", "sto ", "dwieście ", "trzysta ", "czterysta ", "pięćset ", "sześćset ", "siedemset ", "osiemset ", "dziewięćset " };
const char *dziesiatki[] = { "", "", "dwadzieścia ", "trzydzieści ", "czterdzieści ", "pięćdziesiąt ", "sześćdziesiąt", "siedemdziesiąt ", "osiemdziesiąt ", "dziewięćdziesiąt " };
const char *nascie[] = { "dziesięć ", "jedenaście ", "dwanaście ", "trzynaście ", "czternaście ", "piętnaście ", "szesnaście ", "siedemnaście ", "osiemnaście ", "dziewiętnaście " };
const char *jednosci[] = { "", "jeden ", "dwa ", "trzy ", "cztery ", "pięć ", "sześć ", "siedem ", "osiem ", "dziewięć " };

const char *beFakPrint::rozbij_tysiac(int val) {
	static BString tmp;
	int i;
	int t = val;

	tmp = "";
	i = t / 100;
	tmp += setki[i];
	t = t % 100;
	i = t / 10;
	t = t % 10;
	if (i!=1) {
		tmp += dziesiatki[i];
		tmp += jednosci[t];
	} else {
		tmp += nascie[t];
	}
	return tmp.String();
}

const char *beFakPrint::slownie(const char *input) {
	static BString tmp;

	int i = 0;
	int l = strlen(input);
	char c;

	int z = 0;		// zlote
	int g = 0;		// grosze
	int w = 0;		// wykladnik
	bool grosze = false;
	tmp = "";

	while (i<l) {
		c = input[i++];
		if (c == '.')
			grosze = true;
		else
		if (!grosze)
			z = z*10+c-'0';
		else {
			w++;
			if (w<3)
				g=g*10+c-'0';
		}
	}
	if (w==1) g*=10;

	i = z / 1000000;
	if (i!=0) {
		tmp += rozbij_tysiac(i);
		tmp += "mln. ";
	}
	z = z % 1000000;

	i = z / 1000;
	if (i!=0) {
		tmp += rozbij_tysiac(i);
		tmp += "tys. ";
	}
	z = z % 1000;

	i = z;
	if (i!=0) {
		tmp += rozbij_tysiac(i);
	}
	if ((tmp.Length()==0) && (g==0))
		tmp = "zero ";
	if (tmp.Length()!=0)
		tmp += "zł ";

	i = g;
	if (i!=0) {
		tmp += rozbij_tysiac(i);
		tmp += "gr";
	}

	return tmp.String();
}

void beFakPrint::Go(void) {
	printf("override and do sth meaningful with data\n");
}
