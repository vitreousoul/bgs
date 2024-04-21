/*
    TODO: Convert this into an ".h" file and use it in "gui_app.c" or wherever.
    TODO: Namespace all names so that chess_bot can become a library.
*/
#include <stdio.h>
#include <stdint.h>

#include "./clibs/raylib.h"

#define u8   uint8_t
#define u32  uint32_t
#define b32  uint32_t
#define s8   int8_t
#define s32  int32_t

#define global_variable  static
#define internal         static

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))

const s32 SCREEN_WIDTH = 960;
const s32 SCREEN_HEIGHT = 540;
const s32 TARGET_FPS = 30;

#define BOARD_SIZE 8
#define BOARD_PADDING 40
const s32 SQUARE_SIZE_IN_PIXELS = (SCREEN_HEIGHT - (2 * BOARD_PADDING)) / 8;
#define BOARD_SIZE_IN_PIXELS (BOARD_SIZE * SQUARE_SIZE_IN_PIXELS)

const Color BACKGROUND_COLOR = {57,56,58,255};

const Color DARK_SQUARE_COLOR = {58,70,56,255};
const Color LIGHT_SQUARE_COLOR = {148,138,120,255};
const Color SELECTED_SQUARE_COLOR = {105,85,205,59};
const Color SELECTED_SQUARE_OUTLINE_COLOR = {100,20,100,255};

#define PIECE_TEXTURE_SIZE 120

char RANK_TABLE[8] = {'1','2','3','4','5','6','7','8'};
char FILE_TABLE[8] = {'a','b','c','d','e','f','g','h'};

typedef struct
{
    int X;
    int Y;
} ivec2;

typedef u8 piece;
typedef u8 square;

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

#define Black_Pawn_Offset    {4,0}
#define White_Pawn_Offset    {4,PIECE_TEXTURE_SIZE+8}
#define Black_Knight_Offset  {PIECE_TEXTURE_SIZE+12,0}
#define White_Knight_Offset  {PIECE_TEXTURE_SIZE+12,PIECE_TEXTURE_SIZE+8}
#define Black_Bishop_Offset  {2*PIECE_TEXTURE_SIZE+20,0}
#define White_Bishop_Offset  {2*PIECE_TEXTURE_SIZE+20,PIECE_TEXTURE_SIZE+8}
#define Black_Rook_Offset    {3*PIECE_TEXTURE_SIZE+28,0}
#define White_Rook_Offset    {3*PIECE_TEXTURE_SIZE+28,PIECE_TEXTURE_SIZE+8}
#define Black_Queen_Offset   {4*PIECE_TEXTURE_SIZE+36,0}
#define White_Queen_Offset   {4*PIECE_TEXTURE_SIZE+36,PIECE_TEXTURE_SIZE+8}
#define Black_King_Offset    {5*PIECE_TEXTURE_SIZE+44,0}
#define White_King_Offset    {5*PIECE_TEXTURE_SIZE+44,PIECE_TEXTURE_SIZE+8}

ivec2 PIECE_TEXTURE_OFFSET[piece_Count] = {
    [piece_White_Queen_Rook]    = White_Rook_Offset,
    [piece_White_Queen_Knight]  = White_Knight_Offset,
    [piece_White_Queen_Bishop]  = White_Bishop_Offset,
    [piece_White_Queen]         = White_Queen_Offset,
    [piece_White_King]          = White_King_Offset,
    [piece_White_King_Bishop]   = White_Bishop_Offset,
    [piece_White_King_Knight]   = White_Knight_Offset,
    [piece_White_King_Rook]     = White_Rook_Offset,
    [piece_White_Pawn_A]        = White_Pawn_Offset,
    [piece_White_Pawn_B]        = White_Pawn_Offset,
    [piece_White_Pawn_C]        = White_Pawn_Offset,
    [piece_White_Pawn_D]        = White_Pawn_Offset,
    [piece_White_Pawn_E]        = White_Pawn_Offset,
    [piece_White_Pawn_F]        = White_Pawn_Offset,
    [piece_White_Pawn_G]        = White_Pawn_Offset,
    [piece_White_Pawn_H]        = White_Pawn_Offset,

    [piece_Black_Queen_Rook]    = Black_Rook_Offset,
    [piece_Black_Queen_Knight]  = Black_Knight_Offset,
    [piece_Black_Queen_Bishop]  = Black_Bishop_Offset,
    [piece_Black_Queen]         = Black_Queen_Offset,
    [piece_Black_King]          = Black_King_Offset,
    [piece_Black_King_Bishop]   = Black_Bishop_Offset,
    [piece_Black_King_Knight]   = Black_Knight_Offset,
    [piece_Black_King_Rook]     = Black_Rook_Offset,
    [piece_Black_Pawn_A]        = Black_Pawn_Offset,
    [piece_Black_Pawn_B]        = Black_Pawn_Offset,
    [piece_Black_Pawn_C]        = Black_Pawn_Offset,
    [piece_Black_Pawn_D]        = Black_Pawn_Offset,
    [piece_Black_Pawn_E]        = Black_Pawn_Offset,
    [piece_Black_Pawn_F]        = Black_Pawn_Offset,
    [piece_Black_Pawn_G]        = Black_Pawn_Offset,
    [piece_Black_Pawn_H]        = Black_Pawn_Offset,
};

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

