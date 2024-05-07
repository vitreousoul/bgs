/*
    TODO: Create game_state valuing functions.
    TODO: Traverse game_tree and update each node's board-value. Each sibling adds to its parent's value, then the last sibling divides the parent-value by the sibling count. (Does this require a stack of sibling counts?)
    TODO: Make chess-bot into an engine that processes incrementally.

    TODO: Convert this into an ".h" file and use it in "gui_app.c" or wherever.
    TODO: Namespace all names so that chess_bot can become a library.
*/
#include <stdio.h>
#include <stdint.h>

#include "./clibs/raylib.h"

#define u8   uint8_t
#define u32  uint32_t
#define u64  uint64_t
#define b32  uint32_t
#define s8   int8_t
#define s32  int32_t

#define f32  float

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

typedef enum
{
    ui_color_theme_Light,
    ui_color_theme_Dark,
    ui_color_theme_Count,
} ui_color_theme;

typedef enum
{
    ui_color_Background,
    ui_color_Dark_Square,
    ui_color_Light_Square,
    ui_color_Selected_Square,
    ui_color_Selected_Square_Outline,
    ui_color_Active,
    ui_color_Inactive,
    ui_color_TextPrimary,
    ui_color_Count,
} ui_color_type;

global_variable Color UiColor[ui_color_theme_Count][ui_color_Count] = {
    [ui_color_theme_Light] = {
        [ui_color_Background] = (Color){190,190,210,255},
        [ui_color_Dark_Square] = (Color){88,100,86,255},
        [ui_color_Light_Square] = (Color){178,168,140,255},
        [ui_color_Selected_Square] = (Color){105,85,205,59},
        [ui_color_Selected_Square_Outline] = (Color){100,20,100,255},
        [ui_color_Active] = (Color){50,58,50,255},
        [ui_color_Inactive] = (Color){150,158,150,155},
        [ui_color_TextPrimary] = (Color){20,20,20,255},
    },
    [ui_color_theme_Dark] = {
        [ui_color_Background] = (Color){57,56,58,255},
        [ui_color_Dark_Square] = (Color){58,70,56,255},
        [ui_color_Light_Square] = (Color){148,138,120,255},
        [ui_color_Selected_Square] = (Color){105,85,205,59},
        [ui_color_Selected_Square_Outline] = (Color){100,20,100,255},
        [ui_color_Active] = (Color){250,255,250,255},
        [ui_color_Inactive] = (Color){150,158,150,155},
        [ui_color_TextPrimary] = (Color){220,220,220,255},
    }
};

#define DEFAULT_THEME ui_color_theme_Light

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

global_variable f32 PieceValueTable[piece_type_Count] = {
    [piece_type_Pawn]    = 1.0f,
    [piece_type_Knight]  = 3.0f,
    [piece_type_Bishop]  = 3.0f,
    [piece_type_Rook]    = 5.0f,
    [piece_type_Queen]   = 9.0f,
    [piece_type_King]    = 0.0f
};

/* NOTE: Is_###_Piece assumes piece indexes are split by color,
   so all we have to do is check the most significant bit.
*/
#define Get_Piece_Color(p) ((p) & (1 << 4))
#define Is_White_Piece(p) (Get_Piece_Color(p) == 0)
#define Is_Black_Piece(p) (Get_Piece_Color(p) != 0)

#define Is_Valid_Piece(p) ((p) >= (piece_White_Queen_Rook) && (p) < piece_Count)
#define Is_Valid_Square(s) ((s) >= 0 && (s) < 64)
#define Is_Valid_Row_Col(row, col) ((row) >= 0 && (row) < 8 && (col) >= 0 && (col) < 8)

/* #define Get_Square_Index(row, col) ((row) * 8 + (col)) */
internal s8 Get_Square_Index(s8 Row, s8 Col)
{
    if (Row < 0 || Row >= 8 || Col < 0 || Col >= 8)
    {
        return -1;
    }
    else
    {
        return Row * 8 + Col;
    }
}

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

