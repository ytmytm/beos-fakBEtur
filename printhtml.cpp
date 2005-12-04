//
// TODO:
//		wybor szablonu
//		zapis do pliku
// - NetPositive file &
// - sleep 1
// - hey NetPositive do MenuItem [8] of Menu [0] of View [0] of Window [0]
//
// IDEAS:
//		konfiguracja - nazwa pliku szablonu
//

#include "printhtml.h"

#include <File.h>
#include <stdio.h>

// XXX parametr
#define SZABLON_PATH "/boot/home/vatszablon.html"

printHTML::printHTML(int id, sqlite *db) : beFakPrint(id,db) {
}

void printHTML::Go(void) {
	BFile *szablon;
	BString out, tmp, leftab;
	off_t size;
	ssize_t l;
	int i, r;
	char *buf;

	// otworz plik z szablonem
	szablon = new BFile();
	r = szablon->SetTo(SZABLON_PATH, B_READ_ONLY);
	if (r != B_OK) {
		printf("nie ma szablonu???\n");
		return;
	}
	// wczytaj szablon
	szablon->GetSize(&size);
	buf = new char[size+1];
	if ((l = szablon->Read((void*)buf,size)) < 0) {
		printf("read error\n");
		return;
	}
	buf[l] = '\0';
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
		tmp += "NIP: "; tmp += own[7].String();
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
			tmp += "<td>"; tmp += cur->data->data[i]; tmp += "</td>";
		}
		tmp += "</tr>\n";
		cur = cur->nxt;
	}
	// wypisz tabele
	out.ReplaceAll("@TABELA@",tmp.String());
	// oblicz podsumowanie
	makeSummary();
	// wypisz summary
	leftab = "<tr><td></td><td></td><td></td><td></td><td></td><td></td>";
	tmp = "";
	for (i=0;i<fsummarows;i++) {
		tmp += leftab; tmp += "<td></td>";
		tmp += "<td>"; tmp += fsumma[i].summa[0]; tmp += "</td>";
		tmp += "<td>"; tmp += fsumma[i].summa[1]; tmp += "</td>";
		tmp += "<td>"; tmp += fsumma[i].summa[2]; tmp += "</td>";
		tmp += "<td>"; tmp += fsumma[i].summa[3]; tmp += "</td>";
		tmp += "</tr>\n";
	}
	// suma
	tmp += leftab; tmp += "<strong><td>RAZEM:</td></strong>";
	tmp += "<td>"; tmp += razem.summa[0]; tmp += "</td>";
	tmp += "<td>"; tmp += razem.summa[1]; tmp += "</td>";
	tmp += "<td>"; tmp += razem.summa[2]; tmp += "</td>";
	tmp += "<td>"; tmp += razem.summa[3]; tmp += "</td>";
	tmp += "</tr>\n";
	out.ReplaceAll("@TABSUMA@",tmp.String());
	// podsumowanie	
	out.ReplaceAll("@DOZAPLATY@", razem.summa[3].String());
	out.ReplaceAll("@SLOWNIE@", slownie(razem.summa[3].String()));
	// zapisz HTML do pliku wyjściowego
printf("----------\n");
printf("%s\n",out.String());
printf("----------\n");
	// wywołaj hey?
}
