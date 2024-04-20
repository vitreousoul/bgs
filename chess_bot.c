/*
    TODO: Convert this into an ".h" file and use it in "gui_app.c" or wherever.
    TODO: Namespace all names so that chess_bot can become a library.
*/
#include <stdio.h>
#include <stdint.h>

#include "./clibs/raylib.h"

#define u8 uint8_t
#define u32 uint32_t
#define b32 uint32_t
#define s8 int8_t
#define s32 int32_t

#define global_variable static
#define internal static

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))

typedef u8 piece;
typedef u8 square;

typedef enum
{
    piece_type_Rook,
    piece_type_Knight,
    piece_type_Bishop,
    piece_type_Queen,
    piece_type_King,
    piece_type_Pawn,
    piece_type_Count,
} piece_type;

typedef enum
{
    piece_White_Queen_Rook,
    piece_White_Queen_Knight,
    piece_White_Queen_Bishop,
    piece_White_Queen,
    piece_White_King,
    piece_White_King_Bishop,
    piece_White_King_Knight,
    piece_White_King_Rook,
    piece_White_Pawn_A,
    piece_White_Pawn_B,
    piece_White_Pawn_C,
    piece_White_Pawn_D,
    piece_White_Pawn_E,
    piece_White_Pawn_F,
    piece_White_Pawn_G,
    piece_White_Pawn_H,

    piece_Black_Pawn_A,
    piece_Black_Pawn_B,
    piece_Black_Pawn_C,
    piece_Black_Pawn_D,
    piece_Black_Pawn_E,
    piece_Black_Pawn_F,
    piece_Black_Pawn_G,
    piece_Black_Pawn_H,
    piece_Black_Queen_Rook,
    piece_Black_Queen_Knight,
    piece_Black_Queen_Bishop,
    piece_Black_Queen,
    piece_Black_King,
    piece_Black_King_Bishop,
    piece_Black_King_Knight,
    piece_Black_King_Rook,
    piece_Count,
    piece_Null,
} piece_name;

/* NOTE: Is_###_Piece assumes piece indexes are splice by color,
   so all we have to do is check the most significant bit.
*/
#define Get_Piece_Color(p) ((p) & (1 << 4))
#define Is_White_Piece(p) (Get_Piece_Color(p) == 0)
#define Is_Black_Piece(p) (Get_Piece_Color(p) != 0)

#define Is_Valid_Piece(p) ((p) >= (piece_White_Queen_Rook) && (p) < piece_Count)
#define Is_Valid_Square(s) ((s) >= 0 && (s) < 64)
#define Is_Valid_Row_Col(row, col) ((row) >= 0 && (row) < 8 && (col) >= 0 && (col) < 8)
#define Get_Square_Index(row, col) ((row) * 8 + (col))

typedef enum
{
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    square_Null,
} square_name;

global_variable u8 PieceType[piece_Count] = {
    [piece_White_Queen_Rook]    = piece_type_Rook,
    [piece_White_Queen_Knight]  = piece_type_Knight,
    [piece_White_Queen_Bishop]  = piece_type_Bishop,
    [piece_White_Queen]         = piece_type_Queen,
    [piece_White_King]          = piece_type_King,
    [piece_White_King_Bishop]   = piece_type_Bishop,
    [piece_White_King_Knight]   = piece_type_Knight,
    [piece_White_King_Rook]     = piece_type_Rook,
    [piece_White_Pawn_A]        = piece_type_Pawn,
    [piece_White_Pawn_B]        = piece_type_Pawn,
    [piece_White_Pawn_C]        = piece_type_Pawn,
    [piece_White_Pawn_D]        = piece_type_Pawn,
    [piece_White_Pawn_E]        = piece_type_Pawn,
    [piece_White_Pawn_F]        = piece_type_Pawn,
    [piece_White_Pawn_G]        = piece_type_Pawn,
    [piece_White_Pawn_H]        = piece_type_Pawn,

    [piece_Black_Queen_Rook]    = piece_type_Rook,
    [piece_Black_Queen_Knight]  = piece_type_Knight,
    [piece_Black_Queen_Bishop]  = piece_type_Bishop,
    [piece_Black_Queen]         = piece_type_Queen,
    [piece_Black_King]          = piece_type_King,
    [piece_Black_King_Bishop]   = piece_type_Bishop,
    [piece_Black_King_Knight]   = piece_type_Knight,
    [piece_Black_King_Rook]     = piece_type_Rook,
    [piece_Black_Pawn_A]        = piece_type_Pawn,
    [piece_Black_Pawn_B]        = piece_type_Pawn,
    [piece_Black_Pawn_C]        = piece_type_Pawn,
    [piece_Black_Pawn_D]        = piece_type_Pawn,
    [piece_Black_Pawn_E]        = piece_type_Pawn,
    [piece_Black_Pawn_F]        = piece_type_Pawn,
    [piece_Black_Pawn_G]        = piece_type_Pawn,
    [piece_Black_Pawn_H]        = piece_type_Pawn,
};

