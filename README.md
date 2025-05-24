# Donkey King

[English](#english-version) | [Polski](#wersja-polska)

---

## English Version

### Project Description

**Donkey King** is a modern remake of the classic Donkey Kong game implemented in C/C++ using **SDL2**. The player climbs platforms, dodges barrels thrown by the ape, and rescues the princess. The application uses SDL2 for 2D rendering, animations, and input handling.

### Key Features

* **Platforming Gameplay**: move left/right, climb ladders, and jump with gravity physics.
* **Dynamic Animations**: multi-frame animations for the player, barrels, the ape, and score effects.
* **Stage System**: three distinct levels loaded from `.donkey` files, with a preamble validating object counts.
* **Object Management**: platforms, ladders, barrels (static and rolling), ape, trophies, and princess.
* **Scoring & Lives**: earn points by jumping barrels and collecting trophies, life system, and Game Over screen with score display.
* **Menu & UI**: title screen, stage selection, high-score display, in-game life bar, and score counter.

### Requirements & Constraints

* **Languages**: C and C++
* **Libraries**: **SDL2**, SDL2\_image (no other external dependencies)
* **Asset Formats**: `.bmp` for textures, `.donkey` for level definitions
* **Code Style**: short, single-purpose functions per assignment guidelines

### Controls

| Action          | Key                   |
| --------------- | --------------------- |
| Move Right/Left | Arrow → / ←           |
| Climb Up/Down   | Arrow ↑ / ↓           |
| Jump            | SPACE                 |
| New Game        | `N`                   |
| Select Stage    | `E` (1, 2, 3 in menu) |
| High Scores     | `W`                   |
| Quit            | ESC                   |

### File Structure

```text
DonkeyKing/
├── assets/             # .bmp textures, .donkey level files
├── src/
│   ├── main.cpp        # entry point and game loop
│   ├── Renderer.cpp    # SDL2 rendering and animation logic
│   ├── Input.cpp       # input handling
│   ├── LevelLoader.cpp # parsing .donkey files
│   ├── Entities/       # definitions for Player, Barrel, Ape, etc.
│   └── UI/             # menus and HUD rendering
├── lib/                # SDL2 and SDL2_image binaries (extracted)
└── README.md           # documentation
```

---

## Wersja polska

### Opis projektu

**Donkey King** to nowa wersja klasycznej gry Donkey Kong napisana w C/C++ z wykorzystaniem **SDL2**. Gracz wspina się po platformach, unika beczek wyrzucanych przez małpę i ratuje księżniczkę. Aplikacja korzysta z SDL2 do renderowania grafiki 2D, animacji i obsługi wejścia.

### Główne cechy

* **Platformowa rozgrywka**: poruszanie w lewo/prawo, wspinaczka po drabinach, skoki z fizyką grawitacji.
* **Dynamiczne animacje**: wieloklatkowe animacje dla postaci, beczek, małpy i efektów punktów.
* **System etapów**: trzy zróżnicowane poziomy ładowane z plików `.donkey`, z weryfikacją liczby obiektów.
* **Zarządzanie obiektami**: platformy, drabiny, beczki (statyczne i toczące się), małpa, trofea i księżniczka.
* **Punktacja i życie**: zdobywanie punktów za przeskoczenie beczki i zebranie trofeum, system żyć, ekran Game Over z wyświetleniem wyniku.
* **Menu i interfejs**: ekran startowy, wybór etapu, przegląd najlepszych wyników, pasek życia i licznik punktów w trakcie gry.

### Wymagania i ograniczenia

* **Języki**: C i C++
* **Biblioteki**: **SDL2**, SDL2\_image (bez dodatkowych zależności)
* **Formaty zasobów**: `.bmp` dla tekstur, `.donkey` dla definicji poziomów
* **Styl kodu**: krótkie, jednofunkcyjne procedury zgodnie z wymaganiami

### Sterowanie

| Akcja                 | Klawisz              |
| --------------------- | -------------------- |
| Ruch w prawo/lewo     | Strzałki → / ←       |
| Wspinaczka w górę/dół | Strzałki ↑ / ↓       |
| Skok                  | SPACJA               |
| Nowa gra              | `N`                  |
| Wybór etapu           | `E` (1, 2, 3 w menu) |
| Wyniki                | `W`                  |
| Wyjście               | ESC                  |

---

## Kontakt / Contact

* **Igor Tomkowicz**
* GitHub: [npnpdev](https://github.com/npnpdev)
* LinkedIn: [Igor Tomkowicz](https://www.linkedin.com/in/igor-tomkowicz-a5760b358/)
* E-mail: [npnpdev@gmail.com](mailto:npnpdev@gmail.com)

---

## Licencja / License

Project available under the **MIT** license. See [LICENSE](LICENSE) for details.
