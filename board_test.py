"""
Test of current features
Simple board where the user is prompted to make moves for white and black
"""

from Board import Board

# An alternate FEN to test as an input
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