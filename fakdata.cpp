//
// obliczanie:
// 1. w.brutto = round(ilość*c.brutto) = round(ilość*round(c.netto*stawka)) = 3.23
// 2. w.brutto = round(stawka*ilość*c.netto) = 3.24 (3.235)
//
// 2. jest poprawny (wtedy w.netto*stawka = w.brutto!)
// kwota vat = wbrutto-wnetto

#include "fakdata.h"

#include <stdio.h>

firmadat::firmadat(sqlite3 *db) : dbdat(db) {
	clear();
}

void firmadat::clear(void) {
	id = -1;
	for (int i=0;i<=10;i++) {
		data[i] = "";
	}
	odbiorca = dostawca = zablokowany = false;
	aktywny = true;
}

int firmadat::generate_id(void) {
	int newid = 1;
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, "SELECT MAX(id) FROM firma", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows > 0) {
		// there is something in db
		newid = toint(result[1]) + 1;
	}
	sqlite3_free_table(result);
	return newid;
}

void firmadat::commit(void) {
	BString sql;
	int ret;
//printf("commit\n");
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
	char *query = sqlite3_mprintf(sql.String(), 
		data[0].String(), data[1].String(), data[2].String(), data[3].String(),
		data[4].String(), data[5].String(), data[6].String(), data[7].String(),
		data[8].String(), data[9].String(), data[10].String(),
		odbiorca, dostawca, aktywny, zablokowany,
		id);
	ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
	sqlite3_free(query);
//printf("result: %i, %s; id=%i\n", ret, dbErrMsg, id);
}

void firmadat::fetch(void) {
//printf("in fetchcurdata with %i\n",id);
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
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i\n", nRows, nCols);
	// readout data
	i = nCols;
	for (j=0;j<=10;j++) {
		data[j] = result[i++];
	}
	odbiorca = toint(result[i++]);
	dostawca = toint(result[i++]);
	aktywny = toint(result[i++]);
	zablokowany = toint(result[i++]);

	sqlite3_free_table(result);
}

void firmadat::del(void) {
	if (id>=0) {
		char *query = sqlite3_mprintf("DELETE FROM firma WHERE id = %i", id);
		sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
		sqlite3_free(query);
	}
	clear();
}

//----------------------

towardat::towardat(sqlite3 *db) : dbdat(db) {
	clear();
}

void towardat::clear(void) {
	id = -1;
	data[0] = data[1] = data[2] = data[3] = notatki = dodany = magzmiana = "";
	usluga = false;
	ceny[0] = ceny[1] = ceny[2] = ceny[3] = "";
	ceny[4] = "1.00"; ceny[5] = "";
	vatid = -1;
	magazyn = "0.00";
}

int towardat::generate_id(void) {
	int newid = 1;
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, "SELECT MAX(id) FROM towar", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows > 0) {
		// there is something in db
		newid = toint(result[1]) + 1;
	}
	sqlite3_free_table(result);
	return newid;
}

void towardat::commit(void) {
	BString sql;
	int ret;
//printf("commit\n");
	if (id>=0) {	// UPDATE
		sql = "UPDATE towar SET ";
		sql += "nazwa = %Q, symbol = %Q, pkwiu = %Q, jm = %Q";
		sql += ", usluga = %i, dodany = date('now'), notatki = %Q, vatid = %i";
		sql += ", netto = %Q, zakupu = %Q, marza = %Q, rabat = %Q, kurs = %Q, clo = %Q";
		sql += ", magazyn = %Q, magzmiana = date('now')";
		sql += " WHERE id = %i";
	} else {		// INSERT
		id = generate_id();
		sql += "INSERT INTO towar ( ";
		sql += "nazwa, symbol, pkwiu, jm";
		sql += ", usluga, dodany, notatki, vatid";
		sql += ", netto, zakupu, marza, rabat";
		sql += ", kurs, clo";
		sql += ", magazyn, magzmiana";
		sql += ", id ) VALUES ( ";
		sql += "%Q, %Q, %Q, %Q";
		sql += ", %i, date('now'), %Q, %i";
		sql += ", %Q, %Q, %Q, %Q";
		sql += ", %Q, %Q";
		sql += ", %Q, date('now')";
		sql += ", %i)";
	}
//printf("sql:[%s]\n",sql.String());
	char *query = sqlite3_mprintf(sql.String(),	
		data[0].String(), data[1].String(), data[2].String(), data[3].String(),
		usluga, notatki.String(), vatid,
		ceny[0].String(), ceny[1].String(), ceny[2].String(), ceny[3].String(),
		ceny[4].String(), ceny[5].String(),
		magazyn.String(), id);
	ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
	sqlite3_free(query);
//printf("result: %i, %s; id=%i\n", ret, dbErrMsg, id);
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
	sql += ", magazyn, magzmiana";
	sql += " FROM towar WHERE id = ";
	sql << id;
//printf("sql:%s\n",sql.String());
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
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
	magazyn = result[i++];
	magzmiana = result[i++];
	sqlite3_free_table(result);
}

