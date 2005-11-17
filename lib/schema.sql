
--- bool aktywne is there to ensure data integrity (if this would be editable)

CREATE TABLE stawka_vat (
	id INTEGER NOT NULL,
	nazwa TEXT NOT NULL DEFAULT '',
	stawka DECIMAL(6,3),
	aktywne INTEGER,
	PRIMARY KEY(id)
);

INSERT INTO stawka_vat (id, nazwa, stawka, aktywne) VALUES (1, '0%', 0, 1);
INSERT INTO stawka_vat (id, nazwa, stawka, aktywne) VALUES (2, '3%', 3, 1);
INSERT INTO stawka_vat (id, nazwa, stawka, aktywne) VALUES (3, '7%', 7, 1);
INSERT INTO stawka_vat (id, nazwa, stawka, aktywne) VALUES (4, '22%', 22, 1);
INSERT INTO stawka_vat (id, nazwa, stawka, aktywne) VALUES (5, 'zwolnione', 0, 1);

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

--- vatid points to stawka_vat table
--- sqlite doesn't enforce constrains, but data should be set conforming by app 

CREATE TABLE towar (
	id INTEGER NOT NULL,
	nazwa TEXT,
	symbol TEXT NOT NULL DEFAULT '',
	pkwiu TEXT,
	jm TEXT,
	usluga INTEGER NOT NULL DEFAULT 0,
	dodany DATE,

	vatid INTEGER,
	netto DECIMAL(12,2),
	zakupu DECIMAL(12,2),
	marza DECIMAL(12,2),
	rabat DECIMAL(12,2),

	notatki TEXT,
	PRIMARY KEY(id,symbol)
);
 
