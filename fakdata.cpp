
#include "fakdata.h"

#include <stdio.h>

firmadat::firmadat(sqlite *db) {
	clear();
	dbData = db;
}

void firmadat::dump(void) {
	int i;
	for (i=0;i<=10;i++) {
		printf("%i:[%s] ",i,this->data[i].String());
	}
	printf("\n,odb:%i,dos:%i,akt:%i,zab:%i\n-----\n",odbiorca,dostawca,aktywny,zablokowany);
}

void firmadat::clear(void) {
	id = -1;
	for (int i=0;i<=10;i++) {
		data[i] = "";
	}
	odbiorca = dostawca = aktywny = zablokowany = false;
}

int firmadat::generate_id(void) {
	int newid = 1;
	int nRows, nCols;
	char **result;
	sqlite_get_table(dbData, "SELECT MAX(id) FROM firma", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows > 0) {
		// there is something in db
		newid = toint(result[1]) + 1;
	}
	sqlite_free_table(result);
	return newid;
}

void firmadat::commit(void) {
	BString sql;
	int ret;
printf("commit\n");
	if (id>=0) {	// UPDATE
		sql = "UPDATE firma SET ";
		sql += "nazwa = %Q, symbol = %Q, adres = %Q, kod = %Q, miejscowosc = %Q, telefon = %Q, email = %Q";
		sql += ", nip = %Q, regon = %Q, bank = %Q, konto = %Q";
		sql += ", odbiorca = %i, dostawca = %i, aktywny = %i, zablokowany = %i";
		sql += " WHERE id = %i";
	} else {		// INSERT
		id = generate_id();
		sql += "INSERT INTO firma ( ";
		sql += "nazwa, symbol, adres, kod, miejscowosc, telefon, email";
		sql += ", nip, regon, bank, konto";
		sql += ", odbiorca, dostawca, aktywny, zablokowany";
		sql += ", id ) VALUES ( ";
		sql += "%Q, %Q, %Q, %Q, %Q, %Q, %Q";
		sql += ", %Q, %Q, %Q, %Q";
		sql += ", %i, %i, %i, %i";
		sql += ", %i)";
	}
//printf("sql:[%s]\n",sql.String());
	ret = sqlite_exec_printf(dbData, sql.String(), 0, 0, &dbErrMsg,
		data[0].String(), data[1].String(), data[2].String(), data[3].String(),
		data[4].String(), data[5].String(), data[6].String(), data[7].String(),
		data[8].String(), data[9].String(), data[10].String(),
		odbiorca, dostawca, aktywny, zablokowany,
		id);
	printf("result: %i, %s; id=%i\n", ret, dbErrMsg, id);
}

void firmadat::fetch(void) {
printf("in fetchcurdata with %i\n",id);
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;	
	sql = "SELECT ";
	sql += "nazwa, symbol, adres, kod, miejscowosc, telefon, email";
	sql += ", nip, regon, bank, konto";
	sql += ", odbiorca, dostawca, aktywny, zablokowany";
	sql += " FROM firma WHERE id = ";
	sql << id;
//printf("sql:%s\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
printf ("got:%ix%i\n", nRows, nCols);
	// readout data
	i = nCols;
	for (j=0;j<=10;j++) {
		data[j] = result[i++];
	}
	odbiorca = toint(result[i++]);
	dostawca = toint(result[i++]);
	aktywny = toint(result[i++]);
	zablokowany = toint(result[i++]);

	sqlite_free_table(result);
}

void firmadat::del(void) {
	if (id>=0) {
		sqlite_exec_printf(dbData, "DELETE FROM firma WHERE id = %i", 0, 0, &dbErrMsg, id);
	}
	clear();
}

//----------------------

towardat::towardat(sqlite *db) {
	clear();
	dbData = db;
}

void towardat::dump(void) {
	printf("towardat: id=%i, implement rest\n", id);
}

void towardat::clear(void) {
	id = -1;
	data[0] = data[1] = data[2] = data[3] = notatki = dodany = "";
	usluga = false;
	ceny[0] = ceny[1] = ceny[2] = ceny[3] = "";
	ceny[4] = "1.0"; ceny[5] = "";
	vatid = -1;
}

