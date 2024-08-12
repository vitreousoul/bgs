/*
    BUGS:
        TODO: A promoted pawn continues to allow promotion when on the back row, we need to check pawn-promotion of pawns before allowing more promotion moves.
        TODO: The engine does not allow castling (or any kind of moving) out of check, we want to be able to do that!
        TODO: (^----Duplicate TODO?) Sometimes the player is unable to move when in check, even though potential moves exist. Probably an issue with HandleMove or something like that.
        TODO: Sometimes the game thinks a player is in check-mate, when really they are only in check. Must (maybe) be something wrong with the new checkmate code in/around LookDispatch.

    Engine Functionality:
        TODO: The Save/Load functionality is not very useful. It would be better if we had simple UI for storing various games.

    GUI:
        ...

    Dev Features:
        TODO: Having a scrubbable history of frames would help debugging frame rate spikes.

    Code Quality:
        TODO: @Outdated? AddPotential and HandleMove have (or at least _used_ to have) a lot of similarities and have been a source of implementation pains. Maybe we should put these two functions side-by-side and see if they can be combined.
*/
#include <stdio.h>
#include <stdint.h>

#include "./clibs/raylib.h"

#include "./clibs/ryn_prof.h"
#include "./clibs/ryn_memory.h"
#include "./clibs/ryn_python.h"

typedef enum
{
    timed_block_Main,
    timed_block_DrawBoard,
    timed_block_DrawGameTree,
    timed_block_GenerateAllPotentials,
    timed_block_GenerateAllPotentialsGen,
    timed_block_GenerateAllPotentialsFirstChild,
    timed_block_GenerateAllPotentialsNextSibling,
    timed_block_GenerateAllPotentialsParent,
    timed_block_IncrementallySortGameTree,
    timed_block_IncrementallySortGameTreeProbe,
    timed_block_UpdateDisplayNodes,
    timed_block_HandleInputAndMove,
    timed_block_BeginAndClear,
    timed_block_PythonRoundTrip,
} timed_block_kind;

#define u8   uint8_t
#define u32  uint32_t
#define u64  uint64_t
#define b32  uint32_t
#define s8   int8_t
#define s32  int32_t

#define f32  float

#define global_variable  static
#define debug_variable  static
#define internal         static

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))

const s32 SCREEN_WIDTH = 960;
const s32 SCREEN_HEIGHT = 540;
const s32 TARGET_FPS = 30;
#define Panel_Position_X (SCREEN_WIDTH - 200)

#define BOARD_SIZE 8
#define BOARD_PADDING 48
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
        [ui_color_Inactive] = (Color){120,128,120,220},
        [ui_color_TextPrimary] = (Color){20,20,20,255},
    },
    [ui_color_theme_Dark] = {
        [ui_color_Background] = (Color){57,56,58,255},
        [ui_color_Dark_Square] = (Color){58,70,56,255},
        [ui_color_Light_Square] = (Color){148,138,120,255},
        [ui_color_Selected_Square] = (Color){105,85,205,59},
        [ui_color_Selected_Square_Outline] = (Color){100,20,100,255},
        [ui_color_Active] = (Color){250,255,250,255},
        [ui_color_Inactive] = (Color){120,128,120,220},
        [ui_color_TextPrimary] = (Color){220,220,220,255},
    }
};

#define DEFAULT_THEME ui_color_theme_Light

#define PIECE_TEXTURE_SIZE 120

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

global_variable char PieceCharTable[piece_Count] = {
    [piece_White_Queen_Rook] = 'R',
    [piece_White_Queen_Knight] = 'N',
    [piece_White_Queen_Bishop] = 'B',
    [piece_White_Queen] = 'Q',
    [piece_White_King] = 'K',
    [piece_White_King_Bishop] = 'B',
    [piece_White_King_Knight] = 'N',
    [piece_White_King_Rook] = 'R',
    [piece_White_Pawn_A] = 'P',
    [piece_White_Pawn_B] = 'P',
    [piece_White_Pawn_C] = 'P',
    [piece_White_Pawn_D] = 'P',
    [piece_White_Pawn_E] = 'P',
    [piece_White_Pawn_F] = 'P',
    [piece_White_Pawn_G] = 'P',
    [piece_White_Pawn_H] = 'P',
    [piece_Black_Pawn_A] = 'p',
    [piece_Black_Pawn_B] = 'p',
    [piece_Black_Pawn_C] = 'p',
    [piece_Black_Pawn_D] = 'p',
    [piece_Black_Pawn_E] = 'p',
    [piece_Black_Pawn_F] = 'p',
    [piece_Black_Pawn_G] = 'p',
    [piece_Black_Pawn_H] = 'p',
    [piece_Black_Queen_Rook] = 'r',
    [piece_Black_Queen_Knight] = 'n',
    [piece_Black_Queen_Bishop] = 'b',
    [piece_Black_Queen] = 'q',
    [piece_Black_King] = 'k',
    [piece_Black_King_Bishop] = 'b',
    [piece_Black_King_Knight] = 'n',
    [piece_Black_King_Rook] = 'r',
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
#define Get_Piece_Color(p) (((p) & (1 << 4)) != 0)
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

global_variable char SquareNames[64][2] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

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
    [piece_Black_Pawn_H]        = piece_type_Pawn
};

#define Get_Flag(  flags, flag) (((flags) &  (flag)) != 0)
#define Set_Flag(  flags, flag)  ((flags) = (flags) | flag)
#define Unset_Flag(flags, flag)  ((flags) = (flags) & ~(flag))
#define Toggle_Flag(flags, flag)  (Get_Flag((flags), (flag)) ? Unset_Flag((flags), (flag)) : Set_Flag((flags), (flag)))

#define Is_White_Turn(GameState) (Get_Flag((GameState)->Flags, Whose_Turn_Flag) == 0)
#define Is_Black_Turn(GameState) (Get_Flag((GameState)->Flags, Whose_Turn_Flag) != 0)

#define White_To_Move(GameState) Unset_Flag(GameState->Flags, Whose_Turn_Flag)
#define Black_To_Move(GameState) Set_Flag(GameState->Flags, Whose_Turn_Flag)

#define Whose_Turn_Char(GameState) (Is_White_Turn(GameState) ? 'w' : 'b')

typedef enum
{
    move_type_Null,
    move_type_Move,
    move_type_QueenCastle,
    move_type_KingCastle,
    move_type_EnPassant,
    move_type_PromoteQueen,
    move_type_PromoteRook,
    move_type_PromoteBishop,
    move_type_PromoteKnight
} move_type;

global_variable piece PawnPromotionPieceLookup[4][2] = {
    {piece_White_Queen,        piece_Black_Queen},
    {piece_White_Queen_Rook,   piece_Black_Queen_Rook},
    {piece_White_Queen_Bishop, piece_Black_Queen_Bishop},
    {piece_White_Queen_Knight, piece_Black_Queen_Knight}
};

global_variable move_type PawnPromotionMoveTypeLookup[4] = {
    move_type_PromoteQueen,
    move_type_PromoteRook,
    move_type_PromoteBishop,
    move_type_PromoteKnight
};

typedef struct
{
    move_type Type;
    piece Piece;
    square BeginSquare;
    square EndSquare;
} move;

typedef enum
{
    Whose_Turn_Flag              = 0x1,

    White_Queen_Side_Castle_Flag = 0x2,
    White_King_Side_Castle_Flag  = 0x4,
    Black_Queen_Side_Castle_Flag = 0x8,
    Black_King_Side_Castle_Flag  = 0x10,

    White_In_Check_Flag          = 0x20,
    Black_In_Check_Flag          = 0x40,

    Game_Over_Flag               = 0x80,
} game_state_flag;

typedef enum
{
    pawn_promotion_type_Null,
    pawn_promotion_type_Queen,
    pawn_promotion_type_Rook,
    pawn_promotion_type_Bishop,
    pawn_promotion_type_Knight,
    pawn_promotion_type_Count
} pawn_promotion_type;

typedef struct
{
    b32 Flags;
    move LastMove;
    u8 Piece[piece_Count];
    u8 PawnPromotion[piece_Count];

    piece DebugCheckPiece;
} game_state;

typedef struct
{
    f32 SquareBonus[64];
    s32 AttackersAndDefenders[piece_Count];

    b32 ShouldBreakLookLoop;
} evaluation;

typedef struct game_tree game_tree;

struct game_tree
{
    game_tree *PreviousSibling;
    game_tree *NextSibling;
    game_tree *Parent;
    game_tree *FirstChild;

    game_state State;
    f32 Score;
    f32 FollowingScore;
};

typedef struct
{
    Vector2 MousePosition;
    int MousePrimaryDown;

    ivec2 HoverSquare; /* TODO: HoverSquare should eventually be deleted once we have a concept of hot/active in the UI. */
    ivec2 SelectedSquare; /* TODO: SelectedSquare should eventually be deleted once we have a concept of hot/active in the UI. */

    ivec2 PawnPromotionHoverSquare;
    ivec2 PawnPromotionSelectedSquare;

    ivec2 MoveSquare;
    Texture2D ChessPieceTexture;

    f32 GameTreeNodeSize;
    f32 GameTreeNodePadding;

    ui_color_theme Theme;
} ui;

#define Game_Tree_Pool_Size (3*1024*1024) /* TODO: Use index types for indexing arrays of Game_Tree_Pool_Size */
#define Traversal_Chunk_Bits 32
#define Traversal_Chunk_Count (Game_Tree_Pool_Size/Traversal_Chunk_Bits)

#define Is_Game_Tree_Index_In_Range(i) ((i) >= 0 && (i) < Game_Tree_Pool_Size)

typedef enum
{
    engine_flags_GameTreeNodePoolIsFull = 0x1,
    engine_flags_IsBotPlayingAsWhite = 0x2,
    engine_flags_DoNotLetTheBotMakeAMove = 0x4,
} engine_flags;

typedef enum
{
    user_input_mode_Null,
    user_input_mode_TheUsersTurn,
    user_input_mode_TheBotsTurn,
    user_input_mode_PawnPromotion,
    user_input_mode_Count
} user_input_mode;

typedef struct
{
    b32 Visible;
    Vector2 Position;
} display_node;

typedef b32 traversal_node;

typedef struct
{
    /* TODO: Shorten these names? */
    s32 GameTreeIndex;
} traversal_result;

typedef struct
{
    b32 StillTraversing;
    s32 Depth;
    s32 MaxDepth;
    game_tree *CurrentNode;
    traversal_node Nodes[Traversal_Chunk_Count];
} traversal;

#define Move_History_Size 256
typedef struct
{
    s32 Index;
    move Moves[Move_History_Size];
} move_history;

typedef enum
{
    look_type_NULL,
    look_type_AddPotential,
    look_type_AttackersAndDefenders,
    look_type_Checkmate,
} look_type;

#define Max_Fen_Length 128
typedef struct
{
    char String[Max_Fen_Length];
    s32 Length;
} fen;

typedef enum
{
    bot_state_NULL,
    bot_state_WaitingForGenerationRoot,
    bot_state_WaitingForSorting,
    bot_state_ReadyToMove,
} bot_state;

typedef struct
{
    game_tree *RootGameTree;
    game_tree *GameTreeCurrent;
    game_tree *FreeGameTree;

    move_history MoveHistory;

    ui Ui;
    user_input_mode UserInputMode;
    move CurrentMove;

    u8 Squares[64];
    ryn_memory_arena GameTreeArena;
    s32 DrawGameTreeIndex;

    s32 GeneratedStateCount;

    display_node DisplayNodes[Game_Tree_Pool_Size];
    Vector2 DisplayNodeCameraPosition;

    traversal GenerationTraversal;
    traversal SortTraversal;
    traversal UpdateTraversal;

    Vector2 UpdateCurrentPosition;
    traversal_node DebugTraversalNodes[Traversal_Chunk_Count]; /* TODO: Turn this from a traversal_node into a traversal. */

    evaluation Evaluation;

    u32 Flags;

    bot_state BotState;
    u64 SortCount;
    s32 TemporaryTickCount;
} engine;

#define DebugPrint printf

#define Has_Free_Game_Tree(Engine)                                 \
    (((Engine)->FreeGameTree != 0) ||                               \
    ((Engine)->GameTreeArena.Offset < (Engine)->GameTreeArena.Capacity))

#if DEBUG
#define NotImplemented Assert(0)
#define Assert(p) Assert_(p, __FILE__, __LINE__)
internal void Assert_(b32 Proposition, char *FilePath, s32 LineNumber)
{
    if (!Proposition)
    {
        b32 *NullPtr = 0;
        DebugPrint("Assertion failed on line %d in %s\n", LineNumber, FilePath);
        /* NOTE: Dereference a null-pointer and crash the program. */
        Proposition = *NullPtr;
    }
}
#else
#define Assert(p)
#define NotImplemented
#endif

#define V2(x, y) ((Vector2){(x), (y)})

internal s32 MaxS32(s32 A, s32 B)
{
    s32 Result = A < B ? B : A;
    return Result;
}

internal f32 AbsoluteValueF32(f32 Value)
{
    f32 Result = Value < 0.0f ? -1.0f * Value : Value;
    return Result;
}

internal Vector2 AddV2(Vector2 A, Vector2 B)
{
    Vector2 Result = V2( A.x + B.x, A.y + B.y );

    return Result;
}

internal Vector2 SubtractV2(Vector2 A, Vector2 B)
{
    Vector2 Result = V2( A.x - B.x, A.y - B.y );

    return Result;
}

internal Vector2 MultiplySV2(f32 S, Vector2 V)
{
    Vector2 Result = V2( S * V.x, S * V.y );

    return Result;
}

internal f32 DotV2(Vector2 A, Vector2 B)
{
    f32 Result = A.x*B.x + A.y*B.y;
    return Result;
}

internal f32 ClampF32(f32 Value, f32 Min, f32 Max)
{
    f32 Result = Value;

    if (Value < Min)
    {
        Result = Min;
    }
    else if (Value > Max)
    {
        Result = Max;
    }

    return Result;
}

internal void ZeroGameState(game_state *State)
{
    State->Flags = 0;

    State->LastMove.Type = 0;
    State->LastMove.Piece = 0;
    State->LastMove.BeginSquare = 0;
    State->LastMove.EndSquare = 0;

    for (s32 I = 0; I < piece_Count; ++I)
    {
        State->Piece[I] = 0;
        State->PawnPromotion[I] = 0;
    }
}

internal void CopyGameState(game_state *Source, game_state *Destination)
{
    Destination->Flags = Source->Flags;
    Destination->LastMove = Source->LastMove;

    for (s32 I = 0; I < piece_Count; ++I)
    {
        Destination->Piece[I] = Source->Piece[I];
        Destination->PawnPromotion[I] = Source->PawnPromotion[I];
    }
}

internal piece_type PromotePieceIfPossible(game_state *GameState, piece Piece)
{
    piece_type PieceType = PieceTypeTable[Piece];

    if (PieceType == piece_type_Pawn)
    {
        pawn_promotion_type PromotionType = GameState->PawnPromotion[Piece];

        switch (PromotionType)
        {
        case pawn_promotion_type_Queen:  PieceType = piece_type_Queen;  break;
        case pawn_promotion_type_Rook:   PieceType = piece_type_Rook;   break;
        case pawn_promotion_type_Bishop: PieceType = piece_type_Bishop; break;
        case pawn_promotion_type_Knight: PieceType = piece_type_Knight; break;
        default: break;
        }
    }

    return PieceType;
}

#define Digit_To_Ascii(d) ((d) + 48)
#define Write_Fen_Char(f, fi, c) ((f).String[fi++] = (c))

