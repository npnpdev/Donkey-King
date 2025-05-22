# Donkey King

> Klasyczna gra Donkey Kong zaimplementowana w C/C++ z użyciem SDL2, z pełną mechaniką platformową, animacjami i systemem etapów.

---

## Spis treści

* [Opis projektu](#opis-projektu)
* [Główne cechy](#g%C5%82%C3%B3wne-cechy)
* [Wymagania i ograniczenia](#wymagania-i-ograniczenia)
* [Budowa i uruchomienie](#budowa-i-uruchomienie)
* [Sterowanie](#sterowanie)
* [Architektura plików](#architektura-plik%C3%B3w)
* [Kontakt](#kontakt)
* [Licencja](#licencja)

---

## Opis projektu

**Donkey King** to odświeżona wersja klasycznej gry Donkey Kong. Gracz wspina się po platformach, przeskakuje beczki wyrzucane przez małpę i ratuje księżniczkę. Aplikacja używa **SDL2** do renderowania grafiki 2D, odtwarzania animacji i obsługi wejścia od gracza.

---

## Główne cechy

* **Platformowa rozgrywka**: poruszanie w lewo/prawo, wspinanie się po drabinach, skoki z fizyką grawitacji.
* **Dynamiczne animacje**: wielu klatek dla postaci, beczek, małpy i efektów punktów.
* **System etapów**: trzy zróżnicowane poziomy ładowane z plików `.donkey`, z preambułą weryfikującą liczbę obiektów.
* **Zarzadzanie obiektami**: platformy, drabiny, beczki (statyczne i ruchome), małpy, trofea, księżniczki.
* **Punktacja i życie**: zdobywanie punktów za przeskoczenie beczki i zebranie trofeum, system żyć i ekran Game Over z zapisem wyniku.
* **Menu i UI**: ekran startowy, wybór etapu, przegląd najlepszych wyników, pasek życia i wynik w czasie gry.

---

## Wymagania i ograniczenia

* **Język**: C/C++ 
* **Biblioteki**: **SDL2** (bez dodatkowych zewnętrznych bibliotek)
* **Formaty**: bitmapy `.bmp` wykorzystywane do tekstur, pliki `.donkey` definiujące etapy
* **Funkcje**: krótkie, jednofunkcyjne procedury zgodnie z wymaganiami zadania

---

## Budowa i uruchomienie

Ważne jest, żeby wypakować bibliotekę SDL2 z pliku .rar

---

## Sterowanie

| Akcja               | Klawisz            |
| ------------------- | ------------------ |
| Ruch w prawo/lewo   | Strzałki → / ←     |
| Wspinaczka góra/dół | Strzałki ↑ / ↓     |
| Skok                | SPACJA             |
| Nowa gra            | `N`                |
| Wybór etapu         | `E` (1,2,3 w menu) |
| Wyniki              | `W`                |
| Wyjście             | ESC                |

---

## Kontakt

* **Autor**: Igor Tomkowicz
* GitHub: [npnpdev](https://github.com/npnpdev)
* E-mail: [npnpdev@gmail.com](mailto:npnpdev@gmail.com)

---

## Licencja

Projekt udostępniony na licencji **MIT**.
