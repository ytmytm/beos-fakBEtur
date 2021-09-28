//
// TODO:
//		usunąć niepotrzebne .String()?
// IDEAS:
//		szerokosci,parametry dla 80/136 do osobnej tabeli, indeksowac
//

#include "printtext.h"
#include "globals.h"
#include <stdio.h>

printText::printText(int id, sqlite3 *db, int numkopii) : beFakPrint(id,db,numkopii) {
	wide = ( p_textcols > 80 );
	switch ( p_texteol ) {
		case 2:
			eol = "\r";
			break;
		case 1:
			eol = "\r\n";
			break;
		case 0:
		default:
			eol = "\n";
			break;
	}
}

void printText::Go(void) {
	BString tmp, out, line, hline, hline2;

	out = ""; line="", tmp = "";
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
	line = own[9].String(); line += " "; line += own[10].String(); line += ELINE;
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
	line = centerAlign(typfaktury); line += ELINE;
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
	   hline = "+----+-----------------------------------------+-------------+---------+------+-------+----------+----------+---+----------+----------+"; hline += ELINE;
		out += hline;
		line = "|    |                                         |             |         |      |       |          |          |   |          |          |"; line += ELINE;
		out += line;
		line = "| Lp | Nazwa towaru/uslugi                     |    PKWiU    |  Ilosc  |  Jm  | Rabat |  Cena z  |  Wartosc |VAT|  Wartosc |  Wartosc |"; line += ELINE;
		out += line;
		line = "|    |                                         |             |         |      |  (%)  |  rabatem |    netto |   |    VAT   |   brutto |"; line += ELINE;
		out += line;
		out += hline;
	} else {
	   hline = "+--+-------+--------+-------+----+-----+--------+--------+---+--------+--------+"; hline += ELINE;
		out += hline;
		line = "|  |       |        |       |    |     |        |        |   |        |        |"; line += ELINE;
		out += line;
		line = "|Lp| Nazwa | PKWiU  | Ilosc | Jm | Rab.| Cena z | Wartosc|VAT| Wartosc| Wartosc|"; line += ELINE;
		out += line;
		line = "|  |       |        |       |    | (%) |  rab.  |  netto |   |   VAT  | brutto |"; line += ELINE;
		out += line;
		out += hline;
	}
	// iteruj po towarach
	pozfakitem *cur = flist->start;
	while (cur!=NULL) {
		line = "|";
		if (wide) {
			// lp
			tmp = ""; tmp << cur->lp; line += fitAlignR(tmp,4,true); line += "|";
			// nazwa
			line += fitAlignL(cur->data->data[1],41,true); line += "|";
			// pkwiu
			line += fitAlignR(cur->data->data[2],13,true); line += "|";
			// ilosc
			line += fitAlignR(cur->data->data[3],9,true); line += "|";
			// jm
			line += fitAlignR(cur->data->data[4],6,true); line += "|";
			// rabat
			line += fitAlignR(cur->data->data[5],7,true); line += "|";
			// cenajednostkowa
			line += fitAlignR(cur->data->data[6],10,true); line += "|";
			// w.netto
			line += fitAlignR(cur->data->data[7],10,true); line += "|";
			// vat %
			line += fitAlignR(cur->data->data[8],3); line += "|";
			// w.vat
			line += fitAlignR(cur->data->data[9],10,true); line += "|";
			// w.brutto
			line += fitAlignR(cur->data->data[10],10,true); line += "|";
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
		updateSummary(cur->data->data[7].String(), cur->data->vatid, cur->data->data[9].String(), cur->data->data[10].String());
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
			line = leftFill("|", 97);
			line += fitAlignR(fsumma[i].summa[0],10,true); line += "|";
			line += fitAlignR(fsumma[i].summa[1],3); line += "|";
			line += fitAlignR(fsumma[i].summa[2],10,true); line += "|";
			line += fitAlignR(fsumma[i].summa[3],10,true); line += "|";
		} else {
			line = leftFill("|", 48);
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
		hline2 = leftFill("+----------+---+----------+----------+", 97); hline2 += ELINE;
	} else {
		hline2 = leftFill("+--------+---+--------+--------+", 48); hline2 += ELINE;
	}
	out += hline2;
	//[] RAZEM
	if (wide) {
		line = leftFill("RAZEM: |", 97-7);
		line += fitAlignR(razem.summa[0],10,true); line += "|";
		line += fitAlignR(razem.summa[1],3); line += "|";
		line += fitAlignR(razem.summa[2],10,true); line += "|";
		line += fitAlignR(razem.summa[3],10,true); line += "|";
	} else {
		line = leftFill("RAZEM: |", 48-7);
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
//printf("---------------------\n");
//printf("%s",out.String());
//printf("---------------------\n");
	tmp = "faktura-"; tmp += makeName(); tmp += ".txt";
	saveToFile(tmp.String(), &out);
}

const char *printText::rightAlign(const BString line, const BString right) {
	static BString tmp;
	int j;

	tmp = line;
	j = line.CountChars() + right.CountChars();
	if (j < p_textcols) {
		j = p_textcols-j;
		while (j>0) { tmp += " "; j--; };
	}
	tmp += right;
	return tmp.String();
}

const char *printText::centerAlign(const BString line) {
	static BString tmp;
	int j;

	tmp = "";
	j = p_textcols/2 - line.CountChars()/2;
	while (j>0) { tmp += " "; j--; }
	tmp += line;
	return tmp.String();
}

const char *printText::halfAlign(const BString line, const BString right) {
	static BString tmp;
	int j;

	tmp = line;
	j = p_textcols/2 - line.CountChars();
	while (j>0) { tmp += " "; j--; }
	tmp += right;

	return tmp.String();
}

const char *printText::fitAlignR(const BString line, int len, bool space) {
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

const char *printText::fitAlignL(const BString line, int len, bool space) {
	static BString tmp;
	int j;

	tmp = line;
	if (space)
		tmp.Prepend(" ");
	tmp.Truncate(len);
	j = len-tmp.CountChars();
	while (j>0) { tmp.Append(" "); j--; }
	return tmp.String();
}

const char *printText::leftFill(const BString line, int spaces) {
	static BString tmp;
	int j;

	tmp = "";
	j = spaces;
	while (j>0) { tmp.Append(" "); j--; }
	tmp += line;
	return tmp.String();
}