/* TODO: Add half-move counter and full-move number. */
internal fen GameStateToFen(game_state *GameState)
{
    fen Fen = {0};
    s32 FenIndex = 0;
    square Squares[64];

    for (s32 I = 0; I < 64; ++I)
    {
        Squares[I] = square_Null;
    }

    for (s32 PieceIndex = 0; PieceIndex < piece_Count; ++PieceIndex)
    {
        square Square = GameState->Piece[PieceIndex];

        if (Is_Valid_Square(Square))
        {
            Squares[Square] = PieceIndex;
        }
    }

    for (s32 Row = 7; Row >= 0; --Row)
    {
        s32 EmptySquareCount = 0;

        for (s32 Col = 0; Col < 8; ++Col)
        {
            square Square = Row*8 + Col;
            piece Piece = Squares[Square];

            if (Is_Valid_Piece(Piece))
            {
                if (EmptySquareCount > 0)
                {
                    Write_Fen_Char(Fen, FenIndex, Digit_To_Ascii(EmptySquareCount));
                    EmptySquareCount = 0;
                }

                Write_Fen_Char(Fen, FenIndex, PieceCharTable[Piece]);
            }
            else
            {
                EmptySquareCount += 1;
            }
        }

        if (EmptySquareCount > 0)
        {
            Write_Fen_Char(Fen, FenIndex, Digit_To_Ascii(EmptySquareCount));
        }

        if (Row > 0)
        {
            Write_Fen_Char(Fen, FenIndex, '/');
        }
    }

    Write_Fen_Char(Fen, FenIndex, ' ');
    Write_Fen_Char(Fen, FenIndex, Whose_Turn_Char(GameState));
    Write_Fen_Char(Fen, FenIndex, ' ');

    {
        b32 AtLeastOneAvailableCastle = 0;

        if (Get_Flag(GameState->Flags, White_King_Side_Castle_Flag))
        {
            Write_Fen_Char(Fen, FenIndex, 'K');
            AtLeastOneAvailableCastle = 1;
        }

        if (Get_Flag(GameState->Flags, White_Queen_Side_Castle_Flag))
        {
            Write_Fen_Char(Fen, FenIndex, 'Q');
            AtLeastOneAvailableCastle = 1;
        }

        if (Get_Flag(GameState->Flags, Black_King_Side_Castle_Flag))
        {
            Write_Fen_Char(Fen, FenIndex, 'k');
            AtLeastOneAvailableCastle = 1;
        }

        if (Get_Flag(GameState->Flags, Black_Queen_Side_Castle_Flag))
        {
            Write_Fen_Char(Fen, FenIndex, 'q');
            AtLeastOneAvailableCastle = 1;
        }

        if (!AtLeastOneAvailableCastle)
        {
            Write_Fen_Char(Fen, FenIndex, '-');
        }

        Write_Fen_Char(Fen, FenIndex, ' ');
    }

    {
        /*
          move_type Type;
          piece Piece;
          square BeginSquare;
          square EndSquare;
        */
        b32 IsWhiteTurn = Is_White_Turn(GameState);
        piece_type LastPieceType = PromotePieceIfPossible(GameState, GameState->LastMove.Piece);
        square BeginSquare = GameState->LastMove.BeginSquare;
        square EndSquare = GameState->LastMove.EndSquare;
        /* piece_type PieceType = PieceTypeTable[LastPiece]; */

        if (LastPieceType == piece_type_Pawn)
        {
            s32 RangeStart;
            s32 Multiplier;
            /* s32 SquareDifference = GameState.LastMove */

            if (IsWhiteTurn)
            {
                RangeStart = 48;
                Multiplier = -1;
            }
            else
            {
                RangeStart = 8;
                Multiplier = 1;
            }

            s32 RangeEnd = RangeStart + 8;

            b32 IsPawnsFirstMove = BeginSquare >= RangeStart && BeginSquare < RangeEnd;
            b32 IsATwoSquareMove = (EndSquare - (BeginSquare + Multiplier*16)) == 0;

            if (IsPawnsFirstMove && IsATwoSquareMove)
            {
                square PassThroughSquare = BeginSquare + Multiplier*8;
                Write_Fen_Char(Fen, FenIndex, SquareNames[PassThroughSquare][0]);
                Write_Fen_Char(Fen, FenIndex, SquareNames[PassThroughSquare][1]);
                Write_Fen_Char(Fen, FenIndex, ' ');
            }
            else
            {
                Write_Fen_Char(Fen, FenIndex, '-');
            }
        }
        else
        {
            Write_Fen_Char(Fen, FenIndex, '-');
        }
    }

    Fen.Length = FenIndex + 1;
    Assert(Fen.Length < Max_Fen_Length);

    return Fen;
}

internal void RemovePiece(engine *Engine, game_state *GameState, piece Piece)
{
    Engine->Squares[GameState->Piece[Piece]] = piece_Null;
    GameState->Piece[Piece] = square_Null;
}

internal void MovePieceToSquare(engine *Engine, game_state *GameState, piece Piece, square Square)
{
    piece MaybeCapturedPiece = Engine->Squares[Square];

    /* NOTE: Capture. */
    if (Is_Valid_Piece(MaybeCapturedPiece))
    {
        GameState->Piece[MaybeCapturedPiece] = square_Null;
    }

    Engine->Squares[GameState->Piece[Piece]] = piece_Null;
    GameState->Piece[Piece] = Square;
    Engine->Squares[Square] = Piece;
}

internal pawn_promotion_type GetPawnPromotionTypeFromMoveType(move_type MoveType)
{
    pawn_promotion_type PawnPromotionType = pawn_promotion_type_Null;

    switch (MoveType)
    {
    case move_type_PromoteQueen:  { PawnPromotionType = pawn_promotion_type_Queen; }  break;
    case move_type_PromoteRook:   { PawnPromotionType = pawn_promotion_type_Rook; }   break;
    case move_type_PromoteBishop: { PawnPromotionType = pawn_promotion_type_Bishop; } break;
    case move_type_PromoteKnight: { PawnPromotionType = pawn_promotion_type_Knight; } break;
    default: break;
    }

    return PawnPromotionType;
}

internal void MakeMove(engine *Engine, game_state *GameState, move Move)
{
    piece Piece = Move.Piece;
    b32 IsWhitePiece = Is_White_Piece(Piece);

    {
        b32 IsWhiteTurn = Is_White_Turn(GameState);
        b32 WhitePieceAndTurn = IsWhitePiece && IsWhiteTurn;
        b32 BlackPieceAndTurn = !(IsWhitePiece || IsWhiteTurn);

        if (!(WhitePieceAndTurn || BlackPieceAndTurn))
        {
            return;
        }
    }

    switch (Move.Type)
    {
    case move_type_EnPassant:
    {
        RemovePiece(Engine, GameState, GameState->LastMove.Piece);
    } /* Fall through */
    case move_type_Move:
    {
        MovePieceToSquare(Engine, GameState, Piece, Move.EndSquare);
    } break;
    case move_type_QueenCastle:
    {
        if (IsWhitePiece)
        {
            MovePieceToSquare(Engine, GameState, piece_White_Queen_Rook, D1);
            MovePieceToSquare(Engine, GameState, piece_White_King, C1);
        }
        else
        {
            MovePieceToSquare(Engine, GameState, piece_Black_Queen_Rook, D8);
            MovePieceToSquare(Engine, GameState, piece_Black_King, C8);
        }
    } break;
    case move_type_KingCastle:
    {
        if (IsWhitePiece)
        {
            MovePieceToSquare(Engine, GameState, piece_White_King_Rook, F1);
            MovePieceToSquare(Engine, GameState, piece_White_King, G1);
        }
        else
        {
            MovePieceToSquare(Engine, GameState, piece_Black_King_Rook, F8);
            MovePieceToSquare(Engine, GameState, piece_Black_King, G8);
        }
    } break;
    case move_type_PromoteQueen:
    case move_type_PromoteRook:
    case move_type_PromoteBishop:
    case move_type_PromoteKnight:
    {
        MovePieceToSquare(Engine, GameState, Piece, Move.EndSquare);
        GameState->PawnPromotion[Piece] = GetPawnPromotionTypeFromMoveType(Move.Type);
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
        Unset_Flag(GameState->Flags, White_Queen_Side_Castle_Flag);
    } break;
    case piece_Black_Queen_Rook:
    {
        Unset_Flag(GameState->Flags, Black_Queen_Side_Castle_Flag);
    } break;
    case piece_White_King_Rook:
    {
        Unset_Flag(GameState->Flags, White_King_Side_Castle_Flag);
    } break;
    case piece_Black_King_Rook:
    {
        Unset_Flag(GameState->Flags, Black_King_Side_Castle_Flag);
    } break;
    case piece_White_King:
    {
        Unset_Flag(GameState->Flags, White_Queen_Side_Castle_Flag);
        Unset_Flag(GameState->Flags, White_King_Side_Castle_Flag);
    } break;
    case piece_Black_King:
    {
        Unset_Flag(GameState->Flags, Black_Queen_Side_Castle_Flag);
        Unset_Flag(GameState->Flags, Black_King_Side_Castle_Flag);
    } break;
    default: break;
    }

    Toggle_Flag(GameState->Flags, Whose_Turn_Flag);
    GameState->LastMove = Move;
}