/* NOTE: Is_###_Piece assumes piece indexes are split by color,
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

#define Whose_Turn_Flag              (1 << 0)
#define White_Queen_Side_Castle_Flag (1 << 1)
#define White_King_Side_Castle_Flag  (1 << 2)
#define Black_Queen_Side_Castle_Flag (1 << 3)
#define Black_King_Side_Castle_Flag  (1 << 4)

#define Flag_Get(  flags, flag) (((flags) &  (flag)) != 0)
#define Flag_Set(  flags, flag)  ((flags) = (flags) & ~(flag) | flag)
#define Flag_Unset(flags, flag)  ((flags) = (flags) & ~(flag))
#define Flag_Toggle(flags, flag)  (Flag_Get((flags), (flag)) ? Flag_Unset((flags), (flag)) : Flag_Set((flags), (flag)))

#define Is_White_Turn(GameState) (Flag_Get(GameState->Flags, Whose_Turn_Flag) == 0)
#define Is_Black_Turn(GameState) (Flag_Get(GameState->Flags, Whose_Turn_Flag) != 0)

#define White_To_Move(GameState) Flag_Unset(GameState->Flags, Whose_Turn_Flag)
#define Black_To_Move(GameState) Flag_Set(GameState->Flags, Whose_Turn_Flag)

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

struct game_tree
{
    struct game_tree *NextSibling;
    struct game_tree *FirstChild;
    game_state State;
};

typedef struct game_tree game_tree;

typedef struct
{
    Vector2 MousePosition;
    int MousePrimaryDown;
    ivec2 HoverSquare;
    ivec2 SelectedSquare;
    ivec2 MoveSquare;
    Texture2D ChessPieceTexture;
} ui;

#define Game_Tree_Node_Pool_Size 2048

typedef struct
{
    game_tree *GameTreeRoot;
    game_tree *GameTreeCurrent;
    game_tree *FreeGameTree;
    ui Ui;
    s32 GameTreeNodePoolIndex;
    u8 Squares[64];
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

internal void RemovePiece(app_state *AppState, game_state *GameState, piece Piece)
{
    AppState->Squares[GameState->Piece[Piece]] = piece_Null;
    GameState->Piece[Piece] = square_Null;
}

internal void MovePieceToSquare(app_state *AppState, game_state *GameState, piece Piece, square Square)
{
    AppState->Squares[GameState->Piece[Piece]] = piece_Null;
    GameState->Piece[Piece] = Square;
    AppState->Squares[Square] = Piece;
}

internal void MakeMove(app_state *AppState, game_state *GameState, move Move)
{
    piece Piece = Move.Piece;
    b32 IsWhitePiece = Is_White_Piece(Piece);

    {
        b32 IsWhiteTurn = Is_White_Turn(GameState);
        b32 WhitePieceAndTurn = IsWhitePiece && IsWhiteTurn;
        b32 BlackPieceAndTurn = !(IsWhitePiece || IsWhiteTurn);
        Assert(WhitePieceAndTurn || BlackPieceAndTurn);
    }

    switch (Move.Type)
    {
    case move_type_EnPassant:
    {
        RemovePiece(AppState, GameState, GameState->LastMove.Piece);
    } /* Fall through */
    case move_type_Move:
    {
        MovePieceToSquare(AppState, GameState, Piece, Move.EndSquare);
    } break;
    /* TODO: Compress castling code below. */
    case move_type_QueenCastle:
    {
        if (IsWhitePiece)
        {
            MovePieceToSquare(AppState, GameState, piece_White_Queen_Rook, D1);
            MovePieceToSquare(AppState, GameState, piece_White_King, C1);
        }
        else
        {
            MovePieceToSquare(AppState, GameState, piece_Black_Queen_Rook, D8);
            MovePieceToSquare(AppState, GameState, piece_Black_King, C8);
        }
    } break;
    case move_type_KingCastle:
    {
        if (IsWhitePiece)
        {
            MovePieceToSquare(AppState, GameState, piece_White_King_Rook, F1);
            MovePieceToSquare(AppState, GameState, piece_White_King, G1);
        }
        else
        {
            MovePieceToSquare(AppState, GameState, piece_Black_King_Rook, F8);
            MovePieceToSquare(AppState, GameState, piece_Black_King, G8);
        }
    } break;
    default:
    {
        Assert(!"Unknown move type");
    }
    }

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

    Flag_Toggle(GameState->Flags, Whose_Turn_Flag);
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