void towardat::del(void) {
	if (id>=0) {
		char *query = sqlite3_mprintf("DELETE FROM towar WHERE id = %i", id);
		sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
		sqlite3_free(query);
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

#include <time.h>
#include <parsedate.h>

int calcdaysago(const char *olddate) {
	time_t ago;
	double secs;
	int days;

	ago = parsedate(olddate, -1);
	secs = difftime(time(NULL), ago);
	days = int(secs/(60*60*24));
	return days;
}

const char *daysagostring(int days) {
	static char result[11];
	time_t cur;
	struct tm t;

	cur = time(NULL);
	cur -= 60*60*24*days;
	localtime_r(&cur,&t);
	strftime(result,sizeof(result),"%F",&t);
	return result;
}

//----------------------

fakturadat::fakturadat(sqlite3 *db) : dbdat(db) {
	clear();
}

void fakturadat::clear(void) {
	int i;
	id = -1;
	nazwa = "";
	for (i=0;i<=9;i++)
		ogol[i] = "";
	for (i=0;i<=10;i++)
		odata[i] = "";
	uwagi = "";
}

int fakturadat::generate_id(void) {
	int newid = 1;
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, "SELECT MAX(id) FROM faktura", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows > 0) {
		// there is something in db
		newid = toint(result[1]) + 1;
	}
	sqlite3_free_table(result);
	return newid;
}

void fakturadat::commit(void) {
//printf("in commit with %i\n",id);
	BString sql;
	int ret;
	if (id>=0) {	// UPDATE
		sql = "UPDATE faktura SET ";
		sql += "nazwa = %Q";
		sql += ", miejsce_wystawienia = %Q, wystawil = %Q, data_wystawienia = %Q, data_sprzedazy = %Q";
		sql += ", srodek_transportu = %Q, sposob_zaplaty = %Q, termin_zaplaty = %Q",
		sql += ", zapl_kwota = %Q, zapl_dnia = %Q, uwagi = %Q";
		sql += ", onazwa = %Q, oadres = %Q, okod = %Q, omiejscowosc = %Q, otelefon = %Q, oemail = %Q";
		sql += ", onip = %Q, oregon = %Q, obank = %Q, okonto = %Q";
		sql += " WHERE id = %i";
	} else {		// INSERT
		id = generate_id();
		sql += "INSERT INTO faktura ( ";
		sql += "nazwa";
		sql += ", miejsce_wystawienia, wystawil, data_wystawienia, data_sprzedazy";
		sql += ", srodek_transportu, sposob_zaplaty, termin_zaplaty";
		sql += ", zapl_kwota, zapl_dnia, uwagi";
		sql += ", onazwa, oadres, okod, omiejscowosc, otelefon, oemail";
		sql += ", onip, oregon, obank, okonto";
		sql += ", id ) VALUES ( ";
		sql += "%Q";
		sql += ", %Q, %Q, %Q, %Q";
		sql += ", %Q, %Q, %Q";
		sql += ", %Q, %Q, %Q";
		sql += ", %Q, %Q, %Q, %Q, %Q, %Q";
		sql += ", %Q, %Q, %Q, %Q";
		sql += ", %i)";
	}
//printf("sql:[%s]\n",sql.String());
	char *query = sqlite3_mprintf(sql.String(),		
		nazwa.String(), ogol[0].String(), ogol[1].String(), ogol[2].String(), ogol[3].String(),
		ogol[4].String(), ogol[5].String(), ogol[6].String(),
		ogol[8].String(), ogol[9].String(), uwagi.String(),
		odata[0].String(), odata[2].String(), odata[3].String(), odata[4].String(), odata[5].String(), odata[6].String(),
		odata[7].String(), odata[8].String(), odata[9].String(), odata[10].String(),
		id);
	ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
	sqlite3_free(query);
//printf("result: %i, %s; id=%i\n", ret, dbErrMsg, id);
}

void fakturadat::fetch(void) {
//printf("in fetchcurdata with %i\n",id);
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;	
	sql = "SELECT ";
	sql += "nazwa";
	sql += ", miejsce_wystawienia, wystawil, data_wystawienia, data_sprzedazy";
	sql += ", srodek_transportu, sposob_zaplaty, termin_zaplaty";
	sql += ", zapl_kwota, zapl_dnia, uwagi";
	sql += ", onazwa, oadres, okod, omiejscowosc, otelefon, oemail";
	sql += ", onip, oregon, obank, okonto";
	sql += " FROM faktura WHERE id = ";
	sql << id;
//printf("sql:%s\n",sql.String());
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	// readout data
	i = nCols;
	nazwa = result[i++];
	for (j=0;j<=6;j++) {
		ogol[j] = result[i++];
	}
	ogol[8] = result[i++];
	ogol[9] = result[i++];
	uwagi = result[i++];
	odata[0] = result[i++];
	for (j=2;j<=10;j++) {
		odata[j] = result[i++];
	}
	sqlite3_free_table(result);
}

void fakturadat::del(void) {
	if (id>=0) {
		char *query = sqlite3_mprintf("DELETE FROM faktura WHERE id = %i", id);
		sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
		sqlite3_free(query);
		query = sqlite3_mprintf("DELETE FROM pozycjafakt WHERE fakturaid = %i", id);
		sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
		sqlite3_free(query);
	}
	clear();
}

//----------------------

pozfakitem::pozfakitem(pozfakdata *curdata, pozfakitem *prev, pozfakitem *next) {
	data = curdata;
	nxt = next;
	prv = prev;
	// alloc and/or init data
//	printf("init\n");
	lp = 0;
}

pozfaklist::pozfaklist(sqlite3 *db) {
//	printf("constr\n");
	dbData = db;
	start = NULL;
	end = start;
}

pozfaklist::~pozfaklist() {
	clear();
}

void pozfaklist::clear(void) {
	pozfakitem *nxt, *cur = start;

	while (cur!=NULL) {
		nxt = cur->nxt;
		delete cur;
		cur = nxt;
	}
	start = NULL;
	end = start;
}

void pozfaklist::setlp(void) {
	pozfakitem *cur = start;
	int i = 1;
	while (cur!=NULL) {
		cur->lp = i++;
		cur = cur->nxt;
	}
}

void pozfaklist::addlast(pozfakdata *data) {
	pozfakitem *newone = new pozfakitem(data,end,NULL);
	if (end != NULL)
		end->nxt = newone;
	else
		start = newone;
	end = newone;
}

void pozfaklist::addfirst(pozfakdata *data) {
	pozfakitem *newone = new pozfakitem(data,NULL,start);
	if (start != NULL)
		start->prv = newone;
	else
		end = newone;
	start = newone;
}

void pozfaklist::addafter(pozfakdata *data, pozfakitem *afterme) {
	pozfakitem *newone = new pozfakitem(data,afterme,afterme->nxt);
	if (afterme != NULL) {
		if (afterme->nxt != NULL)
			afterme->nxt->prv = newone;
		afterme->nxt = newone;
	}
}

void pozfaklist::addafter(pozfakdata *data, int offset) {
	pozfakitem *cur = start;
	offset--;
	while ((offset>0) && (cur!=NULL)) {
		offset--;
//		printf("skip [%i], %s\n", offset, cur->data->data[1].String());
		cur = cur->nxt;
	}
	if (cur!=NULL)
			addafter(data, cur);
}

void pozfaklist::remove(int offset) {
	pozfakitem *cur = start;
	offset--;
	while ((offset>0) && (cur!=NULL)) {
		offset--;
//		printf("skip [%i], %s\n", offset, cur->data->data[1].String());
		cur = cur->nxt;
	}
	if (cur != NULL) {
		if (cur->prv != NULL)
			cur->prv->nxt = cur->nxt;
		else
			start = cur->nxt;
		if (cur->nxt != NULL)
			cur->nxt->prv = cur->prv;
		else
			end = cur->nxt;
		delete cur;
	}
}

pozfakdata *pozfaklist::itemat(int offset) {
	pozfakitem *cur = start;
	offset--;
	while ((offset>0) && (cur!=NULL)) {
		offset--;
//		printf("skip [%i], %s\n", offset, cur->data->data[1].String());
		cur = cur->nxt;
	}
	if (cur != NULL)
		return cur->data;
	return NULL;
}

// ----------------- db stuff below

int pozfaklist::generate_id(void) {
	int newid = 1;
	int nRows, nCols;
	char **result;
	sqlite3_get_table(dbData, "SELECT MAX(id) FROM pozycjafakt", &result, &nRows, &nCols, &dbErrMsg);
	if (nRows > 0) {
		// there is something in db
		newid = toint(result[1]) + 1;
	}
	sqlite3_free_table(result);
	return newid;
}

void pozfaklist::commit(int fakturaid) {
	int ret;
	/// remove all existing and insert new? convenient!
	char *query = sqlite3_mprintf("DELETE FROM pozycjafakt WHERE fakturaid = %i", fakturaid);
	ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
	sqlite3_free(query);
//printf("result: %i, %s;\n", ret, dbErrMsg);

	// iterate through list, commit items
	pozfakitem *cur = start;
	while (cur!=NULL) {
		commititem(fakturaid, cur);
		cur = cur->nxt;
	}
}

void pozfaklist::commititem(int fakturaid, pozfakitem *item) {
	BString sql;
	pozfakdata *data = item->data;
	int ret, id;
	/// XXX really INSERT only?
	id = generate_id();
	sql = "INSERT INTO pozycjafakt ( ";
	sql += "id, lp, ilosc";
	sql += ", nazwa, pkwiu, jm, vatid, netto, rabat";
	sql += ", fakturaid ) VALUES ( ";
	sql += "%i, %i, %Q";
	sql += ", %Q, %Q, %Q, %i, %Q, %Q";
	sql += ", %i )";
//printf("commit for %i #%i\n", id, fakturaid);
//printf("sql:[%s]\n",sql.String());
	char *query = sqlite3_mprintf(sql.String(),
		id, item->lp, data->data[3].String(),
		data->data[1].String(), data->data[2].String(), data->data[4].String(),
		data->vatid, data->data[11].String(), data->data[5].String(), fakturaid);
	ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
	sqlite3_free(query);
//printf("result: %i, %s; id=%i\n", ret, dbErrMsg, id);
}

void pozfaklist::fetch(int fakturaid) {
///printf("fetchpozfak id=%i\n", fakturaid);
	pozfakdata *data;
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql, cbrutto;	

	// clear current list!
	clear();

	sql = "SELECT ";
	sql += "id, lp, ilosc";
	sql += ", nazwa, pkwiu, jm, vatid, netto, rabat";
	sql += " FROM pozycjafakt WHERE fakturaid = ";
	sql << fakturaid;
	sql += " ORDER BY lp";
//printf("sql:%s\n",sql.String());
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows < 1)
		return;
	// readout data
	i = nCols;
	j = 1;
	while (j <= nRows) {
		data = new pozfakdata();
		i++;							// id, unused
		data->data[0] = result[i++];	// lp, unused
		data->data[3] = result[i++];	// ilosc
		data->data[1] = result[i++];	// nazwa
		data->data[2] = result[i++];	// pkwiu
		data->data[4] = result[i++];	// jm
		data->vatid = toint(result[i++]); //vatid
		data->data[11] = result[i++];	// c.netto
		data->data[5] = result[i++];	// rabat
		addlast(data);
		// 6, 7, 8, 9, 10 - cjednost, w.netto, vat, wvat, wbrutto
		int nCols;
		char **result = calcBrutto(data->data[11].String(), data->data[5].String(), data->data[3].String(), data->vatid, &nCols);
		if (nCols<1) {
			data->data[6] = data->data[7] = data->data[9] = data->data[10] = "";
		} else {
			data->data[6] = result[nCols+0];
			data->data[7] = result[nCols+3];
			data->data[9] = result[nCols+4];
			data->data[10] = result[nCols+5];
		}
		calcBruttoFin(result);
		// vat = stawka
		sql = "SELECT nazwa FROM stawka_vat WHERE id = "; sql << data->vatid;
		data->data[8] = execSQL(sql.String());
		j++;							// next row
	}
	sqlite3_free_table(result);
	setlp();							// reset lp
}