internal b32 CheckIfKingIsInCheck(engine *Engine, game_state *GameState, piece KingPiece)
{
    Assert(KingPiece == piece_White_King || KingPiece == piece_Black_King);
    b32 IsInCheck = 0;

    s8 RowColOffsets[8][2] = {
        {-1,-1},{-1, 0},{-1, 1},
        { 0,-1},        { 0, 1},
        { 1,-1},{ 1, 0},{ 1, 1},
    };

    s8 KingRow = GameState->Piece[KingPiece] / 8;
    s8 KingCol = GameState->Piece[KingPiece] % 8;
    u32 KingPieceColor = Get_Piece_Color(KingPiece);

    for (s32 I = 0; I < 8 && !IsInCheck; ++I)
    {
        s32 RowOffset = RowColOffsets[I][0];
        s32 ColOffset = RowColOffsets[I][1];

        for (s32 Distance = 1; Distance < 8 && !IsInCheck; ++Distance)
        {
            s32 TargetRow = RowOffset * Distance + KingRow;
            s32 TargetCol = ColOffset * Distance + KingCol;

            s32 Square = Get_Square_Index(TargetRow, TargetCol);
            b32 SquareIsValid = Is_Valid_Square(Square);

            if (SquareIsValid)
            {
                piece TargetPiece = Engine->Squares[Square];

                if (Is_Valid_Piece(TargetPiece))
                {
                    piece_type PieceType = PromotePieceIfPossible(GameState, TargetPiece);

                    u8 TargetPieceColor = Get_Piece_Color(TargetPiece);

                    if (KingPieceColor == TargetPieceColor)
                    {
                        break;
                    }

                    switch (I)
                    {
                    case 0: case 2: case 5: case 7:
                    {
                        if (PieceType == piece_type_Bishop || PieceType == piece_type_Queen)
                        {
                            IsInCheck = 1;
                            GameState->DebugCheckPiece = TargetPiece;
                        }
                        else if (PieceType == piece_type_Pawn && Distance == 1)
                        {
                            b32 WhiteKingInCheck = Is_White_Piece(KingPiece) && (I == 5 || I == 7);
                            b32 BlackKingInCheck = Is_Black_Piece(KingPiece) && (I == 0 || I == 2);

                            if (WhiteKingInCheck || BlackKingInCheck)
                            {
                                IsInCheck = 1;
                                GameState->DebugCheckPiece = TargetPiece;
                            }
                        }
                        else if (PieceType == piece_type_King && Distance == 1)
                        {
                            IsInCheck = 1;
                        }
                    } break;
                    case 1: case 3: case 4: case 6:
                    {
                        if (PieceType == piece_type_Rook || PieceType == piece_type_Queen)
                        {
                            IsInCheck = 1;
                            GameState->DebugCheckPiece = TargetPiece;
                        }
                        else if (PieceType == piece_type_King && Distance == 1)
                        {
                            IsInCheck = 1;
                        }
                    } break;
                    }

                    break; /* NOTE: Break out of distance loop if we see any kind of piece. */
                }
            }
            else
            {
                break; /* NOTE: Break out if we ever hit an invalid square. */
            }
        }
    }

    { /* NOTE: Check for knight checks... */
        s8 KnightOffsets[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
        u8 KingPieceColor = Get_Piece_Color(KingPiece);

        for (s32 I = 0; I < 8; ++I)
        {
            s8 TargetRow = KingRow + KnightOffsets[I][0];
            s8 TargetCol = KingCol + KnightOffsets[I][1];

            if (Is_Valid_Row_Col(TargetRow, TargetCol))
            {
                s32 NewSquare = TargetRow * 8 + TargetCol;
                piece TargetPiece = Engine->Squares[NewSquare];

                if (Is_Valid_Square(NewSquare) &&
                    Is_Valid_Piece(TargetPiece) &&
                    (PromotePieceIfPossible(GameState, TargetPiece) == piece_type_Knight) &&
                    (Get_Piece_Color(TargetPiece) != KingPieceColor))
                {
                    IsInCheck = 1;
                    GameState->DebugCheckPiece = TargetPiece;
                    break;
                }
            }
        }
    }

    if (IsInCheck)
    {
        u32 CheckFlag = KingPiece == piece_White_King ? White_In_Check_Flag : Black_In_Check_Flag;
        Set_Flag(GameState->Flags, CheckFlag);
    }

    return IsInCheck;
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

internal b32 CanCastle(engine *Engine, game_state *GameState, b32 QueenSideCastle)
{
    b32 IsWhiteTurn = Is_White_Turn(GameState);

    u8 KingSquare, RookSquare, FirstSquare, SecondSquare, ThirdSquare;
    b32 CastleFlag;
    piece KingPiece, RookPiece;

    if (IsWhiteTurn)
    {
        if (QueenSideCastle)
        {
            KingSquare = E1;
            RookSquare = A1;
            FirstSquare = B1;
            SecondSquare = C1;
            ThirdSquare = D1;

            CastleFlag = White_Queen_Side_Castle_Flag;
            KingPiece = piece_White_King;
            RookPiece = piece_White_Queen_Rook;
        }
        else
        {
            KingSquare = E1;
            RookSquare = H1;
            FirstSquare = F1;
            SecondSquare = G1;
            ThirdSquare = 255;

            CastleFlag = White_King_Side_Castle_Flag;
            KingPiece = piece_White_King;
            RookPiece = piece_White_King_Rook;
        }
    }
    else
    {
        if (QueenSideCastle)
        {
            KingSquare = E8;
            RookSquare = A8;
            FirstSquare = B8;
            SecondSquare = C8;
            ThirdSquare = D8;

            CastleFlag = Black_Queen_Side_Castle_Flag;
            KingPiece = piece_Black_King;
            RookPiece = piece_Black_Queen_Rook;
        }
        else
        {
            KingSquare = E8;
            RookSquare = H8;
            FirstSquare = F8;
            SecondSquare = G8;
            ThirdSquare = 255;

            CastleFlag = Black_King_Side_Castle_Flag;
            KingPiece = piece_Black_King;
            RookPiece = piece_Black_King_Rook;
        }
    }

    b32 KingInOriginalPosition = GameState->Piece[KingPiece] == KingSquare;
    b32 RookInOriginalPosition = GameState->Piece[RookPiece] == RookSquare;
    b32 CastleFlagSet = Get_Flag(GameState->Flags, CastleFlag);
    b32 FirstOpen = !Is_Valid_Piece(Engine->Squares[FirstSquare]);
    b32 SecondOpen = !Is_Valid_Piece(Engine->Squares[SecondSquare]);
    b32 ThirdOpen = ThirdSquare == 255 || !Is_Valid_Piece(Engine->Squares[ThirdSquare]);

    b32 CanCastle = 0;

    if (KingInOriginalPosition && RookInOriginalPosition &&
        CastleFlagSet && FirstOpen && SecondOpen && ThirdOpen)
    {
        b32 KingPassesThroughCheck = 0;
        game_state TempGameState;
        move Move;

        CopyGameState(GameState, &TempGameState);

        Move.Type = move_type_Move;
        Move.Piece = KingPiece;
        Move.BeginSquare = KingSquare;
        Move.EndSquare = FirstSquare;
        InitializeSquares(Engine->Squares, &TempGameState);
        MakeMove(Engine, &TempGameState, Move);

        if (CheckIfKingIsInCheck(Engine, &TempGameState, KingPiece))
        {
            KingPassesThroughCheck = 1;
        }

        if (QueenSideCastle)
        {
            Move.BeginSquare = FirstSquare;
            Move.EndSquare = SecondSquare;
            InitializeSquares(Engine->Squares, &TempGameState);
            MakeMove(Engine, &TempGameState, Move);

            if (CheckIfKingIsInCheck(Engine, &TempGameState, KingPiece))
            {
                KingPassesThroughCheck = 1;
            }
        }

        InitializeSquares(Engine->Squares, GameState);

        CanCastle = !KingPassesThroughCheck;
    }

    return CanCastle;
}

internal void ClearTraversals(engine *Engine, traversal_node *TraversalNodes)
{
    memset(TraversalNodes, 0, sizeof(traversal_node)*Traversal_Chunk_Count);
}

#define Get_Game_Tree_Index_From_Pointer(as, gt) (s32)((gt) - (game_tree *)(as)->GameTreeArena.Data)

internal s32 GetGameTreeIndexFromPointer(engine *Engine, game_tree *GameTree)
{
    s32 Index = (s32)(GameTree - (game_tree *)Engine->GameTreeArena.Data);

    return Index;
}

#define Get_Traversal_Index_Bit_Shift(i) (1 << (i % Traversal_Chunk_Bits))

#define Check_If_Traversal_Node_Is_Visited(traversal_nodes, i) \
    (((traversal_nodes)[i/Traversal_Chunk_Bits] & Get_Traversal_Index_Bit_Shift(i)) != 0)

#define Set_Traversal_Node_As_Visited(traversal_nodes, i) \
    ((traversal_nodes)[i/Traversal_Chunk_Bits] |= Get_Traversal_Index_Bit_Shift(i))

/* TODO: Delete Old_TraverseFirstChild and use TraverseFirstChild. */
internal traversal_result Old_TraverseFirstChild(engine *Engine, traversal_node *TraversalNodes, game_tree *GameTree)
{
    traversal_result Result;
    Result.GameTreeIndex = GetGameTreeIndexFromPointer(Engine, GameTree->FirstChild);

    if (Is_Game_Tree_Index_In_Range(Result.GameTreeIndex))
    {
        if (Check_If_Traversal_Node_Is_Visited(TraversalNodes, Result.GameTreeIndex))
        {
            Result.GameTreeIndex = -1;
        }
    }

    return Result;
}

/* TODO: Delete Old_TraverseNextSibling and use TraverseNextSibling. */
internal traversal_result Old_TraverseNextSibling(engine *Engine, game_tree *GameTree)
{
    traversal_result Result = {0};
    Result.GameTreeIndex = GetGameTreeIndexFromPointer(Engine, GameTree->NextSibling);

    return Result;
}

/* TODO: Maybe also delete TraverseFirstChild/NextSibling since they are slow, or just replace with a faster macro or something. */
internal traversal_result TraverseFirstChild(engine *Engine, traversal Traversal)
{
    traversal_result Result;
    Result.GameTreeIndex = (s32)(Traversal.CurrentNode->FirstChild - (game_tree *)Engine->GameTreeArena.Data);

    if (Is_Game_Tree_Index_In_Range(Result.GameTreeIndex))
    {
        if (Check_If_Traversal_Node_Is_Visited(Traversal.Nodes, Result.GameTreeIndex))
        {
            Result.GameTreeIndex = -1;
        }
    }

    return Result;
}

internal traversal_result TraverseNextSibling(engine *Engine, traversal Traversal)
{
    traversal_result Result = {0};
    Result.GameTreeIndex = GetGameTreeIndexFromPointer(Engine, Traversal.CurrentNode->NextSibling);

    return Result;
}

internal void SetTraversalAsVisited(traversal_node *TraversalNodes, s32 TraversalIndex)
{
    if (Is_Game_Tree_Index_In_Range(TraversalIndex))
    {
        Set_Traversal_Node_As_Visited(TraversalNodes, TraversalIndex);
    }
}

#if DEBUG
#define Debug_WalkGameTreeAndReturnCount(as, gt) Debug_WalkGameTreeAndReturnCount_((as), (gt))
#else
#define Debug_WalkGameTreeAndReturnCount(...) 0
#endif
internal s32 Debug_WalkGameTreeAndReturnCount_(engine *Engine, game_tree *GameTree)
{
    s32 Count = 0;
    game_tree *CurrentNode = GameTree;

    ClearTraversals(Engine, Engine->DebugTraversalNodes);

    while (CurrentNode)
    {
        s32 CurrentNodeIndex = GetGameTreeIndexFromPointer(Engine, CurrentNode);

        traversal_result FirstChildTraversal = Old_TraverseFirstChild(Engine, Engine->DebugTraversalNodes, CurrentNode);
        traversal_result NextSiblingTraversal = Old_TraverseNextSibling(Engine, CurrentNode);

        if (Is_Game_Tree_Index_In_Range(FirstChildTraversal.GameTreeIndex))
        {
            CurrentNode = CurrentNode->FirstChild;
        }
        else if (Is_Game_Tree_Index_In_Range(NextSiblingTraversal.GameTreeIndex))
        {
            CurrentNode = CurrentNode->NextSibling;
            SetTraversalAsVisited(Engine->DebugTraversalNodes, CurrentNodeIndex);
        }
        else
        {
            CurrentNode = CurrentNode->Parent;
            SetTraversalAsVisited(Engine->DebugTraversalNodes, CurrentNodeIndex);
        }
    }

    for (s32 I = 0; I < Game_Tree_Pool_Size; ++I)
    {
        if (Check_If_Traversal_Node_Is_Visited(Engine->DebugTraversalNodes, I))
        {
            Count += 1;
        }
    }

    return Count;
}

#if DEBUG
#define Debug_CheckTheCountsOfTheTrees(as) Debug_CheckTheCountsOfTheTrees_((as))
#else
#define Debug_CheckTheCountsOfTheTrees(...) 0
#endif
internal s32 Debug_CheckTheCountsOfTheTrees_(engine *Engine)
{
    s32 GameTreeCount = Debug_WalkGameTreeAndReturnCount(Engine, Engine->RootGameTree);
    s32 FreeTreeCount = Debug_WalkGameTreeAndReturnCount(Engine, Engine->FreeGameTree);
    s32 UnusedNodeCount = Game_Tree_Pool_Size - (Engine->GameTreeArena.Offset/sizeof(game_tree));

    Assert(GameTreeCount + FreeTreeCount + UnusedNodeCount == Game_Tree_Pool_Size);

    return FreeTreeCount;
}

internal void ZeroGameTree(game_tree *GameTree)
{
    GameTree->Parent = 0;
    GameTree->FirstChild = 0;
    GameTree->NextSibling = 0;
    GameTree->PreviousSibling = 0;
    GameTree->Score = 0.0f;
    GameTree->FollowingScore = 0.0f;

    ZeroGameState(&GameTree->State);
}

internal game_tree *CreateGameTree(engine *Engine)
{
    game_tree *NewGameTree = Engine->FreeGameTree;

    if (NewGameTree)
    {
        while (NewGameTree->FirstChild)
        {
            NewGameTree = NewGameTree->FirstChild;
        }

        if (NewGameTree->Parent && NewGameTree != Engine->FreeGameTree)
        {
            NewGameTree->Parent->FirstChild = NewGameTree->NextSibling;
        }

        if (NewGameTree->NextSibling)
        {
            NewGameTree->NextSibling->PreviousSibling = 0;
        }

        if (NewGameTree == Engine->FreeGameTree)
        {
            Engine->FreeGameTree = NewGameTree->NextSibling;
        }

        ZeroGameTree(NewGameTree);
    }
    else if (Engine->GameTreeArena.Offset < Engine->GameTreeArena.Capacity)
    {
        NewGameTree = (game_tree *)(Engine->GameTreeArena.Data + Engine->GameTreeArena.Offset);
        Engine->GameTreeArena.Offset += sizeof(game_tree);
    }
    else
    {
        Set_Flag(Engine->Flags, engine_flags_GameTreeNodePoolIsFull);
    }

    return NewGameTree;
}

internal void Debug_CheckThatTreeDoesNotContainNode(engine *Engine, game_tree *RootNode, game_tree *TestNode)
{
    ClearTraversals(Engine, Engine->DebugTraversalNodes);

    game_tree *CurrentNode = RootNode;

    while (CurrentNode)
    {
        s32 CurrentNodeIndex = GetGameTreeIndexFromPointer(Engine, CurrentNode);

        traversal_result FirstChildTraversal = Old_TraverseFirstChild(Engine, Engine->DebugTraversalNodes, CurrentNode);
        traversal_result NextSiblingTraversal = Old_TraverseNextSibling(Engine, CurrentNode);

        if (Is_Game_Tree_Index_In_Range(FirstChildTraversal.GameTreeIndex))
        {
            CurrentNode = CurrentNode->FirstChild;
            SetTraversalAsVisited(Engine->DebugTraversalNodes, CurrentNodeIndex);
        }
        else if (Is_Game_Tree_Index_In_Range(NextSiblingTraversal.GameTreeIndex))
        {
            CurrentNode = CurrentNode->NextSibling;
            SetTraversalAsVisited(Engine->DebugTraversalNodes, CurrentNodeIndex);
        }
        else
        {
            CurrentNode = CurrentNode->Parent;
            SetTraversalAsVisited(Engine->DebugTraversalNodes, CurrentNodeIndex);
        }

        Assert(CurrentNode != TestNode);
    }
}

internal game_tree *GetRootGameTree(game_tree *GameTree)
{
    game_tree *Root = GameTree;

    if (Root)
    {
        while(Root->Parent)
        {
            Root = Root->Parent;
        }

        while(Root->PreviousSibling)
        {
            Root = Root->PreviousSibling;
        }
    }

    return Root;
}

internal void SpliceGameTree(game_tree *GameTree)
{
    Assert(GameTree != 0);

    if (GameTree->Parent && GameTree->Parent->FirstChild == GameTree)
    {
        GameTree->Parent->FirstChild = GameTree->NextSibling;
    }

    if (GameTree->NextSibling)
    {
        GameTree->NextSibling->PreviousSibling = GameTree->PreviousSibling;
    }

    if (GameTree->PreviousSibling)
    {
        GameTree->PreviousSibling->NextSibling = GameTree->NextSibling;
    }

    GameTree->Parent = 0;
    GameTree->NextSibling = 0;
    GameTree->PreviousSibling = 0;
}

internal void AddMoveToHistory(engine *Engine, move Move)
{
    s32 *HistoryIndex = &Engine->MoveHistory.Index;
    Assert(*HistoryIndex >= 0 && *HistoryIndex < Move_History_Size);

    Engine->MoveHistory.Moves[*HistoryIndex] = Move;

    *HistoryIndex = (*HistoryIndex + 1) % Move_History_Size;
}

internal void ClearDisplayNodes(engine *Engine)
{
    for (s32 I = 0; I < Game_Tree_Pool_Size; ++I)
    {
        Engine->DisplayNodes[I].Visible = 0;
    }
}

internal void MakeGameTreeTheRoot(engine *Engine, game_tree *NewRoot)
{
    if (NewRoot == 0)
    {
        return;
    }

    SpliceGameTree(NewRoot);

    game_tree *NewFreeTree = Engine->RootGameTree;

    if (NewFreeTree != 0 && NewFreeTree->Parent)
    {
        Assert(NewFreeTree->Parent->FirstChild == NewFreeTree);
        NewFreeTree->Parent->FirstChild = 0;
    }

    for (game_tree *NewFreeSibling = NewFreeTree;
         NewFreeSibling;
         NewFreeSibling = NewFreeSibling->NextSibling)
    {
        NewFreeSibling->Parent = 0;
    }

    if (Engine->FreeGameTree)
    {
        game_tree *LastFreeSibling = Engine->FreeGameTree;
        while (LastFreeSibling->NextSibling)
        {
            LastFreeSibling = LastFreeSibling->NextSibling;
        }

        LastFreeSibling->NextSibling = NewFreeTree;
        NewFreeTree->PreviousSibling = LastFreeSibling;
    }
    else
    {
        Engine->FreeGameTree = NewFreeTree;
    }

    Engine->RootGameTree = NewRoot;
    Engine->GameTreeCurrent = NewRoot;
    AddMoveToHistory(Engine, NewRoot->State.LastMove);

    ClearDisplayNodes(Engine);
}

internal b32 CheckIfGameStatesAreEqual(game_state *StateA, game_state *StateB)
{
    /* TODO: It's a pain to check if the flags are equal, but we should _REALLY_ check if the flags are equal... */
    /* b32 FlagsAreEqual = StateA->Flags == StateB->Flags; */
    b32 PiecesMatch = 1;
    /* TODO: Do we care if the LastMove is the same? Maybe for en-passant? */
    /* move LastMove; */

    for (s32 I = 0; I < piece_Count; ++I)
    {
        if (StateA->Piece[I] != StateB->Piece[I] ||
            StateA->PawnPromotion[I] != StateB->PawnPromotion[I])
        {
            PiecesMatch = 0;
            break;
        }
    }

    b32 GameStatesAreEqual = PiecesMatch;//FlagsAreEqual && PiecesMatch;

    return GameStatesAreEqual;
}

/* NOTE: We have to forward declare the look functions because they get recursively called, but using different dispatches. */
internal void LookDispatch(engine *Engine, game_state *GameState, look_type LookType, piece Piece, square Square, move_type MoveType);
internal void Look(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, s8 RowOffset, s8 ColOffset, u8 MaxLength);
internal void LookRight(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookUp(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookLeft(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookDown(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookUpRight(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookUpLeft(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookDownLeft(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookDownRight(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookAllDirections(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength);
internal void LookKnight(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col);
internal void LookCastle(engine *Engine, game_state *GameState, look_type LookType, piece Piece);
internal void LookPawn(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col);

internal void GeneratePotentials(engine *Engine, game_state *GameState, look_type LookType)
{
    Assert(Engine->GameTreeCurrent != 0);
    game_tree *GameTreeCurrent = Engine->GameTreeCurrent;
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
        if (Get_Flag(Engine->Flags, engine_flags_GameTreeNodePoolIsFull))
        {
            Assert(0);
        }

        piece Piece = I;
        s8 Square = GameState->Piece[Piece];
        s8 Row = Square / 8;
        s8 Col = Square % 8;

        u8 PieceType = PromotePieceIfPossible(GameState, Piece);

        /* @CopyPasta GetGameStateScore */

        if (Is_Valid_Square(Square))
        {
            switch (PieceType)
            {
            case piece_type_Rook:
            {
                LookRight(Engine, GameState, LookType, Piece, Row, Col, 8);
                LookDown(Engine, GameState, LookType, Piece, Row, Col, 8);
                LookLeft(Engine, GameState, LookType, Piece, Row, Col, 8);
                LookUp(Engine, GameState, LookType, Piece, Row, Col, 8);
            } break;
            case piece_type_Knight:
            {
                LookKnight(Engine, GameState, LookType, Piece, Row, Col);
            } break;
            case piece_type_Bishop:
            {
                LookUpRight(Engine, GameState, LookType, Piece, Row, Col, 8);
                LookUpLeft(Engine, GameState, LookType, Piece, Row, Col, 8);
                LookDownLeft(Engine, GameState, LookType, Piece, Row, Col, 8);
                LookDownRight(Engine, GameState, LookType, Piece, Row, Col, 8);
            } break;
            case piece_type_Queen:
            {
                LookAllDirections(Engine, GameState, LookType, Piece, Row, Col, 8);
            } break;
            case piece_type_King:
            {
                LookAllDirections(Engine, GameState, LookType, Piece, Row, Col, 1);
                LookCastle(Engine, GameState, LookType, Piece);
            } break;
            case piece_type_Pawn:
            {
                LookPawn(Engine, GameState, LookType, Piece, Row, Col);
            } break;
            default:
            {
                Assert(!"Unknown piece type\n");
            } break;
            }
        }
    }
}

debug_variable char PieceDisplay[piece_Count] = {
    'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R',
    'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
    'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
    'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
};

internal void Debug_PrintBoard(engine *Engine)
{
    for (s32 Row = 7; Row >= 0; --Row)
    {
        for (s32 Col = 0; Col < 8; ++Col)
        {
            s32 SquareIndex = Get_Square_Index(Row, Col);
            piece Piece = Engine->Squares[SquareIndex];

            if (Is_Valid_Piece(Piece))
            {
                DebugPrint("%c  ", PieceDisplay[Piece]);
            }
            else
            {
                DebugPrint("-  ");
            }
        }
        DebugPrint("\n");
    }
    DebugPrint("\n");
}

internal void ClearAttackersAndDefenders(engine *Engine)
{
    for (s32 Piece = 0; Piece < piece_Count; ++Piece)
    {
        Engine->Evaluation.AttackersAndDefenders[Piece] = 0.0f;
    }
}

internal f32 GetGameStateScore(engine *Engine, game_state *GameState)
{
    /* TODO: Include the following concepts into the score...
       - Material
       - How much space players control
       - Attackers and defenders
       - Check and checkmate
    */
    f32 Score = 0.0f;

    f32 WhiteScore = 0.0f;
    f32 BlackScore = 0.0f;

    InitializeSquares(Engine->Squares, GameState);

    { /* NOTE: Attackers and defenders. */
        ClearAttackersAndDefenders(Engine);
        look_type LookType = look_type_AttackersAndDefenders;
        for (s32 Piece = 0; Piece < piece_Count; ++Piece)
        {
            piece_type PieceType = PromotePieceIfPossible(GameState, Piece);
            square Square = GameState->Piece[Piece];
            s8 Row = Square / 8;
            s8 Col = Square % 8;

            if (Is_Valid_Square(Square))
            {
                switch (PieceType)
                {
                case piece_type_Rook:
                {
                    LookRight(Engine, GameState, LookType, Piece, Row, Col, 8);
                    LookDown(Engine, GameState, LookType, Piece, Row, Col, 8);
                    LookLeft(Engine, GameState, LookType, Piece, Row, Col, 8);
                    LookUp(Engine, GameState, LookType, Piece, Row, Col, 8);
                } break;
                case piece_type_Knight:
                {
                    LookKnight(Engine, GameState, LookType, Piece, Row, Col);
                } break;
                case piece_type_Bishop:
                {
                    LookUpRight(Engine, GameState, LookType, Piece, Row, Col, 8);
                    LookUpLeft(Engine, GameState, LookType, Piece, Row, Col, 8);
                    LookDownLeft(Engine, GameState, LookType, Piece, Row, Col, 8);
                    LookDownRight(Engine, GameState, LookType, Piece, Row, Col, 8);
                } break;
                case piece_type_Queen:
                {
                    LookAllDirections(Engine, GameState, LookType, Piece, Row, Col, 8);
                } break;
                case piece_type_King:
                {
                    LookAllDirections(Engine, GameState, LookType, Piece, Row, Col, 1);
                    /* LookCastle(Engine, GameState, LookType, Piece); */
                } break;
                case piece_type_Pawn:
                {
                    LookPawn(Engine, GameState, LookType, Piece, Row, Col);
                } break;
                default:
                {
                    Assert(!"Unknown piece type\n");
                } break;
                }
            }
        }
#if 0
        /* NOTE: We used to include all attacker/defender values, but I don't know if that is a good idea, so it's turned off for now. */
        for (s32 Piece = 0; Piece < piece_Count; ++Piece)
        {
            s32 AttackersAndDefenders = Engine->Evaluation.AttackersAndDefenders[Piece];

            if (AttackersAndDefenders < 0)
            {
                piece_type PieceType = PieceTypeTable[Piece];
                f32 Multiplier = 1.0f;
                f32 PieceValue = PieceValueTable[PieceType];
                f32 Value = -PieceValue*Multiplier*(f32)AttackersAndDefenders;

                if (Piece < 16)
                {
                    BlackScore += Value;
                }
                else
                {
                    WhiteScore += Value;
                }
            }
        }
#endif
    }

    /* NOTE: Material and center square. */
    for (s32 I = 0; I < piece_Count; ++I)
    {
        square Square = GameState->Piece[I];
        piece Piece = I;
        u8 PieceType = PromotePieceIfPossible(GameState, I);
        b32 IsWhitePiece = Is_White_Piece(Piece);
        b32 IsWhiteTurn = Is_White_Turn(GameState);
        s32 AttackersAndDefenders = Engine->Evaluation.AttackersAndDefenders[Piece];
        b32 IsOffTurnPiece = (IsWhitePiece && !IsWhiteTurn) || (!IsWhitePiece && IsWhiteTurn);
        b32 PieceIsInDanger = IsOffTurnPiece && AttackersAndDefenders >= 0;

        /* @CopyPasta GeneratePotentials */

        if (Is_Valid_Square(Square) && PieceIsInDanger)
        {
            f32 PieceValue = PieceValueTable[PieceType];
            f32 BonusScale = 0.9f;
            f32 Bonus = BonusScale * Engine->Evaluation.SquareBonus[Square];
            /* f32 PieceScore = PieceValue * Bonus; */
            f32 PieceScore = PieceValue + Bonus;

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

    f32 SpaceScale = 0.03f;
    /* NOTE: Estimate space taken up by each player. */
    for (s32 Col = 0; Col < 8; ++Col)
    {
        /* NOTE: Check white space. */
        b32 AddWhiteSpace = 0;
        b32 AddBlackSpace = 0;
        b32 CheckingBlack = 1;
        s32 WhiteRowCount = 0;
        s32 BlackRowCount = 0;

        for (s32 Row = 0; Row < 8; ++Row)
        {
            piece Piece = Engine->Squares[8*Row+Col];

            if (Is_Valid_Piece(Piece))
            {
                if (Is_White_Piece(Piece))
                {
                    AddWhiteSpace = 1;
                    WhiteRowCount = Row;
                }
                else if (CheckingBlack && Is_Black_Piece(Piece))
                {
                    AddBlackSpace = 1;
                    CheckingBlack = 0;
                    BlackRowCount = 7 - Row;
                }
            }
        }

        if (AddWhiteSpace)
        {
            WhiteScore += SpaceScale * (f32)WhiteRowCount;
        }

        if (AddBlackSpace)
        {
            BlackScore += SpaceScale * (f32)BlackRowCount;
        }
    }

    { /* NOTE: Check and checkmate. */
        b32 IsWhiteTurn = Is_White_Turn(GameState);
        b32 WhiteIsInCheck = CheckIfKingIsInCheck(Engine, GameState, piece_White_King);
        b32 BlackIsInCheck = CheckIfKingIsInCheck(Engine, GameState, piece_Black_King);
        b32 KingIsInCheck = WhiteIsInCheck || BlackIsInCheck;
        f32 CheckValue = 4.0f;

        u8 SquareValues[piece_Count] = {0, 1, 2, 39, 4, 5, 6, 7, 8, 9, 10, 11, 20, 13, 14, 15, 48, 49, 50, 51, 52, 37, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};
        for (s32 I = 0; I < piece_Count; ++I)
        {
            if (SquareValues[I] != GameState->Piece[I])
            {
                /* Debug_ShouldPause = 0; */
                break;
            }
        }

        if (KingIsInCheck)
        {
            Engine->GeneratedStateCount = 0;
            GeneratePotentials(Engine, GameState, look_type_Checkmate);

            if (Engine->GeneratedStateCount == 0)
            {
                Set_Flag(GameState->Flags, Game_Over_Flag); /* TODO: Should we set the game_over flag here? */

                if (IsWhiteTurn)
                {
                    BlackScore = 999999999.0f;
                    WhiteScore = 0.0f;
                }
                else
                {
                    WhiteScore = 999999999.0f;
                    BlackScore = 0.0f;
                }
            }
            else
            {
#if 0
                if (IsWhiteTurn)
                {
                    BlackScore += CheckValue;
                }
                else
                {
                    WhiteScore += CheckValue;
                }
#endif
            }
        }
    }

    BlackScore = -1.0f * BlackScore;
    Score = WhiteScore + BlackScore;

    return Score;
}

#if DEBUG
#define Debug_CheckThatGameTreeDoesNotHaveGameTreeAsChild(pt, tt) Debug_CheckThatGameTreeDoesNotHaveGameTreeAsChild_((pt), (tt))
#else
#define Debug_CheckThatGameTreeDoesNotHaveGameTreeAsChild(...) 0
#endif
internal void Debug_CheckThatGameTreeDoesNotHaveGameTreeAsChild_(game_tree *ParentTree, game_tree *TestTree)
{
    game_tree *CurrentSibling = ParentTree->FirstChild;

    for (;;)
    {
        if (CurrentSibling == 0)
        {
            break;
        }

        b32 AreEqual = CheckIfGameStatesAreEqual(&CurrentSibling->State, &TestTree->State);
        Assert(!AreEqual);
        CurrentSibling = CurrentSibling->NextSibling;
    }
}

internal b32 CheckIfCurrentIsBestScore(engine *Engine, game_state *GameState, f32 PreviousScore, f32 CurrentScore)
{
    b32 CurrentIsBest;
    /* b32 BotIsWhite = Get_Flag(Engine->Flags, engine_flags_IsBotPlayingAsWhite); */
    b32 IsWhiteTurn = Is_White_Turn(GameState);

    if (IsWhiteTurn)
    {
        CurrentIsBest = CurrentScore > PreviousScore;
    }
    else
    {
        CurrentIsBest = PreviousScore > CurrentScore;
    }

    return CurrentIsBest;
}

internal void AddPotential(engine *Engine, game_state *GameState, look_type LookType, piece Piece, square EndSquare, move_type MoveType)
{
    Assert(GameState != 0);
    Assert(!Engine->FreeGameTree || Engine->FreeGameTree != Engine->FreeGameTree->NextSibling);
    Assert(Engine->GameTreeCurrent != 0);
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

    MakeMove(Engine, &NewGameState, Move);
    /* NOTE: @Copypasta HandleMove */
    /* b32 WhiteWasInCheck = Get_Flag(NewGameState.Flags, White_In_Check_Flag); */
    /* b32 BlackWasInCheck = Get_Flag(NewGameState.Flags, Black_In_Check_Flag); */
    Unset_Flag(NewGameState.Flags, White_In_Check_Flag | Black_In_Check_Flag);
    b32 WhiteIsInCheck = CheckIfKingIsInCheck(Engine, &NewGameState, piece_White_King);
    b32 BlackIsInCheck = CheckIfKingIsInCheck(Engine, &NewGameState, piece_Black_King);
    /* NOTE: Calling MakeMove mutates our global Squares with the new
       board state. Since we are adding potentials, we want to call
       InitializeSquares after doing MakeMove, to reset Squares to the
       current state. Basically, we recycle Engine->Squares and need to reset it
       after each MakeMove call we do.
    */
    InitializeSquares(Engine->Squares, GameState); /* TODO: We call InitializeSquares at the end of this function, doesn't that mean we can remove the call to InitializeSquares here??? */

    b32 MovingIntoCheck = ((Is_White_Turn(GameState) && WhiteIsInCheck) ||
                           (Is_Black_Turn(GameState) && BlackIsInCheck));

    if (LookType == look_type_AddPotential)
    {
        if (!MovingIntoCheck && Has_Free_Game_Tree(Engine))
        {
            game_tree *GameTree = CreateGameTree(Engine);

            GameTree->State = NewGameState;
            GameTree->NextSibling = Engine->GameTreeCurrent->FirstChild;

            if (GameTree->NextSibling)
            {
                GameTree->NextSibling->PreviousSibling = GameTree;
            }

            Engine->GameTreeCurrent->FirstChild = GameTree;
            GameTree->Parent = Engine->GameTreeCurrent;
            GameTree->Score = GetGameStateScore(Engine, &GameTree->State);

            Engine->CurrentMove.Type = 0;
            Engine->CurrentMove.Piece = 0;
            Engine->CurrentMove.BeginSquare = 0;
            Engine->CurrentMove.EndSquare = 0;
        }
    }
    else if (LookType == look_type_Checkmate)
    {
        if (!MovingIntoCheck)
        {
            Engine->GeneratedStateCount += 1;
        }
    }
    else
    {
        Assert(0);
    }


    InitializeSquares(Engine->Squares, GameState);
}

internal void CalculateAttackersAndDefenders(engine *Engine, game_state *GameState, piece Piece, square Square)
{
    InitializeSquares(Engine->Squares, GameState);

    u32 PieceColor = Get_Piece_Color(Piece);
    piece_type PieceType = PromotePieceIfPossible(GameState, Piece);
    f32 PieceValue = PieceValueTable[PieceType];
    piece TargetPiece = Engine->Squares[Square];

    if (Is_Valid_Piece(TargetPiece))
    {
        piece_type TargetPieceType = PromotePieceIfPossible(GameState, TargetPiece);
        f32 TargetPieceValue = PieceValueTable[TargetPieceType];
        u32 TargetPieceColor = Get_Piece_Color(TargetPiece);

#if 1
        s32 Amount = (PieceColor != TargetPieceColor && PieceValue < TargetPieceValue) ? 2 : 1;
        s32 ValueToAdd = PieceColor == TargetPieceColor ? Amount : -Amount;

        Engine->Evaluation.AttackersAndDefenders[TargetPiece] += ValueToAdd;
#else
#endif
        Engine->Evaluation.ShouldBreakLookLoop = 1; /* NOTE: We could _not_ break here if we wanted to check for pins/discoveries maybe. */
    }
}

internal void LookDispatch(engine *Engine, game_state *GameState, look_type LookType, piece Piece, square Square, move_type MoveType)
{
    switch (LookType)
    {
    case look_type_AttackersAndDefenders:
    {
        CalculateAttackersAndDefenders(Engine, GameState, Piece, Square);
    } break;
    case look_type_Checkmate:
    case look_type_AddPotential:
    {
        AddPotential(Engine, GameState, LookType, Piece, Square, MoveType);
    } break;
    default: Assert(0);
    }
}

/* TODO: Maybe bundle Piece, Row, Col, ... into a struct. */
internal void Look(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, s8 RowOffset, s8 ColOffset, u8 MaxLength)
{
    Assert(RowOffset || ColOffset);

    u8 PieceColor = Get_Piece_Color(Piece);
    s8 CurrentRow = Row + RowOffset;
    s8 CurrentCol = Col + ColOffset;
    u8 TotalLength = 1;

    while(!Engine->Evaluation.ShouldBreakLookLoop)
    {
        b32 ShouldDispatch = 0;
        b32 ShouldBreak = 0;
        b32 PositionInBounds = (CurrentRow >= 0 && CurrentRow < 8 &&
                                CurrentCol >= 0 && CurrentCol < 8);
        b32 CanReach = TotalLength <= MaxLength;

        if (!(PositionInBounds && CanReach))
        {
            break;
        }

        s32 NewSquare = CurrentRow * 8 + CurrentCol;

        if (Is_Valid_Square(NewSquare))
        {
            piece TargetPiece = Engine->Squares[NewSquare];
            u32 TargetPieceColor = Get_Piece_Color(TargetPiece);

            if (Is_Valid_Piece(TargetPiece))
            {
                if (LookType == look_type_AddPotential)
                {
                    if (TargetPieceColor != PieceColor)
                    {
                        ShouldDispatch = 1;
                    }

                    ShouldBreak = 1;
                }
                else if (LookType == look_type_AttackersAndDefenders)
                {
                    ShouldDispatch = 1;
                }
            }
            else
            {
                ShouldDispatch = 1;
            }
        }
        else
        {
            /* NOTE: We hit an invalid square, so break the loop. */
            break;
        }

        if (ShouldDispatch)
        {
            LookDispatch(Engine, GameState, LookType, Piece, NewSquare, move_type_Move);
        }

        if (ShouldBreak)
        {
            break;
        }

        CurrentRow += RowOffset;
        CurrentCol += ColOffset;
        TotalLength += 1;
    }

    Engine->Evaluation.ShouldBreakLookLoop = 0;
}

internal void LookRight(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(Engine, GameState, LookType, Piece, Row, Col, 0, 1, MaxLength);
}

internal void LookUp(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(Engine, GameState, LookType, Piece, Row, Col, 1, 0, MaxLength);
}

internal void LookLeft(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(Engine, GameState, LookType, Piece, Row, Col, 0, -1, MaxLength);
}

internal void LookDown(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(Engine, GameState, LookType, Piece, Row, Col, -1, 0, MaxLength);
}

internal void LookUpRight(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(Engine, GameState, LookType, Piece, Row, Col, 1, 1, MaxLength);
}

internal void LookUpLeft(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(Engine, GameState, LookType, Piece, Row, Col, 1, -1, MaxLength);
}

internal void LookDownLeft(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(Engine, GameState, LookType, Piece, Row, Col, -1, -1, MaxLength);
}

internal void LookDownRight(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    Look(Engine, GameState, LookType, Piece, Row, Col, -1, 1, MaxLength);
}

internal void LookAllDirections(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col, u8 MaxLength)
{
    LookRight(     Engine, GameState, LookType, Piece, Row, Col, MaxLength);
    LookUpRight(   Engine, GameState, LookType, Piece, Row, Col, MaxLength);
    LookUp(        Engine, GameState, LookType, Piece, Row, Col, MaxLength);
    LookUpLeft(    Engine, GameState, LookType, Piece, Row, Col, MaxLength);
    LookLeft(      Engine, GameState, LookType, Piece, Row, Col, MaxLength);
    LookDownLeft(  Engine, GameState, LookType, Piece, Row, Col, MaxLength);
    LookDown(      Engine, GameState, LookType, Piece, Row, Col, MaxLength);
    LookDownRight( Engine, GameState, LookType, Piece, Row, Col, MaxLength);
}

#define En_Passant_Row_White 4
#define En_Passant_Row_Black 3

internal void LookPawnPromotion(engine *Engine, game_state *GameState, look_type LookType, piece Piece, square Square)
{
    for (move_type MoveType = move_type_PromoteQueen;
         MoveType <= move_type_PromoteKnight;
         ++MoveType)
    {
        AddPotential(Engine, GameState, LookType, Piece, Square, MoveType);
    }
}

internal b32 CheckIfLastMoveAllowsEnPassant(game_state *GameState, s8 CurrentCol)
{
    s32 Multiplier = Is_White_Piece(GameState->LastMove.Piece) ? -1 : 1;

    s8 BeginRow = GameState->LastMove.BeginSquare / 8;
    s8 EndRow = GameState->LastMove.EndSquare / 8;
    s8 BeginCol = GameState->LastMove.BeginSquare % 8;
    s8 EndCol = GameState->LastMove.EndSquare % 8;

    piece MovePieceType = PieceTypeTable[GameState->LastMove.Piece];

    b32 IsPawnMove = MovePieceType == piece_type_Pawn;
    b32 WasTwoSquareMove = Multiplier * (EndRow - BeginRow) == -2;
    b32 LastMoveOnSameCol = (BeginCol == EndCol) && (EndCol == CurrentCol);

    b32 AllowsEnPassant = IsPawnMove && WasTwoSquareMove && LastMoveOnSameCol;

    return AllowsEnPassant;
}

internal void LookPawn(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col)
{
    u8 PieceColor = Get_Piece_Color(Piece);
    s8 Multiplier = 1;
    s8 StartingRow = 1;
    s8 EnPassantRow = En_Passant_Row_White;
    s8 LastRow = PieceColor ? 0 : 7;

    if (Is_Black_Piece(Piece))
    {
        Multiplier = -1;
        StartingRow = 6;
        EnPassantRow = En_Passant_Row_Black;
    }

    /* NOTE: Move forward */
    if (LookType == look_type_AddPotential || LookType == look_type_Checkmate)
    {
        for (s32 I = 1; I < 3; ++I)
        {
            if (Row != StartingRow && I == 2)
            {
                break;
            }

            s8 Offset = I * Multiplier;
            s8 CurrentRow = Row + Offset;

            square Square = Get_Square_Index(CurrentRow, Col);

            if (Is_Valid_Square(Square) && !Is_Valid_Piece(Engine->Squares[Square]))
            {
                if (CurrentRow == LastRow)
                {
                    LookPawnPromotion(Engine, GameState, LookType, Piece, Square);
                }
                else
                {
                    AddPotential(Engine, GameState, LookType, Piece, Square, move_type_Move);
                }
            }
            else
            {
                break;
            }
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
            Is_Valid_Piece(Engine->Squares[Square]))
        {
            if ((LookType == look_type_AddPotential || LookType == look_type_Checkmate) &&
                Get_Piece_Color(Engine->Squares[Square]) != PieceColor) {
                if (CurrentRow == LastRow)
                {
                    LookPawnPromotion(Engine, GameState, LookType, Piece, Square);
                }
                else
                {
                    AddPotential(Engine, GameState, LookType, Piece, Square, move_type_Move);
                }
            }
            else if (LookType == look_type_AttackersAndDefenders)
            {
                LookDispatch(Engine, GameState, LookType, Piece, Square, move_type_Move);
            }
        }
    }

    /* NOTE: En-passant */
    if (Row == EnPassantRow && LookType == look_type_AddPotential)
    {
        for (s32 I = -1; I < 2; I += 2)
        {
            s8 CurrentCol = Col + I;

            if (CheckIfLastMoveAllowsEnPassant(GameState, CurrentCol))
            {
                square CaptureSquare = Get_Square_Index(Row + Multiplier, CurrentCol);
                AddPotential(Engine, GameState, LookType, Piece, CaptureSquare, move_type_EnPassant);
            }
        }
    }
}

internal void LookKnight(engine *Engine, game_state *GameState, look_type LookType, piece Piece, s8 Row, s8 Col)
{
    s8 RowColOffsets[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
    u8 PieceColor = Get_Piece_Color(Piece);

    for (s32 I = 0; I < 8; ++I)
    {
        s8 TargetRow = Row + RowColOffsets[I][0];
        s8 TargetCol = Col + RowColOffsets[I][1];
        s32 NewSquare = TargetRow * 8 + TargetCol;

        b32 ShouldDispatch = 0;

        if (Is_Valid_Row_Col(TargetRow, TargetCol))
        {
            if (Is_Valid_Square(NewSquare) && Is_Valid_Piece(Engine->Squares[NewSquare]))
            {
                b32 ColorsMatch = Get_Piece_Color(Engine->Squares[NewSquare]) == PieceColor;

                if ((LookType == look_type_AddPotential && !ColorsMatch) ||
                    LookType == look_type_AttackersAndDefenders)
                {
                    ShouldDispatch = 1;
                }
            }
            else
            {
                ShouldDispatch = 1;
            }
        }

        if (ShouldDispatch)
        {
            LookDispatch(Engine, GameState, LookType, Piece, NewSquare, move_type_Move);
        }
    }
}

internal void LookCastle(engine *Engine, game_state *GameState, look_type LookType, piece Piece)
{
    b32 WhiteInCheck = Get_Flag(GameState->Flags, White_In_Check_Flag);
    b32 BlackInCheck = Get_Flag(GameState->Flags, Black_In_Check_Flag);
    b32 IsWhiteTurn = Is_White_Turn(GameState);

    b32 PlayerNotInCheck = (IsWhiteTurn && !WhiteInCheck) || (!IsWhiteTurn && !BlackInCheck);

    if (PlayerNotInCheck)
    {
        if (CanCastle(Engine, GameState, 1))
        {
            square EndSquare = IsWhiteTurn ? C1 : C8;
            LookDispatch(Engine, GameState, LookType, Piece, EndSquare, move_type_QueenCastle);
        }

        if (CanCastle(Engine, GameState, 0))
        {
            square EndSquare = IsWhiteTurn ? G1 : G8;
            LookDispatch(Engine, GameState, LookType, Piece, EndSquare, move_type_KingCastle);
        }
    }
}

#define MAX_DEPTH 5

internal void GenerateAllPotentials(engine *Engine)
{
    ryn_BEGIN_TIMED_BLOCK(timed_block_GenerateAllPotentials);
    traversal *Traversal = &Engine->GenerationTraversal;
    game_tree *OldGameTreeCurrent = Engine->GameTreeCurrent;
    game_tree *CurrentRoot = GetRootGameTree(Traversal->CurrentNode); /* TODO: Don't search for the root, and instead, whenever a move is made and there is a new root, reset all the traversals. */
    s32 ProcessedNodeCount = 0;
    s32 MaxNodesToBeProcessed = 1*1024; /* TODO: We should maybe find a better way to check how much work to do. The loop below is very spiky with the time it takes depending on if it calls GeneratePotentials. Maybe we just need to start doing time-based work allotment... just do your thing until N number of milliseconds have elapsed. */

    while (ProcessedNodeCount < MaxNodesToBeProcessed)
    {
        if (Traversal->CurrentNode == 0 || CurrentRoot == Engine->FreeGameTree)
        {
            Traversal->CurrentNode = Engine->RootGameTree;
            Traversal->Depth = 1;
        }

        s32 CurrentNodeIndex = GetGameTreeIndexFromPointer(Engine, Traversal->CurrentNode);

        /* traversal_result FirstChildTraversal; */
        s32 FirstChildIndex = (s32)(Traversal->CurrentNode->FirstChild - (game_tree *)Engine->GameTreeArena.Data);
        if (Is_Game_Tree_Index_In_Range(FirstChildIndex) &&
            Check_If_Traversal_Node_Is_Visited(Traversal->Nodes, FirstChildIndex))
        {
            FirstChildIndex = -1;
        }
        /* traversal_result NextSiblingTraversal; */
        s32 NextSiblingIndex = GetGameTreeIndexFromPointer(Engine, Traversal->CurrentNode->NextSibling);

        if (!Is_Game_Tree_Index_In_Range(FirstChildIndex) &&
            Traversal->CurrentNode->FirstChild == 0)
        {
            /* ryn_BEGIN_TIMED_BLOCK(timed_block_GenerateAllPotentialsGen); */

            Engine->GameTreeCurrent = Traversal->CurrentNode;
            InitializeSquares(Engine->Squares, &Engine->GameTreeCurrent->State);
            GeneratePotentials(Engine, &Engine->GameTreeCurrent->State, look_type_AddPotential);

            if (!Traversal->CurrentNode->FirstChild)
            {
                Traversal->CurrentNode = Traversal->CurrentNode->NextSibling;
                SetTraversalAsVisited(Traversal->Nodes, CurrentNodeIndex);
            }
            /* ryn_END_TIMED_BLOCK(timed_block_GenerateAllPotentialsGen); */
        }
        else if (Is_Game_Tree_Index_In_Range(FirstChildIndex) && Traversal->Depth < Traversal->MaxDepth)
        {
            /* ryn_BEGIN_TIMED_BLOCK(timed_block_GenerateAllPotentialsFirstChild); */
            Traversal->CurrentNode = Traversal->CurrentNode->FirstChild;
            Traversal->Depth += 1;
            /* ryn_END_TIMED_BLOCK(timed_block_GenerateAllPotentialsFirstChild); */
        }
        else if (Is_Game_Tree_Index_In_Range(NextSiblingIndex))
        {
            /* ryn_BEGIN_TIMED_BLOCK(timed_block_GenerateAllPotentialsNextSibling); */
            Traversal->CurrentNode = Traversal->CurrentNode->NextSibling;
            SetTraversalAsVisited(Traversal->Nodes, CurrentNodeIndex);
            /* ryn_END_TIMED_BLOCK(timed_block_GenerateAllPotentialsNextSibling); */
        }
        else
        {
            Traversal->CurrentNode = Traversal->CurrentNode->Parent;
            Traversal->Depth = MaxS32(0, Traversal->Depth - 1);

            if (Traversal->Depth == 0)
            {
                ryn_BEGIN_TIMED_BLOCK(timed_block_GenerateAllPotentialsGen);
                s32 NewMaxDepth = (Traversal->MaxDepth + 1) % MAX_DEPTH;
                ClearTraversals(Engine, Traversal->Nodes);
                Traversal->MaxDepth = NewMaxDepth;
                ryn_END_TIMED_BLOCK(timed_block_GenerateAllPotentialsGen);
            }

            if (Is_Game_Tree_Index_In_Range(CurrentNodeIndex))
            {
                SetTraversalAsVisited(Traversal->Nodes, CurrentNodeIndex);
            }
        }

        ProcessedNodeCount += 1;
    }

    Engine->GameTreeCurrent = OldGameTreeCurrent;
    ryn_END_TIMED_BLOCK(timed_block_GenerateAllPotentials);
}

internal void InitializeGameState(game_state *GameState)
{
    GameState->Flags = 0;
    Set_Flag(GameState->Flags, White_Queen_Side_Castle_Flag);
    Set_Flag(GameState->Flags, White_King_Side_Castle_Flag);
    Set_Flag(GameState->Flags, Black_Queen_Side_Castle_Flag);
    Set_Flag(GameState->Flags, Black_King_Side_Castle_Flag);

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
internal void DebugMovePiece(engine *Engine, game_state *GameState, piece Piece, square Square)
{
    move Move = {0};

    Move.Type = move_type_Move;
    Move.Piece = Piece;
    Move.BeginSquare = GameState->Piece[Move.Piece];
    Move.EndSquare = Square;

    MakeMove(Engine, GameState, Move);
}

internal void SetupForTesting(engine *Engine, game_state *GameState)
{
#if 0
    /* NOTE: Test un-passant for black. */
    Black_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_Black_Pawn_D, D3);

    White_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_White_Pawn_E, E4);

    GameState->Flags = Set_Flag(GameState->Flags, Whose_Turn_Flag);
#elif 0
    /* NOTE: Test castling for white. */
    White_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_White_Queen_Knight, E5);

    White_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_White_Queen_Bishop, F5);

    White_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_White_Queen, G5);

    White_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_White_King_Knight, E6);

    White_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_White_King_Bishop, F6);

    White_To_Move(GameState);
#elif 0
    /* NOTE: Test castling for black */
    Black_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_Black_Queen_Knight, E5);

    Black_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_Black_Queen_Bishop, F5);

    Black_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_Black_Queen, G5);

    Black_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_Black_King_Knight, E6);

    Black_To_Move(GameState);
    DebugMovePiece(Engine, GameState, piece_Black_King_Bishop, F6);

    Black_To_Move(GameState);
#elif 0
    /* NOTE: Test un-passant for white. */
    DebugMovePiece(Engine, GameState, piece_White_Pawn_E, E5);
    DebugMovePiece(Engine, GameState, piece_Black_Pawn_D, D5);
#elif 0
    DebugMovePiece(Engine, GameState, piece_White_Pawn_G, G4);
    DebugMovePiece(Engine, GameState, piece_Black_King_Knight, F6);
    DebugMovePiece(Engine, GameState, piece_White_Pawn_G, G5);
    DebugMovePiece(Engine, GameState, piece_Black_Queen_Knight, C6);
#else
    DebugMovePiece(Engine, GameState, piece_White_Pawn_E, E4);
    DebugMovePiece(Engine, GameState, piece_Black_Pawn_D, D5);
    DebugMovePiece(Engine, GameState, piece_White_Queen_Knight, C3);
    DebugMovePiece(Engine, GameState, piece_Black_Pawn_D, D4);
    DebugMovePiece(Engine, GameState, piece_White_Queen_Knight, D5);
    DebugMovePiece(Engine, GameState, piece_Black_Queen_Bishop, G4);
#endif
}

internal int PositionInsideBoard(Vector2 Position)
{
    return (Position.x >= 0.0f && Position.x <= BOARD_SIZE &&
            Position.y >= 0.0f && Position.y <= BOARD_SIZE);
}

internal Vector2 PositionToSquarePosition(Vector2 Position)
{
    Vector2 Result;
    Vector2 OffsetPosition;
    OffsetPosition.x = (Position.x - BOARD_PADDING) / SQUARE_SIZE_IN_PIXELS;
    OffsetPosition.y = BOARD_SIZE - ((Position.y - BOARD_PADDING) / SQUARE_SIZE_IN_PIXELS);

    if(PositionInsideBoard(OffsetPosition))
    {
        Result = V2(OffsetPosition.x, OffsetPosition.y);
    }
    else
    {
        Result = V2(-1,-1);
    }

    return Result;
}

internal ivec2 PositionToPawnPromotionSquarePosition(engine *Engine, Vector2 Position)
{
    ivec2 Result;

    s32 BorderWidth = 6; /* @Copypasta DrawBoard */
    s32 Width = 4 * SQUARE_SIZE_IN_PIXELS;
    s32 Height = SQUARE_SIZE_IN_PIXELS;
    s32 X = 0.5f * (SCREEN_WIDTH - Width) + BorderWidth;
    s32 Y = 0.5f * (SCREEN_HEIGHT - Height) + BorderWidth;

    Result.X = (s32)(Position.x - X) / SQUARE_SIZE_IN_PIXELS;
    Result.Y = (s32)(Position.y - Y) / SQUARE_SIZE_IN_PIXELS;

    if(Result.X < 0 || Result.X >= 4 || Result.Y != 0)
    {
        Result.X = -1;
        Result.Y = -1;
    }

    return Result;
}

internal b32 SquareValueOnBoard(int X, int Y)
{
    return X >= 0 && X < BOARD_SIZE && Y >= 0 && Y < BOARD_SIZE;
}

internal b32 SquareValueOnPawnPromotionPicker(int X, int Y)
{
    return X >= 0 && X < 4 && Y == 0;
}

internal int IVec2Equal(ivec2 A, ivec2 B)
{
    return A.X == B.X && A.Y == B.Y;
}

internal ivec2 RowColFromSquare(square Square)
{
    ivec2 Result;

    Result.X = Square % 8;
    Result.Y = Square / 8;

    return Result;
}

global_variable b32 GlobalShowDebugPanel = 0;
global_variable b32 GlobalDebugPanelIsPaused = 0;
global_variable ryn_profiler DebugProfilerForPause;

internal void Debug_PrintPieces(game_state *GameState)
{
    for (s32 Row = 3; Row >= 0; --Row)
    {
        for (s32 Col = 0; Col < 8; ++Col)
        {
            s32 Index = Row * 8 + Col;
            DebugPrint("%3d ", GameState->Piece[Index]);
        }
        DebugPrint("\n");
    }
    DebugPrint("\n");
}

internal void LetTheBotMakeAMove(engine *Engine)
{
    if (Engine->BotState == bot_state_ReadyToMove)
    {
        game_tree *TheBotsChosenGameState = Engine->GameTreeCurrent->FirstChild;
        MakeGameTreeTheRoot(Engine, TheBotsChosenGameState);
        Engine->BotState = bot_state_NULL;
    }
}

internal void HandleMove(engine *Engine)
{
    ui *Ui = &Engine->Ui;
    ivec2 SelectedSquare = Ui->SelectedSquare;
    ivec2 MoveSquare = Ui->MoveSquare;

    int HasSelectedSquare = SelectedSquare.X >= 0 && SelectedSquare.Y >= 0;
    int HasMoveSquare = Ui->MoveSquare.X >= 0 && Ui->MoveSquare.Y >= 0;

    if (Engine->GameTreeCurrent &&
        !Get_Flag(Engine->GameTreeCurrent->State.Flags, Game_Over_Flag) &&
        Engine->GameTreeCurrent && HasSelectedSquare && HasMoveSquare)
    {
        game_state TempGameState;
        CopyGameState(&Engine->GameTreeCurrent->State, &TempGameState);
        InitializeSquares(Engine->Squares, &TempGameState);

        s32 SquareIndex = Get_Square_Index(SelectedSquare.Y, SelectedSquare.X);
        Assert(SquareIndex >= 0 && SquareIndex < 64);

        move *Move = &Engine->CurrentMove;
        if (!Move->Type)
        {
            Move->Type = move_type_Move;
        }
        Move->Piece = Engine->Squares[SquareIndex];
        Move->BeginSquare = Get_Square_Index(SelectedSquare.Y, SelectedSquare.X);
        Move->EndSquare = Get_Square_Index(MoveSquare.Y, MoveSquare.X);

        b32 MoveSquaresAreDifferent = Move->BeginSquare != Move->EndSquare;
        b32 ShouldClearHoverSelectSquares = 1;

        if (Is_Valid_Square(Move->BeginSquare) && Is_Valid_Square(Move->EndSquare) && MoveSquaresAreDifferent)
        {
            /* @CopyPasta */
            b32 IsWhiteTurn = Get_Flag(TempGameState.Flags, Whose_Turn_Flag) == 0;
            b32 IsWhitePiece = Is_White_Piece(Move->Piece);
            b32 IsMoveablePiece = (IsWhitePiece && IsWhiteTurn) || !(IsWhitePiece || IsWhiteTurn);

            Move->Piece = Engine->Squares[Move->BeginSquare];
            piece TargetPiece = Engine->Squares[Move->EndSquare];

            if (Is_Valid_Piece(Move->Piece) && IsMoveablePiece)
            {
                b32 IsEnPassantCaptureMotion = (MoveSquare.X == SelectedSquare.X + 1 ||
                                                MoveSquare.X == SelectedSquare.X - 1);

                b32 EnPassantAllowed = CheckIfLastMoveAllowsEnPassant(&TempGameState, MoveSquare.X);

                b32 WhiteEnPassant = (Is_White_Turn(&TempGameState) &&
                                      SelectedSquare.Y == En_Passant_Row_White &&
                                      IsEnPassantCaptureMotion);
                b32 BlackEnPassant = (Is_Black_Turn(&TempGameState) &&
                                      SelectedSquare.Y == En_Passant_Row_Black &&
                                      IsEnPassantCaptureMotion);

                s8 LastRow = IsWhiteTurn ? 7 : 0;
                b32 PawnPromotion = 0;

                Assert(Move->Piece >= 0 && Move->Piece < piece_Count);
                if (PieceTypeTable[Move->Piece] == piece_type_Pawn &&
                    (WhiteEnPassant || BlackEnPassant) &&
                    EnPassantAllowed)
                {
                    Move->Type = move_type_EnPassant;
                }

                if (Move->Piece == piece_White_King &&
                    Move->EndSquare == C1 &&
                    CanCastle(Engine, &TempGameState, 1))
                {
                    Move->Type = move_type_QueenCastle;
                }
                else if (Move->Piece == piece_White_King &&
                         Move->EndSquare == G1 &&
                         CanCastle(Engine, &TempGameState, 0))
                {
                    Move->Type = move_type_KingCastle;
                }
                else if (Move->Piece == piece_Black_King &&
                    Move->EndSquare == C8 &&
                    CanCastle(Engine, &TempGameState, 1))
                {
                    Move->Type = move_type_QueenCastle;
                }
                else if (Move->Piece == piece_Black_King &&
                         Move->EndSquare == G8 &&
                         CanCastle(Engine, &TempGameState, 0))
                {
                    Move->Type = move_type_KingCastle;
                }
                else if (PieceTypeTable[Move->Piece] == piece_type_Pawn &&
                         MoveSquare.Y == LastRow &&
                         Ui->PawnPromotionSelectedSquare.X < 0)
                {
                    PawnPromotion = 1;
                    ShouldClearHoverSelectSquares = 0;

                    /* TODO: When the user selects a pawn-promotion piece, this code immediately
                       re-sets UserInputMove back to pawn promotion. Find a way to prevent that or
                       change the way mode-changing works.
                    */
                    if (Engine->UserInputMode != user_input_mode_PawnPromotion)
                    {
                        Engine->UserInputMode = user_input_mode_PawnPromotion;
                    }
                }

                if (!PawnPromotion)
                {
                    MakeMove(Engine, &TempGameState, *Move);

                    /* NOTE: @Copypasta AddPotential */
                    b32 WhiteWasInCheck = Get_Flag(TempGameState.Flags, White_In_Check_Flag);
                    b32 BlackWasInCheck = Get_Flag(TempGameState.Flags, Black_In_Check_Flag);
                    Unset_Flag(TempGameState.Flags, White_In_Check_Flag | Black_In_Check_Flag);
                    b32 WhiteIsInCheck = CheckIfKingIsInCheck(Engine, &TempGameState, piece_White_King);
                    b32 BlackIsInCheck = CheckIfKingIsInCheck(Engine, &TempGameState, piece_Black_King);

                    Ui->PawnPromotionSelectedSquare = (ivec2){-1, -1};

                    game_tree *Sibling = Engine->GameTreeCurrent->FirstChild;

                    /* NOTE: @Copypasta AddPotential */
                    if ((IsWhiteTurn && WhiteIsInCheck) || (!IsWhiteTurn && BlackIsInCheck))
                    {
                        /* NOTE: Don't allow moving into check by zeroing out the Sibling... */
                        Sibling = 0;
                    }

                    while (Sibling)
                    {
                        b32 AreEqual = CheckIfGameStatesAreEqual(&TempGameState, &Sibling->State);

                        if (AreEqual)
                        {
                            MakeGameTreeTheRoot(Engine, Sibling);
                            Engine->BotState = bot_state_WaitingForGenerationRoot;
                            break;
                        }

                        Sibling = Sibling->NextSibling;
                    }
                }
            }
        }

        if (ShouldClearHoverSelectSquares)
        {
            Ui->SelectedSquare = (ivec2){-1,-1};
            Ui->MoveSquare = (ivec2){-1,-1};
        }
    }
}

internal void SaveMoveHistory(engine *Engine)
{
    FILE *File = fopen("./chess_bot_save.bgs", "wb");


    if (Engine->MoveHistory.Index > 0)
    {
        Assert(Engine->MoveHistory.Index < Move_History_Size);
        fwrite(&Engine->MoveHistory, 1, sizeof(Engine->MoveHistory), File);
    }

    fclose(File);
}

internal void LoadMoveHistory(engine *Engine)
{
    FILE *File = fopen("./chess_bot_save.bgs", "rb");

    if (File)
    {
        fread(&Engine->MoveHistory, 1, sizeof(Engine->MoveHistory), File);

        if(Engine->MoveHistory.Index >= 0 && Engine->MoveHistory.Index < Move_History_Size)
        {
            for (s32 I = 0; I < Engine->MoveHistory.Index; ++I)
            {
                Engine->CurrentMove = Engine->MoveHistory.Moves[I];

                Engine->Ui.SelectedSquare = RowColFromSquare(Engine->CurrentMove.BeginSquare);
                Engine->Ui.MoveSquare = RowColFromSquare(Engine->CurrentMove.EndSquare);
                HandleMove(Engine);
            }
        }

        fclose(File);
    }
}

internal void HandleUserInput(engine *Engine)
{
    ui *Ui = &Engine->Ui;
    Ui->MousePosition = GetMousePosition();
    Ui->MousePrimaryDown = IsMouseButtonPressed(0);

    switch (Engine->UserInputMode)
    {
    case user_input_mode_TheBotsTurn:
    {
    } break;
    case user_input_mode_PawnPromotion:
    {
        ivec2 MouseSquarePosition = PositionToPawnPromotionSquarePosition(Engine, Ui->MousePosition);
        Ui->PawnPromotionHoverSquare = (ivec2){MouseSquarePosition.X, MouseSquarePosition.Y};
        b32 OnBoard = SquareValueOnPawnPromotionPicker(Ui->PawnPromotionHoverSquare.X, Ui->PawnPromotionHoverSquare.Y);

        if (Ui->MousePrimaryDown && OnBoard)
        {
            Assert(Ui->PawnPromotionHoverSquare.X >= 0 && Ui->PawnPromotionHoverSquare.X < 4);
            move_type MoveType = PawnPromotionMoveTypeLookup[Ui->PawnPromotionHoverSquare.X];

            Ui->PawnPromotionSelectedSquare = Ui->PawnPromotionHoverSquare;

            if (MoveType)
            {
                Engine->CurrentMove.Type = MoveType;
                Engine->UserInputMode = user_input_mode_TheUsersTurn;
            }
        }
    } break;
    case user_input_mode_TheUsersTurn:
    default:
    {
        Vector2 MouseSquarePosition = PositionToSquarePosition(Ui->MousePosition);
        Ui->HoverSquare = (ivec2){(int)MouseSquarePosition.x, (int)MouseSquarePosition.y};
        b32 OnBoard = SquareValueOnBoard(Ui->HoverSquare.X, Ui->HoverSquare.Y);
        game_tree *GameTreeCurrent = Engine->GameTreeCurrent;

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

        if (IsKeyPressed(KEY_SPACE))
        {
            Toggle_Flag(Engine->Flags, engine_flags_DoNotLetTheBotMakeAMove);
        }

        if (IsKeyPressed(KEY_DOWN))
        {
            if (Engine->GameTreeCurrent->NextSibling)
            {
                Engine->GameTreeCurrent = Engine->GameTreeCurrent->NextSibling;
            }
        }
        else if (IsKeyPressed(KEY_UP))
        {
            if (Engine->GameTreeCurrent->PreviousSibling)
            {
                Engine->GameTreeCurrent = Engine->GameTreeCurrent->PreviousSibling;
            }
        }
        else if (IsKeyPressed(KEY_RIGHT))
        {
            if (Engine->GameTreeCurrent->FirstChild)
            {
                Engine->GameTreeCurrent = Engine->GameTreeCurrent->FirstChild;
            }
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            if (Engine->GameTreeCurrent->Parent)
            {
                Engine->GameTreeCurrent = Engine->GameTreeCurrent->Parent;
            }
        }
    } break;
    }

    if (IsKeyPressed(KEY_S) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)))
    {
        SaveMoveHistory(Engine);
    }

    if (IsKeyPressed(KEY_L) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)))
    {
        LoadMoveHistory(Engine);
    }

    if (IsKeyPressed(KEY_TAB))
    {
        GlobalShowDebugPanel = !GlobalShowDebugPanel;
    }
}

