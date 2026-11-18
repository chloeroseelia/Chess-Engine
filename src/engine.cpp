#pragma once   // makes sure this file is only included once

#include <string>
#include <vector>

// ===================== DATA =====================

struct Position {
    std::string board;           // 64 chars, a8 first, h1 last. '.' = empty
    bool whiteToMove = true;
    bool castleWK = true;        // White kingside
    bool castleWQ = true;        // White queenside
    bool castleBK = true;        // Black kingside
    bool castleBQ = true;        // Black queenside
    int  epSquare = -1;          // en passant target square, -1 = none
};

struct Move {
    int  from;
    int  to;
    char promo;                  // 'q','r','b','n' or 0
};

// ===================== THE ENGINE'S "MENU" =====================

Position startingPosition();

int         squareIndex(char fileChar, char rankChar);   // "e2" -> 52
std::string squareName(int sq);                          // 52 -> "e2"

bool isWhite(char piece);
bool isBlack(char piece);

bool isInCheck(const std::string& board, bool white);
bool isPseudoLegal(const Position& pos, int from, int to);
bool isFullyLegal(const Position& pos, const Move& m);

Position          makeMove(const Position& pos, const Move& m);
std::vector<Move> generateMoves(const Position& pos);
Move              findBestMove(const Position& pos, int depth);