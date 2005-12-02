
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
	kurs DECIMAL(12,2),
	clo DECIMAL(12,2),

	notatki TEXT,
	PRIMARY KEY(id,symbol)
);
 

CREATE TABLE faktura (
	id			INTEGER NOT NULL,
	nazwa			TEXT,

	data_wystawienia	DATE,
	miejsce_wystawienia	TEXT,
	data_sprzedazy		DATE,

	termin_zaplaty		DATE,
	sposob_zaplaty		TEXT,

	srodek_transportu	TEXT,

	wystawil		TEXT,
	uwagi			TEXT,

	zaplacono		TEXT,
	zapl_dnia		TEXT,
	zapl_kwota		TEXT,

	--- odbiorca - kopia struktury z firmy
	onazwa			TEXT,
	oadres			TEXT,
	okod			TEXT,
	omiejscowosc		TEXT,
	otelefon		TEXT,
	oemail			TEXT,
	onip			TEXT,
	oregon			TEXT,
	obank			TEXT,
	okonto			TEXT,

	PRIMARY KEY(id)
);

CREATE TABLE pozycjafakt (
	id		INTEGER NOT NULL,
	fakturaid	INTEGER NOT NULL,
	lp		INTEGER NOT NULL,
	ilosc		DECIMAL(12,2) NOT NULL,

	--- towar - kopia struktury z towar
	nazwa		TEXT,
	pkwiu		TEXT,
	jm		TEXT,

	vatid		INTEGER,
	netto		DECIMAL(12,2),
	rabat		DECIMAL(12,2),

	PRIMARY KEY(id,fakturaid)
);

CREATE TABLE konfiguracja (
--- dane firmy
	nazwa TEXT,
	adres TEXT,
	kod TEXT,
	miejscowosc TEXT,
	telefon TEXT,
	email TEXT,
	nip TEXT,
	regon TEXT,
	bank TEXT,
	konto TEXT,
--- konfiguracja wydruku
	liczbakopii INTEGER NOT NULL DEFAULT 1,
--- konfiguracja faktur
	ostatni_nr INTEGER,
	num_prosta INTEGER,
--- marker, potrzebny w ogole?
	zrobiona INTEGER DEFAULT 1
);

