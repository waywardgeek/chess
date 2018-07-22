#include <ddutil.h>

struct chMove_st {
    uint8 fromRow;
    uint8 fromCol;
    uint8 toRow;
    uint8 toCol;
};

typedef struct chMove_st chMove;

#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
typedef struct _struct_chPiece{char val;} *chPiece;
#else
typedef uint32 chPiece;
#endif

struct chUndoMove_st {
    chMove move;  // The original move.
    chPiece target;  // The piece taken, or castle if castling.
    bool queenedPawn;
    bool firstMove;
};

typedef struct chUndoMove_st chUndoMove;