internal void DrawPiece(engine *Engine, piece Piece, f32 X, f32 Y)
{
    ivec2 PieceTextureOffset = PIECE_TEXTURE_OFFSET[Piece];
    Color Tint = {255,212,255,255};
    Vector2 Origin = {0,0};
    Rectangle Source, Dest;

    Source = (Rectangle){PieceTextureOffset.X, PieceTextureOffset.Y, PIECE_TEXTURE_SIZE, PIECE_TEXTURE_SIZE};
    Dest = (Rectangle){X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS};
    DrawTexturePro(Engine->Ui.ChessPieceTexture, Source, Dest, Origin, 0.0f, Tint);
}

internal f32 GetGameTreeAdjustedScore(game_tree *GameTree)
{
    f32 Score;

    if (GameTree->FirstChild)
    {
        Score = GameTree->FollowingScore;
    }
    else
    {
        Score = GameTree->Score;
    }

    return Score;
}

internal void DrawHoverOrSelectedSquare(engine *Engine, s32 Row, s32 Col, f32 X, f32 Y)
{
    ui *Ui = &Engine->Ui;

    if (Ui->HoverSquare.X == Col && Ui->HoverSquare.Y == Row)
    {
        // draw outline of square if the mouse position is inside the square
        DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, BLACK);
    }

    if (Ui->SelectedSquare.X == Col && Ui->SelectedSquare.Y == Row)
    {
        Color SquareColor = UiColor[Engine->Ui.Theme][ui_color_Selected_Square];
        Color SquareColorOutline = UiColor[Engine->Ui.Theme][ui_color_Selected_Square_Outline];
        DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColor);
        DrawRectangleLines(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColorOutline);
    }
}

