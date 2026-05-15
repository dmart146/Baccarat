# Baccarat — Codebase Audit

A self-hosted EZ-Baccarat game. The **game engine is a C++ binary**, fronted by a **Python (FastAPI) WebSocket bridge**, displayed in a browser as a terminal-style UI. Shell scripts (`Start` / `Close`) manage the lifecycle.

## Usage

- `./Start` — builds the C++ binary if needed, spins up a Python venv, launches the server, opens `http://localhost:8000` in your browser.
- `./Close` — stops the server and cleans build artifacts. `./Close --all` also nukes `.venv`.
- `./test.sh` — interactive CLI play (bypasses the browser). `./test.sh auto` runs scripted smoke tests.

---

## 1. Architecture at a Glance

```
   Browser (HTML/JS)  <──WebSocket──>  FastAPI server (Python)  <──stdin/stdout──>  ./baccarat (C++)
   static/index.html                   server.py                                   baccarat.cpp + cards.* + player.*
```

- **Backend language:** the *game engine* is **C++17** (compiled to a native binary, `./baccarat`). The *web bridge* is **Python 3** using **FastAPI + Uvicorn**. There is no Node.js.
- The C++ binary is a normal interactive console program; the Python server just shovels bytes between its stdin/stdout and a WebSocket.
- Every browser session spawns its own dedicated subprocess of `./baccarat`, so game state is isolated per tab.

---

## 2. File-by-File Audit

### Shell lifecycle scripts

| File | What it does |
|------|--------------|
| [`Start`](Start) | Rebuilds the binary if any `.cpp` is newer than `baccarat`; creates `.venv` and installs `requirements.txt` if missing; checks `.server.pid` and port 8000 for collisions; launches `uvicorn server:app` via `nohup` in the background, writes PID to `.server.pid`, logs to `server.log`; polls `/health` until ready; opens the browser with `open` (macOS) or `xdg-open` (Linux). |
| [`Close`](Close) | Kills the PID in `.server.pid` (SIGTERM, then SIGKILL after ~2s); belt-and-suspenders `lsof -ti:8000` + `pkill -f "uvicorn server:app"` + `pkill -x baccarat`; runs `make clean`; deletes `server.log` and `__pycache__`. `--all` also removes `.venv`. |
| [`test.sh`](test.sh) | Local CLI harness. `play` builds + runs `./baccarat 6 1000`. `auto` pipes scripted bets into the binary to verify parsing, side bets, invalid inputs, and over-balance rejection. |

### C++ game engine

| File | Role |
|------|------|
| [`baccarat.cpp`](baccarat.cpp) | `main()`, the REPL loop, bet-string parsing (regex), baccarat drawing rules, outcome evaluation. |
| [`cards.h`](cards.h) / [`cards.cpp`](cards.cpp) | `Cards` class — the shoe. Builds N standard 52-card decks, shuffles with `mt19937`, supports `draw()` and `size()`. |
| [`player.h`](player.h) / [`player.cpp`](player.cpp) | `Player` class — holds the current hand's bets (main side, Dragon7, Panda8) and computes net payout from a result code. |
| [`makefile`](makefile) | `g++ -std=c++17 -Wall -Wextra -O2`. Builds `baccarat.o`, `cards.o`, `player.o` → `baccarat`. `clean` removes those exact artifacts. |

### Web bridge

| File | Role |
|------|------|
| [`server.py`](server.py) | FastAPI app. `GET /` returns the HTML page. `GET /health` reports binary presence. `WS /ws?decks=N&money=M` spawns a per-session `./baccarat N M` subprocess and pumps stdout → WebSocket, WebSocket → stdin. Validates `decks`/`money` are integers before passing to the binary. |
| [`static/index.html`](static/index.html) | Single-page UI. Inline CSS (dark/gold terminal theme) and a ~60-line JS block that opens a WebSocket, appends incoming text to a `<div id="term">`, and sends typed input on Enter. Two config inputs (Decks, Starting $) get embedded into the WS URL. |
| [`requirements.txt`](requirements.txt) | `fastapi>=0.110`, `uvicorn[standard]>=0.27`. |

### Runtime artifacts (gitignored)

`baccarat`, `*.o`, `.venv/`, `__pycache__/`, `server.log`, `.server.pid`. See [`.gitignore`](.gitignore).

