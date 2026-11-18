#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include "engine.h"

using emscripten::val;

static Position game = startingPosition();
static int lastScore = 0;   // engine's latest evaluation, White's point of view (centipawns)

void newGame() {
    game = startingPosition();
    lastScore = 0;
}

std::string getBoard()    { return game.board; }
bool        whiteToMove() { return game.whiteToMove; }
int         evaluation()  { return lastScore; }

// Plays a move. Returns it in notation (like "Nxf7+"), or "" if it's illegal.
std::string playMove(int from, int to, std::string promo) {
    if (from < 0 || from > 63 || to < 0 || to > 63) return "";
    Move m = {from, to, promo.empty() ? (char)0 : promo[0]};
    if (!isFullyLegal(game, m)) return "";
    std::string san = moveToSAN(game, m);
    game = makeMove(game, m);
    return san;
}

// The engine thinks and plays. onConsider(san) is called for each move it examines.
// Returns JSON like: {"san":"Nf6","from":6,"to":21,"score":-35,"nodes":12408}
std::string engineMove(int depth, val onConsider) {
    if (generateMoves(game).empty()) return "";

    Position before = game;
    SearchResult r = searchBestMove(game, depth, [&](const Move& m) {
        onConsider(moveToSAN(before, m));
    });

    std::string san = moveToSAN(before, r.best);
    lastScore = before.whiteToMove ? r.score : -r.score;
    game = makeMove(game, r.best);

    return "{\"san\":\"" + san + "\","
           "\"from\":"  + std::to_string(r.best.from) + ","
           "\"to\":"    + std::to_string(r.best.to)   + ","
           "\"score\":" + std::to_string(lastScore)   + ","
           "\"nodes\":" + std::to_string(r.nodes)     + "}";
}

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

// NEW: every legal move at once, like "52-36,52-44,57-42,...".
// The page keeps this list so clicking a piece is instant (no waiting on the worker).
std::string allLegalMoves() {
    std::string out;
    for (const Move& m : generateMoves(game)) {
        if (!out.empty()) out += ',';
        out += std::to_string(m.from) + "-" + std::to_string(m.to);
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

EMSCRIPTEN_BINDINGS(chess) {
    emscripten::function("newGame",       &newGame);
    emscripten::function("getBoard",      &getBoard);
    emscripten::function("whiteToMove",   &whiteToMove);
    emscripten::function("evaluation",    &evaluation);
    emscripten::function("playMove",      &playMove);
    emscripten::function("engineMove",    &engineMove);
    emscripten::function("legalTargets",  &legalTargets);
    emscripten::function("allLegalMoves", &allLegalMoves);
    emscripten::function("gameStatus",    &gameStatus);
}
