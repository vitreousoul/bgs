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

TODO Misc.:
    - Reduce hard coded values?
    - Standardize documentation formatting, indenting, etc.
    - I might want to make the board orientation of the board_state array
      match the true board. Right now it's fliiped and it's kind of confusing.
      Make it flip only when it's printing the board (DONE)
    - Make the board_state a 3d array to track the board history
    - I had to add .decode() everywhere to compare the characters from the array.
       Is there a better way I could have stored these or is it fine?
"""
import numpy as np

class Board:
    
    # Assume standard 8x8 board
    BOARD_SIZE = 8
    ASCII_lowA = 97
    ASCII_lowH = 104
    
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
        # Note that the board prints from white's perspective by default so
        #   the board array must be flipped for priting.
        #   An option to print from black's perspective could be added.
        temp_str = '\n'
        for rank in range(self.BOARD_SIZE):
            for file in range(self.BOARD_SIZE):
                if len(np.flipud(self.board_state)[rank,file]) == 0:
                    temp_str += '. '
                else:
                    temp_str += np.flipud(self.board_state)[rank,file].decode("utf-8") + ' '
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
        self.board_state = np.zeros((self.BOARD_SIZE,self.BOARD_SIZE),dtype='S1')
        split_fen = self.FEN_LOG[-1].split('/')
        split_fen[-1] = split_fen[-1].split(' ')[0]
        for i, rank in enumerate(split_fen):
            file_offset = 0
            for j, file in enumerate(rank):
                if file.isalpha():
                    self.board_state[i,file_offset+j] = file
                elif file.isdigit():
                    file_offset += int(file)-1
                # else:
                    # TODO: FEN is not properly formatted
        # Flip the array upside down so that A1 corresponds with index [0,0]
        self.board_state = np.flipud(self.board_state)
        
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
            
        # TODO: Validate Move
        # If it passes the test as a valid move, execute
        if not self.validate_move(f1,r1,f2,r2):
            # print('Error: Invalid move. Please try again.\n')
            return 1
        
        self.board_state[r2,f2] = self.board_state[r1,f1]
        self.board_state[r1,f1] = ''
        
            
        # TODO: Update the following params accordingly
        # - Who's move indicator
        # - Castling rights
        # - En Passant target
        # - Half move counter
        # - Full move counter
        
        # if self.white_to_move:
        #     print('\nWhite plays ' + user_move + '.')
        # else:
        #     print('\nBlack plays ' + user_move + '.')
        
        self.white_to_move = not self.white_to_move
        # print(self)
        return 0
        
    def validate_move(self,f1,r1,f2,r2):
        # TODO: This is temporary!
        # return True
        
        """
        TODO: Delete this?
        Universal validation checks
         - Is the current player's king in check in the resulting position?'
             - Use while loop to span out from king
         - Is the current player in check
             - If so, does this move resolve the check?
         - Except for knight and king: Does the piece run into any pieces / pawns
           en route to its final destination?
        """
        
        """
        Universal validation checks
         - These apply to all pieces
        
        - Are the start and end squares on the board?
        - Is the start square different from the end square?
        TODO: This check may be redundant for human entered moves but we may
           still want it to validate bot moves.
        """
        if  not ((0 <= r1 <= (self.BOARD_SIZE - 1)) and (0 <= r1 <= (self.BOARD_SIZE - 1)) and \
            (0 <= f1 <= (self.BOARD_SIZE - 1)) and (0 <= f1 <= (self.BOARD_SIZE - 1))):
            return False
        if r1 == r2 and f1 == f2:
            return False
        
        """
        Is there a friendly piece on the start square?
         - If not, the move is invalid
        Is there a friendly piece on the end square?
         - If so, the move is invalid
        """
        if self.white_to_move:
            friend_list = ["P", "R", "N", "B", "Q"]
        else:
            friend_list = ["p", "r", "n", "b", "q"]
            
        if not (self.board_state[r1,f1].decode() in friend_list):
            return False
        if self.board_state[r2,f2].decode() in friend_list:
            return False
        
        """
        Is the current player's king in check in the resulting position?'
         - It may make sense to place this check after the others if it's
           relatively expensive.
        "Simulate" the resulting position and determine if the player;s king
         is in check.'
        """
        temp_board = self.board_state
        temp_board[r2,f2] = temp_board[r1,f1]
        temp_board[r1,f1] = ''
        if self.is_check(temp_board,friend_list):
            return False
        
        
        # TODO: Temp
        diagonals = [[1,1],[1,-1],[-1,-1],[-1,1]]
        rank_file = [[0,1],[0,-1],[1,0],[-1,0]]
        
        """
        Validate knight move
         - Is the move an L shape?
        """
        rank_diff = r2 - r1
        file_diff = f2 - f1
        if self.board_state[r1,f1].decode().lower() == "n":
            if not ((np.absolute(rank_diff) == 2 and np.absolute(file_diff) == 1) or \
                (np.absolute(rank_diff) == 1 and np.absolute(file_diff) == 2)):
                return False
        
        """
        Validate non-pawn/king diagonal move
         - Is the move diagonal?
         - Does the bishop or queen "run into" anything?
        """
        if np.absolute(rank_diff) == np.absolute(file_diff):
            if self.board_state[r1,f1].decode().lower() in ["b", "q"]:
                # TODO: Could probably do this in a prettier way...
                if rank_diff > 0 and file_diff > 0:
                    diag_indx = 1
                elif rank_diff > 0 and file_diff < 0:
                    diag_indx = 2
                elif rank_diff < 0 and file_diff < 0:
                    diag_indx = 3
                else:
                    # TODO: Another option shouldn't be possible at this point. Test.
                    diag_indx = 4
            
                # Iterate along the rank / file and if you run into a piece / pawn
                # en-route to your desintation, the move is invalid.
                for square in range(rank_diff - 1):
                    current_square = self.board_state[(r1 + diagonals(diag_indx)[0]), \
                                                      (f1 + diagonals(diag_indx)[1])]
                    if len(current_square) > 0:
                        return False
            
        """
        Validate non-pawn/king lateral or vertical move
         - Is the move along a single rank / file?
         - Does the rook or queen "run into" anything?
        """
        if rank_diff == 0 or file_diff == 0:
            if self.board_state[r1,f1].decode().lower() in ["r", "q"]:
                # TODO: Could probably do this in a prettier way...
                if rank_diff == 0 and file_diff > 0:
                    rank_file_indx = 1
                    rank_file_incr = file_diff
                elif rank_diff == 0 and file_diff < 0:
                    rank_file_indx = 2
                    rank_file_incr = file_diff
                elif rank_diff > 0 and file_diff == 0:
                    rank_file_indx = 3
                    rank_file_incr = rank_diff
                else:
                    # TODO: Another option shouldn't be possible at this point. Test.
                    rank_file_indx = 4
                    rank_file_incr = rank_diff
                
                # Iterate along the rank / file and if you run into a piece / pawn
                # en-route to your desintation, the move is invalid.
                for square in range(rank_file_incr - 1):
                    current_square = self.board_state[(r1 + rank_file(rank_file_indx)[0]), \
                                                      (f1 + rank_file(rank_file_indx)[1])]
                    if len(current_square) > 0:
                        return False
        
        """
        Validate king move
         - Is the king moving laterally two squares?
             - If so, does the knig have castling rights in that direction?
         - Is the move along a single rank / file?
             - If so, is the move only one square away?
        """
        if self.board_state[r1,f1].decode().lower() == "k":
            # Handle castling first
            if rank_diff == 0 and np.absolute(file_diff) == 2:
                # If it's white to move, check castling rights
                if self.white_to_move:
                    if file_diff == -2 and not self.wcq:
                        return False
                    elif not self.wck:
                        return False
                # If it's black's turn to move, check castling rights
                else:
                    if file_diff == -2 and not self.bcq:
                        return False
                    elif not self.bck:
                        return False
                # If it makes it here, castling is allowed.
                # If the king is trying to move more than one square (not castling),
                #    the move is invalid.
            elif rank_diff > 1 or file_diff > 1:
                return False
        
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
        - Is the pawn moving forward 1 square?
        - Else: Move is illegal
        """
        # Handle case where pawn is moving forward two squares
        if np.absolute(rank_diff) == 2 and file_diff == 0:
            # If the pawn isn't on the starting square for the color, return false.
            if self.white_to_move and not(r1 == 1):
                return False
            elif not self.white_to_move and not (r1 == self.BOARD_SIZE - 2):
                return False
        # Handle case where pawn in capturing (moving diagonally)
        elif np.absolute(rank_diff) == np.absolute(file_diff):
            # If there's a friendly piece on the square you're trying to capture,
            #   return false.
            if self.board_state[r2,f2].decode() in friend_list:
                return False
            # If the diagonal square you're trying to move to is empty
            elif len(self.board_state[r2,f2]) == 0:
                # If en passant is not allowed
                if self.ep_target == "-":
                    return False
                # If the en passant target sqaure doesn't match where you're
                #   trying to go, return false.
                elif not (self.ep_target[0] == r2 and self.ep_target[1] == f2):
                    return False
        # If the move is one square forward
        elif np.absolute(rank_diff) == 1 and file_diff == 0:
            # Make sure the move is in the correct direction corresponding to
            #   who's move it is.        
            if self.white_to_move and rank_diff < 0:
                return False
            elif not self.white_to_move and rank_diff > 0:
                return False
        # Any other pawn move not addressed above is invalid
        else:
            return False
        
        # If the move makes it through the above gauntlet, it's allowed.
        return True
    
    def is_check(self,temp_board,friend_list):
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
            enemy_list = ["b", "q"]
            enemy_pawn = "p"
        else:
            enemy_list = ["B", "Q"]
            enemy_pawn = "P"
            
        diagonals = [[1,1],[1,-1],[-1,-1],[-1,1]]
        for i in range(4):
            scan_location = king_location
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
            enemy_list = ["r", "q"]
        else:
            enemy_list = ["R", "Q"]
            
        rank_file = [[0,1],[0,-1],[1,0],[-1,0]]
        for i in range(4):
            scan_location = king_location
            # Iterate until you hit the edge of the board
            while (0 <= scan_location[0] <= (self.BOARD_SIZE - 1)) and \
                (0 <= scan_location[1] <= (self.BOARD_SIZE - 1)):
                # If you run into an enemy rook or queen, it is check.
                if temp_board[scan_location[0],scan_location[1]].decode() in enemy_list:
                    return True
                # If you run into anything else, there is no check on this diagonal.
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
        knight_moves = [[2,1],[1,2],[-1,2],[-2,1],[-2,-1],[-1,-2],[1,-2],[2,-1]]
        for i in range(len(knight_moves)):
            scan_location = king_location
            scan_location[0] += knight_moves[i][0]
            scan_location[1] += knight_moves[i][1]
            
            
            if (0 <= scan_location[0] <= (self.BOARD_SIZE - 1)) and \
                (0 <= scan_location[1] <= (self.BOARD_SIZE - 1)) and \
                    temp_board[scan_location[0],scan_location[1]].decode().lower() in enemy_list:
                        return True
            
        # If you make it through this gauntlet, the king is not in check
        return False

        # TODO: def generate_fen(self):
    
        # TODO: def vaidate_fen(self):
        
        # TODO: def validate_move(self):
        # or generate the entire list of legal moves to compare against
        
        # TODO: Separate module for validate move. Includes is_check, en passant, etc.
        