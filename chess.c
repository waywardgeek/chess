#include <stdlib.h>
#include <readline/readline.h>
#include "chdatabase.h"

#define ROWS 8
#define COLS 8
#define MAX_GAME_MOVES 4096
// This is used to indicated winning by taking the king.
#define WIN 10000000

// Return a score for a piece.
static inline uint32 findPieceScore(chPiece piece) {
    bool white = chPieceWhite(piece);
    // Slight bias to march pieces forward.
    uint8 advance = white? chPieceGetRow(piece) : 7 - chPieceGetRow(piece);
    switch (chPieceGetType(piece)) {
        case CH_PAWN: return 1000 + advance;
        case CH_ROOK: return 5000 + advance;
        case CH_KNIGHT: return 3000 + advance;
        case CH_BISHOP: return 3000 + advance;
        case CH_QUEEN: return 10000 + advance;
        case CH_KING: return 0;  // The special cost WIN is used when taking the king.
        default:
            utExit("Unknown piece type.");
    }
    return 0;  // Dummy return.
}

// Return name of the piece type.
static inline char *getPieceTypeName(chPieceType type) {
    switch (type) {
        case CH_PAWN: return "pawn";
        case CH_ROOK: return "rook";
        case CH_KNIGHT: return "knight";
        case CH_BISHOP: return "bishop";
        case CH_QUEEN: return "queen";
        case CH_KING: return "king";
        default:
            utExit("Unknown piece type.");
    }
    return 0;  // Dummy return.
}

// Reverse a move so we can undo moves.
static inline chMove reverseMove(chMove move) {
    chMove reversedMove;
    reversedMove.fromRow = move.toRow;
    reversedMove.fromCol = move.toCol;
    reversedMove.toRow = move.fromRow;
    reversedMove.toCol = move.fromCol;
    return reversedMove;
}

// Return the piece at (row, col).  (0, 0) is bottome left.
static inline chPiece getPieceAtPosition(chBoard board, uint8 row, uint8 col) {
    return chBoardGetiPosition(board, COLS*row + col);
}

// Return true if the square is empty.
static inline bool squareEmpty(chBoard board, uint8 row, uint8 col) {
    return getPieceAtPosition(board, row, col) == chPieceNull;
}

// Verify the computed score.
void verifyScore(chBoard board) {
    chPiece piece;
    uint32 whiteScore = 0;
    uint32 blackScore = 0;
    chForeachBoardPiece(board, piece) {
        if (chPieceInPlay(piece)) {
            if (chPieceWhite(piece)) {
                whiteScore += findPieceScore(piece);
            } else {
                blackScore += findPieceScore(piece);
            }
        }
    } chEndBoardPiece;
    utAssert(whiteScore == chBoardGetWhiteScore(board));
    utAssert(blackScore == chBoardGetBlackScore(board));
}

// Return the piece at (row, col).  (0, 0) is bottome left.
static inline void setPieceAtPosition(chBoard board, uint8 row, uint8 col, chPiece piece) {
    if (piece != chPieceNull) {
        utAssert(getPieceAtPosition(board, row, col) == chPieceNull);
        chPieceSetRow(piece, row);
        chPieceSetCol(piece, col);
        chPieceSetInPlay(piece, true);
        if (chPieceWhite(piece)) {
            chBoardSetWhiteScore(board, chBoardGetWhiteScore(board) + findPieceScore(piece));
        } else {
            chBoardSetBlackScore(board, chBoardGetBlackScore(board) + findPieceScore(piece));
        }
    }
    chBoardSetiPosition(board, COLS*row + col, piece);
    verifyScore(board);
}

// Remove a piece and return it.
static inline chPiece removePieceAtPosition(chBoard board, uint8 row, uint8 col) {
    chPiece piece = getPieceAtPosition(board, row, col);
    utAssert(piece != chPieceNull);
    setPieceAtPosition(board, row, col, chPieceNull);
    chPieceSetInPlay(piece, false);
    if (chPieceWhite(piece)) {
        chBoardSetWhiteScore(board, chBoardGetWhiteScore(board) - findPieceScore(piece));
    } else {
        chBoardSetBlackScore(board, chBoardGetBlackScore(board) - findPieceScore(piece));
    }
    verifyScore(board);
    return piece;
}