global_variable u8 PieceTypeTable[piece_Count] = {
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

typedef struct
{
    f32 SquareBonus[64];
} evaluation;

typedef struct game_tree game_tree;

struct game_tree
{
    struct game_tree *PreviousSibling;
    struct game_tree *NextSibling;
    struct game_tree *Parent;
    struct game_tree *FirstChild;
    game_state State;
    f32 Score;
    f32 ChildScoreAverage;
};

typedef struct
{
    Vector2 MousePosition;
    int MousePrimaryDown;
    ivec2 HoverSquare;
    ivec2 SelectedSquare;
    ivec2 MoveSquare;
    Texture2D ChessPieceTexture;

    f32 GameTreeNodeSize;
    f32 GameTreeNodePadding;

    ui_color_theme Theme;
} ui;

#define Game_Tree_Node_Pool_Size 1000

typedef enum
{
    app_state_flags_GameTreeNodePoolIsFull,
    app_state_flags_Count,
} app_state_flags;

typedef struct
{
    b32 Visible;
    Vector2 Position;
} display_node;

typedef struct
{
    b32 Visited;
} traversal_node;

typedef struct
{
    game_tree GameTreeRoot;
    game_tree *GameTreeCurrent;
    game_tree *FreeGameTree;

    ui Ui;

    u8 Squares[64];
    game_tree GameTreeNodePool[Game_Tree_Node_Pool_Size];
    s32 GameTreeNodePoolIndex;

    display_node DisplayNodes[Game_Tree_Node_Pool_Size];
    Vector2 DisplayNodeCameraPosition;

    traversal_node TraversalNodes[Game_Tree_Node_Pool_Size];

    evaluation Evaluation;

    u32 Flags;
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

internal Vector2 AddV2(Vector2 A, Vector2 B)
{
    Vector2 Result = (Vector2){ A.x + B.x, A.y + B.y };

    return Result;
}

internal Vector2 SubtractV2(Vector2 A, Vector2 B)
{
    Vector2 Result = (Vector2){ A.x - B.x, A.y - B.y };

    return Result;
}

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
        Flag_Set(AppState->Flags, app_state_flags_GameTreeNodePoolIsFull);
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
    piece MaybeCapturedPiece = AppState->Squares[Square];

    /* NOTE: Capture. */
    if (Is_Valid_Piece(MaybeCapturedPiece))
    {
        GameState->Piece[MaybeCapturedPiece] = square_Null;
    }

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

internal f32 GetGameStateScore(app_state *AppState, game_state *GameState)
{
    f32 Score = 0.0f;

    f32 WhiteScore = 0.0f;
    f32 BlackScore = 0.0f;

    for (s32 I = 0; I < piece_Count; ++I)
    {
        square Square = GameState->Piece[I];
        u8 PieceType = PieceTypeTable[I];

        if (Is_Valid_Square(Square))
        {
            f32 PieceValue = PieceValueTable[PieceType];
            f32 Bonus = AppState->Evaluation.SquareBonus[Square];
            f32 PieceScore = PieceValue * Bonus;

            if (I < 16)
            {
                WhiteScore += PieceScore;
            }
            else
            {
                BlackScore += PieceScore;
            }
        }
    }

    Score = WhiteScore - BlackScore;

    return Score;
}

internal void AddPotential(app_state *AppState, game_state *GameState, piece Piece, square EndSquare, move_type MoveType)
{
    Assert(AppState->GameTreeCurrent != 0);
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

    if (GameTree)
    {
        GameTree->State = NewGameState;
        GameTree->NextSibling = AppState->GameTreeCurrent->FirstChild;

        if (GameTree->NextSibling)
        {
            GameTree->NextSibling->PreviousSibling = GameTree;
        }

        AppState->GameTreeCurrent->FirstChild = GameTree;
        GameTree->Parent = AppState->GameTreeCurrent;

        GameTree->Score = GetGameStateScore(AppState, &NewGameState);
    }
}

/* TODO: Maybe bundle Piece, Row, Col, ... into a struct. */
internal void Look(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, s8 RowOffset, s8 ColOffset, u8 MaxLength)
{
    Assert(RowOffset || ColOffset);

    u8 PieceColor = Get_Piece_Color(Piece);
    s8 CurrentRow = Row + RowOffset;
    s8 CurrentCol = Col + ColOffset;
    u8 TotalLength = 1;

    for (;;)
    {
        b32 PositionInBounds = (CurrentRow >= 0 && CurrentRow < 8 &&
                                CurrentCol >= 0 && CurrentCol < 8);
        b32 CanReach = TotalLength <= MaxLength;

        if (!(PositionInBounds && CanReach))
        {
            break;
        }

        s32 NewSquare = CurrentRow * 8 + CurrentCol;

        if (Is_Valid_Square(NewSquare) && Is_Valid_Piece(AppState->Squares[NewSquare]))
        {
            if (Get_Piece_Color(AppState->Squares[NewSquare]) != PieceColor)
            {
                AddPotential(AppState, GameState, Piece, NewSquare, move_type_Move);
            }

            break;
        }
        else
        {
            AddPotential(AppState, GameState, Piece, NewSquare, move_type_Move);
        }

        CurrentRow += RowOffset;
        CurrentCol += ColOffset;
        TotalLength += 1;
    }
}

internal void LookRight(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 0, 1, MaxLength);
}

internal void LookUp(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 1, 0, MaxLength);
}

