"""
Test of current features
Simple board where the user is prompted to make moves for white and black

TODO: Removed automatic print statements from Board.py. Instead, have the UI
   module handle printing the board and error messages.
"""

from Board import Board

# An alternate FEN to test as an input
# board = Board('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1')
# board = Board('rnbqkbnr/pppp1ppp/8/4p3/4PP1q/8/PPPP2PP/RNBQKBNR w KQkq - 0 1')

board = Board()


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
        print(board)
    
""" 
TODO: Test move valdation
So far, pawn moves from their initial squares, e4 and e5 have been tested.
Need to come up with a testing plan to cover every branch
 of the validation.
"""