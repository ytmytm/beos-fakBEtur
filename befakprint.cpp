
// TODO:
//	- rozdzielic text od klasybazowej w miare szybko
//	- wlaczyc slownie do klasy?
//
// ogolne:
//		wygenerowanie podsumowania z rozbiciem na stawki
//		obliczenie sumy do zaplaty
//			- dwie kwerendy sql
//			- jak przekazac liste wynikow do klasy potomnej? wlasna struktura?
// text:
//		generowanie tabelki
//			szerokosci na sztywno dla 80/136
//		RAZEM osobno, zeby moglo zajsc na sume/slownie
//		konfiguracja - liczba kolumn, znak konca linii
//

// podsumowanie:
// tabela pomocnicza:
// CREATE TEMPORARY TABLE sumawydruk ( wnetto DECIMAL(12,2), vatid INTEGER, wvat DECIMAL(12,2), wbrutto DECIMAL(12,2) );
// wstawiac kolejne rekordy
// INSERT INTO sumawydruk (wnetto,vatid,wvat,wbrutto) VALUES ( %Q, %i, %Q, %Q );
// suma z rozbiciem na stawki
// SELECT SUM(wnetto), vat/stawka, SUM(wvat), SUM(wbrutto) FROM sumawydruk GROUP BY vatid ORDER BY vatid/stawka;
// suma calkowita
// SELECT SUM(wnetto), vat/stawka, SUM(wvat), SUM(wbrutto) FROM sumawydruk;
// koniec sesji
// DROP TABLE sumawydruk;

#include "befakprint.h"

#include <stdio.h>

// baseclass with stuff, inherit to print/export/whatever