// Get a letter representing a piece.  Capitals are white, lower case are black.
static inline char getPieceLetter(chBoard board, uint8 row, uint8 col) {
    chPiece piece = getPieceAtPosition(board, row, col);
    if (piece == chPieceNull) {
        return (row ^ col) & 1? ' ' : '.';
    }
    if (chPieceWhite(piece)) {
        switch (chPieceGetType(piece)) {
            case CH_PAWN: return 'P';
            case CH_ROOK: return 'R';
            case CH_KNIGHT: return 'H';
            case CH_BISHOP: return 'B';
            case CH_QUEEN: return 'Q';
            case CH_KING: return 'K';
            default:
                utExit("Unknown piece type.");
        }
    } else {
        switch (chPieceGetType(piece)) {
            case CH_PAWN: return 'p';
            case CH_ROOK: return 'r';
            case CH_KNIGHT: return 'h';
            case CH_BISHOP: return 'b';
            case CH_QUEEN: return 'q';
            case CH_KING: return 'k';
            default:
                utExit("Unknown piece type.");
        }
    }
    return CH_PAWN;  // Dummy return.
}

// Print the board state.
static void printBoard(chBoard board) {
    putchar('\n');
    if (chBoardPlayerWhite(board)) {
        puts("+-A-B-C-D-E-F-G-H-+");
        for (int8 row = ROWS - 1; row >= 0; row--) {
            printf("%u ", row + 1);
            for (uint8 col = 0; col < COLS; col++) {
                putchar(getPieceLetter(board, row, col));
                putchar(' ');
            }
            printf("%u\n", row + 1);
        }
        puts("+-A-B-C-D-E-F-G-H-+");
    } else {
        puts("+-H-G-F-E-D-C-B-A-+");
        for (uint8 row = 0; row < ROWS; row++) {
            printf("%u ", row + 1);
            for (int32 col = COLS - 1; col >= 0; col--) {
                putchar(getPieceLetter(board, row, col));
                putchar(' ');
            }
            printf("%u\n", row + 1);
        }
        puts("+-H-G-F-E-D-C-B-A-+");
    }
    putchar('\n');
}

// Create a new piece and place it on the board.
static chPiece chPieceCreate(chBoard board, chPieceType type, bool white, uint8 row, uint8 col) {
    chPiece piece = chPieceAlloc();
    chPieceSetType(piece, type);
    chPieceSetWhite(piece, white);
    chPieceSetRow(piece, row);
    chPieceSetCol(piece, col);
    chPieceSetNeverMoved(piece, true);
    chBoardAppendPiece(board, piece);
    setPieceAtPosition(board, row, col, piece);
    return piece;
}

// Add all the pieces to the board.
static void addPieces(chBoard board) {
    // Create all the pawns.
    for (uint8 col = 0; col < COLS; col++) {
        chPieceCreate(board, CH_PAWN, true, 1, col);
        chPieceCreate(board, CH_PAWN, false, 6, col);
    }
    // Rooks.
    for (uint8 row = 0; row < ROWS; row += ROWS - 1) {
        bool white = row == 0;
        // Rooks.
        chPieceCreate(board, CH_ROOK, white, row, 0);
        chPieceCreate(board, CH_ROOK, white, row, 7);
        // Knights.
        chPieceCreate(board, CH_KNIGHT, white, row, 1);
        chPieceCreate(board, CH_KNIGHT, white, row, 6);
        // Bishops.
        chPieceCreate(board, CH_BISHOP, white, row, 2);
        chPieceCreate(board, CH_BISHOP, white, row, 5);
        // Queen.
        chPieceCreate(board, CH_QUEEN, white, row, 3);
        // King..
        chPiece king = chPieceCreate(board, CH_KING, white, row, 4);
        if (white) {
            chBoardSetWhiteKing(board, king);
        } else {
            chBoardSetBlackKing(board, king);
        }
    }
}

// Create a new board, set up to play.
static chBoard chBoardCreate(bool playerWhite) {
    chBoard board = chBoardAlloc();
    chBoardSetPlayerWhite(board, playerWhite);
    chBoardAllocPositions(board, ROWS*COLS);
    chBoardAllocMoves(board, 4096);
    chBoardAllocUndoMoves(board, 4096);
    addPieces(board);
    return board;
}

// Determine if the game is over.
static bool gameOver(chBoard board) {
    return !chPieceInPlay(chBoardGetWhiteKing(board)) ||
        !chPieceInPlay(chBoardGetBlackKing(board));
}

