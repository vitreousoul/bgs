"""
Test of current features
Simple board where the user is prompted to make moves for white and black

TODO: Removed automatic print statements from Board.py. Instead, have the UI
   module handle printing the board and error messages.
   
   - Determine checkmate, stalemate
   - Handle pawn promotion
"""

from Board import Board

# An alternate FEN to test as an input
# board = Board('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1')
# board = Board('rnbqkbnr/pppp1ppp/8/4p3/4PP1q/8/PPPP2PP/RNBQKBNR w KQkq - 0 1')

# board = Board()
# Checkmate test
""" 
Bug found - need to make it illegal for kings to touch
 - Fixed - needed to add a check for king on adjacent square for diagonals
           and ranks/files
"""
board = Board('8/8/8/8/8/5K2/6Q1/7k b - - 0 1')

# Stalemate test
""" 
No bugs found
"""
# board = Board('8/8/8/8/8/5KQ1/8/7k b - - 1 1')


# board = Board('r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4')
# Castling test
# board = Board('r3k2r/pppqbppp/2npbn2/4p3/2B1P3/P1NPBN2/1PPQ1PPP/R3K2R w KQkq - 1 9')
# En passant test
# board = Board('rnbqkbnr/pppp2pp/8/3Pp3/4Pp2/2P5/PP3PPP/RNBQKBNR w KQkq - 0 5')


# Test 1: Try to move a piece outside of the board
# Test 2: Try to move a piece to the same square it's on
# Test 3: Try to move a piece that doesn't exist
# Test 2: Try to move an enemy piece

# test_moves = ["j2j4","e2e2","e3e4","e7e5","e2e4"]
# test_fen = ["","","","",""]
# for i, move in enumerate(test_moves):
#     if len(test_fen[i]) == 0:
#         board = Board()
#     else:
#         board = Board(test_fen[i])
#     test_result = board.move(move)
#     if test_result == 1:
#         print("Test " + str(i+1) + " passed.")
#     else:
#         print("Test " + str(i+1) + " failed.")


if board.is_EOG() == 0:
    print(board)
    while True:
        if board.white_to_move:
            user_move = input("Enter a move for White: ")
        else:
            user_move = input("Enter a move for Black: ")
        
        if user_move.lower() == 'q' or user_move.lower() == 'quit':
            break
        
        move_result = board.move(user_move)
        if move_result == 1:
            print("Invalid move. Try again.\n")
        else:
            # If the board state isn't checkmate, or stalemate continue
            # TODO: Need to check for repetition, etc. as well.
            if board.is_EOG() == 0:
                print(board)
            else:
                if board.is_EOG() == 1:
                    if not board.white_to_move:
                        print("Game over. White wins by checkmate.")
                        break
                    else:
                        print("Game over. Black wins by checkmate.")
                        break
                elif board.is_EOG() == 2:
                    print("Game over. Draw by stalemate.")
                    break
else:
    if board.is_EOG() == 1:
        if not board.white_to_move:
            print("Game over. White wins by checkmate.")
        else:
            print("Game over. Black wins by checkmate.")
    elif board.is_EOG() == 2:
        print("Game over. Draw by stalemate.")


