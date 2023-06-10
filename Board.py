""" 
Initial stab at a basic Board class which initializes a fresh board and prints
    a representation of the board state (FEN format) as ASCII.
    
Bugs:
    - Board print not working for rank "4P3" - only prints the first 4 dots
    - Not a bug but realizing that python doesn't have the concept of a private
      method. So in theory someone could call parse_fen() from the command line
      in the middle of the game even though that's not an intended functionality.'
"""
import numpy as np

class Board:
    
    """
    Constructor for Board class
    - Define the starting state of the game (START_FEN)
    - Initialize the move log (FEN_LOG) with the starting state
    - Generate the ASCII representation of the current board state
    
    TODO: Later, add ability to start from an arbitrary position
    TODO: FEN_LOG.append(new_fen)? This should allow logging of previous 
          board states as moves are made.
    TODO: Is it necessary to initialize all attributes up top?
    """
    def __init__(self,*arg):
        
        if len(arg) == 0:
            self.START_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
        elif len(arg) == 1:
            self.START_FEN = arg[0]
            # TODO: Validate FEN
        # else:
            # TODO: Throw error: Too many input arguments
        self.FEN_LOG = [self.START_FEN]
        
        
        self.parse_fen()
    
    """
    Defines / returns string representation of a Board for printing
    
    TODO: Address error handling for improper FEN formatting
    """
    def __str__(self):
        # temp_str = self.FEN_LOG[-1] + '\n\n'
        temp_str = '\n'
        for rank in range(8):
            for file in range(8):
                if len(self.table_rep[rank,file]) == 0:
                    temp_str += '. '
                else:
                    temp_str += self.table_rep[rank,file].decode("utf-8") + ' '
            temp_str += '\n'
        
        if self.white_to_move:
            temp_str += '\n\nWhite to move...'
        else:
            temp_str += '\n\nBlack to move...'
            
        return temp_str
    
    """
    Parse FEN and generate an ASCII representation of the board
    Each board rank is separated by '/'
     - Also creates an 8x8 table representation of the board state (table_rep)
    See https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
    """
    def parse_fen(self):
        self.table_rep = np.zeros((8,8),dtype='S1')
        split_fen = self.FEN_LOG[-1].split('/')
        split_fen[-1] = split_fen[-1].split(' ')[0]
        for i, rank in enumerate(split_fen):
            file_offset = 0
            for j, file in enumerate(rank):
                if file.isalpha():
                    self.table_rep[i,file_offset+j] = file
                elif file.isdigit():
                    file_offset += int(file)-1
                        
                # else:
                    # TODO: FEN is not properly formatted
        
        split_fen = self.FEN_LOG[-1].split()
        """
        Determine who's move it is'
        """
        if split_fen[1].isalpha() and split_fen[1] == 'w':
            self.white_to_move = True
        elif split_fen[1].isalpha() and split_fen[1] == 'b':
            self.white_to_move = False
        # else:
            # TODO: FEN is not properly formatted
        
        """
        Determine castling rights
         - wck: White castle king site
         - wcq: White castle queen side
         - bck: Black castle king side
         - bcq: Black castle queen side
        """
        self.wck = False
        self.wcq = False
        self.bck = False
        self.bcq = False
        if split_fen[2].__contains__('K'):
            self.wck = True
        if split_fen[2].__contains__('Q'):
            self.wcq = True
        if split_fen[2].__contains__('k'):
            self.bck = True
        if split_fen[2].__contains__('q'):
            self.bcq = True
        
        """
        Determine En Passant target square
        """
        self.ep_target = split_fen[3]
        
        """
        Determine halfmove clock
        Number of half moves since last capture or pawn advance
        Used for the fifty-move rule
        """
        self.hm_clock = int(split_fen[4])
        
        """
        Determine fullmove clock
        Number of full moves since the start of the game
        Starts at 1 and increments after black's move'
        """
        self.fm_clock = int(split_fen[5])
            
        def move(self, user_move):
            # throw error for move formatting
            
            f1 = user_move[0]
            if f1.isalpha() and ord(f1.lower()) >= 97 and \
                ord(f1.lower()) <= 104:
                f1 = ord(f1.lower()) - 97
            # else:
                # throw error
            
            r1 = user_move[1]
            if r1.isdigit() and int(r1) >= 1 and \
                int(r1) <= 8:
                r1 = int(r1) - 1
            # else:
                # throw error
            
            f2 = user_move[2]
            if f2.isalpha() and ord(f2.lower()) >= 97 and \
                ord(f2.lower()) <= 104:
                f2 = ord(f2.lower()) - 97
            # else:
                # throw error
            
            r2 = user_move[4]
            if user_move[3].isdigit() and int(user_move) >= 1 and \
                int(user_move) <= 8:
                r2 = int(r2) - 1
            # else:
                # throw error
            
            # TODO: Validate Move
            # If it passes the test as a valid move, execute
            self.table_rep[r2,f2] = self.table_rep[r1,f1]
            self.table_rep[r1,f1] = ''
            
            # TODO: Update the following params accordingly
            # - Who's move indicator
            # - Castling rights
            # - En Passant target
            # - Half move counter
            # - Full move counter

        # TODO: def generate_fen(self):
    
        # TODO: def vaidate_fen(self):
        
        # TODO: def validate_move(self):
        # or generate the entire list of legal moves to compare against
        
            
""" Test of current features"""
# board = Board('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1')
board = Board()
print(board)