// Parse a move.
static bool parseMove(char *text, chMove *move) {
    if (strlen(text) != 5) {
        return false;
    }
    char fromCol = *text++;
    char fromRow = *text++;
    text++;
    char toCol = *text++;
    char toRow = *text++;
    if (fromCol < 'a' || fromCol > 'h' || fromRow < '1' || fromRow > '8' ||
            toCol < 'a' || toCol > 'h' || toRow < '1' || toRow > '8') {
        return false;
    }
    move->fromRow = fromRow - '1';
    move->toRow = toRow - '1';
    move->fromCol = fromCol - 'a';
    move->toCol = toCol - 'a';
    return true;
}

// Move the fromRow and fromCol square one step closer to the toRow and toCol.
static inline void stepTowardsDestinationSquare(chMove *move) {
    if (move->fromCol < move->toCol) {
        move->fromCol++;
    } else if (move->fromCol > move->toCol) {
        move->fromCol--;
    }
    if (move->fromRow < move->toRow) {
        move->fromRow++;
    } else if (move->fromRow > move->toRow) {
        move->fromRow--;
    }
}

// Return true if the move moves from a square to itself.
static inline bool movesToSameSquare(chMove move) {
    return move.fromRow == move.toRow && move.fromCol == move.toCol;
}

// Determine if the spaces between the from square and to square are empty.
static inline bool spacesEmptyBetween(chBoard board, chMove move) {
    stepTowardsDestinationSquare(&move);
    while (!movesToSameSquare(move)) {
        if (!squareEmpty(board, move.fromRow, move.fromCol)) {
            return false;
        }
        stepTowardsDestinationSquare(&move);
    }
    return true;
}

// Return the absolute value of an int8.
static inline int8 abs8(int8 val) {
    return val >= 0? val : -val;
}

// Check special rules for castling.
static inline bool castling(chPiece king, chMove move) {
    return chPieceGetType(king) == CH_KING && abs8(move.fromCol - move.toCol) >= 2;
}

// Determine if the pawn move is legal.
static inline bool pawnMoveLegal(chBoard board, chPiece piece, chMove move, chPiece target) {
    //TODO: check for special case of taking a pawn that just moved two spaces.
    if (target == chPieceNull) {
        // Can only move straight.
        if (move.fromCol != move.toCol) {
            return false;
        }
        if (chPieceWhite(piece)) {
            if (move.fromRow  + 1 == move.toRow) {
                return true;
            }
            return move.fromRow == 1 && move.toRow == 3 && squareEmpty(board, 2, move.fromCol);
        }
        if (move.fromRow == move.toRow + 1) {
            return true;
        }
        return move.fromRow == 6 && move.toRow == 4 && squareEmpty(board, 5, move.fromCol);
    }
    // Taking a piece.  Must move diagonally upward by 1.
    if (move.fromCol != move.toCol + 1 && move.fromCol + 1 != move.toCol) {
        return false;
    }
    if (chPieceWhite(piece)) {
        return move.fromRow + 1 == move.toRow;
    }
    return move.fromRow == move.toRow + 1;
}

// Determine if the pawn move is legal.
static inline bool rookMoveLegal(chBoard board, chPiece piece, chMove move, chPiece target) {
    if (move.fromRow != move.toRow && move.fromCol != move.toCol) {
        return false;
    }
    return spacesEmptyBetween(board, move);
}

// Determine if the pawn move is legal.
static inline bool knightMoveLegal(chPiece piece, chMove move, chPiece target) {
    uint8 rowDist = abs8(move.toRow - move.fromRow);
    uint8 colDist = abs8(move.toCol - move.fromCol);
    return (rowDist == 1 && colDist == 2) || (rowDist == 2 && colDist == 1);
}

// Determine if the pawn move is legal.
static inline bool bishopMoveLegal(chBoard board, chPiece piece, chMove move, chPiece target) {
    if (move.fromRow - move.fromCol != move.toRow - move.toCol &&
            move.fromRow + move.fromCol != move.toRow + move.toCol) {
        return false;
    }
    return spacesEmptyBetween(board, move);
}

// Determine if the pawn move is legal.
static inline bool queenMoveLegal(chBoard board, chPiece piece, chMove move, chPiece target) {
    if (move.fromRow != move.toRow && move.fromCol != move.toCol &&
            move.fromRow - move.fromCol != move.toRow - move.toCol &&
            move.fromRow + move.fromCol != move.toRow + move.toCol) {
        return false;
    }
    return spacesEmptyBetween(board, move);
}