> **Note on `cards 2.o`:** this is not produced by the build. The `" 2"` suffix is the macOS Finder/iCloud duplicate-naming pattern. `make clean` correctly only removes its own outputs (`baccarat.o cards.o player.o`), which is why it survives `./Close`. Delete it manually.

---

## 3. Key Processes & Algorithms

### 3.1 How a bet like `Banker 100` (actually `B 100`) gets parsed

The user types `B 100` (or `P 50 D 25 PA 10`) into the browser input. The chain:

1. **Browser** ([`static/index.html:191-198`](static/index.html:191)) — on Enter, sends the raw string over the WebSocket via `ws.send(v)`.
2. **Python server** ([`server.py:91-96`](server.py:91)) — `msg = await websocket.receive_text()`, then writes `msg + "\n"` to the C++ subprocess's stdin. No parsing happens here.
3. **C++ binary** ([`baccarat.cpp:43-46`](baccarat.cpp:43)) — `std::getline(std::cin, input)` reads the line and matches it against this case-insensitive regex:

   ```
   ^\s*([bp])\s+(\d+)(?:\s+d\s+(\d+))?(?:\s+pa\s+(\d+))?\s*$
   ```

   Capture groups:
   - `m[1]` — `B` or `P` (main side).
   - `m[2]` — main bet amount (integer).
   - `m[3]` — optional Dragon7 side bet amount (after literal `D`).
   - `m[4]` — optional Panda8 side bet amount (after literal `PA`).

4. **Validation** ([`baccarat.cpp:79-86`](baccarat.cpp:79)) — rejects non-positive main bets and bets exceeding the current balance. On mismatch, prints `Invalid. Example: 'B 100' …` and re-prompts.
5. **Bet object** ([`baccarat.cpp:88-89`](baccarat.cpp:88)) — instantiates a fresh `Player` and calls `player.updateBets(side, betAmt, dragonAmt, pandaAmt)`.

So `Banker 100` literally would *not* parse — the accepted syntax is `B 100`. The full word isn't matched by the regex.

### 3.2 Deck / shoe logic — shuffling and dealing

Lives entirely in [`cards.cpp`](cards.cpp) and [`cards.h`](cards.h).

- **Construction** ([`cards.cpp:10-26`](cards.cpp:10)) — `Cards()` builds one 52-card deck as `std::vector<std::string>` with each entry like `"A♠"`, `"10♦"`. `Cards(int decks)` calls the single-deck constructor `decks` times and concatenates.
- **Shuffle** ([`cards.cpp:28-33`](cards.cpp:28)) — seeds `std::mt19937` from `high_resolution_clock` and calls `std::shuffle` over the vector.
- **Draw** ([`cards.cpp:35-40`](cards.cpp:35)) — pops the `back()` of the vector (O(1)) and returns the card string. Returns empty string if the shoe is empty.
- **Reshuffle trigger** ([`baccarat.cpp:51-59`](baccarat.cpp:51)) — when `deck.size() < 15`, the loop replaces `deck` with a fresh `Cards(numDecks)`, reshuffles, and burns again.
- **Burn-card rule** ([`baccarat.cpp:31-35`](baccarat.cpp:31)) — after each shuffle, the top card is "burned"; its rank value (face cards/10 = 10, ace = 1) determines how many *additional* cards are burned. This mirrors real casino procedure.

### 3.3 Game state — where the variables live

State splits across three places by lifetime:

| Scope | Where | What |
|-------|-------|------|
| **Whole session** (entire stdin/stdout lifetime of the binary) | locals in `main()` in [`baccarat.cpp:15-25`](baccarat.cpp:15) | `numDecks`, `money` (the bankroll), `deck` (the `Cards` shoe). |
| **One hand** | locals in `play()` in [`baccarat.cpp:141-153`](baccarat.cpp:141) | `playerTotal`, `bankerTotal`, the six possible card strings (`playerCard1..3`, `bankerCard1..3`), `playerThirdCard` / `bankerThirdCard` flags. |
| **Current bet** | a fresh `Player` instance per hand in [`baccarat.cpp:88`](baccarat.cpp:88), fields defined in [`player.h:19-25`](player.h:19) | `baseBet` (`"B"`/`"P"`), `baseBalance` (main wager), `dragon`/`panda` flags, `dragonBalance`, `pandaBalance`. |