internal void DrawBoard(engine *Engine)
{
    ryn_BEGIN_TIMED_BLOCK(timed_block_DrawBoard);
    ui *Ui = &Engine->Ui;
    game_tree *CurrentNode = Engine->GameTreeCurrent;

    { /* NOTE: Draw board backing. */
        int BorderWidth = 6; /* @Copypasta PositionToPawnPromotionSquarePosition */
        int X = BOARD_PADDING - BorderWidth;
        int Y = BOARD_PADDING - BorderWidth;
        int Width = (BOARD_SIZE * SQUARE_SIZE_IN_PIXELS) + (2 * BorderWidth);
        int Height = (BOARD_SIZE * SQUARE_SIZE_IN_PIXELS) + (2 * BorderWidth);

        ui_color_type ColorType = ui_color_Active;
        Color SquareColor = UiColor[Engine->Ui.Theme][ColorType];

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
            Color SquareColor = UiColor[Engine->Ui.Theme][SquareColorType];

            DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, SquareColor);
            if (Engine->UserInputMode == user_input_mode_TheUsersTurn)
            {
                DrawHoverOrSelectedSquare(Engine, Row, Col, X, Y);
            }
        }
    }

    if (CurrentNode)
    {
        ClearAttackersAndDefenders(Engine);
        GetGameStateScore(Engine, &CurrentNode->State); /* NOTE: We call GetGameStateScore before drawing squares, to make sure that the Engine->AttackersAndDefenders array is populated with values for the current board. */

        { /* NOTE: Draw last move. */
            square BeginMove = Engine->GameTreeCurrent->State.LastMove.BeginSquare;
            square EndMove = Engine->GameTreeCurrent->State.LastMove.EndSquare;
            s32 Offset = SQUARE_SIZE_IN_PIXELS / 2;
            s32 BeginX = (BeginMove % 8) * SQUARE_SIZE_IN_PIXELS + BOARD_PADDING + Offset;
            s32 EndX   = (EndMove   % 8) * SQUARE_SIZE_IN_PIXELS + BOARD_PADDING + Offset;
            s32 BeginY = SCREEN_HEIGHT - (BeginMove / 8) * SQUARE_SIZE_IN_PIXELS - BOARD_PADDING - Offset;
            s32 EndY   = SCREEN_HEIGHT - (EndMove   / 8) * SQUARE_SIZE_IN_PIXELS - BOARD_PADDING - Offset;
            Color MoveColor = (Color){210,120,20,210};
            Vector2 D = V2(EndX - BeginX, EndY - BeginY);
            Vector2 Origin = AddV2(V2(BeginX, BeginY), MultiplySV2(0.5f, D));
            f32 Length = sqrt(D.x*D.x + D.y*D.y);
            Rectangle Rec = (Rectangle){Origin.x, Origin.y, Length, 10.0f};
            float RadiansToDegrees = (180.0f / 3.14159f);
            f32 DotProduct = DotV2(V2(1.0f, 0.0f), D);
            float Rotation = RadiansToDegrees * acos(DotProduct / AbsoluteValueF32(Length));
            if (D.y < 0.0f)
            {
                Rotation *= -1.0f;
            }
            DrawRectanglePro(Rec, V2(Length/2.0f, 5.0f), Rotation, MoveColor);
        }

        /* NOTE: Draw pieces. */
        for (s32 I = 0; I < piece_Count; ++I)
        {
            square Square = CurrentNode->State.Piece[I];
            b32 IsWhitePiece = Is_White_Piece(I);
            int Row = Square / 8;
            int Col = Square % 8;
            /* NOTE: @Copypasta */
            int X = (Col * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;
            int Y = (((BOARD_SIZE - 1) - Row) * SQUARE_SIZE_IN_PIXELS) + BOARD_PADDING;

#if 0
            s32 AttackAndDefenseValue = Engine->Evaluation.AttackersAndDefenders[I];
            if (AttackAndDefenseValue != 0)
            {
                Color HighlightColor = (Color){0.0f, 0.0f, 0.0f, 0.0f};
                f32 Alpha = 100.0f;
                if (AttackAndDefenseValue >= 0)
                {
                    HighlightColor = (Color){20.0f, 80.0f, 180.0f, Alpha};
                }
                else if (AttackAndDefenseValue < 0)
                {
                    HighlightColor = (Color){190.0f, 20.0f, 80.0f, Alpha};
                }

                DrawRectangle(X, Y, SQUARE_SIZE_IN_PIXELS, SQUARE_SIZE_IN_PIXELS, HighlightColor);
            }
#endif

            if (Is_Valid_Square(Square) && IsTextureReady(Ui->ChessPieceTexture))
            {
                piece Piece = I;
                u8 Promotion = CurrentNode->State.PawnPromotion[I];

                if (Promotion)
                {
                    switch (Promotion)
                    {
                    case pawn_promotion_type_Queen:  { Piece = IsWhitePiece ? piece_White_Queen : piece_Black_Queen; } break;
                    case pawn_promotion_type_Rook:   { Piece = IsWhitePiece ? piece_White_Queen_Rook : piece_Black_Queen_Rook; } break;
                    case pawn_promotion_type_Bishop: { Piece = IsWhitePiece ? piece_White_Queen_Bishop : piece_Black_Queen_Bishop; } break;
                    case pawn_promotion_type_Knight: { Piece = IsWhitePiece ? piece_White_Queen_Knight : piece_Black_Queen_Knight; } break;
                    default: break;
                    }
                }

                DrawPiece(Engine, Piece, X, Y);
            }
        }

        { /* NOTE: Draw score. */
            char Buff[128];
            Color Color = UiColor[Engine->Ui.Theme][ui_color_TextPrimary];

            sprintf(Buff, "Score: %.4f", Engine->GameTreeCurrent->Score);
            DrawText(Buff, 4.0f, 0.0f, 12.0f, Color);

            sprintf(Buff, "Worst: %.4f", Engine->GameTreeCurrent->FollowingScore);
            DrawText(Buff, 4.0f, 14.0f, 12.0f, Color);


            sprintf(Buff, "Adj_S: %.4f", GetGameTreeAdjustedScore(Engine->GameTreeCurrent));
            DrawText(Buff, 4.0f, 30.0f, 12.0f, Color);
        }

        { /* NOTE: Debug display if a player is in check/mate. */
            if (Get_Flag(CurrentNode->State.Flags, Game_Over_Flag))
            {
                b32 IsWhiteTurn = Is_White_Turn(&CurrentNode->State);
                char *Message = IsWhiteTurn ? "Black Wins!" : "White Wins!";
                DrawText(Message, 80, 20, 18, (Color){180,20,20,255});
            }
            else
            {
                b32 WhiteIsInCheck = Get_Flag(CurrentNode->State.Flags, White_In_Check_Flag);
                b32 BlackIsInCheck = Get_Flag(CurrentNode->State.Flags, Black_In_Check_Flag);
                if (WhiteIsInCheck && BlackIsInCheck)
                {
                    Debug_PrintPieces(&CurrentNode->State);
                }
                Assert(!(WhiteIsInCheck && BlackIsInCheck));

                if (WhiteIsInCheck || BlackIsInCheck)
                {
                    char Buff[256];
                    char *FormatString = WhiteIsInCheck ? "White in check %d" : "Black in check %d";
                    sprintf(Buff, FormatString, CurrentNode->State.DebugCheckPiece);
                    DrawText(Buff, 120, 2, 18, (Color){180,20,20,255});
                }
            }
        }
    }

    ryn_END_TIMED_BLOCK(timed_block_DrawBoard);
}

