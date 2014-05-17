
fakBEtur 0.5 (20061228)
=======================

(c) Maciej Witkowiak <mwitkowiak@gmail.com>, <ytm@elysium.pl>
http://ytm.bossstation.dnsalias.org/html/beos.html
http://members.elysium.pl/html/beos.html

fakBEtur to program wspomagający wystawianie faktur. Program korzysta z własnej bazy
danych, w której umieszcza informacje o wydanych dokumentach. Baza służy również do
przechowywania danych kontrahentów i towarów.

#ZRZUTY EKRANU
![Screenshot](/images/fakbetur01.png?raw=true) 
![Screenshot](/images/fakbetur02.png?raw=true) 
![Screenshot](/images/fakbetur03.png?raw=true) 
![Screenshot](/images/fakbetur04.png?raw=true) 
![Screenshot](/images/fakbetur05.png?raw=true) 
![Screenshot](/images/fakbetur06.png?raw=true) 
![Screenshot](/images/fakbetur07.png?raw=true) 
![Screenshot](/images/fakbetur08.png?raw=true) 
![Screenshot](/images/fakbetur09.png?raw=true) 
![Screenshot](/images/fakbetur0a.png?raw=true) 


#DLACZEGO OPEN SOURCE
Wydanie 0.5 programu została udostępniona wraz z kodem źródłowym w nadziei, że okaże się użyteczna.
Sam nie mam zamiaru dalej rozwijać programu (lista rzeczy do zrobienia w pliku TODO), brak mi do tego
zapału i tym bardziej bardziej czasu. Tworzenie programu tego typu bez solidnej wiedzy i/lub wsparcia
merytorycznego ze strony użytkowników nie ma sensu. Dla mnie przestało być zajęciem, przy okazji
którego można się czegoś nauczyć, a zmieniło w pracę.



#LICENCJA
Kod źródłowy programu został uwolniony w nadziei, że okaże się użyteczny. Wolno go kopiować, rozpowszechniać, zmieniać, tworzyć na jego podstawie nowe programy (również komercyjne).
Jedyne, czego nie wolno w wypadku wykorzystania to podania pochodzenia i autorstwa kodu. Byłoby
grzecznie również poinformować mnie i pochwalić się do czego ten kod został wykorzystany.
W przypadku płatnych aplikacji liczę na darmową kopię.

Autor dołożył wszelkich starań, ale nie daje żadnej gwarancji na poprawność działania
programu, ani generowanych przez program dokumentów.



#ZMIANY
###0.5
 - usunięty segfault przy obliczaniu marży przy zerowej cenie netto
###0.4
 - w wypadku błędnych danych focus przenoszony w odpowiednie pole
 - możliwość importu listy towarów z innego dokumentu
 - miesięczne podsumowanie sprzedaży
 - wydruk oryginał/kopie, oryginał+kopie, możliwość konfiguracji liczby kopii
 - konfiguracja domyślnej liczby dni na zapłatę (licząc od dnia sprzedaży)
 - nowa obsługa list, możliwość sortowania wg wybranych kolumn
 - dialog z kalendarzem do wyboru daty
 - raport o zaległych należnościach
 - możliwość ustawienia wartości domyślnej dla pola 'Wystawił'
 - opcja - numeracja uproszczona dokumentów (bez miesiąca)
 - jednolite skróty klawiaturowe: F2/3/4 - zmiana zakładki, F5/6/8/9 - działanie, ENTER - zapis
 - podsumowanie na zakładce z listą pozycji faktury
 - raport o zapłaconych/niezapłaconych fakturach kontrahenta, dostępny z należności i zakładki firm
 - guzik 'zapłacono' do natychmiastowego opłacenia całej faktury
 - informacje o towarach rozszerzone o stan magazynu i ostatnią zmianę
###0.3
 - zmiana formatu bazy, dodane wersjonowanie bazy - obecnie brak możliwości upgrade,
   będzie taka możliwość w przypadku kolejnych zmian
 - zapamiętywanie ścieżek (katalog zapisu, szablon HTML)
 - towary/firmy pierwszy raz występujące na fakturze są dopisywane do bazy danych
 - możliwość edycji stawek podatku
 - pełna konfiguracja wydruku (rodzaj eksportu, liczba kopii, rodzaj dokumentu)
 - po dodaniu towaru do faktury focus przeniesiony na pole "Ilość"
 - wydruk przez print_server (domyślna czcionka to Arial, dla wydruku do pdf należy w opcjach
   dołączyć ją (embed) do pliku)
###0.2
 - rezygnacja z SpLocale
 - sprawdzanie poprawności danych
 - lepszy szablon eksportu do HTML (tabela z ramkami)
 - odświeżanie symboli w zakładce faktura po zmianach w bazie danych
 - nazwy wszystkich pól ustawiane w zależności od aktualnej czcionki, zawsze widoczne
 - automatyczne ustawianie numeru nowej faktury, miejsce wystawienia ustawione na
   siedzibę firmy
###0.1
 - pierwsza publiczna wersja



#WYMAGANIA
Program wymaga skonfigurowania BeOSa do obsługi polskiego układu klawiatury oraz
instalacji czcionek z polskimi znakami.
Do eksportu HTML niezbędny jest szablon, przykładowy szablon został dołączony do
programu. Można go umieścić w dowolnej lokalizacji.