int towardat::generate_id(void) {
	int newid = 1;
	int nRows, nCols;
	char **result;
	sqlite_get_table(dbData, "SELECT MAX(id) FROM towar", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows > 0) {
		// there is something in db
		newid = toint(result[1]) + 1;
	}
	sqlite_free_table(result);
	return newid;
}

void towardat::commit(void) {
	BString sql;
	int ret;
printf("commit\n");
	if (id>=0) {	// UPDATE
		sql = "UPDATE towar SET ";
		sql += "nazwa = %Q, symbol = %Q, pkwiu = %Q, jm = %Q";
		sql += ", usluga = %i, dodany = date('now'), notatki = %Q, vatid = %i";
		sql += ", netto = %Q, zakupu = %Q, marza = %Q, rabat = %Q, kurs = %Q, clo = %Q";
		sql += " WHERE id = %i";
	} else {		// INSERT
		id = generate_id();
		sql += "INSERT INTO towar ( ";
		sql += "nazwa, symbol, pkwiu, jm";
		sql += ", usluga, dodany, notatki, vatid";
		sql += ", netto, zakupu, marza, rabat";
		sql += ", kurs, clo";
		sql += ", id ) VALUES ( ";
		sql += "%Q, %Q, %Q, %Q";
		sql += ", %i, date('now'), %Q, %i";
		sql += ", %Q, %Q, %Q, %Q";
		sql += ", %Q, %Q";
		sql += ", %i)";
	}
printf("sql:[%s]\n",sql.String());
	ret = sqlite_exec_printf(dbData, sql.String(), 0, 0, &dbErrMsg,
		data[0].String(), data[1].String(), data[2].String(), data[3].String(),
		usluga, notatki.String(), vatid,
		ceny[0].String(), ceny[1].String(), ceny[2].String(), ceny[3].String(),
		ceny[4].String(), ceny[5].String(),
		id);
printf("result: %i, %s; id=%i\n", ret, dbErrMsg, id);
}

void towardat::fetch(void) {
//printf("in fetchcurdata with %i\n",id);
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;	
	sql = "SELECT ";
	sql += "nazwa, symbol, pkwiu, jm";
	sql += ", usluga, dodany, notatki, vatid";
	sql += ", netto, zakupu, marza, rabat, kurs, clo";
	sql += " FROM towar WHERE id = ";
	sql << id;
printf("sql:%s\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	// readout data
	i = nCols;
	for (j=0;j<=3;j++) {
		data[j] = result[i++];
	}
	usluga = toint(result[i++]);
	dodany = result[i++];
	notatki = result[i++];
	vatid = toint(result[i++]);
	for (j=0;j<=5;j++) {
		ceny[j] = result[i++];
	}
	sqlite_free_table(result);
}

void towardat::del(void) {
	if (id>=0) {
		sqlite_exec_printf(dbData, "DELETE FROM towar WHERE id = %i", 0, 0, &dbErrMsg, id);
	}
	clear();
}

//----------------------

#include <stdlib.h>

int toint(const char *input) {
	if (input != NULL)
		return strtol(input, NULL, 10);
	else
		return 0;
}

// not thread-safe!
// zaokraglanie zgodnie z rozporzadzeniem Ministra Finansow z 25.05.2005
// (Dz.U. 2005 nr 95 poz. 798), roz. 4, par 9, pkt. 6
const char *decround(const char *input) {
	static char out[512];
	int i = 0;
	int l = strlen(input);
	char c;

	int z = 0;		// zlote
	int g = 0;		// grosze
	int w = 0;		// wykladnik
	bool grosze = false;

	memset(out,0,sizeof(out));
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
				g = g*10+c-'0';
			else
			if (w==3) {
				if ((c-'0')>=5) {
					g++;
					if (g>=100) { z++, g-=100; }
				}
				break;	// nie interesuja nas cyfry poza .00x
			}
		}
	}
	if (w==1) g*=10;
	sprintf(out, "%i.%02i", z, g);
	return out;
}

//----------------------

fakturadat::fakturadat(sqlite *db) {
	clear();
	dbData = db;
}

void fakturadat::dump(void) {
	printf("fakturadat: id=%i, implement rest\n", id);
}

void fakturadat::clear(void) {
	int i;
	id = -1;
	nazwa = "";
	for (i=0;i<=9;i++)
		ogol[i] = "";
	for (i=0;i<=10;i++)
		odata[i] = "";
	zaplacono = false;
	uwagi = "";
}