internal void DrawPawnPromotionBoard(engine *Engine)
{
    s32 BorderWidth = 6;

    s32 BoardWidth = (2 * BorderWidth) + (4 * SQUARE_SIZE_IN_PIXELS);
    s32 BoardHeight = (2 * BorderWidth) + SQUARE_SIZE_IN_PIXELS;

    s32 BoardX = 0.5f * (SCREEN_WIDTH - BoardWidth);
    s32 BoardY = 0.5f * (SCREEN_HEIGHT - BoardHeight);

    b32 WhoseTurn = Get_Flag(Engine->GameTreeCurrent->State.Flags, Whose_Turn_Flag);

    /* NOTE: Alpha curtain */
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 100});

    ui_color_type BackgroundColorType = ui_color_Active;
    Color BackgroundColor = UiColor[Engine->Ui.Theme][BackgroundColorType];
    DrawRectangle(BoardX, BoardY, BoardWidth, BoardHeight, BackgroundColor);

    for (s32 I = 0; I < 4; ++I)
    {
        s32 X = BoardX + BorderWidth + (I * SQUARE_SIZE_IN_PIXELS);
        s32 Y = BoardY + BorderWidth;
        s32 Width = SQUARE_SIZE_IN_PIXELS;
        s32 Height = SQUARE_SIZE_IN_PIXELS;
        b32 IsDarkSquare = (I & 1) == 0;

        ui_color_type SquareColorType = IsDarkSquare ? ui_color_Dark_Square : ui_color_Light_Square;
        Color SquareColor = UiColor[Engine->Ui.Theme][SquareColorType];

        DrawRectangle(X, Y, Width, Height, SquareColor);
        DrawHoverOrSelectedSquare(Engine, 0, I, X, Y);
        DrawPiece(Engine, PawnPromotionPieceLookup[I][WhoseTurn], X, Y);
    }
}