internal void AddPotential(app_state *AppState, game_state *GameState, piece Piece, square EndSquare, move_type MoveType)
{
    {
        b32 IsCastleMove = MoveType == move_type_QueenCastle || MoveType == move_type_KingCastle;
        Assert(Is_Valid_Piece(Piece));
        Assert(IsCastleMove || Is_Valid_Square(EndSquare));
    }

    square BeginSquare = GameState->Piece[Piece];
    Assert(Is_Valid_Square(BeginSquare));

    move Move;
    Move.Type = MoveType;
    Move.Piece = Piece;
    Move.BeginSquare = BeginSquare;
    Move.EndSquare = EndSquare;

    game_state NewGameState;
    CopyGameState(GameState, &NewGameState);

    MakeMove(AppState, &NewGameState, Move);

    /* NOTE: Calling MakeMove mutates our global Squares with the new
       board state. Since we are adding potentials, we want to call
       InitializeSquares after doing MakeMove, to reset Squares to the
       current state. Basically, we recycle AppState and need to reset it
       after each MakeMove call we do.
    */
    InitializeSquares(AppState->Squares, GameState);

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
        Assert(!"TODO: Handle the case that we ran out of free game_trees\n");
    }
}

/* TODO: Maybe bundle Piece, Row, Col, ... into a struct. */
internal void Look(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 RowOffset, u8 ColOffset, u8 MaxLength)
{
    Assert(RowOffset || ColOffset);

    u8 PieceColor = Get_Piece_Color(Piece);
    u8 CurrentRow = Row + RowOffset;
    u8 CurrentCol = Col + ColOffset;
    u8 TotalLength = 1;

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
        s32 TargetPiece = AppState->Squares[NewSquare];

        if (Is_Valid_Piece(TargetPiece))
        {
            if (Get_Piece_Color(TargetPiece) != PieceColor)
            {
                /* NOTE: Add potential capture. */
                AddPotential(AppState, GameState, Piece, NewSquare, move_type_Move);
            }

            break;
        }
        else
        {
            /* NOTE: Add potential to an empty square. */
            AddPotential(AppState, GameState, Piece, NewSquare, move_type_Move);
        }

        CurrentRow += RowOffset;
        CurrentCol += ColOffset;
        TotalLength += 1;
    }
}

internal void LookRight(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 0, 1, MaxLength);
}

internal void LookUp(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 1, 0, MaxLength);
}

internal void LookLeft(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 0, -1, MaxLength);
}

internal void LookDown(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, -1, 0, MaxLength);
}

internal void LookUpRight(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 1, 1, MaxLength);
}

internal void LookUpLeft(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 1, -1, MaxLength);
}

internal void LookDownLeft(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, -1, -1, MaxLength);
}

internal void LookDownRight(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, -1, 1, MaxLength);
}

internal void LookAllDirections(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col, u8 MaxLength)
{
    LookRight(     AppState, GameState, Piece, Row, Col, MaxLength);
    LookUpRight(   AppState, GameState, Piece, Row, Col, MaxLength);
    LookUp(        AppState, GameState, Piece, Row, Col, MaxLength);
    LookUpLeft(    AppState, GameState, Piece, Row, Col, MaxLength);
    LookLeft(      AppState, GameState, Piece, Row, Col, MaxLength);
    LookDownLeft(  AppState, GameState, Piece, Row, Col, MaxLength);
    LookDown(      AppState, GameState, Piece, Row, Col, MaxLength);
    LookDownRight( AppState, GameState, Piece, Row, Col, MaxLength);
}