// Determine if the pawn move is legal.
static inline bool kingMoveLegal(chBoard board, chPiece piece, chMove move, chPiece target) {
    // TODO: Check for moving into check.
    uint8 rowDist = abs8(move.toRow - move.fromRow);
    uint8 colDist = abs8(move.toCol - move.fromCol);
    if (rowDist <= 1 && colDist <= 1) {
        return true;
    }
    // Check for castling.
    if (!chPieceNeverMoved(piece) || move.fromRow != move.toRow || !spacesEmptyBetween(board, move) ||
            (move.fromRow != 0 && move.fromRow != 7)) {
        return false;
    }
    // TODO: check that the king is not in check or moving through check.
    chPiece rook;
    if (move.toCol == 6) {
        rook = getPieceAtPosition(board, move.fromRow, 7);
    } else if (move.toCol == 2) {
        rook = getPieceAtPosition(board, move.fromRow, 0);
    } else {
        return false;
    }
    return chPieceNeverMoved(rook);
}


// Determine if the piece is normally allowed to make moves like this.
static bool pieceCanMakeMove(chBoard board, chPiece piece, chMove move, chPiece target) {
    switch (chPieceGetType(piece)) {
        case CH_PAWN: return pawnMoveLegal(board, piece, move, target);
        case CH_ROOK: return rookMoveLegal(board, piece, move, target);
        case CH_KNIGHT: return knightMoveLegal(piece, move, target);
        case CH_BISHOP: return bishopMoveLegal(board, piece, move, target);
        case CH_QUEEN: return queenMoveLegal(board, piece, move, target);
        case CH_KING: return kingMoveLegal(board, piece, move, target);
        default:
            utExit("Unknown piece type.");
    }
    return false;  // Dummy return.
}

// Determine if the move is valid.
static bool moveValid(chBoard board, chMove move, bool whitesMove) {
    if (movesToSameSquare(move)) {
        return false;
    }
    chPiece piece = getPieceAtPosition(board, move.fromRow, move.fromCol);
    if (piece == chPieceNull || chPieceWhite(piece) != whitesMove) {
        return false;
    }
    chPiece target = getPieceAtPosition(board, move.toRow, move.toCol);
    if (target != chPieceNull && chPieceWhite(target) == chPieceWhite(piece)) {
        return false;
    }
    return pieceCanMakeMove(board, piece, move, target);
}

// Prompt the user for a move.  If the player types 'u', set undo instead.
static chMove readPlayerMove(chBoard board, bool whitesMove, bool *undo) {
    char *response = readline("Enter a valid move like d2 d4: ");
    chMove move = {0, 0, 0, 0};
    *undo = false;
    while (*response != 'u' && (!parseMove(response, &move) || !moveValid(board, move, whitesMove))) {
        response = readline("Invalid move.  Enter a valid move like d2 d4: ");
    }
    if (*response == 'u') {
        *undo = true;
    }
    return move;
}

// Finish castling by moving the rook past the king.
static inline void finishCastling(chBoard board, chMove move) {
    chPiece rook = chPieceNull;
    if (move.toCol == 6) {
        rook = getPieceAtPosition(board, move.fromRow, 7);
    } else if (move.toCol == 2) {
        rook = getPieceAtPosition(board, move.fromRow, 0);
    } else {
        utExit("Illegal castling");
    }
    utAssert(rook != chPieceNull && chPieceGetType(rook) == CH_ROOK && chPieceNeverMoved(rook));
    removePieceAtPosition(board, chPieceGetRow(rook), chPieceGetCol(rook));
    if (move.toCol == 6) {
        setPieceAtPosition(board, move.toRow, 5, rook);
    } else {
        setPieceAtPosition(board, move.toRow, 3, rook);
    }
    chPieceSetNeverMoved(rook, false);
}

// Undo castling.
static inline void finishUndoCastling(chBoard board, chMove move) {
    chPiece rook;
    uint8 origCol;
    if (move.toCol == 6) {
        rook = getPieceAtPosition(board, move.fromRow, 5);
        origCol = 7;
    } else {
        rook = getPieceAtPosition(board, move.fromRow, 3);
        origCol = 0;
    }
    utAssert(rook != chPieceNull && chPieceGetType(rook) == CH_ROOK && !chPieceNeverMoved(rook));
    removePieceAtPosition(board, chPieceGetRow(rook), chPieceGetCol(rook));
    setPieceAtPosition(board, chPieceGetRow(rook), origCol, rook);
    chPieceSetNeverMoved(rook, true);
}