int fakturadat::generate_id(void) {
	int newid = 1;
	int nRows, nCols;
	char **result;
	sqlite_get_table(dbData, "SELECT MAX(id) FROM faktura", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows > 0) {
		// there is something in db
		newid = toint(result[1]) + 1;
	}
	sqlite_free_table(result);
	return newid;
}

void fakturadat::commit(void) {
printf("in commit with %i\n",id);
	BString sql;
	int ret;
	if (id>=0) {	// UPDATE
		sql = "UPDATE faktura SET ";
		sql += "nazwa = %Q";
		sql += ", miejsce_wystawienia = %Q, wystawil = %Q, data_wystawienia = %Q, data_sprzedazy = %Q";
		sql += ", srodek_transportu = %Q, sposob_zaplaty = %Q, termin_zaplaty = %Q",
		sql += ", zapl_kwota = %Q, zapl_dnia = %Q, zaplacono = %i, uwagi = %Q";
		sql += ", onazwa = %Q, oadres = %Q, okod = %Q, omiejscowosc = %Q, otelefon = %Q, oemail = %Q";
		sql += ", onip = %Q, oregon = %Q, obank = %Q, okonto = %Q";
		sql += " WHERE id = %i";
	} else {		// INSERT
		id = generate_id();
		sql += "INSERT INTO faktura ( ";
		sql += "nazwa";
		sql += ", miejsce_wystawienia, wystawil, data_wystawienia, data_sprzedazy";
		sql += ", srodek_transportu, sposob_zaplaty, termin_zaplaty";
		sql += ", zapl_kwota, zapl_dnia, zaplacono, uwagi";
		sql += ", onazwa, oadres, okod, omiejscowosc, otelefon, oemail";
		sql += ", onip, oregon, obank, okonto";
		sql += ", id ) VALUES ( ";
		sql += "%Q";
		sql += ", %Q, %Q, %Q, %Q";
		sql += ", %Q, %Q, %Q";
		sql += ", %Q, %Q, %i, %Q";
		sql += ", %Q, %Q, %Q, %Q, %Q, %Q";
		sql += ", %Q, %Q, %Q, %Q";
		sql += ", %i)";
	}
printf("sql:[%s]\n",sql.String());
	ret = sqlite_exec_printf(dbData, sql.String(), 0, 0, &dbErrMsg,
		nazwa.String(), ogol[0].String(), ogol[1].String(), ogol[2].String(), ogol[3].String(),
		ogol[4].String(), ogol[5].String(), ogol[6].String(),
		ogol[8].String(), ogol[9].String(), zaplacono, uwagi.String(),
		odata[0].String(), odata[2].String(), odata[3].String(), odata[4].String(), odata[5].String(), odata[6].String(),
		odata[7].String(), odata[8].String(), odata[9].String(), odata[10].String(),
		id);
printf("result: %i, %s; id=%i\n", ret, dbErrMsg, id);
}

void fakturadat::fetch(void) {
printf("in fetchcurdata with %i\n",id);
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;	
	sql = "SELECT ";
	sql += "nazwa";
	sql += ", miejsce_wystawienia, wystawil, data_wystawienia, data_sprzedazy";
	sql += ", srodek_transportu, sposob_zaplaty, termin_zaplaty";
	sql += ", zapl_kwota, zapl_dnia, zaplacono, uwagi";
	sql += ", onazwa, oadres, okod, omiejscowosc, otelefon, oemail";
	sql += ", onip, oregon, obank, okonto";
	sql += " FROM faktura WHERE id = ";
	sql << id;
printf("sql:%s\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	// readout data
	i = nCols;
	nazwa = result[i++];
	for (j=0;j<=6;j++) {
		ogol[j] = result[i++];
	}
	ogol[8] = result[i++];
	ogol[9] = result[i++];
	zaplacono = toint(result[i++]);
	uwagi = result[i++];
	odata[0] = result[i++];
	for (j=2;j<=10;j++) {
		odata[j] = result[i++];
	}
	sqlite_free_table(result);
}

void fakturadat::del(void) {
	if (id>=0) {
		sqlite_exec_printf(dbData, "DELETE FROM faktura WHERE id = %i", 0, 0, &dbErrMsg, id);
		sqlite_exec_printf(dbData, "DELETE FROM pozycjafakt WHERE fakturaid = %i", 0, 0, &dbErrMsg, id);
	}
	clear();
}
