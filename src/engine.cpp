#include "engine.h"
#include <cstdlib>   // std::abs
#include <cctype>    // std::tolower, std::toupper

// ===================== SETUP =====================

Position startingPosition() {
    Position pos;
    pos.board =
        "rnbqkbnr"
        "pppppppp"
        "........"
        "........"
        "........"
        "........"
        "PPPPPPPP"
        "RNBQKBNR";
    return pos;
}

// ===================== BOARD BASICS =====================

int squareIndex(char fileChar, char rankChar) {
    if (fileChar < 'a' || fileChar > 'h') return -1;
    if (rankChar < '1' || rankChar > '8') return -1;
    int file = fileChar - 'a';
    int rank = rankChar - '0';
    int row  = 8 - rank;
    return row * 8 + file;
}

std::string squareName(int sq) {
    std::string name;
    name += char('a' + sq % 8);
    name += char('0' + (8 - sq / 8));
    return name;
}

bool isWhite(char piece) { return piece >= 'A' && piece <= 'Z'; }
bool isBlack(char piece) { return piece >= 'a' && piece <= 'z'; }

// ===================== ATTACKS =====================

static bool isPathClear(const std::string& board, int from, int to) {
    int fromRow = from / 8, fromCol = from % 8;
    int toRow   = to / 8,   toCol   = to % 8;
    int stepRow = (toRow > fromRow) - (toRow < fromRow);
    int stepCol = (toCol > fromCol) - (toCol < fromCol);

    int r = fromRow + stepRow;
    int c = fromCol + stepCol;
    while (r != toRow || c != toCol) {
        if (board[r * 8 + c] != '.') return false;
        r += stepRow;
        c += stepCol;
    }
    return true;
}

static bool attacks(const std::string& board, int from, int to) {
    char piece = board[from];
    int fromRow = from / 8, fromCol = from % 8;
    int toRow   = to / 8,   toCol   = to % 8;
    int dRow = toRow - fromRow;
    int dCol = toCol - fromCol;
    int absRow = std::abs(dRow);
    int absCol = std::abs(dCol);

    switch (std::tolower(piece)) {
        case 'n': return (absRow == 2 && absCol == 1) || (absRow == 1 && absCol == 2);
        case 'k': return absRow <= 1 && absCol <= 1;
        case 'r': return (dRow == 0 || dCol == 0) && isPathClear(board, from, to);
        case 'b': return absRow == absCol && isPathClear(board, from, to);
        case 'q': return (dRow == 0 || dCol == 0 || absRow == absCol)
                         && isPathClear(board, from, to);
        case 'p': {
            int forward = isWhite(piece) ? -1 : 1;
            return absCol == 1 && dRow == forward;
        }
    }
    return false;
}

static bool isSquareAttacked(const std::string& board, int sq, bool byWhite) {
    for (int from = 0; from < 64; from++) {
        if (from == sq) continue;
        char p = board[from];
        bool ours = byWhite ? isWhite(p) : isBlack(p);
        if (ours && attacks(board, from, sq)) return true;
    }
    return false;
}

static int findKing(const std::string& board, bool white) {
    char king = white ? 'K' : 'k';
    for (int sq = 0; sq < 64; sq++) {
        if (board[sq] == king) return sq;
    }
    return -1;
}

bool isInCheck(const std::string& board, bool white) {
    return isSquareAttacked(board, findKing(board, white), !white);
}

// ===================== MOVE RULES =====================

static bool canCastle(const Position& pos, int from, int to) {
    const std::string& b = pos.board;
    bool white = isWhite(b[from]);
    int home = white ? 60 : 4;
    if (from != home) return false;

    bool kingside = to > from;
    bool hasRight = white ? (kingside ? pos.castleWK : pos.castleWQ)
                          : (kingside ? pos.castleBK : pos.castleBQ);
    if (!hasRight) return false;

    int rookSq = kingside ? home + 3 : home - 4;
    if (b[rookSq] != (white ? 'R' : 'r')) return false;
    if (!isPathClear(b, from, rookSq)) return false;

    int step = kingside ? 1 : -1;
    if (isSquareAttacked(b, from, !white)) return false;
    if (isSquareAttacked(b, from + step, !white)) return false;
    return true;
}

bool isPseudoLegal(const Position& pos, int from, int to) {
    const std::string& b = pos.board;
    char piece  = b[from];
    char target = b[to];
    int fromRow = from / 8;
    int dRow = to / 8 - fromRow;
    int dCol = to % 8 - from % 8;
    int absCol = std::abs(dCol);
    char kind = std::tolower(piece);

    if (kind == 'p') {
        int forward  = isWhite(piece) ? -1 : 1;
        int startRow = isWhite(piece) ?  6 : 1;
        if (dCol == 0 && dRow == forward && target == '.') return true;
        if (dCol == 0 && dRow == 2 * forward && fromRow == startRow
            && target == '.' && b[from + forward * 8] == '.') return true;
        if (absCol == 1 && dRow == forward && target != '.') return true;
        if (absCol == 1 && dRow == forward && to == pos.epSquare) return true;
        return false;
    }

    if (kind == 'k' && dRow == 0 && absCol == 2) {
        return canCastle(pos, from, to);
    }

    return attacks(b, from, to);
}

