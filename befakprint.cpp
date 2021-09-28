//
// TODO:
//

#include "befakprint.h"
#include "dialfile.h"

#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <stdio.h>

// baseclass with stuff, inherit to print/export/whatever

beFakPrint::beFakPrint(int id, sqlite3 *db, int numkopii) {

	if (id<1) {
//		printf("illegal id!\n");
		return;
	}

	n_kopii = numkopii;
	dbData = db;
	fakturaid = id;

// XXX read config from db
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;
	sql = "SELECT p_typ, p_writepath, p_textcols";
	sql += ", p_texteol, p_htmltemplate";
	sql += ", nazwa, adres, kod, miejscowosc, telefon, email";
	sql += ", nip, regon, bank, konto";
	sql += " FROM konfiguracja WHERE zrobiona = 1";
//printf("sql:%s\n",sql.String());
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i\n", nRows, nCols);
	if (nRows < 1) {
		// XXX brak informacji o własnej firmie, co robić?
	} else {
		i = nCols;
		p_typ = toint(result[i++]);
		p_writepath = result[i++];
		p_textcols = toint(result[i++]);
		p_texteol = toint(result[i++]);
		p_htmltemplate = result[i++];
		own[0] = result[i++];
		for (j=2;j<=10;j++) {
			own[j] = result[i++];
		}
	}
	sqlite3_free_table(result);

	switch (p_typ) {
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

	// tabela pomocnicza do podsumowania
	flist->execSQL("CREATE TEMPORARY TABLE sumawydruk ( wnetto DECIMAL(12,2), vatid INTEGER, wvat DECIMAL(12,2), wbrutto DECIMAL(12,2) )");

//	printf("stuff in memory, do sth with it\n");
}

void beFakPrint::updateSummary(const BString wnetto, const int vatid, const BString wvat, const BString wbrutto) {
	int ret;
	char *query = sqlite3_mprintf("INSERT INTO sumawydruk (wnetto,vatid,wvat,wbrutto) VALUES ( %Q, %i, %Q, %Q )",
		wnetto.String(), vatid, wvat.String(), wbrutto.String());
	ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
	sqlite3_free(query);
//		printf("result: %i, %s\n", ret, dbErrMsg);

}

void beFakPrint::makeSummary(void) {
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;

	// suma z rozbiciem na stawki
	sql = "SELECT DECROUND(SUM(s.wnetto)), v.nazwa, DECROUND(SUM(s.wvat)), DECROUND(SUM(s.wbrutto)) FROM sumawydruk AS s, stawka_vat AS v WHERE v.id = s.vatid GROUP BY s.vatid ORDER BY v.stawka";
//printf("sql:[%s]\n",sql.String());
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows < 1) {
		// nothin'?
	} else {
		i = nCols;
		j = 1;
		fsummarows = nRows;
		fsumma = new summary[nRows];
		while (j <= nRows) {
			fsumma[j-1].summa[0] = result[i++];
			fsumma[j-1].summa[1] = result[i++];
			fsumma[j-1].summa[2] = result[i++];
			fsumma[j-1].summa[3] = result[i++];
			j++;
		}
	}
	sqlite3_free_table(result);
	// obliczyc RAZEM
	sql = "SELECT DECROUND(SUM(s.wnetto)), '', DECROUND(SUM(s.wvat)), DECROUND(SUM(s.wbrutto)) FROM sumawydruk AS s, stawka_vat AS v WHERE v.id = s.vatid";
//printf("sql:[%s]\n",sql.String());
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows < 1) {
		// nothin'?
	} else {
		i = nCols;
		razem.summa[0] = result[i++];
		razem.summa[1] = result[i++];
		razem.summa[2] = result[i++];
		razem.summa[3] = result[i++];
	}
	sqlite3_free_table(result);
}

beFakPrint::~beFakPrint() {
//printf("at the end call destructor from baseclass\n");
	flist->execSQL("DROP TABLE sumawydruk");
	delete [] fsumma;

	delete flist;
	delete fdata;
}

const char *setki[] = { "", "sto ", "dwieście ", "trzysta ", "czterysta ", "pięćset ", "sześćset ", "siedemset ", "osiemset ", "dziewięćset " };
const char *dziesiatki[] = { "", "", "dwadzieścia ", "trzydzieści ", "czterdzieści ", "pięćdziesiąt ", "sześćdziesiąt ", "siedemdziesiąt ", "osiemdziesiąt ", "dziewięćdziesiąt " };
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

const char *beFakPrint::makeName(void) {
	static BString tmp;
	tmp = fdata->nazwa;
	// prepare a safe filename
	tmp.ReplaceAll("/","-");
	tmp.ReplaceAll("\\","-");
	tmp.ReplaceAll(" ","");
	switch (p_typ) {
		case 2:
			tmp += "-";
			tmp += typfaktury;
			break;
		case 1:
			tmp += "-";
			tmp += typfaktury;
			tmp += "-";
			tmp << n_kopii;
			break;
		case 0:
		default:
			break;
	}
	return tmp.String();
}

void beFakPrint::saveToFile(const char *name, const BString *content) {
	BPath path;
	BString tmp;
	tmp = flist->execSQL("SELECT p_writepath FROM konfiguracja WHERE zrobiona = 1");
	BEntry *ent = dialFile::SaveDialog("Zapisz wydruk do pliku", tmp.String(), name);

	ent->GetPath(&path);
	delete ent;

	tmp = path.Path();
	if (tmp.Length() > 0) {
		BFile *savefile = new BFile(tmp.String(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		savefile->Write(content->String(),content->Length());
		savefile->Unset();
		// put path as default writing path
		tmp.RemoveLast(path.Leaf());
		char *query = sqlite3_mprintf("UPDATE konfiguracja SET p_writepath = %Q WHERE zrobiona = 1", tmp.String());
		sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
		sqlite3_free(query);
	}
}

void beFakPrint::Go(void) {
//	printf("override and do sth meaningful with data\n");
}