There is no persistent storage — quitting the binary or closing the WebSocket throws all state away. The browser also discards everything; reloading the page starts a fresh session via the `connect()` handler in [`static/index.html:167-189`](static/index.html:167).

### 3.4 How the line `Player: 9♠ J♠ Banker: 2♠ K♥` is produced

Built incrementally inside `play()` in [`baccarat.cpp:141-203`](baccarat.cpp:141):

1. Four initial cards are drawn ([baccarat.cpp:145-148](baccarat.cpp:145)). Each `deck.draw()` returns a string like `"9♠"` (rank + Unicode suit, see [`cards.cpp:11-18`](cards.cpp:11) where suits live as actual `"♣ ♦ ♥ ♠"` literals).
2. `result` is seeded: `result = "Player: " + playerCard1 + " " + playerCard2;` ([baccarat.cpp:155](baccarat.cpp:155)).
3. **Naturals branch** ([baccarat.cpp:158-161](baccarat.cpp:158)) — if either side has 8 or 9 (mod 10), no third cards; the line becomes `Player: X Y  Banker: A B` and we return.
4. **Player draws on 0–5** ([baccarat.cpp:164-169](baccarat.cpp:164)) — if so, `playerCard3` is appended to the Player half of the string before Banker even appears.
5. **Banker third-card rules** ([baccarat.cpp:171-201](baccarat.cpp:171)) — the standard table:
   - If player did *not* draw, banker draws on 0–5.
   - If player drew, banker's action depends on its current total and the value of the player's third card (the `bm`/`p3v` ladder).
   The Banker half is then appended in the appropriate two-card or three-card form.
6. The final concatenated string is passed back through the `result` reference. `main()` prints it with `std::cout << "\n" << result << "\n"` ([baccarat.cpp:93](baccarat.cpp:93)).
7. `eval(playerTotal, bankerTotal, player3, banker3)` ([baccarat.cpp:127-139](baccarat.cpp:127)) computes mod-10 totals and returns one of: 1=Player, 2=Banker, 3=Tie, 4=Panda8 (player wins with 3-card 8), 5=Dragon7 (banker wins with 3-card 7).
8. `main()` indexes into a `labels[]` array ([baccarat.cpp:96-101](baccarat.cpp:96)) to print the outcome line, then `player.calculatePayout(outcome)` ([player.cpp:14-42](player.cpp:14)) computes net winnings: 1:1 on Player, **0.95:1 on Banker (5% commission)**, push on Tie, **40:1 on Dragon7**, **25:1 on Panda8**.

All output uses `std::cout` with `std::unitbuf` set at startup ([baccarat.cpp:13](baccarat.cpp:13)), so each write flushes immediately — essential because Python is reading from a pipe and needs prompts to arrive without buffering delays.

### 3.5 End-to-end data flow for one bet

```
User types "B 100" → keydown handler (index.html)
  → ws.send("B 100")
  → WebSocket frame to FastAPI (server.py /ws)
  → proc.stdin.write("B 100\n")
  → C++ std::getline picks it up
  → regex match → Player object → play() deals cards
  → std::cout writes "Player: …  Banker: …\nBanker wins\nNet: +95  | Balance: $1095\n"
  → asyncio reads stdout chunk (server.py pump_stdout)
  → websocket.send_text(chunk)
  → ws.onmessage in browser
  → append() to <div id="term">
```

---

## 4. Quirks & Notes

- **No authentication, no persistence.** Anyone who can reach the port can play. Bankrolls evaporate on disconnect.
- **Per-session subprocesses** mean N concurrent tabs = N `baccarat` processes. Fine for personal use; not designed for scale.
- **Input sanitization** in `server.py` is minimal but sufficient: `decks` and `money` are validated as integers before being passed as `argv` to the binary, and freeform text is only ever written to stdin (never to a shell). No command injection surface.
- **The regex is the only input grammar.** Adding new bet types (e.g. tie bets) requires extending both the regex in [baccarat.cpp:43-46](baccarat.cpp:43) and the payout logic in [player.cpp:14-42](player.cpp:14).
- **Reshuffle is naïve** — it discards mid-shoe state and starts fresh rather than truly reconstituting. Acceptable for casual play.
- **`Cards::rng_`** is declared as a member in [`cards.h:30`](cards.h:30) but unused — `shuffle()` constructs its own `mt19937` locally each call. Dead field; safe to remove.
