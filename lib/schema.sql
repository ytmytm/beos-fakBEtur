
CREATE TABLE stawka_vat (
	id INTEGER NOT NULL,
	nazwa TEXT NOT NULL DEFAULT '',
	stawka INTEGER,
	PRIMARY KEY(id)
);

INSERT INTO stawka_vat (id, nazwa, stawka) VALUES (1, '0%', 0);
INSERT INTO stawka_vat (id, nazwa, stawka) VALUES (2, '3%', 3);
INSERT INTO stawka_vat (id, nazwa, stawka) VALUES (3, '7%', 7);
INSERT INTO stawka_vat (id, nazwa, stawka) VALUES (4, '22%', 22);
INSERT INTO stawka_vat (id, nazwa, stawka) VALUES (5, 'zwolnione', 0);

CREATE TABLE firma (
	id INTEGER NOT NULL,
	nazwa TEXT,
	symbol TEXT NOT NULL DEFAULT '',
	adres TEXT,
	kod TEXT,
	miejscowosc TEXT,
	telefon TEXT,
	email TEXT,
	nip TEXT,
	regon TEXT,
	bank TEXT,
	konto TEXT,

	odbiorca INTEGER,
	dostawca INTEGER,
	aktywny INTEGER,
	zablokowany INTEGER,
	PRIMARY KEY(id,symbol)
);

CREATE TABLE towar (
	id INTEGER NOT NULL,
	nazwa TEXT,
	symbol TEXT NOT NULL DEFAULT '',
	pkwiu TEXT,
	jm TEXT,
	usluga INTEGER NOT NULL DEFAULT 0,
	dodany DATE,

	netto INTEGER,
	vat INTEGER,
	zakupu INTEGER,
	marza INTEGER,
	rabat INTEGER,

	notatki TEXT,
	PRIMARY KEY(id,symbol)
);
 
