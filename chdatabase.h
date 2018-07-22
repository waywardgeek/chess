/*----------------------------------------------------------------------------------------
  Module header file for: ch module
----------------------------------------------------------------------------------------*/
#ifndef CHDATABASE_H

#define CHDATABASE_H

#if defined __cplusplus
extern "C" {
#endif

#ifndef DD_UTIL_H
#include "ddutil.h"
#endif

#include "chtypedef.h"

extern uint8 chModuleID;
/* Class reference definitions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
typedef struct _struct_chBoard{char val;} *chBoard;
#define chBoardNull ((chBoard)0)
typedef struct _struct_chPiece{char val;} *chPiece;
#define chPieceNull ((chPiece)0)
#else
typedef uint32 chBoard;
#define chBoardNull 0
typedef uint32 chPiece;
#define chPieceNull 0
#endif

/* PieceType enumerated type */
typedef enum {
    CH_PAWN = 0,
    CH_ROOK = 1,
    CH_KNIGHT = 2,
    CH_BISHOP = 3,
    CH_QUEEN = 4,
    CH_KING = 5
} chPieceType;

/* Constructor/Destructor hooks. */
typedef void (*chBoardCallbackType)(chBoard);
extern chBoardCallbackType chBoardConstructorCallback;
typedef void (*chPieceCallbackType)(chPiece);
extern chPieceCallbackType chPieceConstructorCallback;

/*----------------------------------------------------------------------------------------
  Root structure
----------------------------------------------------------------------------------------*/
struct chRootType_ {
    uint32 hash; /* This depends only on the structure of the database */
    uint32 usedBoard, allocatedBoard;
    uint32 usedBoardPosition, allocatedBoardPosition, freeBoardPosition;
    uint32 usedBoardMove, allocatedBoardMove, freeBoardMove;
    uint32 usedBoardUndoMove, allocatedBoardUndoMove, freeBoardUndoMove;
    uint32 usedPiece, allocatedPiece;
};
extern struct chRootType_ chRootData;

utInlineC uint32 chHash(void) {return chRootData.hash;}
utInlineC uint32 chUsedBoard(void) {return chRootData.usedBoard;}
utInlineC uint32 chAllocatedBoard(void) {return chRootData.allocatedBoard;}
utInlineC void chSetUsedBoard(uint32 value) {chRootData.usedBoard = value;}
utInlineC void chSetAllocatedBoard(uint32 value) {chRootData.allocatedBoard = value;}
utInlineC uint32 chUsedBoardPosition(void) {return chRootData.usedBoardPosition;}
utInlineC uint32 chAllocatedBoardPosition(void) {return chRootData.allocatedBoardPosition;}
utInlineC uint32 chFreeBoardPosition(void) {return chRootData.freeBoardPosition;}
utInlineC void chSetUsedBoardPosition(uint32 value) {chRootData.usedBoardPosition = value;}
utInlineC void chSetAllocatedBoardPosition(uint32 value) {chRootData.allocatedBoardPosition = value;}
utInlineC void chSetFreeBoardPosition(int32 value) {chRootData.freeBoardPosition = value;}
utInlineC uint32 chUsedBoardMove(void) {return chRootData.usedBoardMove;}
utInlineC uint32 chAllocatedBoardMove(void) {return chRootData.allocatedBoardMove;}
utInlineC uint32 chFreeBoardMove(void) {return chRootData.freeBoardMove;}
utInlineC void chSetUsedBoardMove(uint32 value) {chRootData.usedBoardMove = value;}
utInlineC void chSetAllocatedBoardMove(uint32 value) {chRootData.allocatedBoardMove = value;}
utInlineC void chSetFreeBoardMove(int32 value) {chRootData.freeBoardMove = value;}
utInlineC uint32 chUsedBoardUndoMove(void) {return chRootData.usedBoardUndoMove;}
utInlineC uint32 chAllocatedBoardUndoMove(void) {return chRootData.allocatedBoardUndoMove;}
utInlineC uint32 chFreeBoardUndoMove(void) {return chRootData.freeBoardUndoMove;}
utInlineC void chSetUsedBoardUndoMove(uint32 value) {chRootData.usedBoardUndoMove = value;}
utInlineC void chSetAllocatedBoardUndoMove(uint32 value) {chRootData.allocatedBoardUndoMove = value;}
utInlineC void chSetFreeBoardUndoMove(int32 value) {chRootData.freeBoardUndoMove = value;}
utInlineC uint32 chUsedPiece(void) {return chRootData.usedPiece;}
utInlineC uint32 chAllocatedPiece(void) {return chRootData.allocatedPiece;}
utInlineC void chSetUsedPiece(uint32 value) {chRootData.usedPiece = value;}
utInlineC void chSetAllocatedPiece(uint32 value) {chRootData.allocatedPiece = value;}

/* Validate macros */
#if defined(DD_DEBUG)
utInlineC chBoard chValidBoard(chBoard Board) {
    utAssert(utLikely(Board != chBoardNull && (uint32)(Board - (chBoard)0) < chRootData.usedBoard));
    return Board;}
utInlineC chPiece chValidPiece(chPiece Piece) {
    utAssert(utLikely(Piece != chPieceNull && (uint32)(Piece - (chPiece)0) < chRootData.usedPiece));
    return Piece;}
#else
utInlineC chBoard chValidBoard(chBoard Board) {return Board;}
utInlineC chPiece chValidPiece(chPiece Piece) {return Piece;}
#endif

/* Object ref to integer conversions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
utInlineC uint32 chBoard2Index(chBoard Board) {return Board - (chBoard)0;}
utInlineC uint32 chBoard2ValidIndex(chBoard Board) {return chValidBoard(Board) - (chBoard)0;}
utInlineC chBoard chIndex2Board(uint32 xBoard) {return (chBoard)(xBoard + (chBoard)(0));}
utInlineC uint32 chPiece2Index(chPiece Piece) {return Piece - (chPiece)0;}
utInlineC uint32 chPiece2ValidIndex(chPiece Piece) {return chValidPiece(Piece) - (chPiece)0;}
utInlineC chPiece chIndex2Piece(uint32 xPiece) {return (chPiece)(xPiece + (chPiece)(0));}
#else
utInlineC uint32 chBoard2Index(chBoard Board) {return Board;}
utInlineC uint32 chBoard2ValidIndex(chBoard Board) {return chValidBoard(Board);}
utInlineC chBoard chIndex2Board(uint32 xBoard) {return xBoard;}
utInlineC uint32 chPiece2Index(chPiece Piece) {return Piece;}
utInlineC uint32 chPiece2ValidIndex(chPiece Piece) {return chValidPiece(Piece);}
utInlineC chPiece chIndex2Piece(uint32 xPiece) {return xPiece;}
#endif

/*----------------------------------------------------------------------------------------
  Fields for class Board.
----------------------------------------------------------------------------------------*/
struct chBoardFields {
    uint32 *PositionIndex_;
    uint32 *NumPosition;
    chPiece *Position;
    uint8 *PlayerWhite;
    chPiece *WhiteKing;
    chPiece *BlackKing;
    uint32 *MoveIndex_;
    uint32 *NumMove;
    chMove *Move;
    uint32 *MoveStackPos;
    uint32 *UndoMoveIndex_;
    uint32 *NumUndoMove;
    chUndoMove *UndoMove;
    uint32 *UndoMovePos;
    uint32 *WhiteScore;
    uint32 *BlackScore;
    chPiece *FirstPiece;
    chPiece *LastPiece;
};
extern struct chBoardFields chBoards;

void chBoardAllocMore(void);
void chBoardCopyProps(chBoard chOldBoard, chBoard chNewBoard);
void chBoardAllocPositions(chBoard Board, uint32 numPositions);
void chBoardResizePositions(chBoard Board, uint32 numPositions);
void chBoardFreePositions(chBoard Board);
void chCompactBoardPositions(void);
void chBoardAllocMoves(chBoard Board, uint32 numMoves);
void chBoardResizeMoves(chBoard Board, uint32 numMoves);
void chBoardFreeMoves(chBoard Board);
void chCompactBoardMoves(void);
void chBoardAllocUndoMoves(chBoard Board, uint32 numUndoMoves);
void chBoardResizeUndoMoves(chBoard Board, uint32 numUndoMoves);
void chBoardFreeUndoMoves(chBoard Board);
void chCompactBoardUndoMoves(void);
utInlineC uint32 chBoardGetPositionIndex_(chBoard Board) {return chBoards.PositionIndex_[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetPositionIndex_(chBoard Board, uint32 value) {chBoards.PositionIndex_[chBoard2ValidIndex(Board)] = value;}
utInlineC uint32 chBoardGetNumPosition(chBoard Board) {return chBoards.NumPosition[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetNumPosition(chBoard Board, uint32 value) {chBoards.NumPosition[chBoard2ValidIndex(Board)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 chBoardCheckPositionIndex(chBoard Board, uint32 x) {utAssert(x < chBoardGetNumPosition(Board)); return x;}
#else
utInlineC uint32 chBoardCheckPositionIndex(chBoard Board, uint32 x) {return x;}
#endif
utInlineC chPiece chBoardGetiPosition(chBoard Board, uint32 x) {return chBoards.Position[
    chBoardGetPositionIndex_(Board) + chBoardCheckPositionIndex(Board, x)];}
utInlineC chPiece *chBoardGetPosition(chBoard Board) {return chBoards.Position + chBoardGetPositionIndex_(Board);}
#define chBoardGetPositions chBoardGetPosition
utInlineC void chBoardSetPosition(chBoard Board, chPiece *valuePtr, uint32 numPosition) {
    chBoardResizePositions(Board, numPosition);
    memcpy(chBoardGetPositions(Board), valuePtr, numPosition*sizeof(chPiece));}
utInlineC void chBoardSetiPosition(chBoard Board, uint32 x, chPiece value) {
    chBoards.Position[chBoardGetPositionIndex_(Board) + chBoardCheckPositionIndex(Board, (x))] = value;}
utInlineC void chBoardMovePositions(chBoard Board, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= chBoardGetNumPosition(Board));
    utAssert((from+count) <= chBoardGetNumPosition(Board));
    memmove(chBoardGetPositions(Board)+to,chBoardGetPositions(Board)+from,((int32)count)*sizeof(chPiece));
}
utInlineC void chBoardCopyPositions(chBoard Board, uint32 x, chPiece * values, uint32 count) {
    utAssert((x+count) <= chBoardGetNumPosition(Board));
    memcpy(chBoardGetPositions(Board)+x, values, count*sizeof(chPiece));
}
utInlineC void chBoardAppendPositions(chBoard Board, chPiece * values, uint32 count) {
    uint32 num = chBoardGetNumPosition(Board);
    chBoardResizePositions(Board, num+count);
    chBoardCopyPositions(Board, num, values, count);
}
utInlineC void chBoardAppendPosition(chBoard Board, chPiece Position) {
    chBoardResizePositions(Board, chBoardGetNumPosition(Board)+1);
    chBoardSetiPosition(Board, chBoardGetNumPosition(Board)-1, Position);
}
utInlineC void chBoardInsertPositions(chBoard Board, uint32 x, chPiece *Position, uint32 count) {
    utAssert(x <= chBoardGetNumPosition(Board));
    if(x < chBoardGetNumPosition(Board)) {
        chBoardResizePositions(Board, chBoardGetNumPosition(Board)+count);
        chBoardMovePositions(Board, x, x+count, chBoardGetNumPosition(Board)-x-count);
        chBoardCopyPositions(Board, x, Position, count);
    }
    else {
        chBoardAppendPositions(Board, Position, count);
    }
}
utInlineC void chBoardInsertPosition(chBoard Board, uint32 x, chPiece Position) {
    chBoardInsertPositions(Board, x, &Position, 1);
}
utInlineC void chBoardRemovePositions(chBoard Board, uint32 x, uint32 count) {
    utAssert((x+count) <= chBoardGetNumPosition(Board));
    if((x+count) < chBoardGetNumPosition(Board)) {
        chBoardMovePositions(Board, x+count,x,chBoardGetNumPosition(Board)-x-count);
    }
    chBoardResizePositions(Board, chBoardGetNumPosition(Board)-(int32)count);
}
utInlineC void chBoardRemovePosition(chBoard Board, uint32 x) {
    chBoardRemovePositions(Board, x, 1);
}
utInlineC void chBoardSwapPosition(chBoard Board, uint32 from, uint32 to) {
    utAssert(from <= chBoardGetNumPosition(Board));
    utAssert(to <= chBoardGetNumPosition(Board));
    chPiece tmp = chBoardGetiPosition(Board, from);
    chBoardSetiPosition(Board, from, chBoardGetiPosition(Board, to));
    chBoardSetiPosition(Board, to, tmp);
}
utInlineC void chBoardSwapPositions(chBoard Board, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < chBoardGetNumPosition(Board));
    utAssert((to+count) < chBoardGetNumPosition(Board));
    chPiece tmp[count];
    memcpy(tmp, chBoardGetPositions(Board)+from, count*sizeof(chPiece));
    memcpy(chBoardGetPositions(Board)+from, chBoardGetPositions(Board)+to, count*sizeof(chPiece));
    memcpy(chBoardGetPositions(Board)+to, tmp, count*sizeof(chPiece));
}
#define chForeachBoardPosition(pVar, cVar) { \
    uint32 _xPosition; \
    for(_xPosition = 0; _xPosition < chBoardGetNumPosition(pVar); _xPosition++) { \
        cVar = chBoardGetiPosition(pVar, _xPosition);
#define chEndBoardPosition }}
utInlineC uint8 chBoardPlayerWhite(chBoard Board) {return chBoards.PlayerWhite[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetPlayerWhite(chBoard Board, uint8 value) {chBoards.PlayerWhite[chBoard2ValidIndex(Board)] = value;}
utInlineC chPiece chBoardGetWhiteKing(chBoard Board) {return chBoards.WhiteKing[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetWhiteKing(chBoard Board, chPiece value) {chBoards.WhiteKing[chBoard2ValidIndex(Board)] = value;}
utInlineC chPiece chBoardGetBlackKing(chBoard Board) {return chBoards.BlackKing[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetBlackKing(chBoard Board, chPiece value) {chBoards.BlackKing[chBoard2ValidIndex(Board)] = value;}
utInlineC uint32 chBoardGetMoveIndex_(chBoard Board) {return chBoards.MoveIndex_[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetMoveIndex_(chBoard Board, uint32 value) {chBoards.MoveIndex_[chBoard2ValidIndex(Board)] = value;}
utInlineC uint32 chBoardGetNumMove(chBoard Board) {return chBoards.NumMove[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetNumMove(chBoard Board, uint32 value) {chBoards.NumMove[chBoard2ValidIndex(Board)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 chBoardCheckMoveIndex(chBoard Board, uint32 x) {utAssert(x < chBoardGetNumMove(Board)); return x;}
#else
utInlineC uint32 chBoardCheckMoveIndex(chBoard Board, uint32 x) {return x;}
#endif
utInlineC chMove chBoardGetiMove(chBoard Board, uint32 x) {return chBoards.Move[
    chBoardGetMoveIndex_(Board) + chBoardCheckMoveIndex(Board, x)];}
utInlineC chMove *chBoardGetMove(chBoard Board) {return chBoards.Move + chBoardGetMoveIndex_(Board);}
#define chBoardGetMoves chBoardGetMove
utInlineC void chBoardSetMove(chBoard Board, chMove *valuePtr, uint32 numMove) {
    chBoardResizeMoves(Board, numMove);
    memcpy(chBoardGetMoves(Board), valuePtr, numMove*sizeof(chMove));}
utInlineC void chBoardSetiMove(chBoard Board, uint32 x, chMove value) {
    chBoards.Move[chBoardGetMoveIndex_(Board) + chBoardCheckMoveIndex(Board, (x))] = value;}
utInlineC void chBoardMoveMoves(chBoard Board, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= chBoardGetNumMove(Board));
    utAssert((from+count) <= chBoardGetNumMove(Board));
    memmove(chBoardGetMoves(Board)+to,chBoardGetMoves(Board)+from,((int32)count)*sizeof(chMove));
}
utInlineC void chBoardCopyMoves(chBoard Board, uint32 x, chMove * values, uint32 count) {
    utAssert((x+count) <= chBoardGetNumMove(Board));
    memcpy(chBoardGetMoves(Board)+x, values, count*sizeof(chMove));
}
utInlineC void chBoardAppendMoves(chBoard Board, chMove * values, uint32 count) {
    uint32 num = chBoardGetNumMove(Board);
    chBoardResizeMoves(Board, num+count);
    chBoardCopyMoves(Board, num, values, count);
}
utInlineC void chBoardAppendMove(chBoard Board, chMove Move) {
    chBoardResizeMoves(Board, chBoardGetNumMove(Board)+1);
    chBoardSetiMove(Board, chBoardGetNumMove(Board)-1, Move);
}
utInlineC void chBoardInsertMoves(chBoard Board, uint32 x, chMove *Move, uint32 count) {
    utAssert(x <= chBoardGetNumMove(Board));
    if(x < chBoardGetNumMove(Board)) {
        chBoardResizeMoves(Board, chBoardGetNumMove(Board)+count);
        chBoardMoveMoves(Board, x, x+count, chBoardGetNumMove(Board)-x-count);
        chBoardCopyMoves(Board, x, Move, count);
    }
    else {
        chBoardAppendMoves(Board, Move, count);
    }
}
utInlineC void chBoardInsertMove(chBoard Board, uint32 x, chMove Move) {
    chBoardInsertMoves(Board, x, &Move, 1);
}
utInlineC void chBoardRemoveMoves(chBoard Board, uint32 x, uint32 count) {
    utAssert((x+count) <= chBoardGetNumMove(Board));
    if((x+count) < chBoardGetNumMove(Board)) {
        chBoardMoveMoves(Board, x+count,x,chBoardGetNumMove(Board)-x-count);
    }
    chBoardResizeMoves(Board, chBoardGetNumMove(Board)-(int32)count);
}
utInlineC void chBoardRemoveMove(chBoard Board, uint32 x) {
    chBoardRemoveMoves(Board, x, 1);
}
utInlineC void chBoardSwapMove(chBoard Board, uint32 from, uint32 to) {
    utAssert(from <= chBoardGetNumMove(Board));
    utAssert(to <= chBoardGetNumMove(Board));
    chMove tmp = chBoardGetiMove(Board, from);
    chBoardSetiMove(Board, from, chBoardGetiMove(Board, to));
    chBoardSetiMove(Board, to, tmp);
}
utInlineC void chBoardSwapMoves(chBoard Board, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < chBoardGetNumMove(Board));
    utAssert((to+count) < chBoardGetNumMove(Board));
    chMove tmp[count];
    memcpy(tmp, chBoardGetMoves(Board)+from, count*sizeof(chMove));
    memcpy(chBoardGetMoves(Board)+from, chBoardGetMoves(Board)+to, count*sizeof(chMove));
    memcpy(chBoardGetMoves(Board)+to, tmp, count*sizeof(chMove));
}
#define chForeachBoardMove(pVar, cVar) { \
    uint32 _xMove; \
    for(_xMove = 0; _xMove < chBoardGetNumMove(pVar); _xMove++) { \
        cVar = chBoardGetiMove(pVar, _xMove);
#define chEndBoardMove }}
utInlineC uint32 chBoardGetMoveStackPos(chBoard Board) {return chBoards.MoveStackPos[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetMoveStackPos(chBoard Board, uint32 value) {chBoards.MoveStackPos[chBoard2ValidIndex(Board)] = value;}
utInlineC uint32 chBoardGetUndoMoveIndex_(chBoard Board) {return chBoards.UndoMoveIndex_[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetUndoMoveIndex_(chBoard Board, uint32 value) {chBoards.UndoMoveIndex_[chBoard2ValidIndex(Board)] = value;}
utInlineC uint32 chBoardGetNumUndoMove(chBoard Board) {return chBoards.NumUndoMove[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetNumUndoMove(chBoard Board, uint32 value) {chBoards.NumUndoMove[chBoard2ValidIndex(Board)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 chBoardCheckUndoMoveIndex(chBoard Board, uint32 x) {utAssert(x < chBoardGetNumUndoMove(Board)); return x;}
#else
utInlineC uint32 chBoardCheckUndoMoveIndex(chBoard Board, uint32 x) {return x;}
#endif
utInlineC chUndoMove chBoardGetiUndoMove(chBoard Board, uint32 x) {return chBoards.UndoMove[
    chBoardGetUndoMoveIndex_(Board) + chBoardCheckUndoMoveIndex(Board, x)];}
utInlineC chUndoMove *chBoardGetUndoMove(chBoard Board) {return chBoards.UndoMove + chBoardGetUndoMoveIndex_(Board);}
#define chBoardGetUndoMoves chBoardGetUndoMove
utInlineC void chBoardSetUndoMove(chBoard Board, chUndoMove *valuePtr, uint32 numUndoMove) {
    chBoardResizeUndoMoves(Board, numUndoMove);
    memcpy(chBoardGetUndoMoves(Board), valuePtr, numUndoMove*sizeof(chUndoMove));}
utInlineC void chBoardSetiUndoMove(chBoard Board, uint32 x, chUndoMove value) {
    chBoards.UndoMove[chBoardGetUndoMoveIndex_(Board) + chBoardCheckUndoMoveIndex(Board, (x))] = value;}
utInlineC void chBoardMoveUndoMoves(chBoard Board, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= chBoardGetNumUndoMove(Board));
    utAssert((from+count) <= chBoardGetNumUndoMove(Board));
    memmove(chBoardGetUndoMoves(Board)+to,chBoardGetUndoMoves(Board)+from,((int32)count)*sizeof(chUndoMove));
}
utInlineC void chBoardCopyUndoMoves(chBoard Board, uint32 x, chUndoMove * values, uint32 count) {
    utAssert((x+count) <= chBoardGetNumUndoMove(Board));
    memcpy(chBoardGetUndoMoves(Board)+x, values, count*sizeof(chUndoMove));
}
utInlineC void chBoardAppendUndoMoves(chBoard Board, chUndoMove * values, uint32 count) {
    uint32 num = chBoardGetNumUndoMove(Board);
    chBoardResizeUndoMoves(Board, num+count);
    chBoardCopyUndoMoves(Board, num, values, count);
}
utInlineC void chBoardAppendUndoMove(chBoard Board, chUndoMove UndoMove) {
    chBoardResizeUndoMoves(Board, chBoardGetNumUndoMove(Board)+1);
    chBoardSetiUndoMove(Board, chBoardGetNumUndoMove(Board)-1, UndoMove);
}
utInlineC void chBoardInsertUndoMoves(chBoard Board, uint32 x, chUndoMove *UndoMove, uint32 count) {
    utAssert(x <= chBoardGetNumUndoMove(Board));
    if(x < chBoardGetNumUndoMove(Board)) {
        chBoardResizeUndoMoves(Board, chBoardGetNumUndoMove(Board)+count);
        chBoardMoveUndoMoves(Board, x, x+count, chBoardGetNumUndoMove(Board)-x-count);
        chBoardCopyUndoMoves(Board, x, UndoMove, count);
    }
    else {
        chBoardAppendUndoMoves(Board, UndoMove, count);
    }
}
utInlineC void chBoardInsertUndoMove(chBoard Board, uint32 x, chUndoMove UndoMove) {
    chBoardInsertUndoMoves(Board, x, &UndoMove, 1);
}
utInlineC void chBoardRemoveUndoMoves(chBoard Board, uint32 x, uint32 count) {
    utAssert((x+count) <= chBoardGetNumUndoMove(Board));
    if((x+count) < chBoardGetNumUndoMove(Board)) {
        chBoardMoveUndoMoves(Board, x+count,x,chBoardGetNumUndoMove(Board)-x-count);
    }
    chBoardResizeUndoMoves(Board, chBoardGetNumUndoMove(Board)-(int32)count);
}
utInlineC void chBoardRemoveUndoMove(chBoard Board, uint32 x) {
    chBoardRemoveUndoMoves(Board, x, 1);
}
utInlineC void chBoardSwapUndoMove(chBoard Board, uint32 from, uint32 to) {
    utAssert(from <= chBoardGetNumUndoMove(Board));
    utAssert(to <= chBoardGetNumUndoMove(Board));
    chUndoMove tmp = chBoardGetiUndoMove(Board, from);
    chBoardSetiUndoMove(Board, from, chBoardGetiUndoMove(Board, to));
    chBoardSetiUndoMove(Board, to, tmp);
}
utInlineC void chBoardSwapUndoMoves(chBoard Board, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < chBoardGetNumUndoMove(Board));
    utAssert((to+count) < chBoardGetNumUndoMove(Board));
    chUndoMove tmp[count];
    memcpy(tmp, chBoardGetUndoMoves(Board)+from, count*sizeof(chUndoMove));
    memcpy(chBoardGetUndoMoves(Board)+from, chBoardGetUndoMoves(Board)+to, count*sizeof(chUndoMove));
    memcpy(chBoardGetUndoMoves(Board)+to, tmp, count*sizeof(chUndoMove));
}
#define chForeachBoardUndoMove(pVar, cVar) { \
    uint32 _xUndoMove; \
    for(_xUndoMove = 0; _xUndoMove < chBoardGetNumUndoMove(pVar); _xUndoMove++) { \
        cVar = chBoardGetiUndoMove(pVar, _xUndoMove);
#define chEndBoardUndoMove }}
utInlineC uint32 chBoardGetUndoMovePos(chBoard Board) {return chBoards.UndoMovePos[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetUndoMovePos(chBoard Board, uint32 value) {chBoards.UndoMovePos[chBoard2ValidIndex(Board)] = value;}
utInlineC uint32 chBoardGetWhiteScore(chBoard Board) {return chBoards.WhiteScore[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetWhiteScore(chBoard Board, uint32 value) {chBoards.WhiteScore[chBoard2ValidIndex(Board)] = value;}
utInlineC uint32 chBoardGetBlackScore(chBoard Board) {return chBoards.BlackScore[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetBlackScore(chBoard Board, uint32 value) {chBoards.BlackScore[chBoard2ValidIndex(Board)] = value;}
utInlineC chPiece chBoardGetFirstPiece(chBoard Board) {return chBoards.FirstPiece[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetFirstPiece(chBoard Board, chPiece value) {chBoards.FirstPiece[chBoard2ValidIndex(Board)] = value;}
utInlineC chPiece chBoardGetLastPiece(chBoard Board) {return chBoards.LastPiece[chBoard2ValidIndex(Board)];}
utInlineC void chBoardSetLastPiece(chBoard Board, chPiece value) {chBoards.LastPiece[chBoard2ValidIndex(Board)] = value;}
utInlineC void chBoardSetConstructorCallback(void(*func)(chBoard)) {chBoardConstructorCallback = func;}
utInlineC chBoardCallbackType chBoardGetConstructorCallback(void) {return chBoardConstructorCallback;}
utInlineC chBoard chFirstBoard(void) {return chRootData.usedBoard == 1? chBoardNull : chIndex2Board(1);}
utInlineC chBoard chLastBoard(void) {return chRootData.usedBoard == 1? chBoardNull :
    chIndex2Board(chRootData.usedBoard - 1);}
utInlineC chBoard chNextBoard(chBoard Board) {return chBoard2ValidIndex(Board) + 1 == chRootData.usedBoard? chBoardNull :
    Board + 1;}
utInlineC chBoard chPrevBoard(chBoard Board) {return chBoard2ValidIndex(Board) == 1? chBoardNull : Board - 1;}
#define chForeachBoard(var) \
    for(var = chIndex2Board(1); chBoard2Index(var) != chRootData.usedBoard; var++)
#define chEndBoard
utInlineC void chBoardFreeAll(void) {chSetUsedBoard(1); chSetUsedBoardPosition(0); chSetUsedBoardMove(0); chSetUsedBoardUndoMove(0);}
utInlineC chBoard chBoardAllocRaw(void) {
    chBoard Board;
    if(chRootData.usedBoard == chRootData.allocatedBoard) {
        chBoardAllocMore();
    }
    Board = chIndex2Board(chRootData.usedBoard);
    chSetUsedBoard(chUsedBoard() + 1);
    return Board;}
utInlineC chBoard chBoardAlloc(void) {
    chBoard Board = chBoardAllocRaw();
    chBoardSetPositionIndex_(Board, 0);
    chBoardSetNumPosition(Board, 0);
    chBoardSetNumPosition(Board, 0);
    chBoardSetPlayerWhite(Board, 0);
    chBoardSetWhiteKing(Board, chPieceNull);
    chBoardSetBlackKing(Board, chPieceNull);
    chBoardSetMoveIndex_(Board, 0);
    chBoardSetNumMove(Board, 0);
    chBoardSetNumMove(Board, 0);
    chBoardSetMoveStackPos(Board, 0);
    chBoardSetUndoMoveIndex_(Board, 0);
    chBoardSetNumUndoMove(Board, 0);
    chBoardSetNumUndoMove(Board, 0);
    chBoardSetUndoMovePos(Board, 0);
    chBoardSetWhiteScore(Board, 0);
    chBoardSetBlackScore(Board, 0);
    chBoardSetFirstPiece(Board, chPieceNull);
    chBoardSetLastPiece(Board, chPieceNull);
    if(chBoardConstructorCallback != NULL) {
        chBoardConstructorCallback(Board);
    }
    return Board;}

/*----------------------------------------------------------------------------------------
  Fields for class Piece.
----------------------------------------------------------------------------------------*/
struct chPieceFields {
    chPieceType *Type;
    uint8 *White;
    uint32 *Row;
    uint32 *Col;
    uint8 *InPlay;
    uint8 *NeverMoved;
    chBoard *Board;
    chPiece *NextBoardPiece;
    chPiece *PrevBoardPiece;
};
extern struct chPieceFields chPieces;

void chPieceAllocMore(void);
void chPieceCopyProps(chPiece chOldPiece, chPiece chNewPiece);
utInlineC chPieceType chPieceGetType(chPiece Piece) {return chPieces.Type[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetType(chPiece Piece, chPieceType value) {chPieces.Type[chPiece2ValidIndex(Piece)] = value;}
utInlineC uint8 chPieceWhite(chPiece Piece) {return chPieces.White[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetWhite(chPiece Piece, uint8 value) {chPieces.White[chPiece2ValidIndex(Piece)] = value;}
utInlineC uint32 chPieceGetRow(chPiece Piece) {return chPieces.Row[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetRow(chPiece Piece, uint32 value) {chPieces.Row[chPiece2ValidIndex(Piece)] = value;}
utInlineC uint32 chPieceGetCol(chPiece Piece) {return chPieces.Col[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetCol(chPiece Piece, uint32 value) {chPieces.Col[chPiece2ValidIndex(Piece)] = value;}
utInlineC uint8 chPieceInPlay(chPiece Piece) {return chPieces.InPlay[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetInPlay(chPiece Piece, uint8 value) {chPieces.InPlay[chPiece2ValidIndex(Piece)] = value;}
utInlineC uint8 chPieceNeverMoved(chPiece Piece) {return chPieces.NeverMoved[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetNeverMoved(chPiece Piece, uint8 value) {chPieces.NeverMoved[chPiece2ValidIndex(Piece)] = value;}
utInlineC chBoard chPieceGetBoard(chPiece Piece) {return chPieces.Board[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetBoard(chPiece Piece, chBoard value) {chPieces.Board[chPiece2ValidIndex(Piece)] = value;}
utInlineC chPiece chPieceGetNextBoardPiece(chPiece Piece) {return chPieces.NextBoardPiece[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetNextBoardPiece(chPiece Piece, chPiece value) {chPieces.NextBoardPiece[chPiece2ValidIndex(Piece)] = value;}
utInlineC chPiece chPieceGetPrevBoardPiece(chPiece Piece) {return chPieces.PrevBoardPiece[chPiece2ValidIndex(Piece)];}
utInlineC void chPieceSetPrevBoardPiece(chPiece Piece, chPiece value) {chPieces.PrevBoardPiece[chPiece2ValidIndex(Piece)] = value;}
utInlineC void chPieceSetConstructorCallback(void(*func)(chPiece)) {chPieceConstructorCallback = func;}
utInlineC chPieceCallbackType chPieceGetConstructorCallback(void) {return chPieceConstructorCallback;}
utInlineC chPiece chFirstPiece(void) {return chRootData.usedPiece == 1? chPieceNull : chIndex2Piece(1);}
utInlineC chPiece chLastPiece(void) {return chRootData.usedPiece == 1? chPieceNull :
    chIndex2Piece(chRootData.usedPiece - 1);}
utInlineC chPiece chNextPiece(chPiece Piece) {return chPiece2ValidIndex(Piece) + 1 == chRootData.usedPiece? chPieceNull :
    Piece + 1;}
utInlineC chPiece chPrevPiece(chPiece Piece) {return chPiece2ValidIndex(Piece) == 1? chPieceNull : Piece - 1;}
#define chForeachPiece(var) \
    for(var = chIndex2Piece(1); chPiece2Index(var) != chRootData.usedPiece; var++)
#define chEndPiece
utInlineC void chPieceFreeAll(void) {chSetUsedPiece(1);}
utInlineC chPiece chPieceAllocRaw(void) {
    chPiece Piece;
    if(chRootData.usedPiece == chRootData.allocatedPiece) {
        chPieceAllocMore();
    }
    Piece = chIndex2Piece(chRootData.usedPiece);
    chSetUsedPiece(chUsedPiece() + 1);
    return Piece;}
utInlineC chPiece chPieceAlloc(void) {
    chPiece Piece = chPieceAllocRaw();
    chPieceSetType(Piece, CH_PAWN);
    chPieceSetWhite(Piece, 0);
    chPieceSetRow(Piece, 0);
    chPieceSetCol(Piece, 0);
    chPieceSetInPlay(Piece, 0);
    chPieceSetNeverMoved(Piece, 0);
    chPieceSetBoard(Piece, chBoardNull);
    chPieceSetNextBoardPiece(Piece, chPieceNull);
    chPieceSetPrevBoardPiece(Piece, chPieceNull);
    if(chPieceConstructorCallback != NULL) {
        chPieceConstructorCallback(Piece);
    }
    return Piece;}

/*----------------------------------------------------------------------------------------
  Relationship macros between classes.
----------------------------------------------------------------------------------------*/
#define chForeachBoardPiece(pVar, cVar) \
    for(cVar = chBoardGetFirstPiece(pVar); cVar != chPieceNull; \
        cVar = chPieceGetNextBoardPiece(cVar))
#define chEndBoardPiece
#define chSafeForeachBoardPiece(pVar, cVar) { \
    chPiece _nextPiece; \
    for(cVar = chBoardGetFirstPiece(pVar); cVar != chPieceNull; cVar = _nextPiece) { \
        _nextPiece = chPieceGetNextBoardPiece(cVar);
#define chEndSafeBoardPiece }}
void chBoardInsertPiece(chBoard Board, chPiece _Piece);
void chBoardRemovePiece(chBoard Board, chPiece _Piece);
void chBoardInsertAfterPiece(chBoard Board, chPiece prevPiece, chPiece _Piece);
void chBoardAppendPiece(chBoard Board, chPiece _Piece);
void chDatabaseStart(void);
void chDatabaseStop(void);
utInlineC void chDatabaseSetSaved(bool value) {utModuleSetSaved(utModules + chModuleID, value);}
#if defined __cplusplus
}
#endif

#endif