// Make the move on the board.  Return true if we queened a pawn.
static inline void makeMove(chBoard board, chMove move) {
    chUndoMove undoMove;
    undoMove.move = move;
    chPiece piece = getPieceAtPosition(board, move.fromRow, move.fromCol);
    utAssert(piece != chPieceNull);
    removePieceAtPosition(board, move.fromRow, move.fromCol);
    chPiece target = getPieceAtPosition(board, move.toRow, move.toCol);
    undoMove.target = target;
    if (target != chPieceNull) {
        removePieceAtPosition(board, move.toRow, move.toCol);
    }
    undoMove.queenedPawn = false;
    if (chPieceGetType(piece) == CH_PAWN && (chPieceWhite(piece)? move.toRow == 7 : move.toRow == 0)) {
        // TODO: give the player a choice of piece type.
        chPieceSetType(piece, CH_QUEEN);
        undoMove.queenedPawn = true;
    }
    setPieceAtPosition(board, move.toRow, move.toCol, piece);
    if (castling(piece, move)) {
        finishCastling(board, move);
    }
    undoMove.firstMove = chPieceNeverMoved(piece);
    chPieceSetNeverMoved(piece, false);
    uint32 undoMovePos = chBoardGetUndoMovePos(board);
    chBoardSetiUndoMove(board, undoMovePos, undoMove);
    chBoardSetUndoMovePos(board, undoMovePos + 1);
}

// Undo the move.
static void undoMove(chBoard board) {
    uint32 undoMovePos = chBoardGetUndoMovePos(board) - 1;
    chUndoMove undoMove = chBoardGetiUndoMove(board, undoMovePos);
    chMove move = undoMove.move;
    chPiece target = undoMove.target;
    chBoardSetUndoMovePos(board, undoMovePos);
    chPiece piece = getPieceAtPosition(board, move.toRow, move.toCol);
    utAssert(piece != chPieceNull && piece != target);
    removePieceAtPosition(board, move.toRow, move.toCol);
    if (castling(piece, move)) {
        finishUndoCastling(board, move);
    }
    if (undoMove.queenedPawn) {
        chPieceSetType(piece, CH_PAWN);
    }
    setPieceAtPosition(board, move.fromRow, move.fromCol, piece);
    if (undoMove.firstMove) {
        chPieceSetNeverMoved(piece, true);
    }
    if (undoMove.target != chPieceNull) {
        setPieceAtPosition(board, move.toRow, move.toCol, target);
    }
}

// Add a move to the move stack.
static inline void addMove(chBoard board, uint8 fromRow, uint8 fromCol, uint8 toRow, uint8 toCol) {
    chMove move;
    move.fromRow = fromRow;
    move.fromCol = fromCol;
    move.toRow = toRow;
    move.toCol = toCol;
    uint32 stackPos = chBoardGetMoveStackPos(board);
    if (stackPos == chBoardGetNumMove(board)) {
        chBoardResizeMoves(board, chBoardGetNumMove(board) << 1);
    }
    chBoardSetiMove(board, stackPos, move);
    chBoardSetMoveStackPos(board, stackPos + 1);
}

// Find moves for a pawn.
// TODO: Add special move when taking pawn that just moved two squares.
static void findPawnMoves(chBoard board, chPiece piece) {
    bool white = chPieceWhite(piece);
    uint8 row = chPieceGetRow(piece);
    uint8 col = chPieceGetCol(piece);
    int8 oneRow = white? 1 : -1;
    int8 twoRows = white? 2 : -2;
    if ((white && row == 1) || (!white && row == 6)) {
        // Try moving forward 2.
        if (squareEmpty(board, row + oneRow, col) && squareEmpty(board, row + twoRows, col)) {
            addMove(board, row, col, row + twoRows, col);
        }
    }
    // Try moving forward 1.
    if (squareEmpty(board, row + oneRow, col)) {
        addMove(board, row, col, row + oneRow, col);
    }
    if (col > 0) {
        // Try taking left.
        chPiece target = getPieceAtPosition(board, row + oneRow, col - 1);
        if (target != chPieceNull && chPieceWhite(target) != white) {
            addMove(board, row, col, row + oneRow, col - 1);
        }
    }
    if (col < 7) {
        // Try taking right.
        chPiece target = getPieceAtPosition(board, row + oneRow, col + 1);
        if (target != chPieceNull && chPieceWhite(target) != white) {
            addMove(board, row, col, row + oneRow, col + 1);
        }
    }
}

