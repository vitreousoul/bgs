typedef struct
{
    int X;
    int Y;
} ivec2;

typedef struct
{
    Vector2 MousePosition;
    int MousePrimaryDown;
    ivec2 HoverSquare;
    ivec2 SelectedSquare;
    ivec2 MoveSquare;
    Texture2D ChessPieceTexture;
} app_state;