internal void LookLeft(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 0, -1, MaxLength);
}

internal void LookDown(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, -1, 0, MaxLength);
}

internal void LookUpRight(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 1, 1, MaxLength);
}

internal void LookUpLeft(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, 1, -1, MaxLength);
}

internal void LookDownLeft(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, -1, -1, MaxLength);
}

internal void LookDownRight(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(AppState, GameState, Piece, Row, Col, -1, 1, MaxLength);
}

internal void LookAllDirections(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col, u8 MaxLength)
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

internal void LookPawn(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col)
{
    u8 PieceColor = Get_Piece_Color(Piece);
    s8 Multiplier = 1;
    s8 StartingRow = 1;
    s8 EnPassantRow = 5;

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
        s8 CurrentRow = Row + Offset;

        square Square = Get_Square_Index(CurrentRow, Col);

        if (Is_Valid_Square(Square) && !Is_Valid_Piece(AppState->Squares[Square]))
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
        s8 CurrentRow = Row + Offset;
        s8 CurrentCol = Col + I;

        square Square = Get_Square_Index(CurrentRow, CurrentCol);

        if (Is_Valid_Square(Square) &&
            Is_Valid_Piece(AppState->Squares[Square]) &&
            Get_Piece_Color(AppState->Squares[Square]) != PieceColor)
        {
            AddPotential(AppState, GameState, Piece, Square, move_type_Move);
        }
    }

    /* NOTE: En-passant */
    if (Row == EnPassantRow)
    {
        for (s32 I = -1; I < 2; I += 2)
        {
            s8 CurrentCol = Col + I;

            piece MovePieceType = PieceTypeTable[GameState->LastMove.Piece];

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

internal void LookKnight(app_state *AppState, game_state *GameState, piece Piece, s8 Row, s8 Col)
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

            if (Is_Valid_Square(NewSquare) && Is_Valid_Piece(AppState->Squares[NewSquare]))
            {
                if (Get_Piece_Color(AppState->Squares[NewSquare]) != PieceColor)
                {
                    AddPotential(AppState, GameState, Piece, NewSquare, move_type_Move);
                }
            }
            else
            {
                AddPotential(AppState, GameState, Piece, NewSquare, move_type_Move);
            }
        }
    }
}