// in->cnetto, rabat, ilosc, idstawka
// out->cnettojednostkowa, cbrutto, ilosc, wnetto, wvat, wbrutto
char **pozfaklist::calcBrutto(const char *cnetto, const char *rabat, const char *ilosc, const int vatid, int *retcols) {
	BString sql, cjednost;
	int ret;
	int nRows, nCols;
	char **result;

	sql = "SELECT DECROUND(0"; sql += cnetto; sql += "*(100-0";
	sql += rabat; sql += ")/100.0)";
	cjednost = execSQL(sql.String());				// cnettojednostkowa

	sql = "CREATE TEMPORARY TABLE calcs ( cnetto, ilosc, vatid );";
	sql += "INSERT INTO calcs (cnetto, ilosc, vatid) VALUES (%Q, %Q, %i);";
	char *query = sqlite3_mprintf(sql.String(), cjednost.String(), ilosc, vatid);
	ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
	sqlite3_free(query);
//printf("got:%i,[%s]\n",ret,dbErrMsg);
	sql = "SELECT ";
	// [0] - cnettojednostkowa (po rabacie)
	sql += "cnetto AS cnetto";
	// [1] - cbrutto = cnetto*(1+stawka)
	sql += ", DECROUND(cnetto*(100+stawka)/100.0) AS cbrutto";
	// [2] - ilosc
	sql += ", ilosc";
	// [3] - wnetto = cnetto*ilosc
	sql += ", DECROUND(cnetto*ilosc) AS wnetto";
	// [4] - wvat = wbrutto-wnetto = cnetto*ilosc*(1+stawka) - cnetto*ilosc
	sql += ", DECROUND(DECROUND(DECROUND(cnetto*ilosc)*(100+s.stawka)/100.0)-DECROUND(cnetto*ilosc)) AS wvat";
	// [5] - wbrutto = cnetto*ilosc*(1+stawka)
	sql += ",          DECROUND(DECROUND(cnetto*ilosc)*(100+s.stawka)/100.0) AS wbrutto";
	sql += " FROM calcs, stawka_vat AS s WHERE s.id = vatid";
//printf("sql:[%s]\n",sql.String());
	sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows<1)
		*retcols = 0;
	else
		*retcols = nCols;
	return result;
}