internal void DrawGameTree(engine *Engine)
{
    return;
    ryn_BEGIN_TIMED_BLOCK(timed_block_DrawGameTree);
    Vector2 CameraPosition = Engine->DisplayNodeCameraPosition;
    s32 Index = GetGameTreeIndexFromPointer(Engine, Engine->GameTreeCurrent);
    f32 Size = Engine->Ui.GameTreeNodeSize;

    Color ActiveColor = UiColor[Engine->Ui.Theme][ui_color_Active];
    Color InactiveColor = UiColor[Engine->Ui.Theme][ui_color_Inactive];

    b32 IndexIsInRange = Is_Game_Tree_Index_In_Range(Index);

    if (IndexIsInRange)
    {
        Vector2 Position = IndexIsInRange ? Engine->DisplayNodes[Index].Position : V2(0.0f, 0.0f);
        Vector2 DisplayNodeOffset = MultiplySV2(-1.0f, Position);
        Vector2 Offset = AddV2(DisplayNodeOffset,
                               V2( 2.0f * BOARD_PADDING + BOARD_SIZE_IN_PIXELS,
                                          0.0f ));

        { /* NOTE: Always draw the root for now... */
            Vector2 Position = AddV2(Offset, CameraPosition);
            DrawCircle(Position.x, Position.y, Size, ActiveColor);
        }

        s32 Count = 0;

        for (s32 *I = &Engine->DrawGameTreeIndex;
             *I < Game_Tree_Pool_Size && Count < 1024;
             *I = (*I + 1) % Game_Tree_Pool_Size)
        {
            display_node DisplayNode = Engine->DisplayNodes[*I];
            game_tree *GameTree = (game_tree *)Engine->GameTreeArena.Data + *I;
            b32 IsCurrentGameTree = GameTree == Engine->GameTreeCurrent;
            Count += 1;

            if (DisplayNode.Visible)
            {
                /* @Copypasta */
                Vector2 Position = AddV2(Offset, AddV2(CameraPosition, DisplayNode.Position));
                b32 PositionIsInBounds = (Position.x >= 0.0f && Position.x < SCREEN_WIDTH &&
                                          Position.y >= 0.0f && Position.y < SCREEN_HEIGHT);

                if (PositionIsInBounds)
                {
                    b32 IsCurrentNode = ((game_tree *)Engine->GameTreeArena.Data + *I) == Engine->GameTreeCurrent;
                    f32 ScoreRange = 10.0f;
                    f32 Score = GetGameTreeAdjustedScore(GameTree);
                    f32 ClampedGameStateScore = ClampF32(Score, -ScoreRange, ScoreRange);
                    f32 NodeColorValue = 255.0f * (ClampedGameStateScore + ScoreRange) / (2.0f * ScoreRange);
                    Color NodeColor = {NodeColorValue, NodeColorValue, NodeColorValue, 255};

                    if (IsCurrentGameTree)
                    {
                        /* NOTE: Draw an outline around the current game-tree. */
                        DrawCircle(Position.x, Position.y, Size + 2.0f, (Color){20, 100, 160, 255});
                    }

                    DrawCircle(Position.x, Position.y, Size, NodeColor);
                }
            }
        }
    }
    ryn_END_TIMED_BLOCK(timed_block_DrawGameTree);
}

internal void InitializeEvaluation(engine *Engine)
{
    /* f32 CenterRings[4] = {0.9f, 1.0f, 1.1f, 1.2f}; */
    f32 CenterRings[4] = {0.0f, 0.1f, 0.2f, 0.3f};

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

            Engine->Evaluation.SquareBonus[SquareA] = Bonus;
            Engine->Evaluation.SquareBonus[SquareB] = Bonus;
            Engine->Evaluation.SquareBonus[SquareC] = Bonus;
            Engine->Evaluation.SquareBonus[SquareD] = Bonus;
        }
    }

#if 0
    { /* NOTE: Debug viz. */
        for (s32 Row = 0; Row < 8; ++Row)
        {
            for (s32 Col = 0; Col < 8; ++Col)
            {
                DebugPrint("%.1f ", Engine->Evaluation.SquareBonus[Row * 8 + Col]);
            }
            DebugPrint("\n");
        }
    }
#endif
}

internal void SwapGameTreeSiblings(game_tree *NodeA, game_tree *NodeB)
{
    Assert(NodeA != 0 && NodeB != 0);
    Assert(NodeA != NodeB);
    Assert(NodeA->NextSibling == NodeB);
    Assert(NodeB->PreviousSibling == NodeA);

    game_tree *FirstNode = NodeA->PreviousSibling;
    game_tree *LastNode = NodeB->NextSibling;

    NodeB->NextSibling = NodeA;
    NodeA->PreviousSibling = NodeB;

    NodeB->PreviousSibling = FirstNode;
    NodeA->NextSibling = LastNode;

    if (FirstNode)
    {
        FirstNode->NextSibling = NodeB;
    }

    if (LastNode)
    {
        LastNode->PreviousSibling = NodeA;
    }

    if (NodeA->Parent && NodeA->Parent->FirstChild == NodeA)
    {
        NodeA->Parent->FirstChild = NodeB;
    }
}

#define Is_Traversable(traversal, index) (Is_Game_Tree_Index_In_Range(index) && \
                                          !Check_If_Traversal_Node_Is_Visited((traversal)->Nodes, index))

internal s32 SortGameTreeChildren(engine *Engine, game_tree *GameTree)
{
    Assert(GameTree->FirstChild != 0);
    b32 SortingOccured = 0;
    b32 SortDirection = 0;
    s32 SwapCount = 0;

    game_tree *CurrentNode = GameTree->FirstChild;
    game_tree *PreviousNode;
    game_tree *NextNode;
    game_tree *NewCurrentNodeForNextIteration;

    for (;;)
    {
        if (SortDirection)
        {
            NextNode = CurrentNode;
            PreviousNode = CurrentNode->PreviousSibling;
            NewCurrentNodeForNextIteration = PreviousNode;
        }
        else
        {
            NextNode = CurrentNode->NextSibling;
            PreviousNode = CurrentNode;
            NewCurrentNodeForNextIteration = NextNode;
        }

        if (NextNode == 0 || PreviousNode == 0)
        {
            if (!SortingOccured)
            {
                break;
            }

            SortDirection = !SortDirection;
            SortingOccured = 0;
            NewCurrentNodeForNextIteration = CurrentNode;
        }
        else
        {
            f32 PreviousScore = GetGameTreeAdjustedScore(PreviousNode);
            f32 NextScore = GetGameTreeAdjustedScore(NextNode);

            b32 CurrentIsBest = CheckIfCurrentIsBestScore(Engine, &GameTree->State, PreviousScore, NextScore);

            if (CurrentIsBest)
            {
                SortingOccured = 1;
                SwapCount += 1;
                SwapGameTreeSiblings(PreviousNode, NextNode);
            }
        }

        CurrentNode = NewCurrentNodeForNextIteration;
    }

#if 1
    { /* NOTE: Check that things are sorted. */
        game_tree *DebugNode = GameTree->FirstChild;
        Assert(DebugNode != 0);
        while (DebugNode->NextSibling)
        {
            f32 CurrentScore = GetGameTreeAdjustedScore(DebugNode);
            f32 NextScore = GetGameTreeAdjustedScore(DebugNode->NextSibling);
            b32 CurrentIsBest = CheckIfCurrentIsBestScore(Engine, &DebugNode->State, CurrentScore, NextScore);
            Assert(CurrentScore == NextScore || CurrentIsBest != 0);
            DebugNode = DebugNode->NextSibling;
        }
    }
#endif

    return SwapCount;
}

internal b32 CheckIfItsTheBotsTurn(engine *Engine, game_state *GameState)
{
    b32 WhetherItsTheBotsTurn = 0;
    Assert(GameState != 0);

    WhetherItsTheBotsTurn = (Get_Flag(GameState->Flags, Whose_Turn_Flag) !=
                             Get_Flag(Engine->Flags, engine_flags_IsBotPlayingAsWhite));

    return WhetherItsTheBotsTurn;
}

internal void IncrementallySortGameTree(engine *Engine)
{
    ryn_BEGIN_TIMED_BLOCK(timed_block_IncrementallySortGameTree);
    if (!Engine->RootGameTree)
    {
        return;
    }

    traversal *Traversal = &Engine->SortTraversal;
    game_tree *CurrentRoot = GetRootGameTree(Traversal->CurrentNode);

    if (CurrentRoot != 0 && CurrentRoot == Engine->FreeGameTree)
    {
        Traversal->CurrentNode = Engine->RootGameTree;
        ClearTraversals(Engine, Engine->SortTraversal.Nodes);
        Engine->SortCount = 1;
        Engine->TemporaryTickCount = 0;
    }
    else if (Traversal->CurrentNode == 0)
    {
        Traversal->CurrentNode = Engine->RootGameTree;
        ClearTraversals(Engine, Engine->SortTraversal.Nodes);

        /* TODO: Really we should also wait on GenerateAllPotentials to be complete, but for now we just wait a few ticks. */
        if (CheckIfItsTheBotsTurn(Engine, &Engine->RootGameTree->State) &&
            Engine->SortCount == 0 &&
            Engine->TemporaryTickCount > 16)
        {
            if (Engine->BotState == bot_state_WaitingForGenerationRoot)
            {
                Engine->BotState = bot_state_WaitingForSorting;
            }
            else if (Engine->BotState == bot_state_WaitingForSorting)
            {
                Engine->BotState = bot_state_ReadyToMove;
            }

            Engine->TemporaryTickCount = 0;
        }
        else
        {
            Engine->TemporaryTickCount += 1;
        }

        Engine->SortCount = 0;
    }

    s32 MaxTraversalCount = 24*1024;
    s32 TraversalCount = 0;
    s32 CurrentNodeIndex = Get_Game_Tree_Index_From_Pointer(Engine, Traversal->CurrentNode);

    while (Traversal->CurrentNode && TraversalCount < MaxTraversalCount)
    {
            ryn_BEGIN_TIMED_BLOCK(timed_block_IncrementallySortGameTreeProbe);
        s32 CurrentNodeIndex = Get_Game_Tree_Index_From_Pointer(Engine, Traversal->CurrentNode);
        s32 FirstChildIndex = Get_Game_Tree_Index_From_Pointer(Engine, Traversal->CurrentNode->FirstChild);
        s32 NextSiblingIndex = Get_Game_Tree_Index_From_Pointer(Engine, Traversal->CurrentNode->NextSibling);

        TraversalCount += 1;
            ryn_END_TIMED_BLOCK(timed_block_IncrementallySortGameTreeProbe);

        if (Is_Traversable(Traversal, FirstChildIndex))
        {
            b32 IsTheBotsTurnAtNode = CheckIfItsTheBotsTurn(Engine, &Traversal->CurrentNode->State);
            b32 SortingOccured = SortGameTreeChildren(Engine, Traversal->CurrentNode);

            if (SortingOccured)
            {
                Engine->SortCount += 1;
            }

            {
                f32 FirstChildScore = GetGameTreeAdjustedScore(Traversal->CurrentNode->FirstChild);
                Traversal->CurrentNode->FollowingScore = FirstChildScore;
            }

            Traversal->CurrentNode = Traversal->CurrentNode->FirstChild;
        }
        else if (Is_Traversable(Traversal, NextSiblingIndex))
        {
            Traversal->CurrentNode = Traversal->CurrentNode->NextSibling;
            SetTraversalAsVisited(Engine->SortTraversal.Nodes, CurrentNodeIndex);
        }
        else
        {
            Traversal->CurrentNode = Traversal->CurrentNode->Parent;
            SetTraversalAsVisited(Engine->SortTraversal.Nodes, CurrentNodeIndex);
        }
    }

    { /* TODO: delete this debug code */
        char Buff[64];
        sprintf(Buff, "Sort iter %d", TraversalCount);
        DrawText(Buff, Panel_Position_X, 4, 18, (Color){0,0,0,255});
        sprintf(Buff, "Sort count %llu", Engine->SortCount);
        DrawText(Buff, Panel_Position_X - 150, 4, 18, (Color){0,0,0,255});
    }
    ryn_END_TIMED_BLOCK(timed_block_IncrementallySortGameTree);
}

