"""
Take a game-state and produce possible moves!
"""

MIN_BOARD_INDEX = 0
MAX_BOARD_INDEX = 63

BOARD_SIZE = 8
SQUARE_COUNT = BOARD_SIZE * BOARD_SIZE

# directional 2d vectors
MOVE_LEFT_DOWN  = (-1, -1)
MOVE_DOWN       = ( 0, -1)
MOVE_RIGHT_DOWN = ( 1, -1)
MOVE_RIGHT      = ( 1,  0)
MOVE_RIGHT_UP   = ( 1,  1)
MOVE_UP         = ( 0,  1)
MOVE_LEFT_UP    = (-1,  1)
MOVE_LEFT       = (-1,  0)

KNIGHT_MOVE_DIRECTIONS = [
    (-2, -1),
    (-1, -2),
    ( 1, -2),
    ( 2, -1),
    ( 2,  1),
    ( 1,  2),
    (-1,  2),
    (-2,  1),
]

QUEEN_DISTANCE = BOARD_SIZE - 1
QUEEN_DIRECTIONS = [
    MOVE_LEFT_DOWN,
    MOVE_DOWN,
    MOVE_RIGHT_DOWN,
    MOVE_RIGHT,
    MOVE_RIGHT_UP,
    MOVE_UP,
    MOVE_LEFT_UP,
    MOVE_LEFT,
]

KING_DISTANCE = 1
KING_DIRECTIONS = [
    MOVE_LEFT_DOWN,
    MOVE_DOWN,
    MOVE_RIGHT_DOWN,
    MOVE_RIGHT,
    MOVE_RIGHT_UP,
    MOVE_UP,
    MOVE_LEFT_UP,
    MOVE_LEFT,
]

BISHOP_DISTANCE = BOARD_SIZE - 1
BISHOP_DIRECTIONS = [
    MOVE_LEFT_DOWN,
    MOVE_RIGHT_DOWN,
    MOVE_RIGHT_UP,
    MOVE_LEFT_UP,
]

ROOK_DISTANCE = BOARD_SIZE - 1
ROOK_DIRECTIONS = [
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_LEFT,
]

VALID_PIECE_VALUES = {
    'p': True, 'r': True, 'n': True, 'b': True, 'q': True, 'k': True,
    'P': True, 'R': True, 'N': True, 'B': True, 'Q': True, 'K': True,
}

def create_move_tables():
    result = {
        'queen':  create_ray_move_table(QUEEN_DIRECTIONS,  QUEEN_DISTANCE),
        'king':   create_ray_move_table(KING_DIRECTIONS,   KING_DISTANCE),
        'bishop': create_ray_move_table(BISHOP_DIRECTIONS, BISHOP_DISTANCE),
        'rook':   create_ray_move_table(ROOK_DIRECTIONS,   ROOK_DISTANCE),
        'knight': create_knight_move_table()
    }
    return result

def create_knight_move_table():
    result = []
    for y in range(BOARD_SIZE):
        for x in range(BOARD_SIZE):
            moves = []
            for delta_x, delta_y in KNIGHT_MOVE_DIRECTIONS:
                current_x = x + delta_x
                current_y = y + delta_y
                if x_y_in_range(current_x, current_y):
                    moves.append(x_y_to_index(current_x, current_y))
            result.append(moves)
    return result

def create_ray_move_table(DIRECTIONS, DISTANCE):
    result = [
        generate_move_rays((x, y), DIRECTIONS, DISTANCE)
        for y in range(BOARD_SIZE)
        for x in range(BOARD_SIZE)
    ]
    return result

"""
We call it move "ray-casting" because we define the potential moves as
squares eminating from the piece location. Having potential squares defined
this way means we can loop through them, and when we hit a piece or board-edge,
we can stop there.
"""
def generate_move_rays(target_square, ray_directions, distance):
    move_rays = []
    target_x, target_y = target_square
    for ray_x, ray_y in ray_directions:
        current_x = target_x
        current_y = target_y
        current_distance = 0
        while True:
            current_x += ray_x
            current_y += ray_y
            square_index = x_y_to_index(current_x, current_y)
            current_distance += 1
            in_range = x_y_in_range(current_x, current_y)
            piece_can_move_this_far = current_distance <= distance
            if piece_can_move_this_far and in_range:
                move_rays.append(square_index)
            else:
                break
    return move_rays

def x_y_in_range(x, y):
    x_in_range = x >= 0 and x < BOARD_SIZE
    y_in_range = y >= 0 and y < BOARD_SIZE
    return x_in_range and y_in_range

def x_y_to_index(x, y):
    return y * BOARD_SIZE + x

def index_to_x_y(square_index):
    x = square_index % BOARD_SIZE
    y = square_index // BOARD_SIZE
    return x, y

def get_possible_moves_from_table(moves_table, piece_value, square_index):
    result = []
    for possible_move_index in moves_table[square_index]:
        result.append((square_index, possible_move_index))
    return result

def get_possible_moves_for_pawn(game_state, piece_value, square_index):
    pawn_direction_y = 1 if piece_value == 'P' else -1
    square_x, square_y = index_to_x_y(square_index)
    two_square_possible = (square_y == 1) if piece_value == 'P' else (square_y == 6)
    left_up = (square_x - 1, square_y + pawn_direction_y)
    up = (square_x, square_y + pawn_direction_y)
    two_up = (square_x, square_y + (2 *pawn_direction_y))
    right_up = (square_x + 1, square_y + pawn_direction_y)
    possible_moves = []
    for (x, y) in [left_up, up, two_up, right_up]:
        if x_y_in_range(x, y):
            index = x_y_to_index(x, y)
            possible_moves.append((square_index, index))
    return possible_moves

def get_possible_moves(game_state):
    move_tables = create_move_tables()
    possible_moves = []
    for square_index in range(SQUARE_COUNT):
        piece_value = game_state['board'][square_index]
        if piece_value in VALID_PIECE_VALUES:
            if   piece_value == 'p' or piece_value == 'P':
                possible_moves += get_possible_moves_for_pawn(game_state, piece_value, square_index)
            elif piece_value == 'n' or piece_value == 'N':
                possible_moves += get_possible_moves_from_table(move_tables['knight'], piece_value, square_index)
            elif piece_value == 'r' or piece_value == 'R':
                possible_moves += get_possible_moves_from_table(move_tables['rook'],   piece_value, square_index)
            elif piece_value == 'b' or piece_value == 'B':
                possible_moves += get_possible_moves_from_table(move_tables['bishop'], piece_value, square_index)
            elif piece_value == 'q' or piece_value == 'Q':
                possible_moves += get_possible_moves_from_table(move_tables['queen'],  piece_value, square_index)
            elif piece_value == 'k' or piece_value == 'K':
                possible_moves += get_possible_moves_from_table(move_tables['king'],   piece_value, square_index)
    return possible_moves

def test(arg_object):
    result_code = 0
    game_state = {
        'en_passant': -1,
        'castle': {
            'white': { 'queenside': True, 'kingside': True },
            'black': { 'queenside': True, 'kingside': True }
        },
        'board': [
            'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R',
            'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
            'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
            'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
        ]
    }
    possible_moves = get_possible_moves(game_state)
    if 'verbose' in arg_object:
        print(possible_moves)
    return result_code