global_variable char PieceDisplayTable[piece_Count] = {
    'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R',
    'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
    'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
    'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
};

#define Whose_Turn_Flag              (1 << 0)
#define White_Queen_Side_Castle_Flag (1 << 1)
#define White_King_Side_Castle_Flag  (1 << 2)
#define Black_Queen_Side_Castle_Flag (1 << 3)
#define Black_King_Side_Castle_Flag  (1 << 4)

#define Flag_Get(  flags, flag) (((flags) &  (flag)) != 0)
#define Flag_Set(  flags, flag)  ((flags) = (flags) & ~(flag) | flag)
#define Flag_Unset(flags, flag)  ((flags) = (flags) & ~(flag))
#define Flag_Toggle(flags, flag)  (Flag_Get((flags), (flag)) ? Flag_Unset((flags), (flag)) : Flag_Set((flags), (flag)))

#define Is_White_Turn(v) ((v) == 0)
#define Is_Black_Turn(v) ((v) == 1)

typedef enum
{
    move_type_Null,
    move_type_Move,
    move_type_QueenCastle,
    move_type_KingCastle,
    move_type_EnPassant,
} move_type;

typedef struct
{
    move_type Type;
    piece Piece;
    square BeginSquare;
    square EndSquare;
} move;

typedef struct
{
    b32 Flags;
    move LastMove;
    u8 Piece[piece_Count];
} game_state;

#define Max_Potential_Moves \
    (8 * 4 + 2 * 14 + 2 * 8  + 2 * 14 + 28    + 8)
/*   Pawns   Rooks    Knights  Bishops  Queen   King*/

typedef struct
{
    /* TODO: Squares should probably just live in app_state, and then delete move_data. */
    u8 Squares[64];
} move_data;

struct game_tree
{
    game_state State;
    struct game_tree *NextSibling;
    struct game_tree *FirstChild;
};

typedef struct game_tree game_tree;

#define Game_Tree_Node_Pool_Size 2048

typedef struct
{
    game_tree *GameTreeRoot;
    game_tree *GameTreeCurrent;
    game_tree *GameTreeFreeList;
    s32 GameTreeNodePoolIndex;
    game_tree GameTreeNodePool[Game_Tree_Node_Pool_Size];
} app_state;

#if DEBUG
#define Assert(p) Assert_(p, __FILE__, __LINE__)
internal void Assert_(b32 Proposition, char *FilePath, s32 LineNumber)
{
    if (!Proposition)
    {
        b32 *NullPtr = 0;
        printf("Assertion failed on line %d in %s\n", LineNumber, FilePath);
        /* NOTE: Dereference a null-pointer and crash the program. */
        Proposition = *NullPtr;
    }
}
#else
#define Assert(p)
#endif

internal game_tree *PushGameTree(app_state *AppState)
{
    game_tree *GameTree = 0;

    if (AppState->GameTreeNodePoolIndex < Game_Tree_Node_Pool_Size)
    {
        GameTree = AppState->GameTreeNodePool + AppState->GameTreeNodePoolIndex;
        ++AppState->GameTreeNodePoolIndex;
    }
    else
    {
        printf("Error: Pushing game_tree.\n");
    }

    return GameTree;
}

internal void RemovePiece(move_data *MoveData, game_state *GameState, piece Piece)
{
    MoveData->Squares[GameState->Piece[Piece]] = piece_Null;
    GameState->Piece[Piece] = square_Null;
}

