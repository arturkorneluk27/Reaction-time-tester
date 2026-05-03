# Refleksomierz oparty na FRDM-KL05Z

## Opis Projektu

 Celem projektu było stworzenie urządzenia/gry sprawdzającej szybkość czasu reakcji u dwóch osób grających. Projekt został napisany w języku C w środowisku Keil uVision. 


## Warstwa sprzętowa
Do realizacji projektu wykorzystano:
1.  **Mikrokontroler:** NXP Kinetis MKL05Z4VFM4 (ARM Cortex-M0+ @ 48MHz).
2.  **Płytka:** FRDM-KL05Z.
3.  **Wyświetlacz:** LCD 2x16 (sterownik zgodny z HD44780).
4.  **Klawiatura:** Matrycowa 4x4 HW-834.
5.  **Głośnik:** Moduł WSR-04489


## Struktura projektu
* **main.c**
    * główna logika gry
    * generator losowy
    * licznik czasu
    * obsługa przerwania SysTick

* **klaw.c / klaw.h**
    * obsługa 4 przycisków na przerwaniach z eliminacją drgań zestyków

* **lcd1602.c / lcd1602.h**
    * sterowanie ekranem LCD

* **i2c.c / i2c.h**
    * komunikacja i2c

* **DAC.c / DAC.h**
    * obsługa przetwornika C/A (generator dźwięku)

* **ADC.c / ADC.h**
    * inicjalizacja i kalibracja przetwornika A/C

* **frdm_bsp.h**
    * plik konfiguracyjny platformy sprzętowej (FRDM-KL05Z)
 
## Instrukcja użytkownika

  * Podłącz płytkę do zasilania. Poczekaj na napis „REFLEKSOMIERZ  S1-START”.
  * Naciśnij przycisk S1 i rozpocznij grę.
  * Poczekaj na komunikat „Czekaj…”. Nie naciskaj żadnych przycisków w tym czasie. Unikaj falstartu.
  * Czekaj na sygnał dźwiękowy (4-10 sekund).
  * Po sygnale szybko naciśnij przycisk:
    - Gracz 1: S2
    - Gracz 2: S4
  * Sprawdź wynik rundy wyświetlany przez 3 sekundy.
  * Powtórz grę przez 3 rundy.
  * Sprawdź końcowy wynik i zwycięzcę.
  * Naciśnij S1, aby rozpocząć nową grę.
