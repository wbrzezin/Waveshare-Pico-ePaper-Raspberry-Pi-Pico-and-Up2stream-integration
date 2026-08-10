# Dokumentacja protokołu UART UP2Stream

Dokumentacja techniczna komunikacji UART zaobserwowanej w urządzeniu
Arylic UP2Stream Pro i wykorzystywanej w projekcie **UP2Stream Display**.

Dokument łączy:

- informacje z opublikowanej dokumentacji protokołu UART UP2Stream,
- obserwacje wykonane za pomocą fizycznego analizatora UART,
- zachowanie zaobserwowane na testowanym urządzeniu UP2Stream Pro,
- wnioski istotne z punktu widzenia implementacji projektu.

> **Ważne**
>
> Dokument nie jest kompletną ani autorytatywną specyfikacją protokołu
> UART UP2Stream.
>
> Niektóre komendy opisane w opublikowanej dokumentacji protokołu
> nie zachowują się zgodnie z oczekiwaniami na testowanym urządzeniu.
>
> O ile nie zaznaczono inaczej, obserwacje opisane w tym dokumencie
> dotyczą urządzenia Arylic UP2Stream Pro pracującego z firmware:
>
> `35-99e42697-6`

---

# 1. Konfiguracja UART

Interfejs UART wykorzystuje:

| Parametr | Wartość |
|---|---|
| Prędkość transmisji | 115200 |
| Bity danych | 8 |
| Parzystość | Brak |
| Bity stopu | 1 |
| Kontrola przepływu | Brak |

W skrócie:

```text
115200 8N1

Komunikaty są zakończone średnikiem:

VOL:46;

Interfejs UART jest dwukierunkowy:

MCU → UP2Stream
UP2Stream → MCU
2. Najważniejsze cechy komunikacji

Interfejs UART UP2Stream należy traktować jako asynchroniczny strumień
komunikatów, a nie jako prosty protokół typu żądanie/odpowiedź.

W praktyce można wyróżnić dwie istotne grupy komunikatów:

komendy, za pomocą których mikrokontroler może aktywnie odczytać
aktualny stan urządzenia,
komunikaty asynchroniczne generowane samodzielnie przez UP2Stream.

To rozróżnienie jest bardzo istotne dla implementacji parsera.

3. Zapytania o stan urządzenia

Testowane urządzenie odpowiada na komendy służące do odczytu
aktualnego stanu urządzenia.

Przykłady:

STA;
SRC;
PLA;
VOL;
MUT;
AUD;
CHN;
LPM;
NAM;
VER;
ETH;
TME;

Na przykład:

STA;

zwraca:

STA:NET,0,46,2,4,1,1,1,1,0;

Podobnie:

SRC;

zwraca:

SRC:NET;

Komendy te mogą być zatem aktywnie wysyłane przez mikrokontroler
w celu synchronizacji stanu UP2Stream.

4. Metadane odtwarzanego utworu

Informacje dotyczące aktualnie odtwarzanej treści są przesyłane przez
UP2Stream jako komunikaty asynchroniczne.

Zaobserwowane typy komunikatów:

TIT:
ART:
ALB:
VND:
ELP:

Przykład:

TIT:Komandor Tarkin;
ART:Kazik;
ALB:Melassa;
VND:spotify;
ELP:3551/0;
ELP:7117/254026;

Komunikaty te opisują aktualnie odtwarzaną treść oraz pozycję
odtwarzania.

Ważne

Na testowanym firmware metadanych dotyczących odtwarzanej treści nie
należy traktować jako zwykłych zapytań o stan urządzenia.

W szczególności wysłanie:

ART;

nie powoduje w sposób niezawodny otrzymania:

ART:...;

Analogicznie TIT;, ALB;, VND; i ELP; nie powinny być traktowane
jako pewne zapytania zwracające aktualne metadane odtwarzanego utworu.

Dlatego aplikacja traktuje te komunikaty jako asynchroniczne
powiadomienia.

5. Asynchroniczny strumień UART

UP2Stream może wysyłać komunikaty niezależnie od komend wysyłanych
przez mikrokontroler.

Podczas odtwarzania urządzenie generuje na przykład okresowo:

ELP:47762/762946;
ELP:53100/762946;
ELP:59551/762946;
ELP:75899/762946;
ELP:81373/762946;

W tym samym czasie mikrokontroler może wysłać zapytanie:

STA;

Dlatego:

Następny odebrany komunikat UART nie może być automatycznie uznany
za odpowiedź na ostatnią komendę wysłaną przez mikrokontroler.

Przykładowo, jeżeli mikrokontroler wyśle:

ART;

a następnym odebranym komunikatem będzie:

ELP:4168/239360;

nie oznacza to, że ART; zwróciło odpowiedź ELP.

Komunikat ELP może być po prostu asynchronicznym komunikatem
wygenerowanym przez UP2Stream w tym samym czasie.

Wymagania dotyczące parsera

Parser powinien klasyfikować każdy odebrany komunikat niezależnie,
na podstawie jego prefiksu:

STA:  → stan systemu
SRC:  → źródło
PLA:  → stan odtwarzania
VOL:  → głośność
MUT:  → wyciszenie
TIT:  → tytuł
ART:  → wykonawca
ALB:  → album
VND:  → dostawca treści
ELP:  → czas odtwarzania
TME:  → czas urządzenia

Parser nie powinien zależeć od kolejności żądanie/odpowiedź.

6. SYS — stan systemu

Zaobserwowano:

SYS:ON;

oraz:

SYS:STANDBY;

Długie naciśnięcie fizycznego przycisku Power wygenerowało:

SYS:STANDBY;
MUT:1;
VOL:30;

Pozwala to wykryć przejście urządzenia do trybu standby.

7. STA — pełny stan urządzenia

Komunikat STA zawiera szereg parametrów opisujących aktualny stan
urządzenia.

Przykład:

STA:NET,0,46,2,4,1,1,1,1,0;

Według dokumentacji protokołu poszczególne pola oznaczają:

Pole	Przykład	Znaczenie
source	NET	aktualne źródło
mute	0	stan wyciszenia
volume	46	głośność
treble	2	tony wysokie
bass	4	tony niskie
net	1	stan sieci
internet	1	stan Internetu
playing	1	stan odtwarzania
led	1	stan diody LED
upgrading	0	aktualizacja firmware

Zapytanie:

STA;

zostało potwierdzone jako działające na testowanym urządzeniu.

Przykładowa interpretacja
STA:NET,0,46,2,4,1,1,1,1,0;

oznacza:

Source     = NET
Mute       = 0
Volume     = 46
Treble     = 2
Bass       = 4
Network    = 1
Internet   = 1
Playing    = 1
LED        = 1
Upgrading  = 0

STA jest szczególnie użytecznym komunikatem do okresowej synchronizacji
stanu, ponieważ jedno zapytanie zwraca wiele parametrów urządzenia.

8. SRC — aktualne źródło

Zaobserwowano:

SRC:NET;
SRC:BT;
SRC:LINE-IN;

Fizyczny przycisk źródła powoduje przełączanie pomiędzy różnymi
wejściami.

Dokumentacja protokołu wymienia następujące wartości:

NET
USB
USBDAC
LINE-IN
LINE-IN2
BT
OPT
COAX
I2S
HDMI

Zapytanie:

SRC;

zwraca aktualne źródło.

9. PLA — stan odtwarzania sieciowego

Zaobserwowano:

PLA:0;
PLA:1;

Zapytanie:

PLA;

zwraca aktualny stan odtwarzania sieciowego.

Projekt UP2Stream Display wykorzystuje tę informację do sterowania
wskaźnikiem odtwarzania:

PLA:1 → odtwarzanie
PLA:0 → pauza / brak odtwarzania

Informacja ta jest również wykorzystywana przez logikę wyświetlacza
do wyboru ikony PLAY lub PAUSE.

10. VOL — głośność

Zaobserwowano:

VOL:30;
VOL:46;

Zapytanie:

VOL;

zwraca aktualną głośność.

Udokumentowany zakres:

0 ... 100

Protokół umożliwia również ustawienie głośności:

VOL:50;

Projekt UP2Stream Display wykorzystuje VOL do wyświetlania aktualnej
głośności.

11. MUT — wyciszenie

Zaobserwowano:

MUT:1;
MUT:0;

Urządzenie może chwilowo wyciszyć wyjście podczas zmiany źródła.

Przykład:

MUT:1;
SRC:BT;
MUT:0;
VOL:30;

Zapytanie:

MUT;

zwraca aktualny stan wyciszenia.

Protokół przewiduje również:

MUT:0;
MUT:1;
MUT:T;

gdzie T oznacza przełączenie stanu.

12. AUD — wyjście audio

Zaobserwowano:

AUD:1;

Zapytanie:

AUD;

zwraca aktualny stan wyjścia audio.

13. CHN — stan kanałów

Zapytanie:

CHN;

zostało zaobserwowane jako zwracające:

CHN:S;

Udokumentowane wartości:

L
R
S

oznaczają odpowiednio kanał lewy, prawy i stereo.

14. LPM — tryb odtwarzania

Zaobserwowano:

LPM:REPEATALL;

oraz:

LPM:SEQUENCE;

Udokumentowane tryby:

REPEATALL
REPEATONE
REPEATSHUFFLE
SHUFFLE
SEQUENCE

Zapytanie:

LPM;

zwraca aktualny tryb odtwarzania.

15. TIT — tytuł aktualnie odtwarzanego utworu

UP2Stream wysyła tytuł aktualnie odtwarzanego utworu:

TIT:Komandor Tarkin;

Inny zaobserwowany przykład:

TIT:Moritet von Mackie Messer;

Urządzenie wysyła TIT: przy zmianie odtwarzanej treści.

15.1. Ograniczenie długości tytułu

Fizyczny analizator UART wykazał, że długie tytuły mogą być skracane.

Pełny tytuł:

Pieśń w której Mackie prosi wszystkich o przebaczenie - Wersja z pętlą

został przesłany jako:

TIT:Pieśń w której Mackie prosi wszystkich o przebaczenie - Wers;

Projekt przyjmuje więc praktyczne ograniczenie długości tytułu na
poziomie około 60 znaków.

Na obecnym etapie nie jest to traktowane jako problem.

16. Osadzony komunikat ELP w TIT

Zaobserwowano przypadki, w których UP2Stream skleja dwa komunikaty.

Przykład odebranego bufora:

TIT:Ballada oELP:31880/137426;

Tytuł nie zawiera w rzeczywistości tekstu ELP:.

Jest to wynik bezpośredniego doklejenia kolejnego komunikatu do
poprzedniego.

Parser projektu wyszukuje więc wewnątrz komunikatu TIT: osadzony
fragment:

ELP:

i przetwarza go jako oddzielny komunikat ELP.

To zachowanie musi zostać zachowane przy dalszych modyfikacjach
parsera.

17. ART — wykonawca

UP2Stream może wysłać:

ART:Kazik;

przy zmianie odtwarzanej treści.

Testy wykazały jednak, że ART: nie jest gwarantowane przy każdej
zmianie TIT:.

Zaobserwowano na przykład sekwencje:

TIT:...;
ALB:...;

bez towarzyszącego:

ART:...;

Dlatego aplikacja nie powinna zakładać, że wykonawca zawsze zostanie
odebrany razem z tytułem.

Aktualna wartość wykonawcy powinna pozostać zachowana aż do momentu
odebrania nowego komunikatu ART:.

Wysyłanie:

ART;

nie powoduje niezawodnego zwrócenia aktualnego wykonawcy na testowanym
firmware.

18. ALB — album

UP2Stream może wysłać:

ALB:Melassa;

przy zmianie odtwarzanej treści.

Podobnie jak w przypadku ART:, nie należy zakładać, że ALB: będzie
zawsze wysłane dla każdego utworu.

Aplikacja powinna przetwarzać ALB: niezależnie, zawsze gdy taki
komunikat zostanie odebrany.

19. VND — dostawca treści

UP2Stream może wysłać:

VND:spotify;

Informacja ta identyfikuje dostawcę aktualnie odtwarzanej treści.

Jest to bardziej szczegółowa informacja niż:

SRC:NET;

Przykład:

SRC:NET;
VND:spotify;

oznacza, że ogólnym źródłem jest odtwarzanie sieciowe, natomiast
dostawcą treści jest Spotify.

VND: może być przydatne w przyszłości do wyświetlania informacji
o aktualnym serwisie.

20. ELP — czas odtwarzania / czas całkowity

Format komunikatu:

ELP:{elapsed}/{total};

Wartości podawane są w sekundach.

Przykłady:

ELP:2403/239360;
ELP:33500/762946;
ELP:1028/762946;

UP2Stream okresowo przesyła ELP: podczas odtwarzania.

Projekt UP2Stream Display wykorzystuje te informacje do obliczania:

aktualnego czasu odtwarzania,
całkowitego czasu utworu,
procentowego postępu.
Ważne

Projekt zasadniczo nie musi odpytywać o ELP.

Urządzenie samo wysyła te komunikaty okresowo.

21. TME — czas urządzenia

UP2Stream obsługuje zapytanie:

TME;

Testowane urządzenie odpowiada:

TME:2026-08-10 18:37:05 (+1);

Odpowiedź zawiera:

YYYY-MM-DD HH:MM:SS (offset)

UP2Stream synchronizuje swój zegar z aplikacją w smartfonie.

21.1. Czas letni / zimowy

Testy wykonane w Polsce wykazały, że UP2Stream nie uwzględnia
prawidłowo europejskiej zmiany czasu letniego/zimowego.

Przykładowo w czasie polskiego czasu letniego prawidłowy offset wynosi:

UTC+2

natomiast testowane urządzenie może zwracać:

(+1)

Dlatego TME nie powinno być obecnie traktowane jako gotowy czas
lokalny bez zastosowania korekty strefy czasowej po stronie aplikacji.

21.2. Planowane wykorzystanie w projekcie

Docelowo planowana architektura wygląda następująco:

UP2Stream
    │
    │ TME;
    ▼
odpowiedź TME
    │
    ▼
RTC Pico
    │
    ▼
korekta lokalnej strefy czasowej / DST
    │
    ▼
wyświetlacz zegara

Istniejący RTC Pico może utrzymywać czas pomiędzy kolejnymi
synchronizacjami.

22. NAM — nazwa urządzenia

Zaobserwowano:

NAM:55703273747265616D5F50726F;

Wartość jest zakodowana jako szesnastkowy zapis UTF-8.

Po dekodowaniu:

Up2stream_Pro

Zapytanie:

NAM;

zwraca nazwę urządzenia.

23. VER — wersja firmware i API

Zaobserwowano:

VER:35-99e42697-6;

Według dokumentacji format to:

{firmware}-{commit}-{api}

Dla testowanego urządzenia:

Firmware = 35
Commit   = 99e42697
API      = 6

Informacja ta może być przydatna przy porównywaniu zachowania
różnych wersji firmware.

24. Stan sieci

Zaobserwowano:

NET:1;
WWW:1;
ETH:1;

Protokół definiuje również:

WIF;

Komunikaty te dostarczają informacji o:

stanie sieci,
dostępności Internetu,
stanie Ethernetu,
stanie Wi-Fi.

Mogą być wykorzystane w przyszłości na ekranie diagnostycznym.

25. Inne komendy opisane w dokumentacji

Opublikowana dokumentacja protokołu zawiera również między innymi:

SYS
WWW
AUD
SRC
VOL
MUT
BAS
TRE
POP
STP
NXT
PRE
BTC
PLA
CHN
MRM
LED
BEP
PST
VBS
WRS
LPM
NAM
ETH
WIF
PMT
PRG
DLY
MXV
ASW
POM
ZON
TIT
ART
ALB
VND
ELP
VOS
BAL
VOF
TME

Nie wszystkie z tych komend zostały przetestowane na urządzeniu
wykorzystywanym w tym projekcie.

Ich obecność w opublikowanej dokumentacji nie powinna być więc
interpretowana jako potwierdzenie, że są obsługiwane lub zachowują się
identycznie na testowanym firmware.

26. ERR:INVALID

Podczas testów urządzenie zwracało czasami:

ERR:INVALID;

Zjawisko występowało podczas eksperymentów z komendami opisanymi
w opublikowanej dokumentacji protokołu.

Potwierdza to, że opublikowany zestaw komend powinien być traktowany
jako dokumentacja referencyjna, a nie jako gwarancja, że każda komenda
będzie obsługiwana przez każdą wersję firmware lub każdy model
UP2Stream.

27. Zaobserwowana sekwencja startowa

Typowa sekwencja zaobserwowana po uruchomieniu urządzenia:

SYS:ON;
MUT:1;
VER:35-99e42697-6;
STA:NET,1,46,2,4,0,0,0,1,0;
VOL:46;
MUT:0;
VOL:46;

Po krótkim czasie:

AUD:1;
LPM:REPEATALL;
NAM:55703273747265616D5F50726F;
VOL:46;
NET:1;
WWW:1;

Dokładna kolejność i czas wysyłania poszczególnych komunikatów nie
powinny być traktowane jako gwarantowane.

28. Zmiana źródła — obserwacje

Krótkie naciśnięcie fizycznego przycisku Power/source spowodowało:

MUT:1;
SRC:NET;
PLA:0;
MUT:0;
VOL:46;

Kolejne naciśnięcie:

MUT:1;
SRC:BT;
MUT:0;
VOL:30;

Kolejne:

MUT:1;
SRC:LINE-IN;
MUT:0;
VOL:30;

Pokazuje to, że zmiana źródła może wygenerować kilka komunikatów,
a nie tylko pojedynczy SRC:.

29. Standby — obserwacja

Długie naciśnięcie fizycznego przycisku Power spowodowało:

SYS:STANDBY;
MUT:1;
VOL:30;

Aplikacja może zatem wykorzystać SYS:STANDBY jako informację
o przejściu urządzenia w tryb standby.

30. Zalecana konstrukcja parsera

Parser UART powinien być zaimplementowany jako parser asynchronicznego
strumienia.

Powinien:

zbierać przychodzące bajty do momentu odebrania terminatora ;,
rozpoznawać typ komunikatu na podstawie prefiksu,
niezależnie parsować każdy komunikat,
aktualizować tylko odpowiednie pola PlayerState,
nigdy nie zakładać, że następny komunikat jest odpowiedzią na
ostatnią wysłaną komendę.

Schemat:

strumień bajtów UART
        │
        ▼
framing komunikatu
        │
        ▼
rozpoznanie typu
        │
        ├── STA → stan systemu
        ├── SRC → źródło
        ├── PLA → stan odtwarzania
        ├── VOL → głośność
        ├── MUT → wyciszenie
        ├── TIT → tytuł
        ├── ART → wykonawca
        ├── ALB → album
        ├── VND → dostawca
        ├── ELP → czas odtwarzania
        └── TME → czas urządzenia
31. Obsługa brakujących metadanych

Ponieważ UP2Stream nie musi wysyłać wszystkich pól metadanych przy każdej
zmianie utworu, aplikacja nie powinna kasować wszystkich metadanych po
otrzymaniu TIT:.

Przykładowo otrzymanie:

TIT:Nowy tytuł;

nie oznacza automatycznie, że:

ART:
ALB:
VND:

są niedostępne.

Aplikacja powinna zaktualizować tylko:

title

i zachować wcześniej znane wartości:

artist
album
provider

aż do momentu odebrania odpowiednich nowych komunikatów.

32. Okresowa synchronizacja stanu

Niezawodny interfejs zapytań umożliwia okresową synchronizację stanu
urządzenia.

Jedno zapytanie:

STA;

może dostarczyć:

source
mute
volume
treble
bass
network
internet
playing
LED
upgrading

Może to zostać wykorzystane jako lekki mechanizm synchronizacji
i kontroli stanu urządzenia.

Przykładowo projekt może okresowo wysyłać:

STA;

bez polegania wyłącznie na komunikatach asynchronicznych.

Metadane odtwarzania powinny nadal być traktowane jako komunikaty
asynchroniczne.

33. Zalecany model komunikacji

Docelowy model komunikacji wygląda następująco:

                    UP2STREAM PRO
                          │
             ┌────────────┴────────────┐
             │                         │
             │                         │
       ODPOWIEDZI NA              DANE
         ZAPYTANIA             ASYNCHRONICZNE
             │                         │
             │                         │
       STA SRC PLA                TIT ART
       VOL MUT AUD                ALB VND
       CHN LPM NAM                ELP
       VER NET WWW
       ETH WIF TME
             │                         │
             └────────────┬────────────┘
                          │
                          ▼
                 Up2StreamClient
                          │
                          ▼
                     PlayerState
                          │
                          ▼
                       Display

Model ten odpowiada zachowaniu zaobserwowanemu na testowanym
urządzeniu.

34. Komendy potwierdzone na testowanym urządzeniu

Następujące komendy zostały przetestowane jako zapytania:

STA;
SRC;
PLA;
VOL;
MUT;
AUD;
CHN;
LPM;
NAM;
VER;
ETH;
TME;

Następujące typy komunikatów zostały zaobserwowane jako komunikaty
asynchroniczne:

SYS:
TIT:
ART:
ALB:
VND:
ELP:
SRC:
PLA:
VOL:
MUT:
AUD:
CHN:
LPM:
NET:
WWW:
NAM:
VER:
ETH:

Dokładny zestaw komunikatów asynchronicznych oraz ich moment wysyłania
może zależeć od aktualnego stanu urządzenia.

35. Testowane firmware

Główne urządzenie testowe zgłosiło:

VER:35-99e42697-6;

Obserwacje opisane w tym dokumencie należy zatem wiązać przede wszystkim
z:

Arylic UP2Stream Pro
Firmware: 35
Commit:   99e42697
API:      6

Zachowanie może różnić się w innych wersjach firmware lub innych
modelach UP2Stream.

36. Metoda testowania

Zachowanie protokołu było badane za pomocą fizycznego analizatora UART
podłączonego bezpośrednio do UP2Stream Pro.

Metoda ta pozwoliła rozróżnić:

komunikaty generowane automatycznie przez urządzenie,
odpowiedzi na wysyłane komendy,
kolejność i czas pojawiania się komunikatów,
brakujące metadane,
sklejanie komunikatów,
nieobsługiwane komendy.

Fizyczne przechwytywanie transmisji UART jest traktowane jako podstawowe
źródło informacji o rzeczywistym zachowaniu testowanego sprzętu.

37. Najważniejsze ustalenia

Najważniejsze wnioski dla projektu:

Komunikacja UART wykorzystuje 115200 8N1.
Komunikaty są zakończone znakiem ;.
Strumień UART jest asynchroniczny.
Komendy takie jak STA;, SRC;, PLA;, VOL; i MUT; można
aktywnie odpytywać.
Metadane odtwarzania, takie jak TIT:, ART:, ALB: i VND:,
są przesyłane asynchronicznie.
ART: nie jest gwarantowane przy każdej zmianie utworu.
ELP: jest cyklicznie wysyłane przez urządzenie podczas odtwarzania.
TIT: może być skracane.
TIT: może zawierać osadzony komunikat ELP: w wyniku sklejenia
komunikatów.
TME; pozwala uzyskać aktualną datę i godzinę urządzenia.
UP2Stream synchronizuje swój zegar z aplikacją w smartfonie.
Testowane urządzenie nie obsługuje prawidłowo polskiej zmiany czasu
letniego/zimowego.
STA; jest szczególnie przydatne do okresowej synchronizacji stanu.
Parser nie może zakładać, że następny odebrany komunikat jest
odpowiedzią na ostatnio wysłaną komendę.
Zachowanie rzeczywistego firmware powinno mieć pierwszeństwo przed
założeniami wynikającymi wyłącznie z opublikowanej dokumentacji
protokołu.
38. Planowany rozwój

Możliwe przyszłe zastosowania interfejsu UART:

synchronizacja RTC Pico za pomocą TME,
automatyczna korekta polskiej strefy czasowej i DST,
okresowa synchronizacja stanu za pomocą STA,
wyświetlanie dostawcy treści na podstawie VND,
wyświetlanie informacji o albumie na podstawie ALB,
wyświetlanie stanu sieci i Internetu,
wyświetlanie informacji o firmware i API,
przygotowanie ekranu diagnostycznego,
wykrywanie trybu standby UP2Stream,
monitorowanie połączenia Wi-Fi/Ethernet.
Źródła

Dokument został przygotowany na podstawie opublikowanej dokumentacji
protokołu UART UP2Stream oraz bezpośrednich obserwacji transmisji UART
urządzenia Arylic UP2Stream Pro.

Testowane urządzenie zgłosiło:

VER:35-99e42697-6;

W przypadku komend, które nie zostały jeszcze zweryfikowane fizycznie
na urządzeniu, należy traktować dokumentację protokołu jako źródło
referencyjne, a nie jako gwarancję ich działania na konkretnym
firmware.