internal void MakeMove(move_data *MoveData, game_state *GameState, move Move)
{
    piece Piece = Move.Piece;
    b32 IsWhitePiece = Is_White_Piece(Piece);

    MoveData->Squares[GameState->Piece[Piece]] = piece_Null;

    switch (Move.Type)
    {
    case move_type_EnPassant:
    {
        RemovePiece(MoveData, GameState, GameState->LastMove.Piece);
    } /* Fall through */
    case move_type_Move:
    {
        GameState->Piece[Piece] = Move.EndSquare;
        MoveData->Squares[Move.EndSquare] = Piece;
    } break;
    /* TODO: Compress castling code below. */
    case move_type_QueenCastle:
    {
        if (IsWhitePiece)
        {
            MoveData->Squares[GameState->Piece[piece_White_Queen_Rook]] = piece_Null;
            GameState->Piece[piece_White_Queen_Rook] = D1;
            MoveData->Squares[D1] = piece_White_Queen_Rook;

            MoveData->Squares[GameState->Piece[piece_White_King]] = piece_Null;
            GameState->Piece[piece_White_King] = C1;
            MoveData->Squares[C1] = piece_White_King;
        }
        else
        {
            MoveData->Squares[GameState->Piece[piece_Black_Queen_Rook]] = piece_Null;
            GameState->Piece[piece_Black_Queen_Rook] = D8;
            MoveData->Squares[D8] = piece_Black_Queen_Rook;

            MoveData->Squares[GameState->Piece[piece_Black_King]] = piece_Null;
            GameState->Piece[piece_Black_King] = C8;
            MoveData->Squares[C8] = piece_Black_King;
        }
    } break;
    case move_type_KingCastle:
    {
        if (IsWhitePiece)
        {
            MoveData->Squares[GameState->Piece[piece_White_King_Rook]] = piece_Null;
            GameState->Piece[piece_White_King_Rook] = F1;
            MoveData->Squares[F1] = piece_White_King_Rook;

            MoveData->Squares[GameState->Piece[piece_White_King]] = piece_Null;
            GameState->Piece[piece_White_King] = G1;
            MoveData->Squares[G1] = piece_White_King;
        }
        else
        {
            MoveData->Squares[GameState->Piece[piece_Black_King_Rook]] = piece_Null;
            GameState->Piece[piece_Black_King_Rook] = F8;
            MoveData->Squares[F8] = piece_Black_King_Rook;

            MoveData->Squares[GameState->Piece[piece_Black_King]] = piece_Null;
            GameState->Piece[piece_Black_King] = G8;
            MoveData->Squares[G8] = piece_Black_King;
        }
    } break;
    default:
    {
        Assert(!"Unknown move type");
    }
    }

    Flag_Toggle(GameState->Flags, Whose_Turn_Flag);

    switch (Piece)
    {
    case piece_White_Queen_Rook:
    {
        Flag_Unset(GameState->Flags, White_Queen_Side_Castle_Flag);
    } break;
    case piece_Black_Queen_Rook:
    {
        Flag_Unset(GameState->Flags, Black_Queen_Side_Castle_Flag);
    } break;
    case piece_White_King_Rook:
    {
        Flag_Unset(GameState->Flags, White_King_Side_Castle_Flag);
    } break;
    case piece_Black_King_Rook:
    {
        Flag_Unset(GameState->Flags, Black_King_Side_Castle_Flag);
    } break;
    case piece_White_King:
    {
        Flag_Unset(GameState->Flags, White_Queen_Side_Castle_Flag);
        Flag_Unset(GameState->Flags, White_King_Side_Castle_Flag);
    } break;
    case piece_Black_King:
    {
        Flag_Unset(GameState->Flags, Black_Queen_Side_Castle_Flag);
        Flag_Unset(GameState->Flags, Black_King_Side_Castle_Flag);
    } break;
    default: break;
    }

    GameState->LastMove = Move;
}

internal void CopyGameState(game_state *Source, game_state *Destination)
{
    Destination->Flags = Source->Flags;
    Destination->LastMove = Source->LastMove;

    for (s32 I = 0; I < piece_Count; ++I)
    {
        Destination->Piece[I] = Source->Piece[I];
    }
}

internal void InitializeSquares(square *Squares, game_state *GameState)
{
    for (s32 I = 0; I < 64; ++I)
    {
        Squares[I] = piece_Null;
    }

    for (s32 I = 0; I < piece_Count; ++I)
    {
        s32 Square = GameState->Piece[I];

        if (Is_Valid_Square(Square))
        {
            Squares[Square] = I;
        }
    }
}

