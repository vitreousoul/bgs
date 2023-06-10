""" 
Initial stab at a basic Board class which initializes a fresh board and prints
    a representation of the board state (FEN format) as ASCII.
    
Bugs:
    - Board print not working for rank "4P3" - only prints the first 4 dots
    - Not a bug but realizing that python doesn't have the concept of a private
      method. So in theory someone could call parse_fen() from the command line
      in the middle of the game even though that's not an intended functionality.'

TODO Features:
    - Move validation
       - Accpet algebraic notation, allow castling, en passant, etc.
       - Need a way to deal with promotion
    - Generate FEN for each position. Ask user if they want to save the FEN
      log and game info to a log file upon quitting
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
        if not isinstance(user_move,str):
            print('Error: Move must be enterred as a string.')
            return
        elif not len(user_move) == 4:
            print('Error: Move string must contain 4 characters.')
            return
            
        f1 = user_move[0]
        r1 = user_move[1]
        f2 = user_move[2]
        r2 = user_move[3]
        
        if not (f1.isalpha() and f2.isalpha() and (97 <= ord(f1.lower()) <= 104) and \
            (97 <= ord(f2.lower()) <= 104)):
            print('Error: Files must be a character a-h.')
            return
            
        if not (r1.isdigit() and r2.isdigit() and (1 <= int(r1) <= 8) and \
            (1 <= int(r1) <= 8)):
            print('Error: Ranks must be an integer 1-8.')
            return
        
        """
        Convert notation into array indices.
        The ranks have to be flipped since the FEN lists them from the 8th down
        
        TODO: Right now cannot accept Algebric Notation (e4 rathan the e2e4)
        TODO: Castling doesn't work
        """
        f1 = ord(f1.lower()) - 97
        f2 = ord(f2.lower()) - 97
        r1 = -1 * (int(r1) - 1) - 1
        r2 = -1 * (int(r2) - 1) - 1
            
        # TODO: Validate Move
        # If it passes the test as a valid move, execute
        if not self.validate_move(f1,r1,f2,r2):
            print('Invalid move. Please try again.\n')
            return
        
        self.table_rep[r2,f2] = self.table_rep[r1,f1]
        self.table_rep[r1,f1] = ''
        
            
        # TODO: Update the following params accordingly
        # - Who's move indicator
        # - Castling rights
        # - En Passant target
        # - Half move counter
        # - Full move counter
        
        if self.white_to_move:
            print('\nWhite plays ' + user_move + '.')
        else:
            print('\nBlack plays ' + user_move + '.')
        
        self.white_to_move = not self.white_to_move
        print(self)
        
    def validate_move(self):
        
        """
        Universal validation checks
         - Is there a piece of that color on the start square?
         - Is there a piece of that color on the end square?
         - Is the current player's king in check in the resulting position?'
         - Is the current player in check
             - If so, does this move resolve the check?
         - Except for knight and king: Does the piece run into any pieces / pawns
           en route to its final destination?
        """
        
        """
        Validate pawn move
         - Is the pawn making a capture?
             - If so, is it a 1x1 diagonal move?
             - Is there an enemy piece/pawn on the end square?
                 - If not, is En Passant available?
                     - If En Passant is available remove enemy pawn from the 
                       correct square (one in front of where the target square)
        - Is the pawn moving forward 2 squares?
            - If so, is it starting from the 2nd / 7th rank?
        - Is the pawn moving forward 1 square?
        - Else: Move is illegal
        """
        
        """
        Validate knight move
         - Is the move an L shape?
        """
        
        """
        Validate bishop move
         - Is the move diagonal?
        """
        
        """
        Validate rook move
         - Is the move along a single rank / file?
        """
        
        """
        Validate queen move
         - Is the move along a single rank / file OR diagonal?
        """
        
        """
        Validate king move
         - Is the king moving laterally two squares?
             - If so, does the knig have castling rights in that direction?
         - Is the move along a single rank / file?
             - If so, is the move only one square away?
        """
        

        # TODO: def generate_fen(self):
    
        # TODO: def vaidate_fen(self):
        
        # TODO: def validate_move(self):
        # or generate the entire list of legal moves to compare against
        
        
            
""" Test of current features"""
# board = Board('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1')
board = Board()
print(board)
while True:
    
    if board.white_to_move:
        user_move = input("Enter a move for White: ")
    else:
        user_move = input("Enter a move for Black: ")
    
    if user_move.lower() == 'q' or user_move.lower() == 'quit':
        break
    
    board.move(user_move)
    