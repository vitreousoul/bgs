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
    - Need to change user_move validation to where  you generate the list of legal 
      moves and compare the user's move to that.
      Also make the request for validity occur outside of move(), move() should just
      do it. That way it's not being validated twice every time you call the move() command
      for computer moves.

TODO Misc.:
    - Reduce hard coded values? Numpbers representing each piece, etc.
    - Standardize documentation formatting, indenting, etc.
    - Make the board_state a 3d array to track the board history?
    - I had to add .decode() everywhere to compare the characters from the array.
       Is there a better way I could have stored these or is it fine?
    - Consider using different data structures? Seems to be ok for now. I was
      getting very long runtimes before but I reduced that significantly by
      making the valid move search smarter and reducing the number of calls
      to validate_move()
"""

# I dont know if this is actually faster than loading the full numpy
from numpy import zeros
from numpy import base_repr
from numpy import sum
from numpy import flipud
from numpy import absolute

class Board:
    
    # Assume standard 8x8 board
    BOARD_SIZE = 8
    ASCII_lowA = 97
    ASCII_lowH = 104
    WHITE_PIECES = ("P","R","N","B","Q","K")
    BLACK_PIECES = ("p","r","n","b","q","k")
    PIECE_VALUES = [1,5,3,3,9,0]
    #TODO: try first assigning a zero value to king (might want to use high value
    #   to help with checkmate)
    
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
        self.material_count = self.count_material()
    
    """
    Defines / returns string representation of a Board for printing
    
    TODO: Address error handling for improper FEN formatting
    """
    def __str__(self):
        # temp_str = self.FEN_LOG[-1] + '\n\n'
        # Note that the board prints from white's perspective by default so
        #   the board array must be flipped for priting.
        #   An option to print from black's perspective could be added.
        temp_str = '\n'
        for rank in range(self.BOARD_SIZE):
            for file in range(self.BOARD_SIZE):
                if len(flipud(self.board_state)[rank,file]) == 0:
                    temp_str += '. '
                else:
                    temp_str += flipud(self.board_state)[rank,file].decode("utf-8") + ' '
            temp_str += '\n'
        
        return temp_str
    
    """
    Parse FEN and generate an ASCII representation of the board
    Each board rank is separated by '/'
     - Also creates an 8x8 table representation of the board state (self.board_state)
    See https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
    """
    def parse_fen(self):
        # TODO: Consider making this a list instead of a numpy array. Indexing
        #    with a[1][1]. Numpy array may be faster.
        self.board_state = zeros((self.BOARD_SIZE,self.BOARD_SIZE),dtype='S1')
        split_fen = self.FEN_LOG[-1].split('/')
        split_fen[-1] = split_fen[-1].split(' ')[0]
        for i,rank in enumerate(split_fen):
            file_offset = 0
            for j,file in enumerate(rank):
                if file.isalpha():
                    self.board_state[i,file_offset+j] = file
                elif file.isdigit():
                    file_offset += int(file)-1
                # else:
                    # TODO: FEN is not properly formatted
        # Flip the array upside down so that A1 corresponds with index [0,0]
        self.board_state = flipud(self.board_state)
        
        split_fen = self.FEN_LOG[-1].split()
        """
        Determine who's move it is'
        """
        if split_fen[1].isalpha() and split_fen[1] == "w":
            self.white_to_move = True
        elif split_fen[1].isalpha() and split_fen[1] == "b":
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
        TODO: Add additional error checking for ep_target input from FEN
        """
        ep_temp = split_fen[3]
        if len(ep_temp) == 2:
            self.ep_target = [(int(ep_temp[1])-1),(ord(ep_temp[0])-self.ASCII_lowA)]
        else:
            self.ep_target = []
        
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
        
    #TODO: If it ends up being too slow to do this every time, consider just
    #      evaluating material after captures
    def count_material(self):
        
        material_count = 0
        for scan_rank in range(0,self.BOARD_SIZE):
            for scan_file in range(0,self.BOARD_SIZE):
                temp_square = self.board_state[scan_rank,scan_file].decode()
                if temp_square in self.WHITE_PIECES:
                    idx = self.WHITE_PIECES.index(temp_square)
                    material_count += self.PIECE_VALUES[idx]
                elif temp_square in self.BLACK_PIECES:
                    idx = self.BLACK_PIECES.index(temp_square)
                    material_count -= self.PIECE_VALUES[idx]
        return material_count
                    
            
    def move(self,user_move):
            
        # throw error for move formatting
        if not isinstance(user_move,str):
            # print('Error: Move must be enterred as a string.')
            return 1
        elif not len(user_move) == 4:
            # print('Error: Move string must contain 4 characters.')
            return 1
            
        f1 = user_move[0]
        r1 = user_move[1]
        f2 = user_move[2]
        r2 = user_move[3]
        
        if not (f1.isalpha() and f2.isalpha() and \
            (self.ASCII_lowA <= ord(f1.lower()) <= self.ASCII_lowH) and \
            (self.ASCII_lowA <= ord(f2.lower()) <= self.ASCII_lowH)):
            # print('Error: Files must be a character a-h.')
            return 1
            
        if not (r1.isdigit() and r2.isdigit() and (1 <= int(r1) <= self.BOARD_SIZE) and \
            (1 <= int(r1) <= self.BOARD_SIZE)):
            # print('Error: Ranks must be an integer 1-8.')
            return 1
        
        """
        Convert notation into array indices.
        
        TODO: Right now cannot accept Algebric Notation (e4 rathan the e2e4)
        TODO: Castling doesn't work
        """
        f1 = ord(f1.lower()) - self.ASCII_lowA
        f2 = ord(f2.lower()) - self.ASCII_lowA
        r1 = int(r1) - 1
        r2 = int(r2) - 1
        
        rank_diff = r2 - r1
        file_diff = f2 - f1
        
        # If it's a pawn move of two squares, assign the ep_target
        ep_flag = False
        if self.board_state[r1,f1].decode().lower() == 'p' and absolute(r2-r1) == 2:
            if self.white_to_move:
                self.ep_target = [r2-1,f2]
            else:
                self.ep_target = [r2+1,f2]
            ep_flag = True
            
        # TODO: Validate Move
        # If it passes the test as a valid move, execute
        # TODO: Need to do this the new way where you generate the list of legal moves and compare the user's move to that.
        if not self.validate_move(r1,f1,r2,f2):
            # print('Error: Invalid move. Please try again.\n')
            return 1
        
        # If the move is castling
        if self.board_state[r1,f1].decode().lower() == "k" and \
            rank_diff == 0 and absolute(file_diff) == 2:
                # Move the king to the castling square
                self.board_state[r2,f2] = self.board_state[r1,f1]
                self.board_state[r1,f1] = ""
                if self.white_to_move:
                    # If white is castling king side, move the rook over
                    if file_diff > 0:
                        self.board_state[0,f2-1] = self.board_state[0,-1]
                        self.board_state[0,-1] = ""
                    # If white is castling queen side, move the rook over
                    else:
                        self.board_state[0,f2+1] = self.board_state[0,0]
                        self.board_state[0,0] = ""
                    # Remove white's castling rights
                    self.wck = False
                    self.wcq = False
                else:
                    # If black is castling king side, move the rook over
                    if file_diff > 0:
                        self.board_state[-1,f2-1] = self.board_state[-1,-1]
                        self.board_state[-1,-1] = ""
                    # If black is castling queen side, move the rook over
                    else:
                        self.board_state[-1,f2+1] = self.board_state[-1,0]
                        self.board_state[-1,0] = ""
                    # Remove black's castling rights
                    self.bck = False
                    self.bcq = False
        # TODO: If the move is capturing en passant
        elif self.board_state[r1,f1].decode().lower() == 'p' and len(self.ep_target) > 0 and \
            r2 == self.ep_target[0] and f2 == self.ep_target[1]:
            self.board_state[r2,f2] = self.board_state[r1,f1]
            self.board_state[r1,f1] = ""
            if self.white_to_move:
                self.board_state[r2-1,f2] = ""
            else:
                self.board_state[r2+1,f2] = ""
            self.ep_target = []
        # TODO: If the move is a pawn promotion
        #       Auto-queen for now
        elif self.board_state[r1,f1].decode().lower() == 'p' and (r2 == 0 or r2 == (self.BOARD_SIZE - 1)):
            if self.white_to_move:
                self.board_state[r2,f2] = 'Q'
            else:
                self.board_state[r2,f2] = 'q'
            self.board_state[r1,f1] = ''
        # Any other type of move
        else:
            # If it's a king move, remove castling rights
            # TODO: Having it check this every move is probably not efficient
            #       but it's fine for now.
            if self.white_to_move and (self.wcq or self.wck):
                if self.board_state[r1,f1].decode().lower() == 'k':
                    self.wck = False
                    self.wcq = False
                # If white's A1 rook moves, remove queen side castling rights
                elif self.board_state[r1,f1].decode().lower() == 'r' and r1 == 0 and f1 == 0:
                    self.wcq = False
                # If white's H1 rook moves, remove king side castling rights
                elif self.board_state[r1,f1].decode().lower() == 'r' and r1 == 0 and f1 == (self.BOARD_SIZE-1):
                    self.wck = False
            elif not self.white_to_move and (self.bcq or self.bck):
                if self.board_state[r1,f1].decode().lower() == 'k':
                    self.bck = False
                    self.bcq = False 
                # If black's A8 rook moves, remove queen side castling rights
                elif self.board_state[r1,f1].decode().lower() == 'r' and r1 == (self.BOARD_SIZE-1) and f1 == 0:
                    self.bcq = False
                # If black's H8 rook moves, remove king side castling rights
                elif self.board_state[r1,f1].decode().lower() == 'r' and r1 == (self.BOARD_SIZE-1) and f1 == (self.BOARD_SIZE-1):
                    self.bck = False
                    
            self.board_state[r2,f2] = self.board_state[r1,f1]
            self.board_state[r1,f1] = ''
            self.material_count = self.count_material()
            
        # TODO: Update the following params accordingly
        # - Who's move indicator
        # - Castling rights
        # - En Passant target
        # - Half move counter
        # - Full move counter
        
        # If the move just played was not a pawn moving 2 squares, reset the ep_target
        if not ep_flag:
            self.ep_target = []
        self.white_to_move = not self.white_to_move
        # print(self)
        return 0
        
    def validate_move(self,r1,f1,r2,f2):                 
        """
        Check that you're not trying to capture your own piece'
        """
        if self.white_to_move:
            friend_list = self.WHITE_PIECES
        else:
            friend_list = self.BLACK_PIECES
        if self.board_state[r2,f2].decode() in friend_list:
            return False

        """
        Validate king move
         - Is the king moving laterally two squares?
             - If so, does the knig have castling rights in that direction?
         - Is the move along a single rank / file?
             - If so, is the move only one square away?
         TODO: Need to implement castling. Seems like the FEN will not cover
         whether or not there's a piece in the way of castling so need an if
         statement for that.
        """
        if self.board_state[r1,f1].decode().lower() == "k":
            # Handle castling first
            if (r2-r1) == 0 and absolute(f2-f1) == 2:
                # If it's white to move, check castling rights
                if self.white_to_move:
                    if (f2-f1) == -2:
                        if not self.wcq or len(self.board_state[0,1]) > 0 or len(self.board_state[0,2]) > 0:
                            return False
                    else: 
                        if not self.wck or len(self.board_state[0,-2]) > 0 or len(self.board_state[0,-3]) > 0:
                            return False 
                # If it's black's turn to move, check castling rights
                else:
                    if (f2-f1) == -2:
                        if not self.bcq or len(self.board_state[-1,1]) > 0 or len(self.board_state[-1,2]) > 0:
                            return False
                    else: 
                        if not self.bck or len(self.board_state[-1,-2]) > 0 or len(self.board_state[-1,-3]) > 0:
                            return False
                # If it makes it here, castling is allowed.
        
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
            - If so, is there another piece in the way?
        """
        if self.board_state[r1,f1].decode().lower() == "p":
            # Check that the move is either one or two squares along a file or
            # one square along a diagonal.
            if f1 == f2:
                # If the end square is occupied by any piece, return false
                if self.board_state[r2,f2]:
                    return False
                # Handle case where pawn is moving forward two squares
                if absolute(r2 - r1) == 2 and (f1 == f2):
                    if self.white_to_move:
                        # If the pawn isn't on the starting square for the color, return false.
                        if not (r1 == 1):
                            return False
                        # If the square directly in front of the pawn is occupied, return false.
                        if self.board_state[r1+1,f1]:
                            return False
                    else:
                        # If the pawn isn't on the starting square for the color, return false.
                        if not (r1 == self.BOARD_SIZE - 2):
                            return False
                        # If the square directly in front of the pawn is occupied, return false.
                        if self.board_state[r1-1,f1]:
                            return False
                        
            # Handle cases where the pawn is moving diagonally
            else:
                # If the diagonal square you're trying to move to is empty (google en passant)
                if not self.board_state[r2,f2]:
                    # If en passant is not allowed
                    if not len(self.ep_target) == 2:
                        return False
                    # If the en passant target sqaure doesn't match where you're
                    #   trying to go, return false.
                    elif not (self.ep_target[0] == r2 and self.ep_target[1] == f2):
                        return False
            
            """
            Is the current player's king in check in the resulting position?'
             - It may make sense to place this check after the others if it's
               relatively expensive.
            "Simulate" the resulting position and determine if the player;s king
             is in check.'
            """
            temp_board = self.board_state.copy()
            temp_board[r2,f2] = temp_board[r1,f1]
            temp_board[r1,f1] = ''
            if self.is_check(temp_board):
                return False
            
        # If the move makes it through the above gauntlet, it's allowed.
        return True
    
    def is_check(self,temp_board):
        # TODO: See if can incorporate scan_rays into this
        
        # Locate the current player's king
        king_location = [-1,-1]
        if self.white_to_move:
            king_char = 'K'
        else:
            king_char = 'k'
        
        # TODO: Not the cleanest way to do this but should work for now
        # Compare speed of this search to a list where we would only need one
        # for loop and use the .index method.
        king_located = False
        for rank in range(self.BOARD_SIZE):
            if not king_located:
                for file in range(self.BOARD_SIZE):
                    if temp_board[rank,file].decode() == king_char:
                        king_location = [rank,file]
                        king_located = True
                        break
            else:
                break
        
        # Search for checks along the diagonals
        if self.white_to_move:
            enemy_list = ["b","q"]
            enemy_pawn = ["p"]
        else:
            enemy_list = ["B","Q"]
            enemy_pawn = ["P"]
            
        diagonals = ((1,1),(1,-1),(-1,-1),(-1,1))
        for i in range(4):
            scan_location = king_location.copy()
            scan_location[0] += diagonals[i][0]
            scan_location[1] += diagonals[i][1]
            
            # See if the adjacent square contains the enemy king
            if (0 <= scan_location[0] <= (self.BOARD_SIZE - 1)) and \
                (0 <= scan_location[1] <= (self.BOARD_SIZE - 1)) and \
                temp_board[scan_location[0],scan_location[1]].decode().lower() == "k":
                    return True
            
            # Iterate until you hit the edge of the board
            while (0 <= scan_location[0] <= (self.BOARD_SIZE - 1)) and \
                (0 <= scan_location[1] <= (self.BOARD_SIZE - 1)):
                # If you run into an enemy bishop or queen, it is check.
                if temp_board[scan_location[0],scan_location[1]].decode() in enemy_list:
                    return True
                # If you run into an enemy pawn, the pawn has to be touching the king
                # If you're white, the checking pawn has to be on a higher numbered rank
                # If you're black, the checking pawn has to be on a lower numbered rank
                if temp_board[scan_location[0],scan_location[1]].decode() in enemy_pawn:
                    if self.white_to_move and (king_location[0] - scan_location[0] == -1):
                        return True
                    elif not self.white_to_move and (king_location[0] - scan_location[0] == 1):
                        return True
                # If you run into anything else, there is no check on this diagonal.
                if not temp_board[scan_location[0],scan_location[1]].decode() == "":
                    break
                scan_location[0] += diagonals[i][0]
                scan_location[1] += diagonals[i][1]
        
        # Search for checks along the ranks and files
        if self.white_to_move:
            enemy_list = ["r","q"]
        else:
            enemy_list = ["R","Q"]
        
        rank_file = ((0,1),(0,-1),(1,0),(-1,0))
        for i in range(4):
            scan_location = king_location.copy()
            scan_location[0] += rank_file[i][0]
            scan_location[1] += rank_file[i][1]
            
            # See if the adjacent square contains the enemy king
            if (0 <= scan_location[0] <= (self.BOARD_SIZE - 1)) and \
                (0 <= scan_location[1] <= (self.BOARD_SIZE - 1)) and \
                temp_board[scan_location[0],scan_location[1]].decode().lower() == "k":
                    return True
            
            # Iterate until you hit the edge of the board
            while (0 <= scan_location[0] <= (self.BOARD_SIZE - 1)) and \
                (0 <= scan_location[1] <= (self.BOARD_SIZE - 1)):
                # If you run into an enemy rook or queen, it is check.
                if temp_board[scan_location[0],scan_location[1]].decode() in enemy_list:
                    return True
                # If you run into anything else, there is no check on this rank/file.
                if not temp_board[scan_location[0],scan_location[1]].decode() == "":
                    break
                scan_location[0] += rank_file[i][0]
                scan_location[1] += rank_file[i][1]
                
                
        # Search for knight checks
        if self.white_to_move:
            enemy_list = ["n"]
        else:
            enemy_list = ["N"]
        
        # Shout out Bob Seger
        knight_moves = ((2,1),(1,2),(-1,2),(-2,1),(-2,-1),(-1,-2),(1,-2),(2,-1))
        for i in range(len(knight_moves)):
            scan_location = king_location.copy()
            scan_location[0] += knight_moves[i][0]
            scan_location[1] += knight_moves[i][1]
            if (0 <= scan_location[0] <= (self.BOARD_SIZE - 1)) and \
                (0 <= scan_location[1] <= (self.BOARD_SIZE - 1)) and \
                    temp_board[scan_location[0],scan_location[1]].decode() in enemy_list:
                        return True
            
        # If you make it through this gauntlet, the king is not in check
        return False
    
    def get_valid_moves(self):
        valid_moves = []
        if self.white_to_move:
            friend_list = self.WHITE_PIECES
            pawn_direction = 1
        else:
            friend_list = self.BLACK_PIECES
            pawn_direction = -1
            
        for scan_rank in range(0,self.BOARD_SIZE):
            for scan_file in range(0,self.BOARD_SIZE):
                # Scan for pawn moves
                if self.board_state[scan_rank,scan_file].decode() == friend_list[0]:
                    valid_moves += self.scan_rays(scan_rank,scan_file,4,1,pawn_direction)
                # Check for rook moves
                elif self.board_state[scan_rank,scan_file].decode() == friend_list[1]:
                    valid_moves += self.scan_rays(scan_rank,scan_file,1,0,pawn_direction)
                # Check for knight moves
                elif self.board_state[scan_rank,scan_file].decode() == friend_list[2]:
                    valid_moves += self.scan_rays(scan_rank,scan_file,3,1,pawn_direction)
                # Check for bishop moves
                elif self.board_state[scan_rank,scan_file].decode() == friend_list[3]:
                    valid_moves += self.scan_rays(scan_rank,scan_file,2,0,pawn_direction)
                # Check for queen moves
                elif self.board_state[scan_rank,scan_file].decode() == friend_list[4]:
                    valid_moves += self.scan_rays(scan_rank,scan_file,1,0,pawn_direction)
                    valid_moves += self.scan_rays(scan_rank,scan_file,2,0,pawn_direction)
                # Check for king moves
                elif self.board_state[scan_rank,scan_file].decode() == friend_list[5]:
                    valid_moves += self.scan_rays(scan_rank,scan_file,1,1,pawn_direction)
                    valid_moves += self.scan_rays(scan_rank,scan_file,2,1,pawn_direction)
                    # Check castling options
                    r1 = scan_rank; f1 = scan_file
                    if r1 == 0 and f1 == 4:
                        r2 = r1; f2 = f1+2
                        if self.validate_move(r1,f1,r1,f1+2):
                            valid_moves.append((chr(f1+97) + str(r1+1) + chr(f2+97) + str(r2+1)))
                        r2 = r1; f2 = f1-2
                        if self.validate_move(r1,f1,r1,f1-2):
                            valid_moves.append((chr(f1+97) + str(r1+1) + chr(f2+97) + str(r2+1)))
                    
        return valid_moves
                        
                        
    def scan_rays(self,r1,f1,ray_type,short_scan,pawn_direction):
        valid_moves = []
        # Scan along ranks/files
        if ray_type == 1:
            ray_dir = ((0,1),(0,-1),(1,0),(-1,0))
        # Scan along diagonals
        elif ray_type == 2:
            ray_dir = ((1,1),(1,-1),(-1,-1),(-1,1))
        # Scan knight moves
        elif ray_type == 3:
            ray_dir = ((2,1),(1,2),(-1,2),(-2,1),(-2,-1),(-1,-2),(1,-2),(2,-1))
        # Scan pawn moves
        elif ray_type == 4:
            ray_dir = ((pawn_direction,0),(2*pawn_direction,0),(pawn_direction,1),(pawn_direction,-1))
        
        for i in range(len(ray_dir)):
            r2 = r1 + ray_dir[i][0]
            f2 = f1 + ray_dir[i][1]
            while (0 <= r2 < self.BOARD_SIZE) and (0 <= f2 < self.BOARD_SIZE):
                if self.validate_move(r1,f1,r2,f2):
                    valid_moves.append((chr(f1+97) + str(r1+1) + chr(f2+97) + str(r2+1)))
                # If square is ocupied, break out of loop
                if not self.board_state[r2,f2]:
                    break
                # short_scan when onyl one move is permitted in each direction
                if short_scan:
                    break
                r2 += ray_dir[i][0]
                f2 += ray_dir[i][1]
            
        return valid_moves

    def is_EOG(self):
        SQUARE_PAIRS = int("7777",8)
        all_moves = zeros((SQUARE_PAIRS,4),dtype=int)
        for i in range(SQUARE_PAIRS):
            move_str = base_repr(i,base=8,padding=4)
            all_moves[i,-1] = move_str[-1]
            all_moves[i,-2] = move_str[-2]
            all_moves[i,-3] = move_str[-3]
            all_moves[i,-4] = move_str[-4]
            
        valid_flag = zeros((SQUARE_PAIRS,1),dtype=bool)
        for i in range(SQUARE_PAIRS):
            r1 = all_moves[i,0]
            f1 = all_moves[i,1]
            r2 = all_moves[i,2]
            f2 = all_moves[i,3]
            if self.validate_move(r1,f1,r2,f2):
                valid_flag[i] = 1
            else:
                valid_flag[i] = 0
        num_legal_moves = sum(valid_flag)
        
        if num_legal_moves == 0:
            if self.is_check(self.board_state):
                return 1
            else:
                return 2
        else:
            return 0
        # TODO: def generate_fen(self):
    
        # TODO: def vaidate_fen(self):
        
        