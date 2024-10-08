# -*- coding: utf-8 -*-
"""
Created on Sat Jun 24 16:38:37 2023

@author: zachg
"""

# TODO: Could probably find a way to speed this up by filtering out certain moves
#       automatically. Ex. empty square to empty square.
# TODO: Still a lot of debugging work to do on this.

import numpy as np
from Board import Board

SQUARE_PAIRS = int("7777",8)
all_moves = np.zeros((SQUARE_PAIRS,4),dtype=int)
for i in range(SQUARE_PAIRS):
    move_str = np.base_repr(i,base=8,padding=4)
    all_moves[i,-1] = move_str[-1]
    all_moves[i,-2] = move_str[-2]
    all_moves[i,-3] = move_str[-3]
    all_moves[i,-4] = move_str[-4]

board = Board('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1')
# board = Board('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1')
# board = Board('rnbqkbnr/pppp1ppp/8/4p3/4PP1q/8/PPPP2PP/RNBQKBNR w KQkq - 0 1')
# board = Board('8/8/3k4/4r3/3K4/8/8/8 w - - 0 1')


# Starting position with white - looks good
# board = Board()

# Starting position with black - looks good
# board = Board('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1')

""" 
Bugs - Fischer - Spassky Move 2 for white
 - Missing moves for f1 bishop f1e2 = 06 15 = 397
    - fixed (rank_diff == file_diff needs to be absolute)
 - e4e5 shouldn't be possible e4e5 = 34 44 = 1828
    - fixed (needed to clean up pawn rules)
"""
# board = Board('rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2')

"""  
Bugs - Fischer - Spassky Move 3 for white
 - Queen cannot hop over knight d1g4 = 03 36 = 222
   - fixed (diagonal iterations were only looking one square deep)
 - King cannot castle but that's something I haven't dealt with yet
 - Missing the move b2b4 11 31 = 601
    - returning ischeck for some reason
    - fixed this. scan_location = king_location.copy() needed
 - now we're missing d2d3 and d2d4 13 23
    - empty square was registering as an enemy pawn with = "p" instead of ["p"]

 - TLDR; we still have the castling issue. Haven't implemented that yet
"""
# board = Board('r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3')

"""
Bugs - Fischer - Spassky Move 5 for white
 - Fixed issue indexing because rank_file_indx was assinged incorrectly
 - King not allowed to castle is only bug e1g1 = 04 06 = 262
   - Fixed. Due to copy/pasting castling rules from queen to king side
"""
# board = Board('r1bqkb1r/1ppp1ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 2 5')

"""
Bugs - Fischer - Spassky Move 24 for black
 - King should not be allowed to take on f7 g8f7 = 76 65 = 4021
"""
# board = Board('4rrk1/1b3Bp1/1n3q1p/2p1N3/1p6/7P/PP3PP1/R2QR1K1 b - - 0 24')


"""
En passant example
"""
# board = Board('rnbqkbnr/pp1p2pp/5p2/2pPp3/4P3/8/PPP2PPP/RNBQKBNR w KQkq c6 0 4')

"""
Bugs - Fischer - Spassky Move 26 for white
 - There should only be 2 legal moves in this position
 - Found a bug in is_check(). Due to scan_location not be incremented at
   the start of the loop, checks along the ranks/files weren't being identified.
   I.e. the scan was finding the king and stopping there.
"""
# board = Board('6k1/1b3Np1/1n3q1p/2p5/1p6/7P/PP3PP1/R2Qr1K1 w - - 0 26')


"""
Bugs - Fischer - Spassky Move 43 for black
 - No bugs, 17 legal moves correctly identified
"""
# board = Board('8/8/R5p1/2k3p1/1p4P1/1P1b1P2/3K1n2/8 w - - 1 43')


"""
Bugs - Fischer - Spassky Move 43 for black
 - No bugs, 17 legal moves correctly identified 
"""
#cboard = Board('8/8/4R1p1/2k3p1/1p4P1/1P1b1P2/3K1n2/8 b - - 2 43')


