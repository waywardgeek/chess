/*----------------------------------------------------------------------------------------
  Database ch
----------------------------------------------------------------------------------------*/

#include "chdatabase.h"

struct chRootType_ chRootData;
uint8 chModuleID;
struct chBoardFields chBoards;
struct chPieceFields chPieces;

/*----------------------------------------------------------------------------------------
  Constructor/Destructor hooks.
----------------------------------------------------------------------------------------*/
chBoardCallbackType chBoardConstructorCallback;
chPieceCallbackType chPieceConstructorCallback;

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocBoard(void)
{
    chBoard Board = chBoardAlloc();

    return chBoard2Index(Board);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Board.
----------------------------------------------------------------------------------------*/
static void allocBoards(void)
{
    chSetAllocatedBoard(2);
    chSetUsedBoard(1);
    chBoards.PositionIndex_ = utNewAInitFirst(uint32, (chAllocatedBoard()));
    chBoards.NumPosition = utNewAInitFirst(uint32, (chAllocatedBoard()));
    chSetUsedBoardPosition(0);
    chSetAllocatedBoardPosition(2);
    chSetFreeBoardPosition(0);
    chBoards.Position = utNewAInitFirst(chPiece, chAllocatedBoardPosition());
    chBoards.PlayerWhite = utNewAInitFirst(uint8, (chAllocatedBoard()));
    chBoards.WhiteKing = utNewAInitFirst(chPiece, (chAllocatedBoard()));
    chBoards.BlackKing = utNewAInitFirst(chPiece, (chAllocatedBoard()));
    chBoards.MoveIndex_ = utNewAInitFirst(uint32, (chAllocatedBoard()));
    chBoards.NumMove = utNewAInitFirst(uint32, (chAllocatedBoard()));
    chSetUsedBoardMove(0);
    chSetAllocatedBoardMove(2);
    chSetFreeBoardMove(0);
    chBoards.Move = utNewAInitFirst(chMove, chAllocatedBoardMove());
    chBoards.MoveStackPos = utNewAInitFirst(uint32, (chAllocatedBoard()));
    chBoards.UndoMoveIndex_ = utNewAInitFirst(uint32, (chAllocatedBoard()));
    chBoards.NumUndoMove = utNewAInitFirst(uint32, (chAllocatedBoard()));
    chSetUsedBoardUndoMove(0);
    chSetAllocatedBoardUndoMove(2);
    chSetFreeBoardUndoMove(0);
    chBoards.UndoMove = utNewAInitFirst(chUndoMove, chAllocatedBoardUndoMove());
    chBoards.UndoMovePos = utNewAInitFirst(uint32, (chAllocatedBoard()));
    chBoards.WhiteScore = utNewAInitFirst(int32, (chAllocatedBoard()));
    chBoards.BlackScore = utNewAInitFirst(int32, (chAllocatedBoard()));
    chBoards.FirstPiece = utNewAInitFirst(chPiece, (chAllocatedBoard()));
    chBoards.LastPiece = utNewAInitFirst(chPiece, (chAllocatedBoard()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Board.
----------------------------------------------------------------------------------------*/
static void reallocBoards(
    uint32 newSize)
{
    utResizeArray(chBoards.PositionIndex_, (newSize));
    utResizeArray(chBoards.NumPosition, (newSize));
    utResizeArray(chBoards.PlayerWhite, (newSize));
    utResizeArray(chBoards.WhiteKing, (newSize));
    utResizeArray(chBoards.BlackKing, (newSize));
    utResizeArray(chBoards.MoveIndex_, (newSize));
    utResizeArray(chBoards.NumMove, (newSize));
    utResizeArray(chBoards.MoveStackPos, (newSize));
    utResizeArray(chBoards.UndoMoveIndex_, (newSize));
    utResizeArray(chBoards.NumUndoMove, (newSize));
    utResizeArray(chBoards.UndoMovePos, (newSize));
    utResizeArray(chBoards.WhiteScore, (newSize));
    utResizeArray(chBoards.BlackScore, (newSize));
    utResizeArray(chBoards.FirstPiece, (newSize));
    utResizeArray(chBoards.LastPiece, (newSize));
    chSetAllocatedBoard(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Boards.
----------------------------------------------------------------------------------------*/
void chBoardAllocMore(void)
{
    reallocBoards((uint32)(chAllocatedBoard() + (chAllocatedBoard() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Board.Position heap to free memory.
----------------------------------------------------------------------------------------*/
void chCompactBoardPositions(void)
{
    uint32 elementSize = sizeof(chPiece);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    chPiece *toPtr = chBoards.Position;
    chPiece *fromPtr = toPtr;
    chBoard Board;
    uint32 size;

    while(fromPtr < chBoards.Position + chUsedBoardPosition()) {
        Board = *(chBoard *)(void *)fromPtr;
        if(Board != chBoardNull) {
            /* Need to move it to toPtr */
            size = utMax(chBoardGetNumPosition(Board) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            chBoardSetPositionIndex_(Board, toPtr - chBoards.Position + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((chBoard *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    chSetUsedBoardPosition(toPtr - chBoards.Position);
    chSetFreeBoardPosition(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Board.Position heap.
----------------------------------------------------------------------------------------*/
static void allocMoreBoardPositions(
    uint32 spaceNeeded)
{
    uint32 freeSpace = chAllocatedBoardPosition() - chUsedBoardPosition();
    uint32 elementSize = sizeof(chPiece);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    chPiece *ptr = chBoards.Position;
    chBoard Board;
    uint32 size;

    while(ptr < chBoards.Position + chUsedBoardPosition()) {
        Board = *(chBoard*)(void*)ptr;
        if(Board != chBoardNull) {
            chValidBoard(Board);
            size = utMax(chBoardGetNumPosition(Board) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((chBoard *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((chFreeBoardPosition() << 2) > chUsedBoardPosition()) {
        chCompactBoardPositions();
        freeSpace = chAllocatedBoardPosition() - chUsedBoardPosition();
    }
    if(freeSpace < spaceNeeded) {
        chSetAllocatedBoardPosition(chAllocatedBoardPosition() + spaceNeeded - freeSpace +
            (chAllocatedBoardPosition() >> 1));
        utResizeArray(chBoards.Position, chAllocatedBoardPosition());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Board.Position array.
----------------------------------------------------------------------------------------*/
void chBoardAllocPositions(
    chBoard Board,
    uint32 numPositions)
{
    uint32 freeSpace = chAllocatedBoardPosition() - chUsedBoardPosition();
    uint32 elementSize = sizeof(chPiece);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numPositions + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(chBoardGetNumPosition(Board) == 0);
#endif
    if(numPositions == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreBoardPositions(spaceNeeded);
    }
    chBoardSetPositionIndex_(Board, chUsedBoardPosition() + usedHeaderSize);
    chBoardSetNumPosition(Board, numPositions);
    *(chBoard *)(void *)(chBoards.Position + chUsedBoardPosition()) = Board;
    {
        uint32 xValue;
        for(xValue = (uint32)(chBoardGetPositionIndex_(Board)); xValue < chBoardGetPositionIndex_(Board) + numPositions; xValue++) {
            chBoards.Position[xValue] = chPieceNull;
        }
    }
    chSetUsedBoardPosition(chUsedBoardPosition() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around chBoardGetPositions for the database manager.
----------------------------------------------------------------------------------------*/
static void *getBoardPositions(
    uint64 objectNumber,
    uint32 *numValues)
{
    chBoard Board = chIndex2Board((uint32)objectNumber);

    *numValues = chBoardGetNumPosition(Board);
    return chBoardGetPositions(Board);
}

/*----------------------------------------------------------------------------------------
  Wrapper around chBoardAllocPositions for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocBoardPositions(
    uint64 objectNumber,
    uint32 numValues)
{
    chBoard Board = chIndex2Board((uint32)objectNumber);

    chBoardSetPositionIndex_(Board, 0);
    chBoardSetNumPosition(Board, 0);
    if(numValues == 0) {
        return NULL;
    }
    chBoardAllocPositions(Board, numValues);
    return chBoardGetPositions(Board);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Board.Position array.
----------------------------------------------------------------------------------------*/
void chBoardFreePositions(
    chBoard Board)
{
    uint32 elementSize = sizeof(chPiece);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(chBoardGetNumPosition(Board) + usedHeaderSize, freeHeaderSize);
    chPiece *dataPtr = chBoardGetPositions(Board) - usedHeaderSize;

    if(chBoardGetNumPosition(Board) == 0) {
        return;
    }
    *(chBoard *)(void *)(dataPtr) = chBoardNull;
    *(uint32 *)(void *)(((chBoard *)(void *)dataPtr) + 1) = size;
    chBoardSetNumPosition(Board, 0);
    chSetFreeBoardPosition(chFreeBoardPosition() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Board.Position array.
----------------------------------------------------------------------------------------*/
void chBoardResizePositions(
    chBoard Board,
    uint32 numPositions)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(chPiece);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numPositions + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(chBoardGetNumPosition(Board) + usedHeaderSize, freeHeaderSize);
    chPiece *dataPtr;

    if(numPositions == 0) {
        if(chBoardGetNumPosition(Board) != 0) {
            chBoardFreePositions(Board);
        }
        return;
    }
    if(chBoardGetNumPosition(Board) == 0) {
        chBoardAllocPositions(Board, numPositions);
        return;
    }
    freeSpace = chAllocatedBoardPosition() - chUsedBoardPosition();
    if(freeSpace < newSize) {
        allocMoreBoardPositions(newSize);
    }
    dataPtr = chBoardGetPositions(Board) - usedHeaderSize;
    memcpy((void *)(chBoards.Position + chUsedBoardPosition()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(chUsedBoardPosition() + oldSize); xValue < chUsedBoardPosition() + oldSize + newSize - oldSize; xValue++) {
                chBoards.Position[xValue] = chPieceNull;
            }
        }
    }
    *(chBoard *)(void *)dataPtr = chBoardNull;
    *(uint32 *)(void *)(((chBoard *)(void *)dataPtr) + 1) = oldSize;
    chSetFreeBoardPosition(chFreeBoardPosition() + oldSize);
    chBoardSetPositionIndex_(Board, chUsedBoardPosition() + usedHeaderSize);
    chBoardSetNumPosition(Board, numPositions);
    chSetUsedBoardPosition(chUsedBoardPosition() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Board.Move heap to free memory.
----------------------------------------------------------------------------------------*/
void chCompactBoardMoves(void)
{
    uint32 elementSize = sizeof(chMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    chMove *toPtr = chBoards.Move;
    chMove *fromPtr = toPtr;
    chBoard Board;
    uint32 size;

    while(fromPtr < chBoards.Move + chUsedBoardMove()) {
        Board = *(chBoard *)(void *)fromPtr;
        if(Board != chBoardNull) {
            /* Need to move it to toPtr */
            size = utMax(chBoardGetNumMove(Board) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            chBoardSetMoveIndex_(Board, toPtr - chBoards.Move + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((chBoard *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    chSetUsedBoardMove(toPtr - chBoards.Move);
    chSetFreeBoardMove(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Board.Move heap.
----------------------------------------------------------------------------------------*/
static void allocMoreBoardMoves(
    uint32 spaceNeeded)
{
    uint32 freeSpace = chAllocatedBoardMove() - chUsedBoardMove();
    uint32 elementSize = sizeof(chMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    chMove *ptr = chBoards.Move;
    chBoard Board;
    uint32 size;

    while(ptr < chBoards.Move + chUsedBoardMove()) {
        Board = *(chBoard*)(void*)ptr;
        if(Board != chBoardNull) {
            chValidBoard(Board);
            size = utMax(chBoardGetNumMove(Board) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((chBoard *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((chFreeBoardMove() << 2) > chUsedBoardMove()) {
        chCompactBoardMoves();
        freeSpace = chAllocatedBoardMove() - chUsedBoardMove();
    }
    if(freeSpace < spaceNeeded) {
        chSetAllocatedBoardMove(chAllocatedBoardMove() + spaceNeeded - freeSpace +
            (chAllocatedBoardMove() >> 1));
        utResizeArray(chBoards.Move, chAllocatedBoardMove());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Board.Move array.
----------------------------------------------------------------------------------------*/
void chBoardAllocMoves(
    chBoard Board,
    uint32 numMoves)
{
    uint32 freeSpace = chAllocatedBoardMove() - chUsedBoardMove();
    uint32 elementSize = sizeof(chMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numMoves + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(chBoardGetNumMove(Board) == 0);
#endif
    if(numMoves == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreBoardMoves(spaceNeeded);
    }
    chBoardSetMoveIndex_(Board, chUsedBoardMove() + usedHeaderSize);
    chBoardSetNumMove(Board, numMoves);
    *(chBoard *)(void *)(chBoards.Move + chUsedBoardMove()) = Board;
    memset(chBoards.Move + chBoardGetMoveIndex_(Board), 0, ((numMoves))*sizeof(chMove));
    chSetUsedBoardMove(chUsedBoardMove() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around chBoardGetMoves for the database manager.
----------------------------------------------------------------------------------------*/
static void *getBoardMoves(
    uint64 objectNumber,
    uint32 *numValues)
{
    chBoard Board = chIndex2Board((uint32)objectNumber);

    *numValues = chBoardGetNumMove(Board);
    return chBoardGetMoves(Board);
}

/*----------------------------------------------------------------------------------------
  Wrapper around chBoardAllocMoves for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocBoardMoves(
    uint64 objectNumber,
    uint32 numValues)
{
    chBoard Board = chIndex2Board((uint32)objectNumber);

    chBoardSetMoveIndex_(Board, 0);
    chBoardSetNumMove(Board, 0);
    if(numValues == 0) {
        return NULL;
    }
    chBoardAllocMoves(Board, numValues);
    return chBoardGetMoves(Board);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Board.Move array.
----------------------------------------------------------------------------------------*/
void chBoardFreeMoves(
    chBoard Board)
{
    uint32 elementSize = sizeof(chMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(chBoardGetNumMove(Board) + usedHeaderSize, freeHeaderSize);
    chMove *dataPtr = chBoardGetMoves(Board) - usedHeaderSize;

    if(chBoardGetNumMove(Board) == 0) {
        return;
    }
    *(chBoard *)(void *)(dataPtr) = chBoardNull;
    *(uint32 *)(void *)(((chBoard *)(void *)dataPtr) + 1) = size;
    chBoardSetNumMove(Board, 0);
    chSetFreeBoardMove(chFreeBoardMove() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Board.Move array.
----------------------------------------------------------------------------------------*/
void chBoardResizeMoves(
    chBoard Board,
    uint32 numMoves)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(chMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numMoves + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(chBoardGetNumMove(Board) + usedHeaderSize, freeHeaderSize);
    chMove *dataPtr;

    if(numMoves == 0) {
        if(chBoardGetNumMove(Board) != 0) {
            chBoardFreeMoves(Board);
        }
        return;
    }
    if(chBoardGetNumMove(Board) == 0) {
        chBoardAllocMoves(Board, numMoves);
        return;
    }
    freeSpace = chAllocatedBoardMove() - chUsedBoardMove();
    if(freeSpace < newSize) {
        allocMoreBoardMoves(newSize);
    }
    dataPtr = chBoardGetMoves(Board) - usedHeaderSize;
    memcpy((void *)(chBoards.Move + chUsedBoardMove()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        memset(chBoards.Move + chUsedBoardMove() + oldSize, 0, ((newSize - oldSize))*sizeof(chMove));
    }
    *(chBoard *)(void *)dataPtr = chBoardNull;
    *(uint32 *)(void *)(((chBoard *)(void *)dataPtr) + 1) = oldSize;
    chSetFreeBoardMove(chFreeBoardMove() + oldSize);
    chBoardSetMoveIndex_(Board, chUsedBoardMove() + usedHeaderSize);
    chBoardSetNumMove(Board, numMoves);
    chSetUsedBoardMove(chUsedBoardMove() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Board.UndoMove heap to free memory.
----------------------------------------------------------------------------------------*/
void chCompactBoardUndoMoves(void)
{
    uint32 elementSize = sizeof(chUndoMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    chUndoMove *toPtr = chBoards.UndoMove;
    chUndoMove *fromPtr = toPtr;
    chBoard Board;
    uint32 size;

    while(fromPtr < chBoards.UndoMove + chUsedBoardUndoMove()) {
        Board = *(chBoard *)(void *)fromPtr;
        if(Board != chBoardNull) {
            /* Need to move it to toPtr */
            size = utMax(chBoardGetNumUndoMove(Board) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            chBoardSetUndoMoveIndex_(Board, toPtr - chBoards.UndoMove + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((chBoard *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    chSetUsedBoardUndoMove(toPtr - chBoards.UndoMove);
    chSetFreeBoardUndoMove(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Board.UndoMove heap.
----------------------------------------------------------------------------------------*/
static void allocMoreBoardUndoMoves(
    uint32 spaceNeeded)
{
    uint32 freeSpace = chAllocatedBoardUndoMove() - chUsedBoardUndoMove();
    uint32 elementSize = sizeof(chUndoMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    chUndoMove *ptr = chBoards.UndoMove;
    chBoard Board;
    uint32 size;

    while(ptr < chBoards.UndoMove + chUsedBoardUndoMove()) {
        Board = *(chBoard*)(void*)ptr;
        if(Board != chBoardNull) {
            chValidBoard(Board);
            size = utMax(chBoardGetNumUndoMove(Board) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((chBoard *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((chFreeBoardUndoMove() << 2) > chUsedBoardUndoMove()) {
        chCompactBoardUndoMoves();
        freeSpace = chAllocatedBoardUndoMove() - chUsedBoardUndoMove();
    }
    if(freeSpace < spaceNeeded) {
        chSetAllocatedBoardUndoMove(chAllocatedBoardUndoMove() + spaceNeeded - freeSpace +
            (chAllocatedBoardUndoMove() >> 1));
        utResizeArray(chBoards.UndoMove, chAllocatedBoardUndoMove());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Board.UndoMove array.
----------------------------------------------------------------------------------------*/
void chBoardAllocUndoMoves(
    chBoard Board,
    uint32 numUndoMoves)
{
    uint32 freeSpace = chAllocatedBoardUndoMove() - chUsedBoardUndoMove();
    uint32 elementSize = sizeof(chUndoMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numUndoMoves + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(chBoardGetNumUndoMove(Board) == 0);
#endif
    if(numUndoMoves == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreBoardUndoMoves(spaceNeeded);
    }
    chBoardSetUndoMoveIndex_(Board, chUsedBoardUndoMove() + usedHeaderSize);
    chBoardSetNumUndoMove(Board, numUndoMoves);
    *(chBoard *)(void *)(chBoards.UndoMove + chUsedBoardUndoMove()) = Board;
    memset(chBoards.UndoMove + chBoardGetUndoMoveIndex_(Board), 0, ((numUndoMoves))*sizeof(chUndoMove));
    chSetUsedBoardUndoMove(chUsedBoardUndoMove() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around chBoardGetUndoMoves for the database manager.
----------------------------------------------------------------------------------------*/
static void *getBoardUndoMoves(
    uint64 objectNumber,
    uint32 *numValues)
{
    chBoard Board = chIndex2Board((uint32)objectNumber);

    *numValues = chBoardGetNumUndoMove(Board);
    return chBoardGetUndoMoves(Board);
}

/*----------------------------------------------------------------------------------------
  Wrapper around chBoardAllocUndoMoves for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocBoardUndoMoves(
    uint64 objectNumber,
    uint32 numValues)
{
    chBoard Board = chIndex2Board((uint32)objectNumber);

    chBoardSetUndoMoveIndex_(Board, 0);
    chBoardSetNumUndoMove(Board, 0);
    if(numValues == 0) {
        return NULL;
    }
    chBoardAllocUndoMoves(Board, numValues);
    return chBoardGetUndoMoves(Board);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Board.UndoMove array.
----------------------------------------------------------------------------------------*/
void chBoardFreeUndoMoves(
    chBoard Board)
{
    uint32 elementSize = sizeof(chUndoMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(chBoardGetNumUndoMove(Board) + usedHeaderSize, freeHeaderSize);
    chUndoMove *dataPtr = chBoardGetUndoMoves(Board) - usedHeaderSize;

    if(chBoardGetNumUndoMove(Board) == 0) {
        return;
    }
    *(chBoard *)(void *)(dataPtr) = chBoardNull;
    *(uint32 *)(void *)(((chBoard *)(void *)dataPtr) + 1) = size;
    chBoardSetNumUndoMove(Board, 0);
    chSetFreeBoardUndoMove(chFreeBoardUndoMove() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Board.UndoMove array.
----------------------------------------------------------------------------------------*/
void chBoardResizeUndoMoves(
    chBoard Board,
    uint32 numUndoMoves)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(chUndoMove);
    uint32 usedHeaderSize = (sizeof(chBoard) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(chBoard) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numUndoMoves + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(chBoardGetNumUndoMove(Board) + usedHeaderSize, freeHeaderSize);
    chUndoMove *dataPtr;

    if(numUndoMoves == 0) {
        if(chBoardGetNumUndoMove(Board) != 0) {
            chBoardFreeUndoMoves(Board);
        }
        return;
    }
    if(chBoardGetNumUndoMove(Board) == 0) {
        chBoardAllocUndoMoves(Board, numUndoMoves);
        return;
    }
    freeSpace = chAllocatedBoardUndoMove() - chUsedBoardUndoMove();
    if(freeSpace < newSize) {
        allocMoreBoardUndoMoves(newSize);
    }
    dataPtr = chBoardGetUndoMoves(Board) - usedHeaderSize;
    memcpy((void *)(chBoards.UndoMove + chUsedBoardUndoMove()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        memset(chBoards.UndoMove + chUsedBoardUndoMove() + oldSize, 0, ((newSize - oldSize))*sizeof(chUndoMove));
    }
    *(chBoard *)(void *)dataPtr = chBoardNull;
    *(uint32 *)(void *)(((chBoard *)(void *)dataPtr) + 1) = oldSize;
    chSetFreeBoardUndoMove(chFreeBoardUndoMove() + oldSize);
    chBoardSetUndoMoveIndex_(Board, chUsedBoardUndoMove() + usedHeaderSize);
    chBoardSetNumUndoMove(Board, numUndoMoves);
    chSetUsedBoardUndoMove(chUsedBoardUndoMove() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Board.
----------------------------------------------------------------------------------------*/
void chBoardCopyProps(
    chBoard oldBoard,
    chBoard newBoard)
{
    chBoardSetPlayerWhite(newBoard, chBoardPlayerWhite(oldBoard));
    chBoardSetMoveStackPos(newBoard, chBoardGetMoveStackPos(oldBoard));
    chBoardSetUndoMovePos(newBoard, chBoardGetUndoMovePos(oldBoard));
    chBoardSetWhiteScore(newBoard, chBoardGetWhiteScore(oldBoard));
    chBoardSetBlackScore(newBoard, chBoardGetBlackScore(oldBoard));
}

/*----------------------------------------------------------------------------------------
  Add the Piece to the head of the list on the Board.
----------------------------------------------------------------------------------------*/
void chBoardInsertPiece(
    chBoard Board,
    chPiece _Piece)
{
#if defined(DD_DEBUG)
    if(Board == chBoardNull) {
        utExit("Non-existent Board");
    }
    if(_Piece == chPieceNull) {
        utExit("Non-existent Piece");
    }
    if(chPieceGetBoard(_Piece) != chBoardNull) {
        utExit("Attempting to add Piece to Board twice");
    }
#endif
    chPieceSetNextBoardPiece(_Piece, chBoardGetFirstPiece(Board));
    if(chBoardGetFirstPiece(Board) != chPieceNull) {
        chPieceSetPrevBoardPiece(chBoardGetFirstPiece(Board), _Piece);
    }
    chBoardSetFirstPiece(Board, _Piece);
    chPieceSetPrevBoardPiece(_Piece, chPieceNull);
    if(chBoardGetLastPiece(Board) == chPieceNull) {
        chBoardSetLastPiece(Board, _Piece);
    }
    chPieceSetBoard(_Piece, Board);
}

/*----------------------------------------------------------------------------------------
  Add the Piece to the end of the list on the Board.
----------------------------------------------------------------------------------------*/
void chBoardAppendPiece(
    chBoard Board,
    chPiece _Piece)
{
#if defined(DD_DEBUG)
    if(Board == chBoardNull) {
        utExit("Non-existent Board");
    }
    if(_Piece == chPieceNull) {
        utExit("Non-existent Piece");
    }
    if(chPieceGetBoard(_Piece) != chBoardNull) {
        utExit("Attempting to add Piece to Board twice");
    }
#endif
    chPieceSetPrevBoardPiece(_Piece, chBoardGetLastPiece(Board));
    if(chBoardGetLastPiece(Board) != chPieceNull) {
        chPieceSetNextBoardPiece(chBoardGetLastPiece(Board), _Piece);
    }
    chBoardSetLastPiece(Board, _Piece);
    chPieceSetNextBoardPiece(_Piece, chPieceNull);
    if(chBoardGetFirstPiece(Board) == chPieceNull) {
        chBoardSetFirstPiece(Board, _Piece);
    }
    chPieceSetBoard(_Piece, Board);
}

/*----------------------------------------------------------------------------------------
  Insert the Piece to the Board after the previous Piece.
----------------------------------------------------------------------------------------*/
void chBoardInsertAfterPiece(
    chBoard Board,
    chPiece prevPiece,
    chPiece _Piece)
{
    chPiece nextPiece = chPieceGetNextBoardPiece(prevPiece);

#if defined(DD_DEBUG)
    if(Board == chBoardNull) {
        utExit("Non-existent Board");
    }
    if(_Piece == chPieceNull) {
        utExit("Non-existent Piece");
    }
    if(chPieceGetBoard(_Piece) != chBoardNull) {
        utExit("Attempting to add Piece to Board twice");
    }
#endif
    chPieceSetNextBoardPiece(_Piece, nextPiece);
    chPieceSetNextBoardPiece(prevPiece, _Piece);
    chPieceSetPrevBoardPiece(_Piece, prevPiece);
    if(nextPiece != chPieceNull) {
        chPieceSetPrevBoardPiece(nextPiece, _Piece);
    }
    if(chBoardGetLastPiece(Board) == prevPiece) {
        chBoardSetLastPiece(Board, _Piece);
    }
    chPieceSetBoard(_Piece, Board);
}

/*----------------------------------------------------------------------------------------
 Remove the Piece from the Board.
----------------------------------------------------------------------------------------*/
void chBoardRemovePiece(
    chBoard Board,
    chPiece _Piece)
{
    chPiece pPiece, nPiece;

#if defined(DD_DEBUG)
    if(_Piece == chPieceNull) {
        utExit("Non-existent Piece");
    }
    if(chPieceGetBoard(_Piece) != chBoardNull && chPieceGetBoard(_Piece) != Board) {
        utExit("Delete Piece from non-owning Board");
    }
#endif
    nPiece = chPieceGetNextBoardPiece(_Piece);
    pPiece = chPieceGetPrevBoardPiece(_Piece);
    if(pPiece != chPieceNull) {
        chPieceSetNextBoardPiece(pPiece, nPiece);
    } else if(chBoardGetFirstPiece(Board) == _Piece) {
        chBoardSetFirstPiece(Board, nPiece);
    }
    if(nPiece != chPieceNull) {
        chPieceSetPrevBoardPiece(nPiece, pPiece);
    } else if(chBoardGetLastPiece(Board) == _Piece) {
        chBoardSetLastPiece(Board, pPiece);
    }
    chPieceSetNextBoardPiece(_Piece, chPieceNull);
    chPieceSetPrevBoardPiece(_Piece, chPieceNull);
    chPieceSetBoard(_Piece, chBoardNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void chShowBoard(
    chBoard Board)
{
    utDatabaseShowObject("ch", "Board", chBoard2Index(Board));
}
#endif

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocPiece(void)
{
    chPiece Piece = chPieceAlloc();

    return chPiece2Index(Piece);
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Piece.
----------------------------------------------------------------------------------------*/
static void allocPieces(void)
{
    chSetAllocatedPiece(2);
    chSetUsedPiece(1);
    chPieces.Type = utNewAInitFirst(chPieceType, (chAllocatedPiece()));
    chPieces.White = utNewAInitFirst(uint8, (chAllocatedPiece()));
    chPieces.Row = utNewAInitFirst(uint32, (chAllocatedPiece()));
    chPieces.Col = utNewAInitFirst(uint32, (chAllocatedPiece()));
    chPieces.InPlay = utNewAInitFirst(uint8, (chAllocatedPiece()));
    chPieces.NeverMoved = utNewAInitFirst(uint8, (chAllocatedPiece()));
    chPieces.Board = utNewAInitFirst(chBoard, (chAllocatedPiece()));
    chPieces.NextBoardPiece = utNewAInitFirst(chPiece, (chAllocatedPiece()));
    chPieces.PrevBoardPiece = utNewAInitFirst(chPiece, (chAllocatedPiece()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Piece.
----------------------------------------------------------------------------------------*/
static void reallocPieces(
    uint32 newSize)
{
    utResizeArray(chPieces.Type, (newSize));
    utResizeArray(chPieces.White, (newSize));
    utResizeArray(chPieces.Row, (newSize));
    utResizeArray(chPieces.Col, (newSize));
    utResizeArray(chPieces.InPlay, (newSize));
    utResizeArray(chPieces.NeverMoved, (newSize));
    utResizeArray(chPieces.Board, (newSize));
    utResizeArray(chPieces.NextBoardPiece, (newSize));
    utResizeArray(chPieces.PrevBoardPiece, (newSize));
    chSetAllocatedPiece(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Pieces.
----------------------------------------------------------------------------------------*/
void chPieceAllocMore(void)
{
    reallocPieces((uint32)(chAllocatedPiece() + (chAllocatedPiece() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Piece.
----------------------------------------------------------------------------------------*/
void chPieceCopyProps(
    chPiece oldPiece,
    chPiece newPiece)
{
    chPieceSetType(newPiece, chPieceGetType(oldPiece));
    chPieceSetWhite(newPiece, chPieceWhite(oldPiece));
    chPieceSetRow(newPiece, chPieceGetRow(oldPiece));
    chPieceSetCol(newPiece, chPieceGetCol(oldPiece));
    chPieceSetInPlay(newPiece, chPieceInPlay(oldPiece));
    chPieceSetNeverMoved(newPiece, chPieceNeverMoved(oldPiece));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void chShowPiece(
    chPiece Piece)
{
    utDatabaseShowObject("ch", "Piece", chPiece2Index(Piece));
}
#endif

/*----------------------------------------------------------------------------------------
  Free memory used by the ch database.
----------------------------------------------------------------------------------------*/
void chDatabaseStop(void)
{
    utFree(chBoards.PositionIndex_);
    utFree(chBoards.NumPosition);
    utFree(chBoards.Position);
    utFree(chBoards.PlayerWhite);
    utFree(chBoards.WhiteKing);
    utFree(chBoards.BlackKing);
    utFree(chBoards.MoveIndex_);
    utFree(chBoards.NumMove);
    utFree(chBoards.Move);
    utFree(chBoards.MoveStackPos);
    utFree(chBoards.UndoMoveIndex_);
    utFree(chBoards.NumUndoMove);
    utFree(chBoards.UndoMove);
    utFree(chBoards.UndoMovePos);
    utFree(chBoards.WhiteScore);
    utFree(chBoards.BlackScore);
    utFree(chBoards.FirstPiece);
    utFree(chBoards.LastPiece);
    utFree(chPieces.Type);
    utFree(chPieces.White);
    utFree(chPieces.Row);
    utFree(chPieces.Col);
    utFree(chPieces.InPlay);
    utFree(chPieces.NeverMoved);
    utFree(chPieces.Board);
    utFree(chPieces.NextBoardPiece);
    utFree(chPieces.PrevBoardPiece);
    utUnregisterModule(chModuleID);
}

/*----------------------------------------------------------------------------------------
  Allocate memory used by the ch database.
----------------------------------------------------------------------------------------*/
void chDatabaseStart(void)
{
    if(!utInitialized()) {
        utStart();
    }
    chRootData.hash = 0x83eb0015;
    chModuleID = utRegisterModule("ch", false, chHash(), 2, 27, 1, sizeof(struct chRootType_),
        &chRootData, chDatabaseStart, chDatabaseStop);
    utRegisterEnum("PieceType", 6);
    utRegisterEntry("CH_PAWN", 0);
    utRegisterEntry("CH_ROOK", 1);
    utRegisterEntry("CH_KNIGHT", 2);
    utRegisterEntry("CH_BISHOP", 3);
    utRegisterEntry("CH_QUEEN", 4);
    utRegisterEntry("CH_KING", 5);
    utRegisterClass("Board", 18, &chRootData.usedBoard, &chRootData.allocatedBoard,
        NULL, 65535, 4, allocBoard, NULL);
    utRegisterField("PositionIndex_", &chBoards.PositionIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumPosition", &chBoards.NumPosition, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Position", &chBoards.Position, sizeof(chPiece), UT_POINTER, "Piece");
    utRegisterArray(&chRootData.usedBoardPosition, &chRootData.allocatedBoardPosition,
        getBoardPositions, allocBoardPositions, chCompactBoardPositions);
    utRegisterField("PlayerWhite", &chBoards.PlayerWhite, sizeof(uint8), UT_BOOL, NULL);
    utRegisterField("WhiteKing", &chBoards.WhiteKing, sizeof(chPiece), UT_POINTER, "Piece");
    utRegisterField("BlackKing", &chBoards.BlackKing, sizeof(chPiece), UT_POINTER, "Piece");
    utRegisterField("MoveIndex_", &chBoards.MoveIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumMove", &chBoards.NumMove, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Move", &chBoards.Move, sizeof(chMove), UT_TYPEDEF, NULL);
    utRegisterArray(&chRootData.usedBoardMove, &chRootData.allocatedBoardMove,
        getBoardMoves, allocBoardMoves, chCompactBoardMoves);
    utRegisterField("MoveStackPos", &chBoards.MoveStackPos, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("UndoMoveIndex_", &chBoards.UndoMoveIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumUndoMove", &chBoards.NumUndoMove, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("UndoMove", &chBoards.UndoMove, sizeof(chUndoMove), UT_TYPEDEF, NULL);
    utRegisterArray(&chRootData.usedBoardUndoMove, &chRootData.allocatedBoardUndoMove,
        getBoardUndoMoves, allocBoardUndoMoves, chCompactBoardUndoMoves);
    utRegisterField("UndoMovePos", &chBoards.UndoMovePos, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("WhiteScore", &chBoards.WhiteScore, sizeof(int32), UT_INT, NULL);
    utRegisterField("BlackScore", &chBoards.BlackScore, sizeof(int32), UT_INT, NULL);
    utRegisterField("FirstPiece", &chBoards.FirstPiece, sizeof(chPiece), UT_POINTER, "Piece");
    utRegisterField("LastPiece", &chBoards.LastPiece, sizeof(chPiece), UT_POINTER, "Piece");
    utRegisterClass("Piece", 9, &chRootData.usedPiece, &chRootData.allocatedPiece,
        NULL, 65535, 4, allocPiece, NULL);
    utRegisterField("Type", &chPieces.Type, sizeof(chPieceType), UT_ENUM, "PieceType");
    utRegisterField("White", &chPieces.White, sizeof(uint8), UT_BOOL, NULL);
    utRegisterField("Row", &chPieces.Row, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Col", &chPieces.Col, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("InPlay", &chPieces.InPlay, sizeof(uint8), UT_BOOL, NULL);
    utRegisterField("NeverMoved", &chPieces.NeverMoved, sizeof(uint8), UT_BOOL, NULL);
    utRegisterField("Board", &chPieces.Board, sizeof(chBoard), UT_POINTER, "Board");
    utRegisterField("NextBoardPiece", &chPieces.NextBoardPiece, sizeof(chPiece), UT_POINTER, "Piece");
    utRegisterField("PrevBoardPiece", &chPieces.PrevBoardPiece, sizeof(chPiece), UT_POINTER, "Piece");
    allocBoards();
    allocPieces();
}

