//
// IDEAS:
//

#include "globals.h"
#include "printhtml.h"
#include "dialfile.h"

#include <Alert.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <stdio.h>

printHTML::printHTML(int id, sqlite3 *db, int numkopii) : beFakPrint(id,db, numkopii) {

}

void printHTML::Go(void) {
	BFile *szablon;
	BString out, tmp, tpl;
	off_t size;
	ssize_t l;
	int i, r;
	char *buf;

	// otworz plik z szablonem
	szablon = new BFile();
	tpl = flist->execSQL("SELECT p_htmltemplate FROM konfiguracja WHERE zrobiona = 1");
	r = szablon->SetTo(tpl.String(), B_READ_ONLY);
	if (r != B_OK) {
		tmp = "Nie znaleziono pliku szablonu:\n"; tmp += tpl;
		tmp += "\nProszę wskazać plik z szablonem eksportu do HTML.";
		BAlert *error = new BAlert(APP_NAME, tmp.String(), "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		error->Go();
		BEntry *ent = dialFile::OpenDialog("Wybierz plik z szablonem HTML", tpl.String());
		BPath path;
		ent->GetPath(&path);
		delete ent;
		tpl = path.Path();
		if (tpl.Length() == 0)
			return;
		// zapisać nową ścieżkę do szablonu
		char *query = sqlite3_mprintf("UPDATE konfiguracja SET p_htmltemplate = %Q WHERE zrobiona = 1", tpl.String());
		sqlite3_exec(dbData, query, 0, 0, &dbErrMsg);
		sqlite3_free(query);
		szablon->SetTo(tpl.String(), B_READ_ONLY);
	}
	// wczytaj szablon
	szablon->GetSize(&size);
	buf = new char[size+1];
	if ((l = szablon->Read((void*)buf,size)) < 0) {
		tmp = "Błąd przy czytaniu pliku szablonu: "; tmp += tpl;
		BAlert *error = new BAlert(APP_NAME, tmp.String(), "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		error->Go();
		return;
	}
	buf[l] = '\0';
	szablon->Unset();
	out = buf;
	delete buf;
	tmp = "";
	// zamień tokeny
	// sprzedawca
	out.ReplaceAll("@SNAZWA@", own[0].String());
	out.ReplaceAll("@SADRES@", own[2].String());
	out.ReplaceAll("@SKOD@", own[3].String());
	out.ReplaceAll("@SMIEJSCE@", own[4].String());
	out.ReplaceAll("@STEL@", own[5].String());
	out.ReplaceAll("@SEMAIL@", own[6].String());
	out.ReplaceAll("@SBANK@", own[9].String());
	out.ReplaceAll("@SKONTO@", own[10].String());
	if (own[8].Length()>0) {
		tmp = "REGON: "; tmp += own[8].String();
		out.ReplaceAll("@SREGON@", tmp.String());
	} else {
		out.ReplaceAll("@SREGON@", "");
	}
	if (own[7].Length()>0) {
		tmp = "NIP: "; tmp += own[7].String();
		out.ReplaceAll("@SNIP@", tmp.String());
	} else {
		out.ReplaceAll("@SNIP@", "");
	}
	// faktura
	out.ReplaceAll("@FNUMER@", fdata->nazwa.String());
	out.ReplaceAll("@FTYP@", typfaktury.String());
	out.ReplaceAll("@FMIEJSCEW@", fdata->ogol[0].String());
	out.ReplaceAll("@FWYSTAWIL@", fdata->ogol[1].String());
	out.ReplaceAll("@FDATAW@", fdata->ogol[2].String());
	out.ReplaceAll("@FDATAS@", fdata->ogol[3].String());
	out.ReplaceAll("@FSRODEKT@", fdata->ogol[4].String());
	out.ReplaceAll("@FSPOSOBZ@", fdata->ogol[5].String());
	out.ReplaceAll("@FTERMINZ@", fdata->ogol[6].String());
	// nabywca
	out.ReplaceAll("@NNAZWA@", fdata->odata[0].String());
	out.ReplaceAll("@NADRES@", fdata->odata[2].String());
	out.ReplaceAll("@NKOD@", fdata->odata[3].String());
	out.ReplaceAll("@NMIEJSCE@", fdata->odata[4].String());
	out.ReplaceAll("@NTEL@", fdata->odata[5].String());
	out.ReplaceAll("@NEMAIL@", fdata->odata[6].String());
	if (fdata->odata[8].Length()>0) {
		tmp = "REGON: "; tmp += fdata->odata[8].String();
		out.ReplaceAll("@NREGON@", tmp.String());
	} else {
		out.ReplaceAll("@NREGON@", "");
	}
	if (fdata->odata[7].Length()>0) {
		tmp = "NIP: "; tmp += fdata->odata[7].String();
		out.ReplaceAll("@NNIP@", tmp.String());
	} else {
		out.ReplaceAll("@NNIP@", "");
	}
	// iteruj po towarach
	pozfakitem *cur = flist->start;
	tmp = "";
	while (cur!=NULL) {
		updateSummary(cur->data->data[7].String(), cur->data->vatid, cur->data->data[9].String(), cur->data->data[10].String());
		tmp += "<tr><td>"; tmp << cur->lp; tmp += "</td>";
		for (i=1;i<=10;i++) {
			if (i>2)
				tmp += "<td align=\"right\">";
			else
				tmp += "<td>";
			tmp += cur->data->data[i]; tmp += "</td>";
		}
		tmp += "</tr>\n";
		cur = cur->nxt;
	}
	// wypisz tabele
	out.ReplaceAll("@TABELA@",tmp.String());
	// oblicz podsumowanie
	makeSummary();
	// wypisz summary
	tmp = "<tr><td rowspan=\""; tmp << fsummarows; tmp+="\" colspan=\"7\"></td>";
	for (i=0;i<fsummarows;i++) {
		tmp += "<td align=\"right\">"; tmp += fsumma[i].summa[0]; tmp += "</td>";
		tmp += "<td align=\"right\">"; tmp += fsumma[i].summa[1]; tmp += "</td>";
		tmp += "<td align=\"right\">"; tmp += fsumma[i].summa[2]; tmp += "</td>";
		tmp += "<td align=\"right\">"; tmp += fsumma[i].summa[3]; tmp += "</td>";
		tmp += "</tr>\n";
	}
	// suma
	tmp += "<tr><th colspan=\"7\" align=\"right\">RAZEM:</th>";
	tmp += "<th align=\"right\">"; tmp += razem.summa[0]; tmp += "</th>";
	tmp += "<th align=\"right\">"; tmp += razem.summa[1]; tmp += "</th>";
	tmp += "<th align=\"right\">"; tmp += razem.summa[2]; tmp += "&nbsp;</th>";
	tmp += "<th align=\"right\">"; tmp += razem.summa[3]; tmp += "</th>";
	tmp += "</tr>\n";
	out.ReplaceAll("@TABSUMA@",tmp.String());
	// podsumowanie	
	out.ReplaceAll("@DOZAPLATY@", razem.summa[3].String());
	out.ReplaceAll("@SLOWNIE@", slownie(razem.summa[3].String()));
	// zapisz HTML do pliku wyjściowego
//printf("----------\n");
//printf("%s\n",out.String());
//printf("----------\n");
	tmp = "faktura-"; tmp += makeName(); tmp += ".html";
	saveToFile(tmp.String(), &out);
}