internal void LookCastle(app_state *AppState, game_state *GameState, piece Piece)
{
    s8 KingPosition;

    /* TODO: Compress the following code. */
    if (Is_White_Piece(Piece))
    {
        KingPosition = E1;
        Assert(GameState->Piece[Piece] == KingPosition);

        if (Flag_Get(GameState->Flags, White_Queen_Side_Castle_Flag) &&
            GameState->Piece[piece_White_Queen_Rook] == A1)
        {
            b32 B1Open = !Is_Valid_Piece(AppState->Squares[B1]);
            b32 C1Open = !Is_Valid_Piece(AppState->Squares[C1]);
            b32 D1Open = !Is_Valid_Piece(AppState->Squares[D1]);

            if (B1Open && C1Open && D1Open)
            {
                AddPotential(AppState, GameState, Piece, 255, move_type_QueenCastle);
            }
        }


        if (Flag_Get(GameState->Flags, White_King_Side_Castle_Flag) &&
            GameState->Piece[piece_White_King_Rook] == H1)
        {
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
        if (Flag_Get(GameState->Flags, Black_Queen_Side_Castle_Flag) &&
            GameState->Piece[piece_Black_Queen_Rook] == A8)
        {
            b32 B8Open = !Is_Valid_Piece(AppState->Squares[B8]);
            b32 C8Open = !Is_Valid_Piece(AppState->Squares[C8]);
            b32 D8Open = !Is_Valid_Piece(AppState->Squares[D8]);

            if (B8Open && C8Open && D8Open)
            {
                AddPotential(AppState, GameState, Piece, 255, move_type_QueenCastle);
            }
        }


        if (Flag_Get(GameState->Flags, Black_King_Side_Castle_Flag) &&
            GameState->Piece[piece_Black_King_Rook] == H8)
        {
            b32 F8Open = !Is_Valid_Piece(AppState->Squares[F8]);
            b32 G8Open = !Is_Valid_Piece(AppState->Squares[G8]);

            if (F8Open && G8Open)
            {
                AddPotential(AppState, GameState, Piece, 255, move_type_KingCastle);
            }
        }
    }
}

/* TODO: Remove the GameState argument and just get the
   state from AppState->GameTreeCurrent.
*/
internal void GeneratePotentials(app_state *AppState, game_state *GameState)
{
    Assert(AppState->GameTreeCurrent != 0);
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
        if (Flag_Get(AppState->Flags, app_state_flags_GameTreeNodePoolIsFull))
        {
            /* NOTE: We check here if we have run out of nodes, becuase it is easier,
               but this means we sometimes try to call AddPotential mutliple times
               before reaching this point and breaking out.

               Maybe it's better to check after look calls below, but that would add
               a bunch of if's so maybes it's just cleaner to check after calling
               multiple Look*() functions.
            */
            printf("we ran out of nodes, breaking.....\n");
            break;
        }

        piece Piece = I;
        s8 Square = GameState->Piece[I];
        s8 Row = Square / 8;
        s8 Col = Square % 8;

        if (Is_Valid_Square(Square))
        {
            switch (PieceTypeTable[I])
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

internal s32 GetGameTreeIndexFromPointer(app_state *AppState, game_tree *GameTree)
{
    s32 Index = (s32)(GameTree - AppState->GameTreeNodePool);

    return Index;
}

/* @Copypasta */
internal game_tree *TraverseFirstChild(app_state *AppState, game_tree *GameTree, s32 *Index)
{
    game_tree *FirstChild = 0;
    *Index = GetGameTreeIndexFromPointer(AppState, GameTree->FirstChild);

    if (*Index >= 0 && *Index < Game_Tree_Node_Pool_Size)
    {
        traversal_node *TraversalNode = AppState->TraversalNodes + *Index;

        if (!TraversalNode->Visited)
        {
            FirstChild = AppState->GameTreeNodePool + *Index;
        }
    }

    return FirstChild;
}

/* @Copypasta */
internal game_tree *TraverseNextSibling(app_state *AppState, game_tree *GameTree, s32 *Index)
{
    game_tree *NextSibling = 0;
    *Index = GetGameTreeIndexFromPointer(AppState, GameTree->NextSibling);

    if (*Index >= 0 && *Index < Game_Tree_Node_Pool_Size)
    {
        NextSibling = AppState->GameTreeNodePool + *Index;
    }

    return NextSibling;
}

internal void GenerateAllPotentials(app_state *AppState)
{
    Assert(AppState->GameTreeCurrent != 0);
    Assert(AppState->GameTreeCurrent->FirstChild == 0);

    /* NOTE: Generate potentials. */
    game_tree *CurrentNode = AppState->GameTreeCurrent;
    while (CurrentNode)
    {
        if (CurrentNode->FirstChild == 0)
        {
            AppState->GameTreeCurrent = CurrentNode;
            InitializeSquares(AppState->Squares, &AppState->GameTreeCurrent->State);
            GeneratePotentials(AppState, &AppState->GameTreeCurrent->State);
        }
        else if (CurrentNode->NextSibling != 0)
        {
            CurrentNode = CurrentNode->NextSibling;
        }
        else
        {
            CurrentNode = CurrentNode->FirstChild;
        }

        if (AppState->GameTreeNodePoolIndex >= Game_Tree_Node_Pool_Size)
        {
            break;
        }
    }
    printf("potential count %d\n", AppState->GameTreeNodePoolIndex + 1);

    { /* NOTE: Update display nodes */
        Vector2 Position = (Vector2){0.0f, 0.0f};
        f32 NodeSizePlusPadding = AppState->Ui.GameTreeNodeSize + AppState->Ui.GameTreeNodePadding;
        CurrentNode = &AppState->GameTreeRoot;
        s32 DebugCount = 0;

        while (CurrentNode)
        {
            DebugCount++;
            s32 FirstChildIndex;
            s32 NextSiblingIndex;
            s32 CurrentNodeIndex = GetGameTreeIndexFromPointer(AppState, CurrentNode);

            game_tree *FirstChild = TraverseFirstChild(AppState, CurrentNode, &FirstChildIndex);
            game_tree *NextSibling = TraverseNextSibling(AppState, CurrentNode, &NextSiblingIndex);

            if (FirstChild)
            {
                CurrentNode = CurrentNode->FirstChild;
                Position.y += NodeSizePlusPadding;
                AppState->DisplayNodes[FirstChildIndex].Position = Position;
                AppState->DisplayNodes[FirstChildIndex].Visible = 1;

            }
            else if (NextSibling)
            {
                CurrentNode = CurrentNode->NextSibling;
                Position.x += NodeSizePlusPadding;
                AppState->DisplayNodes[NextSiblingIndex].Position = Position;
                AppState->DisplayNodes[NextSiblingIndex].Visible = 1;

                AppState->TraversalNodes[CurrentNodeIndex].Visited = 1;
            }
            else if (CurrentNode->Parent)
            {
                CurrentNode = CurrentNode->Parent;
                Position.x += NodeSizePlusPadding;
                Position.y -= NodeSizePlusPadding;

                AppState->TraversalNodes[CurrentNodeIndex].Visited = 1;
            }
            else
            {
                break;
            }
        }
        printf("display node count %d\n", DebugCount);
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


    { /* NOTE: Draw board backing. */
        int BorderWidth = 6;
        int X = BOARD_PADDING - BorderWidth;
        int Y = BOARD_PADDING - BorderWidth;
        int Width = (BOARD_SIZE * SQUARE_SIZE_IN_PIXELS) + (2 * BorderWidth);
        int Height = (BOARD_SIZE * SQUARE_SIZE_IN_PIXELS) + (2 * BorderWidth);

        ui_color_type ColorType = ui_color_Active;
        Color SquareColor = UiColor[AppState->Ui.Theme][ColorType];

        DrawRectangle(X, Y, Width, Height, SquareColor);
    }

    /* NOTE: Draw squares. */
    for (s32 Row = 0; Row < BOARD_SIZE; ++Row)
    {
        for (s32 Col = 0; Col < BOARD_SIZE; ++Col)
        {
            /* NOTE: @Copypasta */
            int X = (Col * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int Y = (((BOARD_SIZE - 1) - Row) * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int IsDarkSquare = (X % 2) != (Y % 2);
            ui_color_type SquareColorType = IsDarkSquare ? ui_color_Dark_Square : ui_color_Light_Square;
            Color SquareColor = UiColor[AppState->Ui.Theme][SquareColorType];

            DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColor);

            if (Ui->HoverSquare.X == Col && Ui->HoverSquare.Y == Row)
            {
                // draw outline of square if the mouse position is inside the square
                DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, BLACK);
            }

            if (Ui->SelectedSquare.X == Col && Ui->SelectedSquare.Y == Row)
            {
                Color SquareColor = UiColor[AppState->Ui.Theme][ui_color_Selected_Square];
                Color SquareColorOutline = UiColor[AppState->Ui.Theme][ui_color_Selected_Square_Outline];
                DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColor);
                DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColorOutline);
            }
        }
    }

    /* NOTE: Draw pieces. */
    for (s32 I = 0; I < piece_Count; ++I)
    {
        square Square = AppState->GameTreeCurrent->State.Piece[I];

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

    { /* NOTE: Draw score. */
        char Buff[128];
        sprintf(Buff, "%.4f", AppState->GameTreeCurrent->Score);
        Color Color = UiColor[AppState->Ui.Theme][ui_color_TextPrimary];

        DrawText(Buff, 4.0f, 4.0f, 16.0f, Color);
    }

#if 0
    {
        /* NOTE: Draw familial options for current game-tree. */
        b32 HasNextSibling = AppState->GameTreeCurrent->NextSibling != 0;
        b32 HasPreviousSibling = AppState->GameTreeCurrent->PreviousSibling != 0;
        b32 HasFirstChild = AppState->GameTreeCurrent->FirstChild != 0;
        b32 HasParent = AppState->GameTreeCurrent->Parent != 0;

#define Get_Color(x) UiColor[AppState->Ui.Theme][x ? ui_color_Active : ui_color_Inactive]
        Color HasNextSiblingColor = Get_Color(HasNextSibling);
        Color HasPreviousSiblingColor = Get_Color(HasPreviousSibling);
        Color HasFirstChildColor = Get_Color(HasFirstChild);
        Color HasParentColor = Get_Color(HasParent);
#undef Get_Color

        DrawCircle(SCREEN_WIDTH - 40, 60, 13.0f, HasNextSiblingColor);
        DrawCircle(SCREEN_WIDTH - 80, 60, 13.0f, HasPreviousSiblingColor);
        DrawCircle(SCREEN_WIDTH - 60, 80, 13.0f, HasFirstChildColor);
        DrawCircle(SCREEN_WIDTH - 60, 40, 13.0f, HasParentColor);
    }
#endif
}

internal void DrawGameTree(app_state *AppState)
{
    Vector2 CurrentPosition = {0};
    s32 Index = GetGameTreeIndexFromPointer(AppState, AppState->GameTreeCurrent);
    f32 Size = AppState->Ui.GameTreeNodeSize;

    if (Index >= 0 && Index < Game_Tree_Node_Pool_Size)
    {
        CurrentPosition = AppState->DisplayNodes[Index].Position;
    }

    { /* NOTE: Always draw the root for now... */
        /* @Copypasta */
        Vector2 DeltaPosition = SubtractV2((Vector2){0.0f, 0.0f}, CurrentPosition);
        Vector2 Position = AddV2(AppState->DisplayNodeCameraPosition, DeltaPosition);
        DrawCircle(Position.x, Position.y, Size, (Color){20, 70, 20, 180});
    }

    for (s32 I = 0; I < Game_Tree_Node_Pool_Size; ++I)
    {
        display_node DisplayNode = AppState->DisplayNodes[I];

        if (DisplayNode.Visible)
        {
            /* @Copypasta */
            Vector2 DeltaPosition = SubtractV2(DisplayNode.Position, CurrentPosition);
            Vector2 Position = AddV2(AppState->DisplayNodeCameraPosition, DeltaPosition);
            b32 PositionIsInBounds = (Position.x >= 0.0f && Position.x < SCREEN_WIDTH &&
                                      Position.y >= 0.0f && Position.y < SCREEN_HEIGHT);

            if (PositionIsInBounds)
            {
                b32 IsCurrentNode = (AppState->GameTreeNodePool + I) == AppState->GameTreeCurrent;
                Color NodeColor = IsCurrentNode ? (Color){20, 20, 190, 200} : (Color){20, 20, 20, 160};

                DrawCircle(Position.x, Position.y, Size, NodeColor);
            }
        }
    }
}

internal void InitializeEvaluation(app_state *AppState)
{
    f32 CenterRings[4] = {0.9f, 1.0f, 1.1f, 1.2f};

    /* NOTE: Fill out square bonuses based on distance from center. Closer to center means better. */
    for (s32 I = 0; I < 4; ++I)
    {
        s32 Length = 7 - 2*I;
        f32 Bonus = CenterRings[I];

        for (s32 J = 0; J < Length; ++J)
        {
            square SquareA = Get_Square_Index(I,       I+J);
            square SquareB = Get_Square_Index(I+J,     7-I);
            square SquareC = Get_Square_Index(7-I,     7-(I+J));
            square SquareD = Get_Square_Index(7-(I+J), I);

            AppState->Evaluation.SquareBonus[SquareA] = Bonus;
            AppState->Evaluation.SquareBonus[SquareB] = Bonus;
            AppState->Evaluation.SquareBonus[SquareC] = Bonus;
            AppState->Evaluation.SquareBonus[SquareD] = Bonus;
        }
    }

#if 0
    { /* NOTE: Debug viz. */
        for (s32 Row = 0; Row < 8; ++Row)
        {
            for (s32 Col = 0; Col < 8; ++Col)
            {
                printf("%.1f ", AppState->Evaluation.SquareBonus[Row * 8 + Col]);
            }
            printf("\n");
        }
    }
#endif
}

int main(void)
{
    Assert(app_state_flags_Count < 32); /* NOTE: Assume the app-state flags value is 32-bit. */

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CHESS BOT");
    SetTargetFPS(TARGET_FPS);

    app_state AppState = {0};

    InitializeGameState(&AppState.GameTreeRoot.State);
    InitializeSquares(AppState.Squares, &AppState.GameTreeRoot.State);
    InitializeEvaluation(&AppState);

#if 0
    SetupForTesting(&AppState, &AppState.GameTreeRoot.State);
#endif

    AppState.Ui.ChessPieceTexture = LoadTexture("./assets/chess_pieces.png");
    AppState.Ui.Theme = DEFAULT_THEME;
    AppState.Ui.GameTreeNodeSize = 8.0f;
    AppState.Ui.GameTreeNodePadding = 5.0f;

    AppState.GameTreeCurrent = &AppState.GameTreeRoot;
    GenerateAllPotentials(&AppState);
    AppState.GameTreeCurrent = &AppState.GameTreeRoot;

    AppState.DisplayNodeCameraPosition = (Vector2){SCREEN_HEIGHT + 30.0f, 30.0f};

    if (!IsWindowReady())
    {
        printf("Error: Window not ready\n");
        return 1;
    }

    while (!WindowShouldClose())
    {
        UpdateInput(&AppState);
        HandleMove(&AppState);

        if (IsKeyPressed(KEY_RIGHT))
        {
            if (AppState.GameTreeCurrent->NextSibling)
            {
                AppState.GameTreeCurrent = AppState.GameTreeCurrent->NextSibling;
            }
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            if (AppState.GameTreeCurrent->PreviousSibling)
            {
                AppState.GameTreeCurrent = AppState.GameTreeCurrent->PreviousSibling;
            }
        }
        else if (IsKeyPressed(KEY_DOWN))
        {
            if (AppState.GameTreeCurrent->FirstChild)
            {
                AppState.GameTreeCurrent = AppState.GameTreeCurrent->FirstChild;
            }
        }
        else if (IsKeyPressed(KEY_UP))
        {
            if (AppState.GameTreeCurrent->Parent)
            {
                AppState.GameTreeCurrent = AppState.GameTreeCurrent->Parent;
            }
        }

        BeginDrawing();
        ClearBackground(UiColor[AppState.Ui.Theme][ui_color_Background]);
        DrawBoard(&AppState);
        DrawGameTree(&AppState);
        EndDrawing();
    }

    CloseWindow();

    printf("sizeof(app_state) %lu\n", sizeof(app_state));
    printf("sizeof(game_tree) %lu\n", sizeof(game_tree));

    return 0;
}

/* TODO: Remove these undefs, unless this file really will become a .h library... */
#undef u8
#undef u32
#undef b32
#undef s8
#undef s32