beFakPrint::beFakPrint(int id, sqlite *db) {

	dbData = db;
	fakturaid = id;
	typ = 0;		// XXX parametr!
	wide = false;	// XXX parametr!
	ncols = wide ? 136 : 80;

	if (id<1) {
		printf("illegal id!\n");
		return;
	}
	// readout stuff
printf("reading stuff for id=%i\n",fakturaid);
	fdata = new fakturadat(db);
	flist = new pozfaklist(db);

	fdata->id = fakturaid;
	fdata->fetch();
	flist->fetch(fdata->id);

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

	printf("stuff in memory, do sth with it\n");
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
printf("sql:[%s]\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
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
printf("at the end call destructor from baseclass\n");

	flist->execSQL("DROP TABLE sumawydruk");
	delete [] fsumma;

	delete flist;
	delete fdata;
}

const char *setki[] = { "", "sto ", "dwieście ", "trzysta ", "czterysta ", "pięćset ", "sześćset ", "siedemset ", "osiemset ", "dziewięćset " };
const char *dziesiatki[] = { "", "", "dwadzieścia ", "trzydzieści ", "czterdzieści ", "pięćdziesiąt ", "sześćdziesiąt", "siedemdziesiąt ", "osiemdziesiąt ", "dziewięćdziesiąt " };
const char *nascie[] = { "dziesięć ", "jedenaście ", "dwanaście ", "trzynaście ", "czternaście ", "piętnaście ", "szesnaście ", "siedemnaście ", "osiemnaście ", "dziewiętnaście " };
const char *jednosci[] = { "", "jeden ", "dwa ", "trzy ", "cztery ", "pięć ", "sześć ", "siedem ", "osiem ", "dziewięć " };

const char *rozbij_tysiac(int val) {
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

const char *slownie(const char *input) {
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

//----------------------
// sample implementation

#define ELINE	"\n"	// znak końca linii

void beFakPrint::Go(void) {
	printf("override and do sth with data, then killyourself\n");
// text dump
BString tmp, out, line, hline, hline2;
int ret;
	out = ""; line="", tmp = "";
	printf("--------------------\n");
	//[1] nazwasprzedawcy .... miejscewyst,datawyst
	line = own[0].String();
	tmp = fdata->ogol[0].String(); tmp += ", "; tmp += fdata->ogol[2];
	line = rightAlign(line, tmp);
	line += ELINE;
	out += line;
	//[2] kodsprz miejscesprz, adres sprz
	line = own[3].String(); line += " "; line += own[4].String();
	line += ", "; line += own[2].String(); line += ELINE;
	out += line;
	//[3] telsprz,  emailsprz
	line = "tel. "; line += own[5].String(); line += ", ";
	line += own[6].String(); line += ELINE;
	out += line;
	//[4] banksprz kontosprz
	line = own[9].String(), line += " "; line += own[10].String(); line += ELINE;
	out += line;
	//[5] [REGON: XXX][ NIP XXX]
	line = "";
	if (own[8].Length()>0) { line += "REGON: "; line += own[8].String(); }
	if (own[7].Length()>0) { if (line.Length()>0) line += ", ";
		line += "NIP: "; line += own[7].String();
	}
	if (line.Length()>0) line += ELINE;
	out += line;
	//[6] [wolna/missing]
	out += ELINE;
	//[7] [wolna]
	out += ELINE;
	//[8] ...Faktura VAT nr XXX....
	line = "Faktura VAT nr "; line += fdata->nazwa.String();
	line = centerAlign(line); line += ELINE;
	out += line;
	//[9] ...[typ faktury]...
	switch (typ) {
		case 2:
			line = "Duplikat";
			break;
		case 1:
			line = "Kopia";
			break;
		case 0:
		default:
			line = "Oryginał";
			break;
	}
	line = centerAlign(line); line += ELINE;
	out += line;
	//[10] [wolna]
	out += ELINE;
	//[11] Nabywca: [nabywca]
	line = "Nabywca: "; line += fdata->odata[0].String(); line += ELINE;
	out += line;
	//[12] Adres: [adresnab], [kodnab] [miejscnab]
	line = "  Adres: "; line += fdata->odata[2].String(); line += ", "; line += fdata->odata[3]; line += " "; line += fdata->odata[4]; line += ELINE;
	out += line;
	//[13]        [telnab], [emailnab]
	line = "         tel. "; line += fdata->odata[5].String(); line += ", "; line += fdata->odata[6]; line += ELINE;
	out += line;
	//[13]        [NIP: [nipnab]], [REGON: [regonab]]
	line = "         ";
	if (fdata->odata[8].Length()>0) { line += "REGON: "; line += fdata->odata[8].String(); }
	if (fdata->odata[7].Length()>0) { if (line.Length()>0) line += ",  ";
		line += "NIP: "; line += fdata->odata[7].String();
	}
	if (line.Length()>0) line += ELINE;
	out += line;
	//[14] [wolna]
	out += ELINE;
	//[15] sposob zaplaty [sposob]...termin zaplaty [termin]
	line = "Sposób zapłaty: "; line += fdata->ogol[5];
	tmp  = "Termin zapłaty: "; tmp += fdata->ogol[6];
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[16] data sprzedazy [data]...srodek transportu [srodek]
	line = "Data sprzedaży: "; line += fdata->ogol[3];
	tmp  = "Środek transp.: "; tmp += fdata->ogol[4];
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[17] [wolna]
	out += ELINE;
	//[] [tabela]
	// [naglowek]
	if (wide) {
	} else {
	   hline = "+--+-------+--------+-------+----+-----+--------+--------+---+--------+--------+"; hline += ELINE;
		out += hline;
		line = "|  |       |        |       |    |     |        |        |   |        |        |"; line += ELINE;
		out += line;
		line = "|Lp| Nazwa | PKWiU  | Ilosc | Jm | Rab.| Cena z | Wartosc|VAT| Wartosc| Wartosc|"; line += ELINE;
		out += line;
		line = "|  |       |        |       |    | (%) |  rab.  |  netto |(%)|   VAT  | brutto |"; line += ELINE;
		out += line;
		out += hline;
	}
	// iteruj po towarach
	pozfakitem *cur = flist->start;
	while (cur!=NULL) {
		line = "|";
		if (wide) {
		} else {
			// lp
			tmp = ""; tmp << cur->lp; line += fitAlignR(tmp,2); line += "|";
			// nazwa
			line += fitAlignL(cur->data->data[1],7); line += "|";
			// pkwiu
			line += fitAlignR(cur->data->data[2],8); line += "|";
			// ilosc
			line += fitAlignR(cur->data->data[3],7); line += "|";
			// jm
			line += fitAlignR(cur->data->data[4],4); line += "|";
			// rabat
			line += fitAlignR(cur->data->data[5],5); line += "|";
			// cenajednostkowa
			line += fitAlignR(cur->data->data[6],8); line += "|";
			// w.netto
			line += fitAlignR(cur->data->data[7],8); line += "|";
			// vat %
			line += fitAlignR(cur->data->data[8],3); line += "|";
			// w.vat
			line += fitAlignR(cur->data->data[9],8); line += "|";
			// w.brutto
			line += fitAlignR(cur->data->data[10],8); line += "|";
		}
		// wstawic podsumowanie do tymczasowej tabeli
		// INSERT INTO sumawydruk (wnetto,vatid,wvat,wbrutto) VALUES ( %Q, %i, %Q, %Q );
		tmp = "INSERT INTO sumawydruk (wnetto,vatid,wvat,wbrutto) VALUES ( %Q, %i, %Q, %Q )";
		ret = sqlite_exec_printf(dbData, tmp.String(), 0, 0, &dbErrMsg,
			cur->data->data[7].String(), cur->data->vatid, cur->data->data[9].String(), cur->data->data[10].String() );
//		printf("result: %i, %s\n", ret, dbErrMsg);
		cur = cur->nxt;
		line += ELINE;
		out += line;
	}
	//[] stopka
	out += hline;
	// podsumuj
	makeSummary();
	//[] wypisac podsumowanie
	for (int i=0;i<fsummarows;i++) {
		if (wide) {
		} else {
			line = "                                                |";
			line += fitAlignR(fsumma[i].summa[0],8); line += "|";
			line += fitAlignR(fsumma[i].summa[1],3); line += "|";
			line += fitAlignR(fsumma[i].summa[2],8); line += "|";
			line += fitAlignR(fsumma[i].summa[3],8); line += "|";
		}
		line += ELINE;
		out += line;
	}
	//[] oddzielenie od podsumowania
	if (wide) {
	} else {
		hline2 = "                                                +--------+---+--------+--------+"; hline2 += ELINE;
	}
	out += hline2;
	//[] RAZEM
	if (wide) {
	} else {
		line = "                                         RAZEM: |";
		line += fitAlignR(razem.summa[0],8); line += "|";
		line += fitAlignR(razem.summa[1],3); line += "|";
		line += fitAlignR(razem.summa[2],8); line += "|";
		line += fitAlignR(razem.summa[3],8); line += "|";
	}
	line += ELINE;
	out += line;
	//[] stopka
	out += hline2;
	//[] wolna
	out += ELINE;
	//[] Do zapłaty: [kwota], lub polaczone z RAZEM |
	line = " Do zapłaty zł: "; line += razem.summa[3]; line += ELINE;
	out += line;
	//[] Słownie: [kwota]     lib polaczone z +---+---+ pod razem
	line = "       Słownie: "; line += slownie(razem.summa[3].String()); line += ELINE;
	out += line;
	//[] wolna x 3
	out += ELINE; out += ELINE; out += ELINE;
	//[] uwagi: [uwagi, multiline, wrap!]
	//[] wystawil: [wystawil]...odebral:
	line = "    wystawił: "; line += fdata->ogol[1];
	tmp  = "odebrał: ";
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[]           ----------           --------------
	line = "              ----------------------";
	tmp  =      "         ----------------------";
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[]       podpis osoby upow.       podpis osoby upow.
	line = "                podpis osoby upow.";
	tmp  =      "           podpis osoby upow.";
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[] wolna
	out += ELINE;
	printf("%s",out.String());
	printf("---------------------\n");
}

const char *beFakPrint::rightAlign(const BString line, const BString right) {
	static BString tmp;
	int j;

	tmp = line;
	j = line.CountChars() + right.CountChars();
	if (j < ncols) {
		j = ncols-j;
		while (j>0) { tmp += " "; j--; };
	}
	tmp += right;
	return tmp.String();
}

const char *beFakPrint::centerAlign(const BString line) {
	static BString tmp;
	int j;

	tmp = "";
	j = ncols/2 - line.CountChars()/2;
	while (j>0) { tmp += " "; j--; }
	tmp += line;
	return tmp.String();
}

const char *beFakPrint::halfAlign(const BString line, const BString right) {
	static BString tmp;
	int j;

	tmp = line;
	j = ncols/2 - line.CountChars();
	while (j>0) { tmp += " "; j--; }
	tmp += right;

	return tmp.String();
}

const char *beFakPrint::fitAlignR(const BString line, int len, bool space = false) {
	static BString tmp;
	int j;

	if (space)
		len--;

	tmp = line;
	tmp.Truncate(len);
	j = tmp.CountChars();
	if (j<len) {
		j = len-j;
		while (j>0) { tmp.Prepend(" "); j--; }
	}
	if (space)
		tmp.Append(" ");
	return tmp.String();
}

const char *beFakPrint::fitAlignL(const BString line, int len, bool space = false) {
	static BString tmp;
	if (space)
		len--;
	tmp = line;
	tmp.Truncate(len);
	if (space)
		tmp.Prepend(" ");
	return tmp.String();
}
