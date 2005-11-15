
CREATE TABLE karta (
	id 		INTEGER NOT NULL,

	t1miejscowosc	TEXT NOT NULL,
	t1nazwalokalna	TEXT,
	t1gmina		TEXT,
	t1powiat	TEXT,
	t1wojewodztwo	TEXT,
	t1nrobszaru	TEXT,
	t1nrinwentarza	TEXT,
	t1x		TEXT,
	t1y		TEXT,
	t1nrstanmiejsc	TEXT,
	t1nrstanobszar	TEXT,
	t1zrodloinformacji	INTEGER,

	t2nadmorska		INTEGER,
	t2duzedoliny	INTEGER,
	t2maledoliny	INTEGER,
	t2pozadolinami	INTEGER,

	t2ekswys		INTEGER,
	t2eksstop		INTEGER,
	t2ekskier		INTEGER,
	t2ekspozycja	INTEGER,
	t2ekspozycja2	INTEGER,
	t2forma		TEXT,
---
	t3zabudowa		INTEGER,
	t3rodzaj		INTEGER,
	t3okreslenie	TEXT,

	t5gleba			INTEGER,
	t5kamienie		INTEGER,
	t5okreslenie	TEXT,

	t6obserwacja	INTEGER,
	t6pole			INTEGER,
	t6nasycenie		INTEGER,
	t6koncen		INTEGER,
	t6pow			INTEGER,
	t6gestosc		INTEGER,

	t7zagrozenie	INTEGER,
	t7stale			INTEGER,
	t7przez1		INTEGER,
	t7przez2		INTEGER,
	t7dodatkowe		TEXT,

	t8ocena			INTEGER,
	t8inwent		INTEGER,
	t8badania		INTEGER,
	t8interwen		INTEGER,
	t8dodatkowe		TEXT,

	t9autor			TEXT,
	t9data			TEXT,
	t9chrono		TEXT,
	t9sprawdzil		TEXT,

	t10materialy	TEXT,
	t10dalsze		TEXT,

	PRIMARY KEY(id,t1miejscowosc)
 );