internal void AddPotential(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, square EndSquare, move_type MoveType)
{
    b32 IsCastleMove = MoveType == move_type_QueenCastle || MoveType == move_type_KingCastle;
    Assert(Is_Valid_Piece(Piece));
    Assert(IsCastleMove || Is_Valid_Square(EndSquare));

    square BeginSquare = GameState->Piece[Piece];
    Assert(Is_Valid_Square(BeginSquare));

    move Move;
    Move.Type = MoveType;
    Move.Piece = Piece;
    Move.BeginSquare = BeginSquare;
    Move.EndSquare = EndSquare;

    game_state NewGameState;
    CopyGameState(GameState, &NewGameState);

    MakeMove(MoveData, &NewGameState, Move);

    game_tree *GameTree = PushGameTree(AppState);
    GameTree->State = NewGameState;

    if (GameTree)
    {
        if (!AppState->GameTreeCurrent)
        {
            AppState->GameTreeCurrent = GameTree;
        }
        else
        {
            GameTree->NextSibling = AppState->GameTreeCurrent;
            AppState->GameTreeCurrent = GameTree;
        }

        AppState->GameTreeRoot->NextSibling = GameTree;
    }
    else
    {
        Assert(0);
    }
}

/* TODO: Maybe bundle Piece, Row, Col, ... into a struct. */
internal void Look(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 RowOffset, u8 ColOffset, u8 MaxLength)
{
    Assert(RowOffset || ColOffset);

    u8 PieceColor = Get_Piece_Color(Piece);
    u8 CurrentRow = Row + RowOffset;
    u8 CurrentCol = Col + ColOffset;
    u8 TotalLength = 0;

    for (;;)
    {
        b32 PositionInBounds = (CurrentRow >= 0 && CurrentRow < 8 &&
                                CurrentCol >= 0 && CurrentCol < 8);
        b32 CanReach = TotalLength < MaxLength;

        if (!(PositionInBounds && CanReach))
        {
            break;
        }

        s32 NewSquare = CurrentRow * 8 + CurrentCol;
        s32 TargetPiece = MoveData->Squares[NewSquare];

        if (Is_Valid_Piece(TargetPiece))
        {
            if (Get_Piece_Color(TargetPiece) != PieceColor)
            {
                AddPotential(AppState, MoveData, GameState, Piece, NewSquare, move_type_Move);
            }

            break;
        }
        else
        {
            AddPotential(AppState, MoveData, GameState, Piece, NewSquare, move_type_Move);
        }

        CurrentRow += RowOffset;
        CurrentCol += ColOffset;
        TotalLength += 1;
    }
}

internal void LookRight(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, MoveData, GameState, Piece, Row, Col, 0, 1, MaxLength);
}

internal void LookUp(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, MoveData, GameState, Piece, Row, Col, 1, 0, MaxLength);
}

internal void LookLeft(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, MoveData, GameState, Piece, Row, Col, 0, -1, MaxLength);
}

internal void LookDown(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, MoveData, GameState, Piece, Row, Col, -1, 0, MaxLength);
}

internal void LookUpRight(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, MoveData, GameState, Piece, Row, Col, 1, 1, MaxLength);
}

internal void LookUpLeft(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, MoveData, GameState, Piece, Row, Col, 1, -1, MaxLength);
}

internal void LookDownLeft(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, MoveData, GameState, Piece, Row, Col, -1, -1, MaxLength);
}

internal void LookDownRight(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, MoveData, GameState, Piece, Row, Col, -1, 1, MaxLength);
}

internal void LookAllDirections(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    LookRight(AppState, MoveData, GameState, Piece, Row, Col, MaxLength);
    LookUpRight(AppState, MoveData, GameState, Piece, Row, Col, MaxLength);
    LookUp(AppState, MoveData, GameState, Piece, Row, Col, MaxLength);
    LookUpLeft(AppState, MoveData, GameState, Piece, Row, Col, MaxLength);
    LookLeft(AppState, MoveData, GameState, Piece, Row, Col, MaxLength);
    LookDownLeft(AppState, MoveData, GameState, Piece, Row, Col, MaxLength);
    LookDown(AppState, MoveData, GameState, Piece, Row, Col, MaxLength);
    LookDownRight(AppState, MoveData, GameState, Piece, Row, Col, MaxLength);
}