// Try moving in a direction, adding moves as we go until we hit the edge of the
// board, one of our own pieces, or take a piece.
static inline void tryMoves(chBoard board, chPiece piece, int8 rowDelta, int8 colDelta) {
    bool white = chPieceWhite(piece);
    uint8 origRow = chPieceGetRow(piece);
    uint8 origCol = chPieceGetCol(piece);
    int8 row = origRow + rowDelta;
    int8 col = origCol + colDelta;
    while (row >= 0 && col >= 0 && row < ROWS && col < COLS) {
        chPiece target = getPieceAtPosition(board, row, col);
        if (target == chPieceNull || chPieceWhite(target) != white) {
            addMove(board, origRow, origCol, row, col);
        }
        if (target != chPieceNull) {
            return;
        }
        row += rowDelta;
        col += colDelta;
    }
}

// Find moves for a rook.
static void findRookMoves(chBoard board, chPiece piece) {
    tryMoves(board, piece, 1, 0);
    tryMoves(board, piece, -1, 0);
    tryMoves(board, piece, 0, 1);
    tryMoves(board, piece, 0, -1);
}

// Just check that the position is on the board.
static inline bool rangeLegal(uint8 toRow, uint8 toCol) {
    return toRow >= 0 && toRow < ROWS && toCol >= 0 && toCol < COLS;
}

// Each pair are deltas to try from the current position.  Try them all.
static void tryDeltas(chBoard board, chPiece piece, int8 *deltas, uint8 deltasLen) {
    uint8 row = chPieceGetRow(piece);
    uint8 col = chPieceGetCol(piece);
    for (uint8 i = 0; i < deltasLen; i += 2) {
        int8 rowDelta = deltas[i];
        int8 colDelta = deltas[i + 1];
        int8 toRow = row + rowDelta;
        int8 toCol = col + colDelta;
        if (rangeLegal(toRow, toCol)) {
            chPiece target = getPieceAtPosition(board, toRow, toCol);
            if (target == chPieceNull || chPieceWhite(target) != chPieceWhite(piece)) {
                addMove(board, row, col, toRow, toCol);
            }
        }
    }
}

// Find moves for a knight.
static void findKnightMoves(chBoard board, chPiece piece) {
    int8 deltas[] = {-2, -1, -2, 1, -1, -2, -1, 2, 1, -2, 1, 2, 2, -1, 2, 1};
    tryDeltas(board, piece, deltas, sizeof(deltas));
}

// Find moves for a bishop.
static void findBishopMoves(chBoard board, chPiece piece) {
    tryMoves(board, piece, 1, 1);
    tryMoves(board, piece, 1, -1);
    tryMoves(board, piece, -1, 1);
    tryMoves(board, piece, -1, -1);
}

// Find moves for the queen.
static void findQueenMoves(chBoard board, chPiece piece) {
    tryMoves(board, piece, 1, 0);
    tryMoves(board, piece, -1, 0);
    tryMoves(board, piece, 0, 1);
    tryMoves(board, piece, 0, -1);
    tryMoves(board, piece, 1, 1);
    tryMoves(board, piece, 1, -1);
    tryMoves(board, piece, -1, 1);
    tryMoves(board, piece, -1, -1);
}

// Find moves for the king.
static void findKingMoves(chBoard board, chPiece piece) {
    int8 deltas[] = {-1, -1, -1, 0, -1, 1, 0, -1, 0, 1, 1, -1, 1, 0, 1, 1};
    tryDeltas(board, piece, deltas, sizeof(deltas));
    // Check for castling.
    if (!chPieceNeverMoved(piece)) {
        return;
    }
    utAssert(chPieceGetCol(piece) == 4);
    uint8 row = chPieceGetRow(piece);
    chPiece rook = getPieceAtPosition(board, row, 7);
    // TODO: Check for king moving through or being in check.
    if (rook != chPieceNull && chPieceNeverMoved(rook) &&
            squareEmpty(board, row, 5) && squareEmpty(board, row, 6)) {
        addMove(board, row, 4, row, 6);
    }
    rook = getPieceAtPosition(board, row, 0);
    if (rook != chPieceNull && chPieceNeverMoved(rook) && squareEmpty(board, row, 3) &&
            squareEmpty(board, row, 2) && squareEmpty(board, row, 1)) {
        addMove(board, row, 4, row, 2);
    }
}

// Find all the moves the piece can make.
static void findPieceMoves(chBoard board, chPiece piece) {
    switch (chPieceGetType(piece)) {
        case CH_PAWN: return findPawnMoves(board, piece);
        case CH_ROOK: return findRookMoves(board, piece);
        case CH_KNIGHT: return findKnightMoves(board, piece);
        case CH_BISHOP: return findBishopMoves(board, piece);
        case CH_QUEEN: return findQueenMoves(board, piece);
        case CH_KING: return findKingMoves(board, piece);
        default:
            utExit("Unknown piece type.");
    }
}