void pozfaklist::calcBruttoFin(char **result) {
	sqlite3_free_table(result);
	execSQL("DROP TABLE calcs");
}

const char *pozfaklist::calcSumPayment(void) {
	static BString result;
	int ret;

	execSQL("CREATE TEMPORARY TABLE sumpayment ( cbrutto )");

	pozfakitem *cur = start;

	while (cur!=NULL) {
		char *query = sqlite3_mprintf("INSERT INTO sumpayment (cbrutto) VALUES (%Q);", cur->data->data[10].String());
		ret = sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
		sqlite3_free(query);
		cur = cur->nxt;
	}
	result = execSQL("SELECT DECROUND(SUM(cbrutto)) FROM sumpayment");
	execSQL("DROP TABLE sumpayment");
	return result.String();
}

void pozfaklist::updateStorage(int fakturaid) {
	BString sql, nazwa, magazyn, stare;
	pozfakitem *cur = start;
	int ret;
	int nRows, nCols;
	char **result;
	bool usluga;

	while (cur!=NULL) {
		// get state
		nazwa = cur->data->data[1]; nazwa.ReplaceAll("'","''"); nazwa.Prepend("'"); nazwa.Append("'");
		sql = "SELECT usluga,magazyn FROM towar WHERE nazwa = "; sql += nazwa;
		sqlite3_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
		if (nRows<1) {
			// nie ma takiego w bazie, nic nie robic
		} else {
			// usluga?
			usluga = toint(result[nCols+0]);
//			printf("usluga:%i\n", usluga);
			if (!usluga) {
				// calc new magazyn state
				magazyn = result[nCols+1];
//				printf("[%s]:mag:%s, nowe:%s\n",nazwa.String(),magazyn.String(),cur->data->data[3].String());
				if (fakturaid > 0) {
					// get old state
					sql = "SELECT ilosc FROM pozycjafakt WHERE fakturaid = "; sql << fakturaid;
					sql += " AND nazwa = "; sql += nazwa;
					stare = execSQL(sql.String());
				} else {
					stare = "0";
				}
				sql = "SELECT 0"; sql += stare; sql += "+0"; sql += magazyn; sql += "-0"; sql += cur->data->data[3];
//printf("sql=[%s],magazyn=[%s]\n",sql.String(),execSQL(sql.String()));
				magazyn = execSQL(sql.String());
//				printf("nowy mag[%s]: [%s]\n",nazwa.String(), magazyn.String());
				// update magazyn state (note that nazwa() is already quoted)
				char *query = sqlite3_mprintf("UPDATE towar SET magazyn = %Q, magzmiana = DATE('now') WHERE nazwa = %s",
					 magazyn.String(), nazwa.String());
				ret = sqlite3_exec(dbData, query,	0, 0, &dbErrMsg);
				sqlite3_free(query);
//				printf("got:%i,%s\n",ret,dbErrMsg);
			}
		}
		sqlite3_free_table(result);
		cur = cur->nxt;
	}
}

// XXX this is duplicated in befaktab!
const char *pozfaklist::execSQL(const char *input) {
	int nRows, nCols;
	char **result;
	static BString res;
//printf("sql=[%s]\n",sql.String());
	sqlite3_get_table(dbData, input, &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i, %s\n", nRows, nCols, dbErrMsg);
	if (nRows < 1)
		res = "";
	else
		res = result[1];
	sqlite3_free_table(result);
	return res.String();
}