internal void LookPawn(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col)
{
    u8 PieceColor = Get_Piece_Color(Piece);
    s8 Multiplier = 1;
    u8 StartingRow = 1;
    u8 EnPassantRow = 5;

    if (Is_Black_Piece(Piece))
    {
        Multiplier = -1;
        StartingRow = 6;
        EnPassantRow = 2;
    }

    /* NOTE: Move forward */
    for (s32 I = 1; I < 3; ++I)
    {
        if (Row != StartingRow && I == 2)
        {
            break;
        }

        s8 Offset = I * Multiplier;
        u8 CurrentRow = Row + Offset;

        square Square = Get_Square_Index(CurrentRow, Col);
        s32 TargetPiece = MoveData->Squares[Square];

        if (!Is_Valid_Piece(TargetPiece))
        {
            AddPotential(AppState, MoveData, GameState, Piece, Square, move_type_Move);
        }
        else
        {
            break;
        }
    }

    /* NOTE: Captures */
    for (s32 I = -1; I < 2; I += 2)
    {
        s8 Offset = Multiplier;
        u8 CurrentRow = Row + Offset;
        u8 CurrentCol = Col + I;

        square Square = Get_Square_Index(CurrentRow, CurrentCol);
        s32 TargetPiece = MoveData->Squares[Square];

        if (Is_Valid_Piece(TargetPiece) && Get_Piece_Color(TargetPiece) != PieceColor)
        {
            AddPotential(AppState, MoveData, GameState, Piece, Square, move_type_Move);
        }
    }

    /* NOTE: En-passant */
    if (Row == EnPassantRow)
    {
        for (s32 I = -1; I < 2; I += 2)
        {
            u8 CurrentCol = Col + I;

            piece MovePieceType = PieceType[GameState->LastMove.Piece];

            s8 BeginRow = GameState->LastMove.BeginSquare / 8;
            s8 EndRow = GameState->LastMove.EndSquare / 8;

            s8 BeginCol = GameState->LastMove.BeginSquare % 8;
            s8 EndCol = GameState->LastMove.EndSquare % 8;

            b32 IsPawnMove = MovePieceType == piece_type_Pawn;
            b32 WasTwoSquareMove = Multiplier * (EndRow - BeginRow) == -2;
            b32 LastMoveOnSameCol = (BeginCol == EndCol) && (EndCol == CurrentCol);

            if (IsPawnMove && WasTwoSquareMove && LastMoveOnSameCol)
            {
                square CaptureSquare = Get_Square_Index(Row + Multiplier, CurrentCol);
                AddPotential(AppState, MoveData, GameState, Piece, CaptureSquare, move_type_EnPassant);
            }
        }
    }
}