internal void LookPawn(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col)
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
        s32 TargetPiece = AppState->Squares[Square];

        if (!Is_Valid_Piece(TargetPiece))
        {
            AddPotential(AppState, GameState, Piece, Square, move_type_Move);
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
        s32 TargetPiece = AppState->Squares[Square];

        if (Is_Valid_Piece(TargetPiece) && Get_Piece_Color(TargetPiece) != PieceColor)
        {
            AddPotential(AppState, GameState, Piece, Square, move_type_Move);
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
                AddPotential(AppState, GameState, Piece, CaptureSquare, move_type_EnPassant);
            }
        }
    }
}

internal void LookKnight(app_state *AppState, game_state *GameState, piece Piece, u8 Row, u8 Col)
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
            s32 TargetPiece = AppState->Squares[NewSquare];

            if (Is_Valid_Piece(TargetPiece))
            {
                if (Get_Piece_Color(TargetPiece) != PieceColor)
                {
                    /* NOTE: Add potential capture. */
                    AddPotential(AppState, GameState, Piece, NewSquare, move_type_Move);
                }
            }
            else
            {
                /* NOTE: Add potential to an empty square. */
                AddPotential(AppState, GameState, Piece, NewSquare, move_type_Move);
            }
        }
    }
}

internal void LookCastle(app_state *AppState, game_state *GameState, piece Piece)
{
    u8 KingPosition;

    /* TODO: Compress the following code. */
    if (Is_White_Piece(Piece))
    {
        KingPosition = E1;
        Assert(GameState->Piece[Piece] == KingPosition);

        if (Flag_Get(GameState->Flags, White_Queen_Side_Castle_Flag))
        {
            Assert(GameState->Piece[piece_White_Queen_Rook] == A1);

            b32 B1Open = !Is_Valid_Piece(AppState->Squares[B1]);
            b32 C1Open = !Is_Valid_Piece(AppState->Squares[C1]);
            b32 D1Open = !Is_Valid_Piece(AppState->Squares[D1]);

            if (B1Open && C1Open && D1Open)
            {
                AddPotential(AppState, GameState, Piece, 255, move_type_QueenCastle);
            }
        }


        if (Flag_Get(GameState->Flags, White_King_Side_Castle_Flag))
        {
            Assert(GameState->Piece[piece_White_King_Rook] == H1);

            b32 F1Open = !Is_Valid_Piece(AppState->Squares[F1]);
            b32 G1Open = !Is_Valid_Piece(AppState->Squares[G1]);

            if (F1Open && G1Open)
            {
                AddPotential(AppState, GameState, Piece, 255, move_type_KingCastle);
            }
        }
    }
    else
    {
        KingPosition = E8;
        Assert(GameState->Piece[Piece] == KingPosition);
        if (Flag_Get(GameState->Flags, Black_Queen_Side_Castle_Flag))
        {
            Assert(GameState->Piece[piece_Black_Queen_Rook] == A8);

            b32 B8Open = !Is_Valid_Piece(AppState->Squares[B8]);
            b32 C8Open = !Is_Valid_Piece(AppState->Squares[C8]);
            b32 D8Open = !Is_Valid_Piece(AppState->Squares[D8]);

            if (B8Open && C8Open && D8Open)
            {
                AddPotential(AppState, GameState, Piece, 255, move_type_QueenCastle);
            }
        }


        if (Flag_Get(GameState->Flags, Black_King_Side_Castle_Flag))
        {
            Assert(GameState->Piece[piece_Black_King_Rook] == H8);

            b32 F8Open = !Is_Valid_Piece(AppState->Squares[F8]);
            b32 G8Open = !Is_Valid_Piece(AppState->Squares[G8]);

            if (F8Open && G8Open)
            {
                AddPotential(AppState, GameState, Piece, 255, move_type_KingCastle);
            }
        }
    }
}

