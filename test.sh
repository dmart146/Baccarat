#!/usr/bin/env bash
# Build and test the baccarat game.
# Usage:
#   ./test.sh          — build and drop into interactive play (6-deck, $1000)
#   ./test.sh auto     — run automated smoke tests with scripted inputs
#   ./test.sh clean    — clean build artifacts

set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

# ── Build ──────────────────────────────────────────────────────────────────────
build() {
    echo "==> Building..."
    make -s
    echo "==> Build OK"
}

# ── Interactive mode ───────────────────────────────────────────────────────────
play() {
    echo ""
    echo "Starting interactive game: 6 decks, \$1000 starting balance."
    echo "Examples:  'B 100'   'P 50 D 25'   'P 100 PA 10'   'quit'"
    echo ""
    ./baccarat 6 1000
}

# ── Smoke tests ───────────────────────────────────────────────────────────────
run_tests() {
    echo ""
    echo "==> Smoke test 1: Basic player bet, then quit"
    printf "P 100\nquit\n" | ./baccarat 1 500
    echo ""

    echo "==> Smoke test 2: Banker bet with dragon side bet"
    printf "B 50 D 10\nquit\n" | ./baccarat 1 500
    echo ""

    echo "==> Smoke test 3: Player bet with panda side bet"
    printf "P 50 PA 10\nquit\n" | ./baccarat 1 500
    echo ""

    echo "==> Smoke test 4: All three bets at once"
    printf "B 100 D 25 PA 10\nquit\n" | ./baccarat 1 1000
    echo ""

    echo "==> Smoke test 5: Invalid input is rejected gracefully"
    printf "invalid\nX 100\nP 0\nP 100\nquit\n" | ./baccarat 1 500
    echo ""

    echo "==> Smoke test 6: Bet exceeding balance is rejected"
    printf "P 9999\nquit\n" | ./baccarat 1 100
    echo ""

    echo "==> All smoke tests passed."
}

# ── Entry point ────────────────────────────────────────────────────────────────
case "${1:-play}" in
    auto)   build; run_tests ;;
    clean)  make clean; echo "==> Cleaned." ;;
    play|*) build; play ;;
esac