internal void LookKnight(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece, u8 Row, u8 Col)
{
    s8 RowColOffsets[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
    u8 PieceColor = Get_Piece_Color(Piece);

    for (s32 I = 0; I < 8; ++I)
    {
        s8 TargetRow = Row + RowColOffsets[I][0];
        s8 TargetCol = Col + RowColOffsets[I][1];

        if (Is_Valid_Row_Col(TargetRow, TargetCol))
        {
            s32 NewSquare = TargetRow * 8 + TargetCol;
            s32 TargetPiece = MoveData->Squares[NewSquare];

            if (Is_Valid_Piece(TargetPiece))
            {
                if (Get_Piece_Color(TargetPiece) != PieceColor)
                {
                    AddPotential(AppState, MoveData, GameState, Piece, NewSquare, move_type_Move);
                }
            }
            else
            {
                AddPotential(AppState, MoveData, GameState, Piece, NewSquare, move_type_Move);
            }
        }
    }
}

internal void DebugPrintSquares(square *Squares)
{
    for (s32 Row = 0; Row < 8; ++Row)
    {
        for (s32 Col = 0; Col < 8; ++Col)
        {
            s32 SquareIndex = (7 - Row) * 8 + Col;
            s32 Piece = Squares[SquareIndex];
            char SquareDisplay = '-';

            if (Is_Valid_Piece(Piece))
            {
                SquareDisplay = PieceDisplayTable[Piece];
            }

            printf("%c ", SquareDisplay);
        }
        printf("\n");
    }
    printf("\n");
}

internal void LookCastle(app_state *AppState, move_data *MoveData, game_state *GameState, piece Piece)
{
    u8 KingPosition;

    if (Is_White_Piece(Piece))
    {
        InitializeSquares(MoveData->Squares, GameState);
        KingPosition = E1;
        Assert(GameState->Piece[Piece] == KingPosition);

        if (Flag_Get(GameState->Flags, White_Queen_Side_Castle_Flag))
        {
            Assert(GameState->Piece[piece_White_Queen_Rook] == A1);

            b32 B1Open = !Is_Valid_Piece(MoveData->Squares[B1]);
            b32 C1Open = !Is_Valid_Piece(MoveData->Squares[C1]);
            b32 D1Open = !Is_Valid_Piece(MoveData->Squares[D1]);

            if (B1Open && C1Open && D1Open)
            {
                AddPotential(AppState, MoveData, GameState, Piece, 255, move_type_QueenCastle);
            }
        }

        InitializeSquares(MoveData->Squares, GameState);

        if (Flag_Get(GameState->Flags, White_King_Side_Castle_Flag))
        {
            Assert(GameState->Piece[piece_White_King_Rook] == H1);

            b32 F1Open = !Is_Valid_Piece(MoveData->Squares[F1]);
            b32 G1Open = !Is_Valid_Piece(MoveData->Squares[G1]);

            if (F1Open && G1Open)
            {
                AddPotential(AppState, MoveData, GameState, Piece, 255, move_type_KingCastle);
            }
        }
    }
    else
    {
        InitializeSquares(MoveData->Squares, GameState);
        KingPosition = E8;
        Assert(GameState->Piece[Piece] == KingPosition);
        if (Flag_Get(GameState->Flags, Black_Queen_Side_Castle_Flag))
        {
            Assert(GameState->Piece[piece_Black_Queen_Rook] == A8);

            b32 B8Open = !Is_Valid_Piece(MoveData->Squares[B8]);
            b32 C8Open = !Is_Valid_Piece(MoveData->Squares[C8]);
            b32 D8Open = !Is_Valid_Piece(MoveData->Squares[D8]);

            if (B8Open && C8Open && D8Open)
            {
                AddPotential(AppState, MoveData, GameState, Piece, 255, move_type_QueenCastle);
            }
        }

        InitializeSquares(MoveData->Squares, GameState);

        if (Flag_Get(GameState->Flags, Black_King_Side_Castle_Flag))
        {
            Assert(GameState->Piece[piece_Black_King_Rook] == H8);

            b32 F8Open = !Is_Valid_Piece(MoveData->Squares[F8]);
            b32 G8Open = !Is_Valid_Piece(MoveData->Squares[G8]);

            if (F8Open && G8Open)
            {
                AddPotential(AppState, MoveData, GameState, Piece, 255, move_type_KingCastle);
            }
        }
    }
}

internal void GeneratePotentials(app_state *AppState, move_data *MoveData, game_state *GameState)
{
    s32 Start;
    b32 TurnFlag = Flag_Get(GameState->Flags, Whose_Turn_Flag);

    if (Is_White_Turn(TurnFlag))
    {
        Start = piece_White_Queen_Rook;
    }
    else
    {
        Start = piece_Black_Pawn_A;
    }

    s32 End = Start + 16;

    for (s32 I = Start; I < End; ++I)
    {
        piece Piece = I;
        u8 Square = GameState->Piece[I];
        u8 Row = Square / 8;
        u8 Col = Square % 8;

        InitializeSquares(MoveData->Squares, GameState);

        if (Is_Valid_Square(Square))
        {
            switch (PieceType[I])
            {
            case piece_type_Rook:
            {
                LookRight(AppState, MoveData, GameState, Piece, Row, Col, 8);
                LookDown(AppState, MoveData, GameState, Piece, Row, Col, 8);
                LookLeft(AppState, MoveData, GameState, Piece, Row, Col, 8);
                LookUp(AppState, MoveData, GameState, Piece, Row, Col, 8);
            } break;
            case piece_type_Knight:
            {
                LookKnight(AppState, MoveData, GameState, Piece, Row, Col);
            } break;
            case piece_type_Bishop:
            {
                LookUpRight(AppState, MoveData, GameState, Piece, Row, Col, 8);
                LookUpLeft(AppState, MoveData, GameState, Piece, Row, Col, 8);
                LookDownLeft(AppState, MoveData, GameState, Piece, Row, Col, 8);
                LookDownRight(AppState, MoveData, GameState, Piece, Row, Col, 8);
            } break;
            case piece_type_Queen:
            {
                LookAllDirections(AppState, MoveData, GameState, Piece, Row, Col, 8);
            } break;
            case piece_type_King:
            {
                LookAllDirections(AppState, MoveData, GameState, Piece, Row, Col, 1);
                LookCastle(AppState, MoveData, GameState, Piece);
            } break;
            case piece_type_Pawn:
            {
                LookPawn(AppState, MoveData, GameState, Piece, Row, Col);
            } break;
            default:
            {
                Assert(!"Unknown piece type\n");
            } break;
            }
        }
    }
}

internal void InitializeGameState(game_state *GameState)
{
    GameState->Flags = 0;
    Flag_Set(GameState->Flags, White_Queen_Side_Castle_Flag);
    Flag_Set(GameState->Flags, White_King_Side_Castle_Flag);
    Flag_Set(GameState->Flags, Black_Queen_Side_Castle_Flag);
    Flag_Set(GameState->Flags, Black_King_Side_Castle_Flag);

    for (s32 I = 0; I < 2; ++I)
    {
        s32 PieceOffset = 48 * I;

        for (s32 J = 0; J < 16; ++J)
        {
            s32 SquareIndex = J + PieceOffset;
            s32 PieceIndex = (16 * I) + J;

            GameState->Piece[PieceIndex] = SquareIndex;
        }
    }
}

internal void MovePiece(game_state *GameState, move_data *MoveData, piece Piece, square Square)
{
    move Move = {0};

    Move.Type = move_type_Move;
    Move.Piece = Piece;
    Move.BeginSquare = GameState->Piece[Move.Piece];
    Move.EndSquare = Square;

    MakeMove(MoveData, GameState, Move);
}

internal void SetupForTesting(game_state *GameState, move_data *MoveData)
{
#if 1
    /* NOTE: Test un-passant for black. */
    MovePiece(GameState, MoveData, piece_Black_Pawn_D, D3);
    MovePiece(GameState, MoveData, piece_White_Pawn_E, E4);

    GameState->Flags = Flag_Set(GameState->Flags, Whose_Turn_Flag);
#elif 0
    /* NOTE: Test castling for white. */
    MovePiece(GameState, MoveData, piece_White_Queen_Knight, E5);
    MovePiece(GameState, MoveData, piece_White_Queen_Bishop, F5);
    MovePiece(GameState, MoveData, piece_White_Queen, G5);
    MovePiece(GameState, MoveData, piece_White_King_Knight, E6);
    MovePiece(GameState, MoveData, piece_White_King_Bishop, F6);

    Flag_Unset(GameState.Flags, Whose_Turn_Flag);
#elif 0
    /* NOTE: Test castling for black */
    MovePiece(GameState, MoveData, piece_Black_Queen_Knight, E5);
    MovePiece(GameState, MoveData, piece_Black_Queen_Bishop, F5);
    MovePiece(GameState, MoveData, piece_Black_Queen, G5);
    MovePiece(GameState, MoveData, piece_Black_King_Knight, E6);
    MovePiece(GameState, MoveData, piece_Black_King_Bishop, F6);

    Flag_Set(GameState->Flags, Whose_Turn_Flag);
#else
    /* NOTE: Test un-passant for white. */
    MovePiece(GameState, MoveData, piece_White_Pawn_E, E6);
    MovePiece(GameState, MoveData, piece_Black_Pawn_D, D5);
#endif
}

int main(void)
{
    int Result = 0;

    app_state AppState = {0};
    game_state GameState;
    move_data MoveData = {0};
    game_tree GameTree = {0};

    InitializeGameState(&GameState);
    InitializeSquares(MoveData.Squares, &GameState);

    SetupForTesting(&GameState, &MoveData);
    GameTree.State = GameState;

    AppState.GameTreeRoot = &GameTree;

    GeneratePotentials(&AppState, &MoveData, &GameState);

    { /* Debug print out potential states. */
        game_tree *PreviousNode = 0;
        game_tree *CurrentNode = AppState.GameTreeRoot;

        game_state PotentialState = CurrentNode->State;
        InitializeSquares(MoveData.Squares, &PotentialState);
        DebugPrintSquares(MoveData.Squares);

        do
        {
            while (CurrentNode->NextSibling)
            {
                PreviousNode = CurrentNode;
                CurrentNode = CurrentNode->NextSibling;
            }

            game_state PotentialState = CurrentNode->State;
            InitializeSquares(MoveData.Squares, &PotentialState);
            DebugPrintSquares(MoveData.Squares);

            PreviousNode->NextSibling = 0;
            CurrentNode = AppState.GameTreeRoot;
        } while (CurrentNode->NextSibling);
    }

    return Result;
}

#undef u8
#undef u32
#undef b32
#undef s8
#undef s32