internal void GeneratePotentials(app_state *AppState, game_state *GameState)
{
    s32 Start;

    if (Is_White_Turn(GameState))
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

        if (Is_Valid_Square(Square))
        {
            switch (PieceType[I])
            {
            case piece_type_Rook:
            {
                LookRight(AppState, GameState, Piece, Row, Col, 8);
                LookDown(AppState, GameState, Piece, Row, Col, 8);
                LookLeft(AppState, GameState, Piece, Row, Col, 8);
                LookUp(AppState, GameState, Piece, Row, Col, 8);
            } break;
            case piece_type_Knight:
            {
                LookKnight(AppState, GameState, Piece, Row, Col);
            } break;
            case piece_type_Bishop:
            {
                LookUpRight(AppState, GameState, Piece, Row, Col, 8);
                LookUpLeft(AppState, GameState, Piece, Row, Col, 8);
                LookDownLeft(AppState, GameState, Piece, Row, Col, 8);
                LookDownRight(AppState, GameState, Piece, Row, Col, 8);
            } break;
            case piece_type_Queen:
            {
                LookAllDirections(AppState, GameState, Piece, Row, Col, 8);
            } break;
            case piece_type_King:
            {
                LookAllDirections(AppState, GameState, Piece, Row, Col, 1);
                LookCastle(AppState, GameState, Piece);
            } break;
            case piece_type_Pawn:
            {
                LookPawn(AppState, GameState, Piece, Row, Col);
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

/* NOTE: This can be used to perform illegal moves for debugging/testing purposes. */
internal void DebugMovePiece(app_state *AppState, game_state *GameState, piece Piece, square Square)
{
    move Move = {0};

    Move.Type = move_type_Move;
    Move.Piece = Piece;
    Move.BeginSquare = GameState->Piece[Move.Piece];
    Move.EndSquare = Square;

    MakeMove(AppState, GameState, Move);
}

internal void SetupForTesting(app_state *AppState, game_state *GameState)
{
#if 0
    /* NOTE: Test un-passant for black. */
    Black_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_Black_Pawn_D, D3);

    White_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_White_Pawn_E, E4);

    GameState->Flags = Flag_Set(GameState->Flags, Whose_Turn_Flag);
#elif 0
    /* NOTE: Test castling for white. */
    White_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_White_Queen_Knight, E5);

    White_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_White_Queen_Bishop, F5);

    White_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_White_Queen, G5);

    White_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_White_King_Knight, E6);

    White_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_White_King_Bishop, F6);

    White_To_Move(GameState);
#elif 1
    /* NOTE: Test castling for black */
    Black_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_Black_Queen_Knight, E5);

    Black_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_Black_Queen_Bishop, F5);

    Black_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_Black_Queen, G5);

    Black_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_Black_King_Knight, E6);

    Black_To_Move(GameState);
    DebugMovePiece(AppState, GameState, piece_Black_King_Bishop, F6);

    Black_To_Move(GameState);
#else
    /* NOTE: Test un-passant for white. */
    DebugMovePiece(AppState, GameState, piece_White_Pawn_E, E6);
    DebugMovePiece(AppState, GameState, piece_Black_Pawn_D, D5);
#endif
}

static int PositionInsideBoard(Vector2 Position)
{
    return (Position.x >= 0.0f && Position.x <= BOARD_SIZE &&
            Position.y >= 0.0f && Position.y <= BOARD_SIZE);
}

static Vector2 PositionToSquarePosition(Vector2 Position)
{
    Vector2 Result;
    Vector2 OffsetPosition;
    OffsetPosition.x = (Position.x - BOARD_PADDING) / SQUARE_SIZE_IN_PIXELS;
    OffsetPosition.y = BOARD_SIZE - ((Position.y - BOARD_PADDING) / SQUARE_SIZE_IN_PIXELS);

    if(PositionInsideBoard(OffsetPosition))
    {
        Result = (Vector2){OffsetPosition.x, OffsetPosition.y};
    }
    else
    {
        Result = (Vector2){-1,-1};
    }

    return Result;
}

static int SquareValueOnBoard(int X, int Y)
{
    return X >= 0 && X < BOARD_SIZE && Y >= 0 && Y < BOARD_SIZE;
}

static int IVec2Equal(ivec2 A, ivec2 B)
{
    return A.X == B.X && A.Y == B.Y;
}