#INSTALACJA
Program można rozpakować gdziekolwiek, do eksportu w HTML niezbędny jest szablon.
Szablon należy umieścić domyślnie w pliku: /boot/home/vatszablon.html, jeśli go tam nie
będzie, program poprosi o wskazanie szablonu. Jego lokalizacja zostanie zapamiętana.



#KONFIGURACJA
Przy pierwszym uruchomieniu program utworzy bazę danych i ustali domyślną konfigurację.
Następnie zostanie otwarty dialog, w którym należy podać dane firmy wystawiającej faktury.



#OBSŁUGA
Większość operacji i przełączanie się między zakładkami można wykonać klawiszami funkcyjnymi.
Klawisz 'ENTER' jest równoważny zapisowi aktualnej karty do bazy danych. Uwaga! Na zakładce
faktur nie należy pomylić zapisu towaru na fakturę z zapisem całej faktury.

Pole 'SYMBOL' obecne w spisie kontrahentów i towarów służy do wprowadzenia skróconej
nazwy, dzięki której możliwe jest szybkie odnajdywanie rekordów i wypełnianie pól faktury.
Na wszystkich zakładkach pola 'Nazwa' i 'SYMBOL' muszą być wypełnione i unikalne.

W polach numerycznych można wpisywać proste działania arytmetyczne, jednak aby uniknąć
błędnego zaokrąglenia liczby należy zawsze podawać w postaci ułamka dziesiętnego.
Np. zamiast "3/7" należałoby wpisać "3.0/7.0".

##a) MENU
###DOKUMENT
Służy do wyboru rodzaju drukowanych dokumentów.

###OPCJE
Tu można wybrać metodę drukowania/eksportu, zmienić parametry firmy, ustalić
nowe stawki podatków oraz wartości domyślne - liczbę drukowanych kopii oraz termin płatności.
Ponadto można włączyć numerację uproszczoną faktur - bez miesiąca. Przy edycji już istniejących
stawek podatku nie jest możliwa zmiana stawki, a jedynie zmiana nazwy. Pozwala to zachować
integralność danych.

###PODSUMOWANIA
To menu podaje podsumowania sprzedaży miesięcznej i zaległych należności. Należności bieżące
każdego z kontrahentów można przywołać na karcie kontrahenta. Ujemna liczba dni do zapłaty (można
ją również podać w dialogu należności) oznacza ile dni zostało do powstania zaległości.

##b) TOWARY
W zakładce 'Towary' umieszczony został prosty kalkulator umożliwiający obliczanie cen.
Przeznaczenie przycisków:
- Cena sprzedaży - na podstawie ceny zakupu i rabatu oblicza nową cenę sprzedaży netto
- Marża - na podstawie ceny zakupu, rabatu oraz ceny sprzedaży netto oblicza marżę
- Import - na podstawie kursu waluty, ceny zakupu (jednostką jest wówczas obca waluta),
  cła, marży oraz rabatu oblicza cenę sprzedaży netto
Na tej zakładce można również zmienić aktualny stan magazynu.

##c) KONTRAHENCI
Pola odbiorca/dostawca nie mają obecnie żadnego znaczenia.
Pole 'Aktywny' kontroluje czy symbol kontrahenta ma być możliwy do wyboru w zakładce
faktur.
Pole 'Zablokowany' blokuje możliwość edycji danych kontrahenta i zabezpiecza przed przypadkowymi
zmianami.

##c) FAKTURY
W polach 'Płatność' można wybrać/wpisać datę płatności lub liczbę dni od daty sprzedaży.
W drugim przypadku data zostanie uaktualniona automatycznie po przeniesieniu focusu na inne
pole tekstowe (np. tabulatorem).
Przy dodawaniu/edycji pozycji należy zawsze kliknąć 'Zapisz' (w ramce pozycji faktury),
by dopisać nowe dane do faktury, a następnie wcisnąć ENTER lub kliknąć 'Zapisz' na dole,
aby zapisać całą informację o fakturze.
Guzik "Import z innej" służy do skopiowania listy pozycji z innego dokumentu na bieżący.
Uwaga! Jeśli edytujemy dokument, który zawiera datę sprzedaży wcześniejszą niż data
ostatniej zmiany magazynowej danego towaru, to stan magazynu nie ulegnie zmianie.



#WYDRUK
Aby wydrukować dokument należy wybrać rodzaj dokumentu z menu 'Dokument' oraz
sposób drukowania z menu 'Opcje', następnie przejść na zakładkę 'FAKTURY', wybrać
fakturę z listy i kliknąć u dołu 'Drukuj' lub wcisnąć F9.



#EKSPORT HTML
Wszelkich zmian wyglądu można dokonać korzystając z przykładowego szablonu. Teksty
otoczone znakami @, np. @FNAZWA@ zostaną zamienione na odpowiednie dane z
faktury. Pole @TABELA@ zostanie zamienione na ciąg wierszy tabeli HTML, bez dodatkowych atrybutów, podobnie pole @TABSUMA@.



#EKSPORT TXT
Zapis do pliku tekstowego służy raczej jako demonstracja możliwości niż rzeczywiście użyteczna
opcja. Odpowiednio przerobiony może być bazą do bezpośredniego wydruku na drukarkach igłowych.



#ARCHIWIZACJA
Do wykonania archiwum bazy danych wystarczy zachować w bezpieczny sposób plik
bazy: /boot/home/config/share/fakbetur.db



#DEINSTALACJA
Usunąć program, plik z szablonem HTML oraz plik bazy danych
/boot/home/config/share/fakbetur.db
