# Sieciowa turowa gra logiczna: System gry w warcaby (serwer, klient, moduł board)

## Wprowadzenie

Sprawozdanie opisuje projekt stworzenia systemu gry w warcaby z wykorzystaniem trzech głównych elementów: serwera obsługującego gry, klienta umożliwiającego grę oraz modułu board do zarządzania planszą. Wszsytkie pliki stworzone zostały w języku C z wykorzystaniem bsd-sockets.

## Uruchomienie gry
Kompilacja i uruchomienie odbywa się poprzez terminal otwarty w folderze z odpowiednimi plikami.
- Kompilacja serwera poprzez użycie komendy
  ```
  gcc -Wall -pthread serwer.c -o serwer.out
  ```
- Uruchomienie serwera poprzez użycie komendy
  ```
  ./serwer.out
  ```
- Kompilacja serwera poprzez użycie komendy
  ```
  gcc -Wall klient.c -o klient.out
  ```
- Uruchomienie serwera poprzez użycie komendy
  ```
  ./klient.out
  ```

## Komunikacja między serwerem, a klientem
- W skrócie komunikacja polega na systemie wysyłania statusów od serwera do graczy, przy jednoczesnym wysyłaniu statusu aktywności klientów do serwera.
  
Serwer w nieskończonej pętli nasłuchuje na wybranym porcie, klient próbuje nawiązać połączenie i gdy uda mu się połączyć dostaje od serwera podstawowe dane ( numer gracza, numer tablicy ), jeśli jedank nie ma wolnego miejsca klient otrzymuje wartość -1 i gra zostaje zakończona. Klient w pętli przed otrzymaniem wiadomości od serwera wysyła mu status, że wciąż jest aktywny. Jeśli klient jest jedynym graczem dostaje status od serwera, że musi poczekać na kolejnego gracza. Gdy połączy się gracz drugi, otrzymują oni status, że gra jest w toku i odpowiedni gracz otrzymuje wiadomość, że teraz jest jego ruch. Serwer oczekuje na wiadomość z ruchem od klienta, następnie stara się go wykonać. Jeśli ruch jest prawidłowy i nie jest to wielokrotne bicie to następuje zmiana tury na kolejnego gracza, w przeciwnym wypadku tura wciąż przypda temu graczu i dostaje ons tatus, że teraz jego kolej. Drugi gracz, otrzymuje w pętli status, że musi poczekać na ruch przeciwnika. Jeśli ktorykolwiek z graczy został bez pionków, obydwoje odstają status z przegraną lub wygraną. Gdy jeden z graczy się rozłączy, drugi otrzymuje wiadomość, że gra została zakończona i może albo wyjść z programu albo zacząć kolejną grę. 

## Opis funkcjonalności

### Serwer
- Socket Server: Serwer gry w warcaby wykorzystuje mechanizm socketów do nasłuchiwania połączeń od klientów.
- Wielowątkowość: Implementuje wielowątkowość przy użyciu biblioteki pthreads w celu obsługi wielu gier jednocześnie, każda w osobnym wątku.
- Game Slots: Zarządza dostępnymi slotami gier i przydziela graczy do tych slotów, ewentualnie informuje gracza o braku wolnego slotu.
- Logika gry: Nadzoruje stan gry, tury graczy, dostępne ruchy, sprawdza poprawność ruchów i decyduje o rezultatach gier.
- Walidacja Ruchów: Sprawdza poprawność ruchów graczy zgodnie z zasadami warcabów ( zarządza zamianą pionków w damki, wielokrotnym biciem, przymusowym biciem, ruchem pionków )
- Stan Gry: Utrzymuje stan gry, monitoruje możliwe ruchy i decyduje o końcowym wyniku.
- Obsługa gracza: Wykrywa i obsługuje rozłączenie gracza. 
### Klient
- Komunikacja z Serwerem: Pozwala graczom na połączenie się z serwerem, komunikację z nim i interakcję w grze.
- Ruchy i Interfejs: Umożliwia graczom wykonywanie ruchów na podstawie informacji zwrotnej od serwera oraz prezentuje interfejs gry.
- Menu: Po zakończeniu gry możliwość wyjścia lub zaczęcia kolejnej rozgrywki.
### Moduł Board
- Plansza: Zarządza planszą gry i umożliwa jej odczyt w prawidłowy sposób.
  
## Opis Techniczny

### Implementacja Serwera
- Wykorzystuje język C do stworzenia serwera gry w warcaby.
- Używa socketów do komunikacji sieciowej z klientami.
- Wielowątkowość zapewnia obsługę wielu gier jednocześnie.
- Waliduje ruchy graczy zgodnie z zasadami gry w warcaby, gry turowej.
- Serwer ma dostępne przygotowane pięć slotów do gry, obsługuję więc zadaną liczbę par graczy, tak aby skupić się na połączeniu między klientem, a serwerem.
### Implementacja Klienta
- Klient również jest napisany w języku C z użyciem socketów do połączenia z serwerem.
- Klient ma zapewnioną interaktywną rozgrywką, tak aby ten mógł wykonywać ruchy i obserwować stan gry.
- Wysyła żądania do serwera i odbiera informacje dotyczące ruchów i stanu gry.
### Implementacja Modułu Board
- Moduł board implementuje planszę gry w warcaby.
- Zarządza planszą gry, deklaruje początkowe położenie pionków, wyświetla plansze i czyści.

## Podsumowanie

Projekt systemu gry w warcaby obejmujący serwer, klienta i moduł board jest kompleksowym systemem, który umożliwia graczom rozgrywkę w warcaby za pośrednictwem sieci. Każdy z elementów (serwer, klient, moduł board) został zaprojektowany i zaimplementowany tak, aby współpracować ze sobą w celu zapewnienia płynnego przebiegu gry.

Opracowanie tego projektu wymagało solidnej znajomości języka C, technologii sieciowych oraz zasad programowania wielowątkowego. Integracja tych elementów pozwoliła na stworzenie funkcjonalnego systemu gry w warcaby, który umożliwia wielu graczom rywalizację online. 

Sama implementacja zasad warcab, ruch pionków i damek oraz bicie nie były łatwym wyzwaniem i wymagały wielu testów oraz rozbicia na przypadki.