Position makeMove(const Position& pos, const Move& m) {
    Position next = pos;
    std::string& b = next.board;
    char piece = b[m.from];
    char kind  = std::tolower(piece);
    bool white = isWhite(piece);

    if (kind == 'p' && m.to == pos.epSquare) {
        int capturedSq = m.to + (white ? 8 : -8);
        b[capturedSq] = '.';
    }

    if (kind == 'k' && std::abs(m.to - m.from) == 2) {
        bool kingside = m.to > m.from;
        int rookFrom = kingside ? m.from + 3 : m.from - 4;
        int rookTo   = kingside ? m.from + 1 : m.from - 1;
        b[rookTo]   = b[rookFrom];
        b[rookFrom] = '.';
    }

    b[m.to]   = piece;
    b[m.from] = '.';

    int toRow = m.to / 8;
    if (kind == 'p' && (toRow == 0 || toRow == 7)) {
        char promo = m.promo ? m.promo : 'q';
        b[m.to] = white ? std::toupper(promo) : std::tolower(promo);
    }

    next.epSquare = -1;
    if (kind == 'p' && std::abs(m.to - m.from) == 16) {
        next.epSquare = (m.from + m.to) / 2;
    }

    if (piece == 'K') { next.castleWK = false; next.castleWQ = false; }
    if (piece == 'k') { next.castleBK = false; next.castleBQ = false; }
    if (m.from == 63 || m.to == 63) next.castleWK = false;
    if (m.from == 56 || m.to == 56) next.castleWQ = false;
    if (m.from == 7  || m.to == 7)  next.castleBK = false;
    if (m.from == 0  || m.to == 0)  next.castleBQ = false;

    next.whiteToMove = !pos.whiteToMove;
    return next;
}

bool isFullyLegal(const Position& pos, const Move& m) {
    char piece  = pos.board[m.from];
    char target = pos.board[m.to];
    bool white  = pos.whiteToMove;

    if (white ? !isWhite(piece) : !isBlack(piece)) return false;
    if (white ? isWhite(target) : isBlack(target)) return false;
    if (!isPseudoLegal(pos, m.from, m.to)) return false;

    Position next = makeMove(pos, m);
    return !isInCheck(next.board, white);
}

std::vector<Move> generateMoves(const Position& pos) {
    std::vector<Move> moves;
    for (int from = 0; from < 64; from++) {
        char p = pos.board[from];
        if (pos.whiteToMove ? !isWhite(p) : !isBlack(p)) continue;
        for (int to = 0; to < 64; to++) {
            Move m = {from, to, 0};
            if (isFullyLegal(pos, m)) moves.push_back(m);
        }
    }
    return moves;
}

// ===================== THE BRAIN =====================

static int pieceValue(char piece) {
    switch (std::tolower(piece)) {
        case 'p': return 100;
        case 'n': return 320;
        case 'b': return 330;
        case 'r': return 500;
        case 'q': return 900;
    }
    return 0;
}

static int centerBonus(int sq) {
    int row = sq / 8, col = sq % 8;
    return 7 - (std::abs(2 * col - 7) + std::abs(2 * row - 7)) / 2;
}

static int evaluate(const std::string& board) {
    int score = 0;
    for (int sq = 0; sq < 64; sq++) {
        char p = board[sq];
        if (p == '.') continue;
        int value = pieceValue(p);
        char kind = std::tolower(p);
        if (kind == 'n' || kind == 'b' || kind == 'p') {
            value += centerBonus(sq) * 5;
        }
        if (isWhite(p)) score += value;
        else            score -= value;
    }
    return score;
}

static const int INF  = 1000000;
static const int MATE = 100000;

static int search(const Position& pos, int depth, int alpha, int beta) {
    if (depth == 0) {
        int score = evaluate(pos.board);
        return pos.whiteToMove ? score : -score;
    }

    std::vector<Move> moves = generateMoves(pos);
    if (moves.empty()) {
        if (isInCheck(pos.board, pos.whiteToMove)) return -MATE - depth;
        return 0;
    }

    for (const Move& m : moves) {
        int score = -search(makeMove(pos, m), depth - 1, -beta, -alpha);
        if (score >= beta) return score;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

Move findBestMove(const Position& pos, int depth) {
    std::vector<Move> moves = generateMoves(pos);
    Move best = moves[0];
    int bestScore = -INF;

    for (const Move& m : moves) {
        int score = -search(makeMove(pos, m), depth - 1, -INF, -bestScore);
        if (score > bestScore) {
            bestScore = score;
            best = m;
        }
    }
    return best;
}