#include <emscripten/bind.h>
#include <string>
#include "engine.h"

// The one game the web page is playing
static Position game = startingPosition();

void newGame() {
    game = startingPosition();
}

std::string getBoard() {
    return game.board;
}

bool whiteToMove() {
    return game.whiteToMove;
}

// Tries a move. Returns true if it was legal (and plays it).
bool playMove(int from, int to, std::string promo) {
    if (from < 0 || from > 63 || to < 0 || to > 63) return false;
    Move m = {from, to, promo.empty() ? (char)0 : promo[0]};
    if (!isFullyLegal(game, m)) return false;
    game = makeMove(game, m);
    return true;
}

// The engine thinks and plays. Returns its move like "e7e5".
std::string engineMove(int depth) {
    if (generateMoves(game).empty()) return "";
    Move m = findBestMove(game, depth);
    game = makeMove(game, m);
    return squareName(m.from) + squareName(m.to);
}

// Where can the piece on this square go? Returns "36,44" style list.
// (We'll use this to highlight moves on the board.)
std::string legalTargets(int from) {
    std::string out;
    for (const Move& m : generateMoves(game)) {
        if (m.from == from) {
            if (!out.empty()) out += ',';
            out += std::to_string(m.to);
        }
    }
    return out;
}

std::string gameStatus() {
    bool inCheck = isInCheck(game.board, game.whiteToMove);
    bool canMove = !generateMoves(game).empty();
    if (inCheck && !canMove) return "checkmate";
    if (!canMove)            return "stalemate";
    if (inCheck)             return "check";
    return "playing";
}

// The "bridge": these names become callable from JavaScript
EMSCRIPTEN_BINDINGS(chess) {
    emscripten::function("newGame",      &newGame);
    emscripten::function("getBoard",     &getBoard);
    emscripten::function("whiteToMove",  &whiteToMove);
    emscripten::function("playMove",     &playMove);
    emscripten::function("engineMove",   &engineMove);
    emscripten::function("legalTargets", &legalTargets);
    emscripten::function("gameStatus",   &gameStatus);
}