"""
Bugs - Checkmate example, scholar's mate
 - Should zero legal moves. Correctly identified.
"""
# board = Board('r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4')

"""
Bugs - Stalemate example
 - Should zero legal moves. Correctly identified.
"""
# board = Board('8/8/8/8/8/5KQ1/8/7k b - - 1 1')


"""
Bugs - Opera Game - Move 11 for black
 - King in check from bishop - Correct.
"""
# board = Board('rn2kb1r/p3qppp/5n2/1B2p1B1/4P3/1Q6/PPP2PPP/R3K2R b KQkq - 0 11')

"""
Bugs - Opera Game - Move 12 for black
 - Knight is pinned to king. King can castle queen side - Correct
"""
# board = Board('r3kb1r/p2nqppp/5n2/1B2p1B1/4P3/1Q6/PPP2PPP/2KR3R b kq - 2 12')

"""
Bugs - Opera Game - Move 14 for black
 - Rook is pinned to king - Correct
"""
# board = Board('4kb1r/p2rqppp/5n2/1B2p1B1/4P3/1Q6/PPP2PPP/2KR4 b k - 1 14')

"""
Bugs - Opera Game - Move 15 for black
 - King is in check from bishop - Correct
"""
# board = Board('4kb1r/p2B1ppp/4qn2/4p1B1/4P3/1Q6/PPP2PPP/2KR4 b k - 0 15')


"""
Bugs - Opera Game - Move 16 for black
 - Queen sacrafice, black has to accept - Correct
"""
# board = Board('1Q2kb1r/p2n1ppp/4q3/4p1B1/4P3/8/PPP2PPP/2KR4 b k - 1 16')


"""
Bugs - Opera Game - Move 17 for white
 - Mate in one on the board - Correct
"""
# board = Board('1n2kb1r/p4ppp/4q3/4p1B1/4P3/8/PPP2PPP/2KR4 w k - 0 17')

"""
Bugs - Opera Game - Move 17 for black
 - Checkmate - Correct
"""
# board = Board('1n1Rkb1r/p4ppp/4q3/4p1B1/4P3/8/PPP2PPP/2K5 b k - 1 17')

"""
Bugs - Endgame example - pawn promotion
 - Correct
"""
# board = Board('8/6P1/4k3/8/3K4/8/8/8 w - - 0 1')
# Queen has been made. Black king in check
# board = Board('6Q1/8/4k3/8/3K4/8/8/8 b - - 0 1')

"""
Bugs - Double check test
 - Black is in double check and only has one move - Correct
"""
#board = Board('r6r/2qknP1p/p5p1/1p2pB2/8/2P1B3/P1PR2PP/5RK1 b - - 2 22')

"""
Bugs - Counter check test
"""
# White is in check
# board = Board('8/6P1/8/8/1q2K3/8/5Q2/k7 w - - 0 1')
# Now black is in check after the counter
# board = Board('8/6P1/8/8/1q1QK3/8/8/k7 b - - 1 1')

valid_flag = np.zeros((SQUARE_PAIRS,1),dtype=bool)
legal_moves = []
for i in range(SQUARE_PAIRS):
    r1 = all_moves[i,0]
    f1 = all_moves[i,1]
    r2 = all_moves[i,2]
    f2 = all_moves[i,3]
    
    # Do something like this if it's too slow
    move = x = "0x" + str(r1) + str(f1) + str(r2) + str(f2)
    
    # if i == 1:
        # print("here")
    if board.validate_move(r1,f1,r2,f2):
        valid_flag[i] = 1
        # I cannot figure out how to index all_moves to extract
        # the rows where valid_flag is true so I'm doing this instead.
        # all_moves[valid_flag, ...] ???
        # print(chr(f1+97) + str(r1+1) + " " + chr(f2+97) + str(r2+1))
        legal_moves.append((chr(f1+97) + str(r1+1) + chr(f2+97) + str(r2+1)))
        
    else:
        valid_flag[i] = 0
    
print(str(np.sum(valid_flag)) + " legal moves in this position.")
    