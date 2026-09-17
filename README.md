# Waveshare Pico e-Paper Display for Arylic UP2Stream

[![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi%20Pico%20%2F%20RP2040-blue)](https://www.raspberrypi.com/products/raspberry-pi-pico/)
[![Display](https://img.shields.io/badge/display-Waveshare%202.13%22%20e--Paper-lightgrey)](https://www.waveshare.com/2.13inch-e-paper.htm)
[![Interface](https://img.shields.io/badge/interface-UART%20115200-orange)](#up2stream-uart-protocol)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue)](LICENSE)

**English / Polski**

A Raspberry Pi Pico (RP2040) based e-paper display for the **Arylic UP2Stream** audio platform.

The project connects a **Raspberry Pi Pico** to an **Arylic UP2Stream Pro** module over UART and displays playback and device information on a **Waveshare 2.13-inch black-and-white e-paper display**.

The firmware is structured into separate components for UART communication, UP2Stream protocol parsing, player-state management, display rendering, text scrolling, UTF-8 handling and time/RTC management.

---

## Features

- UART communication with UP2Stream at **115200 8N1**
- Asynchronous UP2Stream message parsing
- Playback source and state detection
- Content provider detection (`VND`)
- Volume and mute status
- Track title, artist and album information
- Elapsed and total playback time
- Playback progress indicator
- Raspberry Pi Pico hardware RTC synchronization
- Polish local date/time handling, including daylight-saving-time correction
- Player screen
- Idle clock screen
- Automatic player/clock screen switching
- Standby detection
- E-paper display clearing during standby
- Automatic restoration of the player screen
- Scrolling of long text
- Polish UTF-8 character support
- Custom playback/status glyphs
- Optional test-data mode for UI development without an active UP2Stream connection

---

## Project status

The project is in the **working and verification stage**.

The implementation has been tested with a physical **Arylic UP2Stream Pro** and a UART analyzer. The repository also contains detailed documentation of the observed UP2Stream UART protocol.

> **Important:** The UART protocol documentation in this repository combines published protocol information with observations from a physical device. It should not be treated as a complete or authoritative specification of every UP2Stream firmware version.

---

# Hardware

## Required hardware

- Raspberry Pi Pico / RP2040
- Waveshare **2.13-inch black-and-white e-paper display**
- Arylic **UP2Stream** module
- UART connection between the Pico and UP2Stream
- USB connection for programming/debugging the Pico

The current display implementation uses the `GxEPD2_213_B74` driver.

## Wiring

### E-paper display → Raspberry Pi Pico

| E-paper | Raspberry Pi Pico |
|---|---:|
| CS | GPIO 9 |
| DC | GPIO 8 |
| RST | GPIO 12 |
| BUSY | GPIO 13 |
| SPI MOSI | SPI MOSI |
| SPI SCK | SPI SCK |
| VCC | 3.3 V |
| GND | GND |

### UP2Stream → Raspberry Pi Pico

| UP2Stream | Raspberry Pi Pico |
|---|---:|
| TX | GPIO 5 (Pico RX) |
| RX | GPIO 4 (Pico TX) |
| GND | GND |

The project uses the RP2040 UART exposed as `Serial2` by the Arduino-Pico core.

> **Warning:** Verify the electrical levels and pinout of your particular UP2Stream hardware before connecting it. Do not connect incompatible voltage levels directly.

---

# Software requirements

The project is intended for the **Arduino IDE** with an RP2040-compatible Arduino core.

Required libraries:

- **Arduino-Pico** by Earle F. Philhower, III
- **GxEPD2**
- **Adafruit GFX Library**

The project also uses standard Arduino/RP2040 functionality such as SPI, UART and the RP2040 hardware RTC.

The repository contains generated font header files used by the display interface.

---

# Installation

1. Install the Arduino IDE.
2. Install the **Raspberry Pi Pico / RP2040 Arduino core by Earle F. Philhower, III**.
3. Select the appropriate Raspberry Pi Pico board.
4. Install:
   - `GxEPD2`
   - `Adafruit GFX Library`
5. Open:

```text
Waveshare-Pico-ePaper-Raspberry-Pi-Pico-and-Up2stream-integr.ino
```

6. Check the hardware configuration in:

```text
HardwareConfig.h
```

7. Connect the hardware.
8. Compile and upload the firmware to the Raspberry Pi Pico.

---

# Configuration

Hardware-specific definitions are centralized in `HardwareConfig.h`.

Current configuration:

```cpp
constexpr uint8_t DISPLAY_CS_PIN   = 9;
constexpr uint8_t DISPLAY_DC_PIN   = 8;
constexpr uint8_t DISPLAY_RST_PIN  = 12;
constexpr uint8_t DISPLAY_BUSY_PIN = 13;

constexpr uint8_t UP2STREAM_UART_TX_PIN = 4;
constexpr uint8_t UP2STREAM_UART_RX_PIN = 5;

#define UP2STREAM_SERIAL Serial2
```

UART configuration:

```text
115200 baud
8 data bits
No parity
1 stop bit
No flow control
```

The UART receive FIFO is increased to 128 bytes to reduce the risk of losing characters while longer messages are received, particularly while the e-paper display is being updated.

---

# Test mode

The firmware contains an optional test-data mode:

```cpp
constexpr bool USE_TEST_DATA = false;
```

Set it to:

```cpp
constexpr bool USE_TEST_DATA = true;
```

to display example player data without requiring an active UP2Stream connection.

This is useful when developing or modifying the graphical user interface.

---

# Architecture

The project is divided into several functional components:

```text
                     Arylic UP2Stream
                            │
                            │ UART 115200
                            ▼
                  ┌─────────────────────┐
                  │  Up2StreamClient    │
                  │ UART + protocol     │
                  │ parser              │
                  └──────────┬──────────┘
                             │
                             ▼
                  ┌─────────────────────┐
                  │    PlayerState      │
                  │ current player      │
                  │ information         │
                  └──────────┬──────────┘
                             │
                 ┌───────────┴───────────┐
                 ▼                       ▼
        ┌─────────────────┐      ┌─────────────────┐
        │ StateComparer   │      │  PolishTime     │
        │ change detection│      │ RTC / local time│
        └────────┬────────┘      └────────┬────────┘
                 │                       │
                 └───────────┬───────────┘
                             ▼
                  ┌─────────────────────┐
                  │      Display        │
                  │ UI / e-paper /     │
                  │ scrolling / glyphs │
                  └─────────────────────┘
```

Main source files:

| File | Purpose |
|---|---|
| `Waveshare-Pico-ePaper-Raspberry-Pi-Pico-and-Up2stream-integr.ino` | Main application |
| `Up2StreamClient.cpp/.h` | UART communication and UP2Stream protocol handling |
| `PlayerState.h` | Player state structure |
| `StateComparer.cpp/.h` | Detection of state changes |
| `Display.cpp/.h` | E-paper display and user interface |
| `HardwareConfig.h` | Central hardware/pin configuration |
| `PolishTime.cpp/.h` | Date/time and local time handling |
| `UTF8Print.cpp/.h` | UTF-8 text output support |
| `ScrollState.h` | Text scrolling state |
| `ChangeFlags.h` | Display/state change flags |
| `Theme.h` | UI layout/theme definitions |
| `fonts/` | Fonts and generated font data |
| `UP2STREAM_UART.md` | English UP2Stream UART protocol notes |
| `UP2STREAM_UART.pl.md` | Polish UP2Stream UART protocol notes |

---

# UP2Stream UART protocol

The project treats the UP2Stream interface as an **asynchronous message stream**, not as a simple request/response protocol.

Messages are terminated with a semicolon:

```text
VOL:46;
SRC:NET;
TIT:Example title;
ELP:1028/762946;
```

The firmware independently classifies received messages by their prefix.

Examples:

| Prefix | Meaning |
|---|---|
| `STA:` | System state |
| `SRC:` | Audio source |
| `PLA:` | Playback state |
| `VOL:` | Volume |
| `MUT:` | Mute |
| `TIT:` | Track title |
| `ART:` | Artist |
| `ALB:` | Album |
| `VND:` | Content provider |
| `ELP:` | Elapsed/total playback time |
| `TME:` | Device time |
| `NAM:` | Device name |
| `VER:` | Firmware/API version |

During playback, messages such as `ELP:` can be transmitted periodically by the UP2Stream without being requested by the Pico.

Therefore, the firmware must **not assume that the next received message is the response to the last command sent**.

Detailed protocol observations are documented in:

- `UP2STREAM_UART.md`
- `UP2STREAM_UART.pl.md`

---

# Time and RTC

The project can obtain the current time from the UP2Stream using:

```text
TME;
```

The returned time is used to synchronize the Raspberry Pi Pico hardware RTC.

The local date/time calculation includes handling intended for Polish/European daylight-saving-time rules.

The RTC is periodically synchronized with the UP2Stream.

Current synchronization interval:

```text
60 seconds
```

The UP2Stream's own reported UTC offset should not necessarily be treated as a correct local European offset. The application therefore performs its own local-time correction.

---

# E-paper considerations

E-paper displays are fundamentally different from conventional TFT/OLED displays. The project therefore avoids unnecessary full-screen refreshes and uses state/change detection to determine when the display needs to be updated.

The firmware handles:

- player screen refresh,
- clock screen refresh,
- standby,
- display clearing,
- restoration after standby,
- scrolling text.

The display driver currently used is:

```text
GxEPD2_213_B74
```

---

# Repository structure

```text
.
├── Waveshare-Pico-ePaper-Raspberry-Pi-Pico-and-Up2stream-integr.ino
├── HardwareConfig.h
├── Display.cpp
├── Display.h
├── Up2StreamClient.cpp
├── Up2StreamClient.h
├── PlayerState.h
├── StateComparer.cpp
├── StateComparer.h
├── PolishTime.cpp
├── PolishTime.h
├── UTF8Print.cpp
├── UTF8Print.h
├── ScrollState.h
├── ChangeFlags.h
├── Theme.h
├── UP2STREAM_UART.md
├── UP2STREAM_UART.pl.md
├── THIRD-PARTY-LICENSES.md
├── LICENSE
├── fonts/
│   ├── OFL-1.1.txt
│   ├── README.md
│   └── NotoSans/
└── .gitignore
```

---

# Known limitations

- UP2Stream UART behavior can vary between firmware versions.
- Some metadata messages are asynchronous and are not reliably returned in response to queries.
- Long track titles can be truncated by the UP2Stream device.
- `ART:` and `ALB:` are not guaranteed to be transmitted with every new title.
- `ELP:` is normally transmitted periodically by the UP2Stream and should not be treated as a conventional query response.
- The `TME:` response from the tested device may not represent the correct European daylight-saving offset.
- The project has been tested primarily with an Arylic UP2Stream Pro; compatibility with other UP2Stream variants should be verified.

---

# Development

The project is intentionally structured so that hardware configuration, communication, state handling and display rendering can be modified independently.

Possible future development areas include:

- support for additional UP2Stream models,
- additional display variants,
- improved power management,
- deeper standby/sleep integration,
- additional playback metadata,
- configurable UI layouts,
- additional languages,
- further reduction of e-paper refreshes,
- extended protocol compatibility across UP2Stream firmware versions.

---

# Third-party components and licenses

This project uses third-party software and font resources.

The project's source code is distributed under **GNU General Public License v3.0 (GPL-3.0)** unless otherwise stated.

Noto Sans fonts included in the repository are distributed under the **SIL Open Font License 1.1 (OFL-1.1)**.

For details about third-party components and their licenses, see:

```text
THIRD-PARTY-LICENSES.md
```

The complete Noto Sans OFL 1.1 license text is included in:

```text
fonts/OFL-1.1.txt
```

The project does not relicense third-party libraries or font software.

---

# License

The original source code of this project is distributed under the **GNU General Public License v3.0**.

See [`LICENSE`](LICENSE) for the complete license text.

Third-party components remain subject to their respective licenses. See [`THIRD-PARTY-LICENSES.md`](THIRD-PARTY-LICENSES.md).

---

# Author

**Waldemar Brzeziński**

GitHub repository:

`wbrzezin/Waveshare-Pico-ePaper-Raspberry-Pi-Pico-and-Up2stream-integration`

---

# Polski

## Wyświetlacz e-paper Waveshare dla Arylic UP2Stream

Projekt jest opartym na **Raspberry Pi Pico (RP2040)** wyświetlaczem e-paper przeznaczonym do współpracy z platformą audio **Arylic UP2Stream**.

Raspberry Pi Pico komunikuje się z modułem **Arylic UP2Stream Pro** przez UART, a informacje o odtwarzaniu i stanie urządzenia są prezentowane na **2,13-calowym, czarno-białym wyświetlaczu e-paper firmy Waveshare**.

Firmware został podzielony na niezależne komponenty odpowiedzialne za komunikację UART, analizę protokołu UP2Stream, zarządzanie stanem odtwarzacza, obsługę wyświetlacza, przewijanie tekstu, UTF-8 oraz zegar RTC.

---

## Funkcje

- komunikacja z UP2Stream przez UART **115200 8N1**,
- asynchroniczna analiza komunikatów UP2Stream,
- rozpoznawanie źródła i stanu odtwarzania,
- rozpoznawanie dostawcy treści (`VND`),
- obsługa głośności i wyciszenia,
- wyświetlanie tytułu, wykonawcy i albumu,
- wyświetlanie czasu odtwarzania i całkowitego czasu utworu,
- wskaźnik postępu odtwarzania,
- synchronizacja sprzętowego RTC Raspberry Pi Pico,
- obsługa polskiej daty i czasu wraz z korektą czasu letniego,
- ekran odtwarzacza,
- ekran zegara w stanie bezczynności,
- automatyczne przełączanie ekranu odtwarzacza i zegara,
- wykrywanie trybu standby,
- czyszczenie wyświetlacza e-paper w standby,
- automatyczne przywracanie ekranu odtwarzacza,
- przewijanie długich tekstów,
- obsługa polskich znaków UTF-8,
- własne symbole i ikony stanu,
- opcjonalny tryb danych testowych.

---

## Status projektu

Projekt znajduje się na etapie **działającej implementacji i weryfikacji**.

Firmware był testowany z fizycznym modułem **Arylic UP2Stream Pro** oraz analizatorem UART. Repozytorium zawiera również szczegółową dokumentację zaobserwowanego protokołu UART UP2Stream.

> **Ważne:** dokumentacja protokołu UART znajdująca się w tym repozytorium łączy informacje z opublikowanej dokumentacji z obserwacjami wykonanymi na fizycznym urządzeniu. Nie należy traktować jej jako kompletnej i autorytatywnej specyfikacji wszystkich wersji firmware UP2Stream.

---

# Sprzęt

## Wymagany sprzęt

- Raspberry Pi Pico / RP2040,
- Waveshare **2,13" czarno-biały wyświetlacz e-paper**,
- moduł Arylic **UP2Stream**,
- połączenie UART pomiędzy Pico i UP2Stream,
- połączenie USB do programowania/debugowania Pico.

Aktualna implementacja wykorzystuje sterownik:

```text
GxEPD2_213_B74
```

## Połączenia

### Wyświetlacz e-paper → Raspberry Pi Pico

| E-paper | Raspberry Pi Pico |
|---|---:|
| CS | GPIO 9 |
| DC | GPIO 8 |
| RST | GPIO 12 |
| BUSY | GPIO 13 |
| SPI MOSI | SPI MOSI |
| SPI SCK | SPI SCK |
| VCC | 3.3 V |
| GND | GND |

### UP2Stream → Raspberry Pi Pico

| UP2Stream | Raspberry Pi Pico |
|---|---:|
| TX | GPIO 5 (RX Pico) |
| RX | GPIO 4 (TX Pico) |
| GND | GND |

Projekt wykorzystuje UART RP2040 udostępniany jako `Serial2` przez Arduino-Pico.

> **Uwaga:** przed podłączeniem należy sprawdzić poziomy napięć oraz pinout konkretnej wersji modułu UP2Stream. Nie należy bezpośrednio łączyć interfejsów o niezgodnych poziomach logicznych.

---

# Wymagane oprogramowanie

Projekt przeznaczony jest do kompilacji w **Arduino IDE** z rdzeniem Arduino dla RP2040.

Wymagane biblioteki:

- **Arduino-Pico** firmy Earle F. Philhower, III,
- **GxEPD2**,
- **Adafruit GFX Library**.

Projekt wykorzystuje również standardowe funkcje Arduino/RP2040, między innymi SPI, UART oraz sprzętowy RTC RP2040.

W repozytorium znajdują się wygenerowane nagłówki czcionek wykorzystywanych przez interfejs.

---

# Instalacja

1. Zainstaluj Arduino IDE.
2. Zainstaluj **Raspberry Pi Pico / RP2040 Arduino core firmy Earle F. Philhower, III**.
3. Wybierz odpowiednią płytkę Raspberry Pi Pico.
4. Zainstaluj biblioteki:
   - `GxEPD2`,
   - `Adafruit GFX Library`.
5. Otwórz:

```text
Waveshare-Pico-ePaper-Raspberry-Pi-Pico-and-Up2stream-integr.ino
```

6. Sprawdź konfigurację sprzętową w:

```text
HardwareConfig.h
```

7. Podłącz sprzęt.
8. Skompiluj i wgraj firmware do Raspberry Pi Pico.

---

# Konfiguracja

Konfiguracja sprzętowa znajduje się w `HardwareConfig.h`.

Aktualna konfiguracja:

```cpp
constexpr uint8_t DISPLAY_CS_PIN   = 9;
constexpr uint8_t DISPLAY_DC_PIN   = 8;
constexpr uint8_t DISPLAY_RST_PIN  = 12;
constexpr uint8_t DISPLAY_BUSY_PIN = 13;

constexpr uint8_t UP2STREAM_UART_TX_PIN = 4;
constexpr uint8_t UP2STREAM_UART_RX_PIN = 5;

#define UP2STREAM_SERIAL Serial2
```

Konfiguracja UART:

```text
115200 baud
8 bitów danych
brak parzystości
1 bit stopu
brak kontroli przepływu
```

Bufor FIFO odbiornika UART został zwiększony do 128 bajtów, aby zmniejszyć ryzyko utraty znaków podczas odbierania dłuższych komunikatów, szczególnie w czasie aktualizacji wyświetlacza e-paper.

---

# Tryb testowy

Firmware posiada opcjonalny tryb danych testowych:

```cpp
constexpr bool USE_TEST_DATA = false;
```

Po zmianie na:

```cpp
constexpr bool USE_TEST_DATA = true;
```

wyświetlane są przykładowe dane odtwarzacza bez konieczności podłączania aktywnego modułu UP2Stream.

Tryb ten jest przydatny podczas projektowania i modyfikowania interfejsu użytkownika.

---

# Architektura

Projekt jest podzielony na kilka funkcjonalnych komponentów:

```text
                     Arylic UP2Stream
                            │
                            │ UART 115200
                            ▼
                  ┌─────────────────────┐
                  │  Up2StreamClient    │
                  │ UART + parser       │
                  │ protokołu           │
                  └──────────┬──────────┘
                             │
                             ▼
                  ┌─────────────────────┐
                  │    PlayerState      │
                  │ informacje o        │
                  │ odtwarzaczu         │
                  └──────────┬──────────┘
                             │
                 ┌───────────┴───────────┐
                 ▼                       ▼
        ┌─────────────────┐      ┌─────────────────┐
        │ StateComparer   │      │  PolishTime     │
        │ wykrywanie zmian│      │ RTC / czas      │
        └────────┬────────┘      └────────┬────────┘
                 │                       │
                 └───────────┬───────────┘
                             ▼
                  ┌─────────────────────┐
                  │      Display        │
                  │ UI / e-paper /     │
                  │ przewijanie /      │
                  │ symbole             │
                  └─────────────────────┘
```

---

# Protokół UART UP2Stream

Interfejs UART UP2Stream należy traktować jako **asynchroniczny strumień komunikatów**, a nie jako prosty protokół pytanie–odpowiedź.

Komunikaty kończą się średnikiem:

```text
VOL:46;
SRC:NET;
TIT:Przykładowy tytuł;
ELP:1028/762946;
```

Firmware klasyfikuje każdy odebrany komunikat niezależnie, na podstawie jego prefiksu.

Przykłady:

| Prefiks | Znaczenie |
|---|---|
| `STA:` | stan systemu |
| `SRC:` | źródło audio |
| `PLA:` | stan odtwarzania |
| `VOL:` | głośność |
| `MUT:` | wyciszenie |
| `TIT:` | tytuł utworu |
| `ART:` | wykonawca |
| `ALB:` | album |
| `VND:` | dostawca treści |
| `ELP:` | czas odtwarzania / czas całkowity |
| `TME:` | czas urządzenia |
| `NAM:` | nazwa urządzenia |
| `VER:` | wersja firmware/API |

Podczas odtwarzania UP2Stream może samoczynnie wysyłać komunikaty `ELP:`.

Dlatego firmware **nie może zakładać, że kolejny odebrany komunikat jest odpowiedzią na ostatnio wysłane zapytanie**.

Szczegółowe informacje znajdują się w:

- `UP2STREAM_UART.md` — dokumentacja angielska,
- `UP2STREAM_UART.pl.md` — dokumentacja polska.

---

# Czas i RTC

Projekt może pobierać aktualny czas z UP2Stream za pomocą:

```text
TME;
```

Odebrany czas jest wykorzystywany do synchronizacji sprzętowego RTC Raspberry Pi Pico.

Obliczenia lokalnej daty i czasu uwzględniają reguły czasu letniego stosowane w Polsce i Europie.

RTC jest okresowo synchronizowany z UP2Stream.

Aktualny okres synchronizacji:

```text
60 sekund
```

Offset czasowy zwracany przez UP2Stream nie powinien być bezpośrednio traktowany jako prawidłowy lokalny offset europejski. Aplikacja wykonuje własną korektę czasu lokalnego.

---

# Wyświetlacz e-paper

Wyświetlacze e-paper różnią się sposobem pracy od klasycznych TFT/OLED. Projekt ogranicza więc niepotrzebne pełne odświeżenia ekranu i wykorzystuje mechanizm wykrywania zmian stanu.

Firmware obsługuje między innymi:

- odświeżanie ekranu odtwarzacza,
- ekran zegara,
- standby,
- czyszczenie ekranu,
- przywracanie ekranu po standby,
- przewijanie tekstu.

Aktualnie używany sterownik:

```text
GxEPD2_213_B74
```

---

# Ograniczenia

- Zachowanie interfejsu UART może różnić się pomiędzy wersjami firmware UP2Stream.
- Część komunikatów z metadanymi jest asynchroniczna i nie jest niezawodnie zwracana w odpowiedzi na zapytania.
- Długie tytuły utworów mogą być skracane przez urządzenie UP2Stream.
- `ART:` i `ALB:` nie muszą być wysyłane przy każdej zmianie tytułu.
- `ELP:` jest zazwyczaj okresowo wysyłany przez UP2Stream i nie powinien być traktowany jak klasyczna odpowiedź na zapytanie.
- Odpowiedź `TME:` testowanego urządzenia może nie zawierać prawidłowego europejskiego offsetu czasu letniego.
- Projekt był przede wszystkim testowany z Arylic UP2Stream Pro; zgodność z innymi wariantami UP2Stream powinna zostać zweryfikowana.

---

# Rozwój projektu

Architektura została przygotowana tak, aby konfiguracja sprzętowa, komunikacja, zarządzanie stanem oraz renderowanie interfejsu mogły być rozwijane niezależnie.

Możliwe kierunki dalszego rozwoju:

- obsługa kolejnych modeli UP2Stream,
- obsługa kolejnych wersji wyświetlaczy,
- rozbudowane zarządzanie energią,
- głębsza integracja z trybem standby/sleep,
- obsługa dodatkowych metadanych odtwarzania,
- konfigurowalne układy interfejsu,
- kolejne wersje językowe,
- dalsze ograniczenie liczby odświeżeń e-paper,
- zwiększenie kompatybilności z różnymi wersjami firmware UP2Stream.

---

# Komponenty zewnętrzne i licencje

Projekt wykorzystuje zewnętrzne biblioteki programistyczne oraz zasoby czcionek.

Kod źródłowy projektu jest rozpowszechniany na licencji **GNU General Public License v3.0 (GPL-3.0)**, chyba że w konkretnym pliku lub katalogu zaznaczono inaczej.

Czcionki Noto Sans znajdujące się w repozytorium są rozpowszechniane na licencji **SIL Open Font License 1.1 (OFL-1.1)**.

Szczegółowe informacje dotyczące komponentów zewnętrznych i ich licencji znajdują się w:

```text
THIRD-PARTY-LICENSES.md
```

Pełny tekst licencji OFL 1.1 znajduje się w:

```text
fonts/OFL-1.1.txt
```

Projekt nie zmienia ani nie nadaje własnych licencji zewnętrznym bibliotekom i fontom.

---

# Licencja

Własny kod źródłowy projektu jest rozpowszechniany na licencji **GNU General Public License v3.0**.

Pełny tekst licencji znajduje się w pliku [`LICENSE`](LICENSE).

Komponenty zewnętrzne pozostają objęte ich własnymi licencjami. Szczegóły znajdują się w [`THIRD-PARTY-LICENSES.md`](THIRD-PARTY-LICENSES.md).

---

# Autor

**Waldemar Brzeziński**

Repozytorium GitHub:

`wbrzezin/Waveshare-Pico-ePaper-Raspberry-Pi-Pico-and-Up2stream-integration`
