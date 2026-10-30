# cardcalc

Small command-line poker hand calculators in C++, built mainly to help with hand decisions in **Balatro**.

Each program deals a 7-card hand from a shuffled 52-card deck, shows what poker hands you currently have, and lets you discard and redraw until the deck runs out.

## Programs

| File | What it does |
|------|--------------|
| `card2.cpp` | The main calculator. Shows your possible hands and can calculate the exact odds of every hand type for a discard **before** you make it. |
| `handfinder.cpp` | Simpler version: deals, discards, and lists the hands you have. |
| `calc.cpp` | The original basic deal-and-discard loop. |

## Build

Requires a C++17 compiler (e.g. g++):

```bash
g++ -std=c++17 -O2 card2.cpp -o card2
./card2
```

## Usage

Cards are written as **suit + value**:

- Suits: `H` (Hearts), `D` (Diamonds), `C` (Clubs), `S` (Spades)
- Values: `2`–`10`, `J`, `Q`, `K`, `A`
- Examples: `H2`, `D10`, `CK`, `SA`

At the prompt:

- `H2, D5` — discard those cards and draw replacements
- `? H2, D5` — show the odds of each hand if you discarded those cards, without actually discarding (1–5 cards, `card2` only)
- `quit` or `q` — exit

Example — hand `S4 H5 DK S2 HJ S6 C10`, checking `? S4, H5`:

```
If you discard 2 card(s) and draw 2 from 45 remaining:
  Royal Flush        0.00%
  Straight Flush     0.00%
  Four of a Kind     0.00%
  Full House         0.00%
  Flush              0.00%
  Straight           3.23%
  Three of a Kind    1.52%
  Two Pair           9.09%
  Pair              60.30%
  High Card        100.00%
(990 possible draws checked)
```

Odds are exact — every possible draw from the remaining deck is checked, not simulated. Each line is the chance your new hand *contains* that hand type, so a full house also counts toward pair, three of a kind, etc.
