#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun May 19 20:49:15 2024

@author: ejgipson
"""
import re
# Risk Bot Option 1.1

# Input: (Current Board State, Board State After Legal Move, [for whom]) <--([FEN],[FEN])
sFEN = "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 w"
eFEN = "8/8/8/4p1K1/2k1P3/8/8/8 b"

# Strip off a trailing b from either FEN

# White
sW = re.findall("[A-Z]", sFEN)
eW = re.findall("[A-Z]", eFEN)

# Black 
sB = re.findall("[a-z]", sFEN)
eB = re.findall("[a-z]", eFEN)

wCOUNT = [0,0]
bCOUNT = [0,0]

# p = 1, P = 1

# r = , R = 

# n = , N = 

# b = , B = 

# q = , Q = 
# Output: Floating point number between [0.00, 1.00]; 0.00 is No Risk, 1.00 maximal risk (assuming checking for Check and Checkmate has already been done)

# HOW:
    # Using the "standard" point value for each piece, the number of points is calculated based on current board state. E.g., if you have all of your pieces, you have all 39 points. If you have lost a rook, you have 34 points.

    # Using the "standard" point value for each piece, the number of points is calculated based on Board State After Legal Move. 
    
    # Switch case alert!
    # If the difference between starting points and ending points is 0, you have lost no pieces--> Risk=0.00
    # If the difference between starting points and ending points is 1, you have lost a pawn-->Risk>0.00
    # If the difference is 3, you have lost either a knight or bishop --> Risk>0.00 & greater than the Risk value for losing a pawn
    # If the difference is 5, you have lost a rook --> Risk >0.00 & greater than the risk value for losing a knight or bishop
    # If the difference is 9, you have lost your queen --> Risk=1.00
    
# %%

# Input thoughts: do I need to have the two FENs or simply "instructions on where to find each FEN" (pointer); for now I can start with two FENs but this is an opportunity to make the input for agnostic in the future

# Input thoughts: [for whom] is w or b and calculates who the risk score is for; if the first FEN indicates black to move, and the second indicates white to move, you'd like to know the risk to white associated with black's movements

# Output thoughts: I scale down between 0 and 1 because that's what you do in ML for weights; integer numbers also could be fine but who cars

# Output thoughts: You could also use riskbot in "reverse"; if you count your opponenets points before and after a legal move, the move that produces the biggest change in points (i.e., taking their queen), would have a high anti-risk score --> you would want to do this over another move

# Output thoughts: See previous, you could also count your points before and after a move and pick a move that increases your own points. Promoting a paw earns you 9 points and would have a high anti-risk score --> you would want to do this over another move

# Output though: There's weighting to be done. If promoting a pawn is not possible from the list of legal moves that anti-riskbot is applied to (e.g., riskbot([start_FEN], [end_FEN_1]) through riskbot([start_FEN], [end_FEN_*]) does not ever contain a pawn promotion, then you don't give a shit), then you only care about moves that minimize your opponents points. If promoting a pawn is possible AND taking a queen is possible, then anti-riskbot would give both equal weights. If promoting a pawn is possible AND you cannot take a queen AND you can take a rook, then anti-riskbot would rank promoting the pawn higher than taking the rook etc. etc. 

# SWITCH thoughts:
    # Obviously this is limitted only to piece value; space for more robust execution could include a pieces point value with some scaling factor assocaited with position. An opponent moving a knight from the center to the edge would have a lower risk value than an opponent moving a knight to the center, regardless of total point value
    # Losing a pawn: "How" the lost pawn affects the board could be a place for depth --> losing a pawn that opens up a bishop might be beneficial, losing a pawn that protects your king would not be beneficial etc. etc. 
    # Losing a knight or bishop: since these pieces have the same points, you could play with where the lost piece was (a knight lost from the center is worse than a knight lost at the edge) OR you could play with style. A true chess bot might play a knight heavy game and losing a knight might be worth 4 points rather than 3, making a bishop worth only 2 or vise versa. 3-3 assumes that the player relies and implements knights and bishops equitably
    
# Implimentation plan:
    # Counting tool: reads in a board state and counts piece value
    # Assigning a risk score based on the switch case
