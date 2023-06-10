""" 
Initial stab at a basic Board class which initializes a fresh board and prints
    a representation of the board state (FEN format) as ASCII.
"""

class Board:
    
    """
    Constructor for Board class
    - Define the starting state of the game (START_FEN)
    - Initialize the move log (FEN_LOG) with the starting state
    - Generate the ASCII representation of the current board state
    
    TODO: Later, add ability to start from an arbitrary position
    TODO: FEN_LOG.append(new_fen)? This should allow logging of previous 
          board states as moves are made.
    """
    def __init__(self):
        
        self.START_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
        self.FEN_LOG = [self.START_FEN]
        self.ascii_rep = ''
        self.generate_ascii()
    
    """
    Defines / returns string representation of a Board for printing
     
    TODO: Address error handling for improper FEN formatting
    """
    def __str__(self):
        temp_str = self.FEN_LOG[-1] + '\n\n' + self.ascii_rep
        split_fen = self.FEN_LOG[-1].split()
        if split_fen[1].isalpha() and split_fen[1] == 'w':
            temp_str += '\n\nWhite to move...'
        elif split_fen[1].isalpha() and split_fen[1] == 'b':
            temp_str += '\n\nBlack to move...'
        else:
            """FEN is not properly formatted"""
        return temp_str
    
    """
    Parse FEN and generate an ASCII representation of the board
    Each board rank is separated by '/'
    See https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
    """
    def generate_ascii(self):
        split_fen = self.FEN_LOG[-1].split('/')
        split_fen[-1] = split_fen[-1].split(' ')[0]
        for i in split_fen:
            for j in i:
                if j.isalpha():
                    self.ascii_rep += j + ' '
                elif j.isdigit():
                    for k in range(1,int(j)+1):
                        self.ascii_rep += '. '
                else:
                    """FEN is not properly formatted"""
            self.ascii_rep += '\n'

def test(arg_object):
    """ Test of current features"""
    board = Board()
    print(board)
    return 0