static void UpdateInput(app_state *AppState)
{
    ui *Ui = &AppState->Ui;
    Ui->MousePosition = GetMousePosition();
    Ui->MousePrimaryDown = IsMouseButtonPressed(0);

    Vector2 MouseSquarePosition = PositionToSquarePosition(Ui->MousePosition);
    Ui->HoverSquare = (ivec2){(int)MouseSquarePosition.x, (int)MouseSquarePosition.y};
    int OnBoard = SquareValueOnBoard(Ui->HoverSquare.X, Ui->HoverSquare.Y);

    if (Ui->MousePrimaryDown && OnBoard)
    {
        if (Ui->SelectedSquare.X == -1 && Ui->SelectedSquare.Y == -1)
        {
            Ui->SelectedSquare = (ivec2){Ui->HoverSquare.X, Ui->HoverSquare.Y};
        }
        else if (!IVec2Equal(Ui->SelectedSquare, Ui->HoverSquare))
        {
            Ui->MoveSquare = (ivec2){Ui->HoverSquare.X, Ui->HoverSquare.Y};
        }
    }
}

static void HandleMove(app_state *AppState)
{
    ui *Ui = &AppState->Ui;
    int HasSelectedSquare = Ui->SelectedSquare.X >= 0 && Ui->SelectedSquare.Y >= 0;
    int HasMoveSquare = Ui->MoveSquare.X >= 0 && Ui->MoveSquare.Y >= 0;

    if (HasSelectedSquare && HasMoveSquare)
    {
        /* TODO: Call MakeMove... */
        /* MakeMove(); */
        Ui->SelectedSquare = (ivec2){-1,-1};
        Ui->MoveSquare = (ivec2){-1,-1};
    }
}

static void DrawBoard(app_state *AppState)
{
    ui *Ui = &AppState->Ui;

    /* NOTE: Draw squares. */
    for (s32 Row = 0; Row < BOARD_SIZE; ++Row)
    {
        for (s32 Col = 0; Col < BOARD_SIZE; ++Col)
        {
            int X = (Col * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int Y = (((BOARD_SIZE - 1) - Row) * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int IsDarkSquare = (X % 2) != (Y % 2);
            Color SquareColor = IsDarkSquare ? DARK_SQUARE_COLOR : LIGHT_SQUARE_COLOR;

            DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColor);

            if (Ui->HoverSquare.X == Col && Ui->HoverSquare.Y == Row)
            {
                // draw outline of square if the mouse position is inside the square
                DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, BLACK);
            }

            if (Ui->SelectedSquare.X == Col && Ui->SelectedSquare.Y == Row)
            {
                DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SELECTED_SQUARE_COLOR);
                DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SELECTED_SQUARE_OUTLINE_COLOR);
            }
        }
    }

    /* NOTE: Draw pieces. */
    for (s32 I = 0; I < piece_Count; ++I)
    {
        if (!(AppState && AppState->GameTreeRoot))
        {
            /* NOTE: We only show AppState->GameTreeRoot for now, so break
               the loop if GameTreeRoot does _not_ exist.
            */
            break;
        }

        square Square = AppState->GameTreeRoot->State.Piece[I];

        if (Is_Valid_Square(Square) && IsTextureReady(Ui->ChessPieceTexture))
        {
            int Row = Square / 8;
            int Col = Square % 8;
            /* NOTE: @Copypasta */
            int X = (Col * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int Y = (((BOARD_SIZE - 1) - Row) * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;

            ivec2 PieceTextureOffset = PIECE_TEXTURE_OFFSET[I];
            Color Tint = {255,212,255,255};
            Vector2 Origin = {0,0};
            Rectangle Source, Dest;

            Source = (Rectangle){PieceTextureOffset.X, PieceTextureOffset.Y, PIECE_TEXTURE_SIZE, PIECE_TEXTURE_SIZE};
            Dest = (Rectangle){X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS};
            DrawTexturePro(Ui->ChessPieceTexture, Source, Dest, Origin, 0.0f, Tint);
        }
    }
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CHESS BOT");
    SetTargetFPS(TARGET_FPS);

    app_state AppState = {0};
    game_state GameState;
    game_tree GameTree = {0};

    InitializeGameState(&GameState);
    InitializeSquares(AppState.Squares, &GameState);

    SetupForTesting(&AppState, &GameState);

    GameTree.State = GameState;
    AppState.GameTreeRoot = &GameTree;

    AppState.Ui.ChessPieceTexture = LoadTexture("./assets/chess_pieces.png");

    if (!IsWindowReady())
    {
        printf("Error: Window not ready\n");
        return 1;
    }

    while (!WindowShouldClose())
    {
        UpdateInput(&AppState);
        HandleMove(&AppState);

        BeginDrawing();
        DrawBoard(&AppState);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

/* TODO: Remove these undefs, unless this file really will become a .h library... */
#undef u8
#undef u32
#undef b32
#undef s8
#undef s32