// Find all the possible moves for the computer and add them to the array of
// moves on the board.
static void findAllMoves(chBoard board, bool whitesTurn) {
    chPiece piece;
    chForeachBoardPiece(board, piece) {
        if (chPieceInPlay(piece) && chPieceWhite(piece) == whitesTurn) {
            findPieceMoves(board, piece);
        }
    } chEndBoardPiece;
}

// Find the index in the move stack of the given move.
static uint32 findMoveIndex(chBoard board, chMove move, uint32 oldMoveStackPos) {
    uint32 numMoves = chBoardGetMoveStackPos(board) - oldMoveStackPos;
    for (uint32 i = oldMoveStackPos; i < oldMoveStackPos + numMoves; i++) {
        chMove otherMove = chBoardGetiMove(board, i);
        if (!memcmp(&move, &otherMove, sizeof(struct chMove_st))) {
            return i;
        }
    }
    utExit("findMoveIndex: Unable to find move");
    return 0;  // Dummy return.
}

// Suggest a move, looking difficulty moves ahead.  Initially, just use brute
// force and a crappy scoring algorithm.  Perform alpha-beta tree pruning.
static chMove suggestMove(chBoard board, uint8 difficulty, bool whitesTurn,
        int32 minScore, int32 maxScore, int32 *retScore, uint32 *retMovesEvaluated) {
    uint32 oldMoveStackPos = chBoardGetMoveStackPos(board);
    findAllMoves(board, whitesTurn);
    chMove bestMove;
    int32 bestScore = INT32_MIN;  // Less than any possible move.
    bool done = false;
    uint32 numMoves = chBoardGetMoveStackPos(board) - oldMoveStackPos;
    int32 score;
    // Randomize the selected move by evaluting moves starting at a random
    // position.
    uint32 randStart = rand() % numMoves;
    uint32 moveIndex;
    uint32 totalMovesEvaluated = 0;
    if (difficulty > 2) {
        // If we still have enough depth, it is worth it to do a fast call with
        // less depth to find a good first piece.  This helps alpha-beta tree
        // pruning.
        chMove bestMoveGuess = suggestMove(board, difficulty - 2, whitesTurn,
                minScore, maxScore, &score, &totalMovesEvaluated);
        uint32 moveIndex = findMoveIndex(board, bestMoveGuess, oldMoveStackPos);
        // Swap the best guess move to the random start position.
        chMove tempMove = chBoardGetiMove(board, oldMoveStackPos + randStart);
        chBoardSetiMove(board, oldMoveStackPos + randStart, bestMoveGuess);
        chBoardSetiMove(board, moveIndex, tempMove);
    }
    for (uint32 i = 0; i < numMoves && !done; i++) {
        moveIndex = i + randStart;
        if (moveIndex >= numMoves) {
            moveIndex -= numMoves;
        }
        chMove move = chBoardGetiMove(board, oldMoveStackPos + moveIndex);
        chPiece target = getPieceAtPosition(board, move.toRow, move.toCol);
        makeMove(board, move);
        totalMovesEvaluated++;
        if (target != chPieceNull && chPieceGetType(target) == CH_KING) {
            // Always go for the win.  Don't bother looking ahead past that.
            // Also, prefer to win sooner.
            score = WIN + difficulty;
        } else {
            if (difficulty > 0) {
                uint32 movesEvaluated;
                suggestMove(board, difficulty - 1, !whitesTurn, -maxScore, -minScore, &score, &movesEvaluated);
                totalMovesEvaluated += movesEvaluated;
                score = -score;
            } else {
                score = whitesTurn? chBoardGetWhiteScore(board) - chBoardGetBlackScore(board) :
                    chBoardGetBlackScore(board) - chBoardGetWhiteScore(board);
            }
        }
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            if (minScore < bestScore) {
                minScore = bestScore;
                if (minScore >= maxScore) {
                    // Our oponent will not allow this scenario since she has found
                    // a better move that wont let us get this good of a score.
                    done = true;
                }
            }
        }
        undoMove(board);
    }
    chBoardSetMoveStackPos(board, oldMoveStackPos);
    *retScore = bestScore;
    *retMovesEvaluated = totalMovesEvaluated;
    return bestMove;
}

