# Sprawozdanie z projektu „Symulator Windy"

**Autorzy:** Bartłomiej Fedorowicz, Jakub Gomułkiewicz

---

## 1. Cel projektu
Celem projektu jest stworzenie prostej symulacji działania windy w budynku pięciopiętrowym. Aplikacja wizualizuje jazdę windy, ładowanie i rozładowywanie pasażerów oraz obsługę przycisków wywołania na każdym piętrze.

## 2. Struktura plików

- **SymulatorWindy.cpp** – punkt wejścia do aplikacji. Inicjalizuje okno graficzne i uruchamia pętlę komunikatów.
- **GUI.h** – definicja klasy `GdiplusWindow`, która zarządza oknem, grafiką, animacjami, przyciskami i tekstami.
- **ElevatorLogic.cpp** – implementacja mechaniki windy w klasie `ElevatorLogic`:
  - Obsługa załadunku i rozładunku pasażerów na aktualnym piętrze.
  - Decydowanie o kierunku jazdy na podstawie żądań z poszczególnych pięter.
  - Animacja ruchu windy i pasażerów.

## 3. Opis działania

1. **Inicjalizacja**
   - Tworzony jest obiekt `GdiplusWindow` o zadanych wymiarach i tle.
   - Inicjalizowana jest instancja `ElevatorLogic`, powiązana z oknem.
   - Do okna dodawane są przyciski wywołania windy na każdym piętrze (po lewej i prawej stronie).

2. **Pętla główna**
   - Odczytywane są komunikaty systemowe (np. zamknięcie okna).
   - Wywoływana jest metoda `elevatorLoop()`, która:
     1. Rozładowuje pasażerów, których docelowe piętro to obecne piętro windy.
     2. Ładuje pasażerów oczekujących na aktualnym piętrze i chcących jechać w aktualnym kierunku.
     3. Określa nowy kierunek windy (w górę lub w dół) oraz przesuwa windę o jedno piętro.
     4. Animuje przesunięcie windy i pasażerów.
   - Okno aktualizuje wszystkie animacje sprite’ów.

3. **Obsługa przycisków**
   - Po kliknięciu przycisku wywołania na dowolnym piętrze tworzony jest nowy obiekt pasażera w kolejce odpowiedniego piętra.

## 4. Wymagania i zależności

- Biblioteka **GDI+** (Windows).
- Kompilator C++ wspierający standard C++11 lub nowszy.
- Ścieżki do obrazków w folderze `zdjencia` (np. `winda.png`, `0ludziknonbasic.png` itd.).

## 5. Sposób uruchomienia

1. Skompilować projekt (np. za pomocą Visual Studio).
2. Umieścić pliki wykonywalne i folder `zdjencia` w jednym katalogu.
3. Uruchomić plik `SymulatorWindy.exe`.

## 6. Możliwe rozszerzenia

- Obsługa większej liczby pięter.
- Różne typy pasażerów (o różnych wagach i prędkościach).
- System kolejkowania z priorytetami.
- Interaktywna zmiana parametrów windy (np. prędkość, pojemność).

## 7. Podsumowanie
Projekt przedstawia podstawową strukturę symulatora windy, uwzględniającą najważniejsze aspekty: interfejs użytkownika, logikę obsługi pasażerów oraz animacje. Stanowi dobrą bazę do dalszych rozbudów i eksperymentów.

