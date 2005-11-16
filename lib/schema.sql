
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
	symbol TEXT NOT NULL DEFAULT '',
	nazwa TEXT,
	adres TEXT,
	kod TEXT,
	miasto TEXT,
	telefon TEXT,
	email TEXT,
	nip TEXT,
	bank TEXT,
	konto TEXT,

	aktywny INTEGER,
	blokada INTEGER,
	odbiorca INTEGER,
	dostawca INTEGER,
	PRIMARY KEY(id,symbol)
);
 