// Suggest and make a move.
static void suggestAndMakeMove(chBoard board, bool white, uint32 difficulty,
        char *myName, char *myPossessive, char *yourPossessive, uint32 *retMovesEvaluated) {
    int32 score;
    uint32 moveNum = chBoardGetUndoMovePos(board);
    uint32 movesEvaluated;
    chMove move = suggestMove(board, difficulty, white, -INT32_MAX, INT32_MAX, &score, &movesEvaluated);
    chPiece piece = getPieceAtPosition(board, move.fromRow, move.fromCol);
    printf("%u) %s move %s %s from %c%d to %c%u", moveNum, myName, myPossessive,
            getPieceTypeName(chPieceGetType(piece)), move.fromCol + 'a',
            move.fromRow + 1, move.toCol + 'a', move.toRow + 1);
    chPiece target = getPieceAtPosition(board, move.toRow, move.toCol);
    if (target != chPieceNull) {
        if (chPieceWhite(piece) != chPieceWhite(target)) {
            printf(" taking %s %s\n", yourPossessive, getPieceTypeName(chPieceGetType(target)));
        } else {
            printf(" castling.\n");
        }
    } else {
        putchar('\n');
    }
    printf("Evaluated %u moves at difficulty %u\n", movesEvaluated, difficulty);
    makeMove(board, move);
    *retMovesEvaluated = movesEvaluated;
}

// Read the move from the player and return it.  If it starts with 'u', undo two
// moves, and try again.
static void letPlayerMove(chBoard board, bool playerWhite, uint32 difficulty) {
    chMove move;
    bool undo = false;
    do {
        move = readPlayerMove(board, playerWhite, &undo);
        if (undo) {
            // Player wants to undo the last two moves.
            if (chBoardGetUndoMovePos(board) < 2) {
                printf("No moves left to undo.\n");
            } else {
                undoMove(board);
                undoMove(board);
            }
        }
        if (undo) {
            printBoard(board);
        }
    } while (undo);
    makeMove(board, move);
}

int main(int argc, char **argv) {
    utStart();
    chDatabaseStart();
    bool playerWhite = true;
    bool autoPlay = false;
    uint8 difficulty = 4;
    int32 seed = 1;
    if (argc == 2 && !strcmp(argv[1], "-a")) {
        autoPlay = true;
    }
    if (!autoPlay) {
        seed = atoi(readline("Enter a game seed as an integer: "));
    }
    srand(seed);
    if (!autoPlay) {
        char* response = readline("How many moves ahead should the computer look? ");
        difficulty = atoi(response);
        response = readline("Would you prefer to play white (enter 'a' for auto-play)? (y/n/a) ");
        while (*response != 'y' && *response != 'n' && *response != 'a') {
            response = readline("Only y and n are allowed.  Whould you like to play white? (y/n) ");
        }
        if (*response == 'n') {
            playerWhite = false;
        } else if (*response == 'a') {
            printf("You selected automatic play\n");
            autoPlay = true;
        }
    }
    chBoard board = chBoardCreate(playerWhite);
    printBoard(board);
    bool playersTurn = playerWhite;
    uint32 initialMovesEvaluated = 0;
    while (!gameOver(board)) {
        verifyScore(board);
        uint32 movesEvaluated;
        if (playersTurn) {
            if (autoPlay) {
                suggestAndMakeMove(board, playerWhite, difficulty, "You", "your", "my", &movesEvaluated);
            } else {
                letPlayerMove(board, playerWhite, difficulty);
            }
        } else {
            suggestAndMakeMove(board, !playerWhite, difficulty, "I", "my", "your", &movesEvaluated);
            if (initialMovesEvaluated == 0) {
                initialMovesEvaluated = movesEvaluated;
            }
            if (movesEvaluated > 5*initialMovesEvaluated) {
                difficulty--;
                printf("Decreasing difficulty to %u\n", difficulty);
            } else if (5*movesEvaluated < initialMovesEvaluated) {
                difficulty++;
                printf("Increasing difficulty to %u\n", difficulty);
            }
        }
        printBoard(board);
        playersTurn = !playersTurn;
        int32 score = playerWhite? chBoardGetWhiteScore(board) - chBoardGetBlackScore(board) :
                chBoardGetBlackScore(board) - chBoardGetWhiteScore(board);
        printf("Score = %.3f\n", 0.001*score);
        fflush(stdout);
    }
    if (!playersTurn) {
        printf("You win!\n");
    } else {
        printf("Sorry, better luck next time.\n");
    }
    chDatabaseStop();
    utStop(false);
    return 0;
}