internal void UpdateDisplayNodes(engine *Engine)
{
    ryn_BEGIN_TIMED_BLOCK(timed_block_UpdateDisplayNodes);
    Vector2 *Position = &Engine->UpdateCurrentPosition;
    f32 NodeSizePlusPadding = Engine->Ui.GameTreeNodeSize + Engine->Ui.GameTreeNodePadding;
    traversal *Traversal = &Engine->UpdateTraversal;
    game_tree *CurrentRoot = GetRootGameTree(Traversal->CurrentNode);

    if (CurrentRoot == Engine->FreeGameTree || Traversal->CurrentNode == 0)
    {
        Traversal->CurrentNode = Engine->RootGameTree;
        ClearTraversals(Engine, Engine->UpdateTraversal.Nodes);
        Engine->UpdateCurrentPosition = V2(0.0f, 0.0f);
    }

    s32 MaxTraversalCount = 1024;
    s32 TraversalCount = 0;

    while (Traversal->CurrentNode && TraversalCount < MaxTraversalCount)
    {
        s32 CurrentNodeIndex = GetGameTreeIndexFromPointer(Engine, Traversal->CurrentNode);

        TraversalCount += 1;

        traversal_result FirstChildTraversal = Old_TraverseFirstChild(Engine, Engine->UpdateTraversal.Nodes, Traversal->CurrentNode);
        traversal_result NextSiblingTraversal = Old_TraverseNextSibling(Engine, Traversal->CurrentNode);

        display_node *FirstChildDisplay = Engine->DisplayNodes + FirstChildTraversal.GameTreeIndex;
        display_node *NextSiblingDisplay = Engine->DisplayNodes + NextSiblingTraversal.GameTreeIndex;

        if (Is_Game_Tree_Index_In_Range(FirstChildTraversal.GameTreeIndex))
        {
            Traversal->CurrentNode = Traversal->CurrentNode->FirstChild;
            Position->x += NodeSizePlusPadding;

            FirstChildDisplay->Position = *Position;
            FirstChildDisplay->Visible = 1;
        }
        else if (Is_Game_Tree_Index_In_Range(NextSiblingTraversal.GameTreeIndex))
        {
            Traversal->CurrentNode = Traversal->CurrentNode->NextSibling;
            Position->y += NodeSizePlusPadding;

            NextSiblingDisplay->Position = *Position;
            NextSiblingDisplay->Visible = 1;

            SetTraversalAsVisited(Engine->UpdateTraversal.Nodes, CurrentNodeIndex);
        }
        else
        {
            Traversal->CurrentNode = Traversal->CurrentNode->Parent;

            if (Traversal->CurrentNode != 0)
            {
                Position->y += NodeSizePlusPadding;
                Position->x -= NodeSizePlusPadding;

                SetTraversalAsVisited(Engine->UpdateTraversal.Nodes, CurrentNodeIndex);
            }
        }
    }
    ryn_END_TIMED_BLOCK(timed_block_UpdateDisplayNodes);

    { /* TODO: delete this debug code */
        char Buff[64];
        sprintf(Buff, "Update count %d", TraversalCount);
        DrawText(Buff, Panel_Position_X, 24, 18, (Color){0,0,0,255});
    }
}

global_variable u64 GlobalEstimatedCpuFrequency = 0;

internal void DebugDrawProfile(void)
{
    char Buff[1024];
    Color TextColor = (Color){255, 255, 255, 255};
    Vector2 TextPosition = V2( 10.0f, 10.0f );
    f32 FontSize = 16.0;
    f32 LineHeight = FontSize + 4.0f;

    ryn_profiler Profiler = GlobalDebugPanelIsPaused ? DebugProfilerForPause : ryn_GlobalProfiler;

    uint64_t TotalElapsedTime = Profiler.EndTime - Profiler.StartTime;

    DrawRectangle(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 100});

    if(GlobalEstimatedCpuFrequency)
    {
        float TotalElapsedTimeInMs = 1000.0 * (double)TotalElapsedTime / (double)GlobalEstimatedCpuFrequency;
        sprintf(Buff, "Total time: %0.4fms (CPU freq %llu)", TotalElapsedTimeInMs, GlobalEstimatedCpuFrequency);
        DrawText(Buff, TextPosition.x, TextPosition.y, FontSize, TextColor);
        TextPosition.y += LineHeight;
    }

    for(uint32_t TimerIndex = 0; TimerIndex < ArrayCount(Profiler.Timers); ++TimerIndex)
    {
        ryn_timer_data *Timer = Profiler.Timers + TimerIndex;
        if(Timer->ElapsedInclusive)
        {
            double Megabyte = 1024.0f*1024.0f;
            double Gigabyte = Megabyte*1024.0f;

            double Percent = 100.0 * ((double)Timer->ElapsedExclusive / (double)TotalElapsedTime);
            double PercentWithChildren = 100.0 * ((double)Timer->ElapsedInclusive / (double)TotalElapsedTime);

            double Seconds = (double)Timer->ElapsedInclusive / (double)GlobalEstimatedCpuFrequency;
            double BytesPerSecond = (double)Timer->ProcessedByteCount / Seconds;
            double Megabytes = (double)Timer->ProcessedByteCount / (double)Megabyte;
            double GigabytesPerSecond = BytesPerSecond / Gigabyte;

            b32 HasChildren = Timer->ElapsedInclusive != Timer->ElapsedExclusive;
            b32 HasByteCount = Timer->ProcessedByteCount;

            char *FormatString = "";

            if (!HasChildren && !HasByteCount)
            {
                sprintf(Buff, "    %s[%llu]: %llu (%.2f%%)", Timer->Label, Timer->HitCount, Timer->ElapsedExclusive, Percent);
            }
            else if (HasChildren && !HasByteCount)
            {
                sprintf(Buff, "    %s[%llu]: %llu (%.2f%%, %.2f%% w/children)", Timer->Label, Timer->HitCount, Timer->ElapsedExclusive, Percent, PercentWithChildren);
            }
            else if (!HasChildren && HasByteCount)
            {
                sprintf(Buff, "    %s[%llu]: %llu (%.2f%%)  %.3fmb at %.2fgb/s", Timer->Label, Timer->HitCount, Timer->ElapsedExclusive, Percent, Megabytes, GigabytesPerSecond);
            }
            else if (HasChildren && HasByteCount)
            {
                sprintf(Buff, "    %s[%llu]: %llu (%.2f%%, %.2f%% w/children)  %.3fmb at %.2fgb/s", Timer->Label, Timer->HitCount, Timer->ElapsedExclusive, Percent, PercentWithChildren, Megabytes, GigabytesPerSecond);
            }

            DrawText(Buff, TextPosition.x, TextPosition.y, FontSize, TextColor);
            TextPosition.y += LineHeight;
        }
    }
}

internal void DebugDrawFreeTreeCount(engine *Engine)
{
    game_tree *CurrentNode = Engine->FreeGameTree;
    s32 Count = 0;

    ClearTraversals(Engine, Engine->DebugTraversalNodes);

    while (CurrentNode)
    {
        Assert(CurrentNode != Engine->RootGameTree);
        Count += 1;
        s32 FirstChildIndex;
        s32 NextSiblingIndex;

        s32 CurrentNodeIndex = GetGameTreeIndexFromPointer(Engine, CurrentNode);

        traversal_result FirstChildTraversal = Old_TraverseFirstChild(Engine, Engine->DebugTraversalNodes, CurrentNode);
        traversal_result NextSiblingTraversal = Old_TraverseNextSibling(Engine, CurrentNode);

        if (Is_Game_Tree_Index_In_Range(FirstChildTraversal.GameTreeIndex))
        {
            CurrentNode = CurrentNode->FirstChild;
        }
        else if (Is_Game_Tree_Index_In_Range(NextSiblingTraversal.GameTreeIndex))
        {
            CurrentNode = CurrentNode->NextSibling;
            SetTraversalAsVisited(Engine->DebugTraversalNodes, CurrentNodeIndex);
        }
        else if (CurrentNode->Parent && CurrentNode->Parent != Engine->RootGameTree)
        {
            CurrentNode = CurrentNode->Parent;
            SetTraversalAsVisited(Engine->DebugTraversalNodes, CurrentNodeIndex);
        }
        else
        {
            break;
        }
    }

    {
        char Buff[64];
        sprintf(Buff, "Free Tree %d", Count);
        DrawText(Buff, Panel_Position_X, 46, 18, (Color){0,0,0,255});
    }
}

internal void InitEngine(engine **EngineReference)
{
    ryn_memory_arena EngineArena = ryn_memory_CreateArena(sizeof(engine));
    *EngineReference = (engine *)EngineArena.Data;
    engine *Engine = *EngineReference;

    Engine->SortCount = 1; /* TODO: This feels a little hacky. */

    Engine->GameTreeArena = ryn_memory_CreateArena(Game_Tree_Pool_Size*sizeof(game_tree));

    Engine->RootGameTree = CreateGameTree(Engine);
    Engine->UserInputMode = user_input_mode_TheUsersTurn;

    InitializeGameState(&Engine->RootGameTree->State);
    InitializeSquares(Engine->Squares, &Engine->RootGameTree->State);
    InitializeEvaluation(Engine);

#if 0
    SetupForTesting(Engine, &Engine->RootGameTree->State);
#endif

    Engine->Ui.ChessPieceTexture = LoadTexture("./assets/chess_pieces.png");
    Engine->Ui.Theme = DEFAULT_THEME;
    Engine->Ui.GameTreeNodeSize = 4.0f;
    Engine->Ui.GameTreeNodePadding = 2.0f;

    Engine->GameTreeCurrent = Engine->RootGameTree;
    GenerateAllPotentials(Engine); /* TODO: This probably doesn't need to be run here... */
    ClearTraversals(Engine, Engine->DebugTraversalNodes);
    ClearDisplayNodes(Engine);
    Engine->GameTreeCurrent = Engine->RootGameTree;
    Engine->DisplayNodeCameraPosition = V2(30.0f, 30.0f);
}

/* TODO: Put these in a struct. */
global_variable PyObject *ReturnANumberFunction;

internal b32 InitPython(void)
{
    /* TODO: Make a struct out of the global references to python objects. */
    b32 Success = 1;
    ryn_InitializePython();

    PyObject *ChessBotModule = PyImport_ImportModule("chess_bot");
    if (!ChessBotModule)
    {
        DebugPrint("Error(Python) importing chess_bot module\n");
        goto end_of_python_init;
    }

    ReturnANumberFunction = PyObject_GetAttrString(ChessBotModule, "return_a_number");
    if (!ReturnANumberFunction)
    {
        DebugPrint("Error(Python) getting the functions 'return_a_number' from the chess_bot module.\n");
        goto end_of_python_init;
    }

end_of_python_init:
    return Success;
}

int main(void)
{
    DebugPrint("sizeof(engine) %lu\n", sizeof(engine));
    DebugPrint("sizeof(game_tree) %lu\n", sizeof(game_tree));

    GlobalEstimatedCpuFrequency = ryn_EstimateCpuFrequency();

    InitPython();

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CHESS BOT");
    SetWindowPosition(0, 0);
    SetTargetFPS(TARGET_FPS);

    if (!IsWindowReady())
    {
        DebugPrint("Error: Window not ready\n");
        return 1;
    }

    engine *Engine = 0;
    InitEngine(&Engine);

    b32 ShouldCopyDebugProfilerForPause = 0;

    while (!WindowShouldClose())
    {
        ryn_BeginProfile();
        ryn_BEGIN_TIMED_BLOCK(timed_block_Main);

        b32 IsTheBotsTurn = CheckIfItsTheBotsTurn(Engine, &Engine->GameTreeCurrent->State);

        ryn_BEGIN_TIMED_BLOCK(timed_block_HandleInputAndMove);
        HandleUserInput(Engine);
        if (GlobalShowDebugPanel)
        {
            if (IsKeyPressed(KEY_P))
            {
                GlobalDebugPanelIsPaused = !GlobalDebugPanelIsPaused;

                if (GlobalDebugPanelIsPaused)
                {
                    ShouldCopyDebugProfilerForPause = 1;
                }
            }
        }
        else
        {
            if (!Get_Flag(Engine->Flags, engine_flags_DoNotLetTheBotMakeAMove) && IsTheBotsTurn)
            {
                LetTheBotMakeAMove(Engine);
            }
            else
            {
                HandleMove(Engine);
            }
        }
        ryn_END_TIMED_BLOCK(timed_block_HandleInputAndMove);

        ryn_BEGIN_TIMED_BLOCK(timed_block_BeginAndClear);
        BeginDrawing();
        ClearBackground(UiColor[Engine->Ui.Theme][ui_color_Background]);
        ryn_END_TIMED_BLOCK(timed_block_BeginAndClear);

        if (Has_Free_Game_Tree(Engine))
        {
            GenerateAllPotentials(Engine);
        }

        UpdateDisplayNodes(Engine);
        IncrementallySortGameTree(Engine);

        DrawBoard(Engine);
        /* fen Fen = GameStateToFen(&Engine.RootGameTree->State); */
        /* DrawGameTree(Engine); */
        { /* NOTE: Draw if the bot is currently allowed to make moves. */
            b32 BotCanNotMakeAMove = Get_Flag(Engine->Flags, engine_flags_DoNotLetTheBotMakeAMove);
            char *Message = BotCanNotMakeAMove ? "Bot paused" : "Bot is playing";
            s32 TextWidth = MeasureText(Message, 16.0f);
            Color TextColor = BotCanNotMakeAMove ? (Color){26, 40, 26, 255}: (Color){20, 90, 20, 255};
            DrawText(Message, 0.5f*(SCREEN_WIDTH - TextWidth), 4.0f, 16.0f, TextColor);
        }
        {
             char *Message = IsTheBotsTurn ? "Bot's turn" : "Your turn";
             s32 TextWidth = MeasureText(Message, 16.0f);

             DrawText(Message, 0.5f*(SCREEN_WIDTH - TextWidth), 22.0f, 16.0f, (Color){20, 20, 20, 255});
        }

        if (Engine->UserInputMode == user_input_mode_PawnPromotion)
        {
            DrawPawnPromotionBoard(Engine);
        }
#if 0
        /* NOTE: We already eat traversal-costs checking if tree counts are conserved, so eating more traversal-cost here just to draw a number seems silly.
           Maybe what we can do is accumulate the tree count was we traverse the free-tree, and update the count everytime we restart the traversal.
        */
        DebugDrawFreeTreeCount(Engine);
#endif


#if 0
        {
            ryn_BEGIN_TIMED_BLOCK(timed_block_PythonRoundTrip);
            if (PyCallable_Check(ReturnANumberFunction))
            {
                /* TODO: Do we need to dec_ref "ANumber", the return value of ReturnANumberFunction? */
                PyObject *ANumber = PyObject_CallNoArgs(ReturnANumberFunction);

                if (PyFloat_Check(ANumber))
                {
                    double Value = PyFloat_AsDouble(ANumber);
                }
                else
                {
                    printf("That ain't a float... %p\n", ANumber);
                }
            }
            ryn_END_TIMED_BLOCK(timed_block_PythonRoundTrip);
        }
#endif

        ryn_END_TIMED_BLOCK(timed_block_Main);
        ryn_EndProfile();

        if (GlobalShowDebugPanel)
        {
            DebugDrawProfile();

            if (ShouldCopyDebugProfilerForPause)
            {
                DebugProfilerForPause = ryn_GlobalProfiler;
                ShouldCopyDebugProfilerForPause = 0;
            }
        }
        { /* NOTE: Clear profile timers. */
            for(uint32_t TimerIndex = 0; TimerIndex < ArrayCount(ryn_GlobalProfiler.Timers); ++TimerIndex)
            {
                ryn_timer_data *Timer = ryn_GlobalProfiler.Timers + TimerIndex;
                Timer->ElapsedExclusive = 0;
                Timer->ElapsedInclusive = 0;
                Timer->HitCount = 0;
                Timer->ProcessedByteCount = 0;
            }
        }

        { /* NOTE: Debug viz */
            char Buff[64];
            sprintf(Buff, "Node pool index %llu", Engine->GameTreeArena.Offset/sizeof(game_tree));
            DrawText(Buff, Panel_Position_X, 66, 18, (Color){0,0,0,255});
            sprintf(Buff, "Node pool size %d", Game_Tree_Pool_Size);
            DrawText(Buff, Panel_Position_X, 88, 18, (Color){0,0,0,255});
        }

        Debug_CheckTheCountsOfTheTrees(Engine);
        Debug_CheckThatTreeDoesNotContainNode(Engine, Engine->FreeGameTree, Engine->RootGameTree);

        EndDrawing();
    }


    CloseWindow();

    DebugPrint("sizeof(engine) %lu\n", sizeof(engine));
    DebugPrint("sizeof(game_tree) %lu\n", sizeof(game_tree));
    DebugPrint("sizeof(Engine->SortTraversal) %lu\n", sizeof(Engine->SortTraversal));

    return 0;
}
