#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint32_t b32;
typedef int32_t s32;

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
#define Is_White_Piece(p) (Piece_Color(p) == 0)
#define Is_Black_Piece(p) (Piece_Color(p) == 1)

#define Is_Valid_Piece(p) ((p) >= (piece_White_Queen_Rook) && (p) < piece_Count)
#define Is_Valid_Square(s) ((s) >= 0 && (s) < 64)

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

#define Flag_Get(  flags, flag) (((flags) &  (flag)) == 1)
#define Flag_Set(  flags, flag)  ((flags) & ~(flag) | flag)
#define Flag_Unset(flags, flag)  ((flags) & ~(flag))

#define Is_White_Turn(v) ((v) == 0)
#define Is_Black_Turn(v) ((v) == 1)

typedef struct
{
    u8 Piece[piece_Count];
    b32 Flags;
} game_state;

#define Max_Potential_Moves \
    (8 * 4 + 2 * 14 + 2 * 8  + 2 * 14 + 28    + 8)
/*   Pawns   Rooks    Knights  Bishops  Queen   King*/

typedef struct
{
    piece Piece;
    square Square;
} move;

typedef struct
{
    move Potentials[Max_Potential_Moves];
    s32 PotentialCount;

    u8 Squares[64];
} move_data;

typedef struct
{
} move_result;

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

internal void AddPotential(move_data *MoveData, piece Piece, square Square)
{
    Assert(Is_Valid_Piece(Piece));
    Assert(Is_Valid_Square(Square));

    if (MoveData->PotentialCount < Max_Potential_Moves)
    {
        MoveData->Potentials[MoveData->PotentialCount].Piece = Piece;
        MoveData->Potentials[MoveData->PotentialCount].Square = Square;
        ++MoveData->PotentialCount;
    }
}

internal void GeneratePotentials(move_data *MoveData, game_state *GameState)
{
    MoveData->PotentialCount = 0;

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
        u8 PieceColor = Get_Piece_Color(I);

        if (Is_Valid_Square(Square))
        {
            switch (PieceType[I])
            {
            case piece_type_Rook:
            {
                /* Look left */
                for (s32 J = Col - 1; J >= 0; --J)
                {
                    s32 NewSquare = Row * 8 + J;
                    s32 TargetPiece = MoveData->Squares[NewSquare];

                    if (Is_Valid_Piece(TargetPiece))
                    {
                        if (Get_Piece_Color(TargetPiece) != PieceColor)
                        {
                            AddPotential(MoveData, Piece, NewSquare);
                        }

                        break;
                    }
                    else
                    {
                        AddPotential(MoveData, Piece, NewSquare);
                    }
                }
                /* Look up */
                /* Look right */
                /* Look down */
            } break;
            case piece_type_Knight:
            {
            } break;
            case piece_type_Bishop:
            {
            } break;
            case piece_type_Queen:
            {
            } break;
            case piece_type_King:
            {
            } break;
            case piece_type_Pawn:
            {
            } break;
            default: break;
            }
        }
    }
}

internal void InitializeGameState(game_state *GameState)
{
    GameState->Flags = 0;

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

internal void CopyGameState(game_state *Source, game_state *Destination)
{
    Destination->Flags = Source->Flags;

    for (s32 I = 0; I < piece_Count; ++I)
    {
        Destination->Piece[I] = Source->Piece[I];
    }
}

internal void MakeMove(move_data *MoveData, game_state *GameState, move Move)
{
    piece Piece = Move.Piece;
    square Square = Move.Square;

    MoveData->Squares[GameState->Piece[Piece]] = piece_Null;

    GameState->Piece[Piece] = Square;
    MoveData->Squares[Square] = Piece;
}

int main(void)
{
    int Result = 0;

    game_state GameState;
    move_data MoveData = {0};

    InitializeGameState(&GameState);
    InitializeSquares(MoveData.Squares, &GameState);

    move Move;
    Move.Piece = piece_White_Queen_Rook;
    Move.Square = E4;
    MakeMove(&MoveData, &GameState, Move);
    DebugPrintSquares(MoveData.Squares);

#if 1
    GeneratePotentials(&MoveData, &GameState);

    for (s32 I = 0; I < MoveData.PotentialCount; ++I)
    {
        game_state PotentialState = {0};
        move Move = MoveData.Potentials[I];

        CopyGameState(&GameState, &PotentialState);
        InitializeSquares(MoveData.Squares, &PotentialState);
        MakeMove(&MoveData, &PotentialState, Move);
        DebugPrintSquares(MoveData.Squares);
    }
#endif

    return Result;
}
