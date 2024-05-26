#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun May 19 20:49:15 2024

@author: ejgipson
"""
import re
# Pesudo Risk Bot Option 1.1
# This function assumes that both starting and ending boardstates are legal and that the end state is achievable from the start state
# This function assumes that a test has been performed for a King in Check
# This function assigns a "risk score" for both White and Black that reflects the risk associated with moving from a starting board state to an ending board state

sFEN = "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1 w"
eFEN = "8/8/8/4p1K1/2k1P3/8/8/8 b"

# Input: (Current Board State, Board State After Legal Move) <--([FEN],[FEN])
def pseudo_risk(sFEN,eFEN):
    # [Placeholder]
    # Throw an error for inapropriate inputs
    
    # Take only the portion of the FEN that defines material
    sFEN = sFEN.split(" ")[0]
    eFEN = eFEN.split(" ")[0]
    
    # Initialize a storage location for material points
    wCOUNT = [0,0]
    bCOUNT = [0,0]

    # Counting pawns : p = 1, P = 1
    bCOUNT[0] = len(re.findall("p",sFEN))
    wCOUNT[0] = len(re.findall("P",sFEN))
    bCOUNT[1] = len(re.findall("p",eFEN))
    wCOUNT[1] = len(re.findall("P",eFEN))
    # Counting rooks : r = 5, R = 5
    bCOUNT[0] = bCOUNT[0] + 5*len(re.findall("r",sFEN))
    wCOUNT[0] = wCOUNT[0] + 5*len(re.findall("R",sFEN))
    bCOUNT[1] = bCOUNT[1] + 5*len(re.findall("r",eFEN))
    wCOUNT[1] = wCOUNT[1] + 5*len(re.findall("R",eFEN))
    # Counting knights : n = 3, N = 3
    bCOUNT[0] = bCOUNT[0] + 3*len(re.findall("n",sFEN))
    wCOUNT[0] = wCOUNT[0] + 3*len(re.findall("N",sFEN))
    bCOUNT[1] = bCOUNT[1] + 3*len(re.findall("n",eFEN))
    wCOUNT[1] = wCOUNT[1] + 3*len(re.findall("N",eFEN))
    # Counting bishops : b = 3, B = 3
    bCOUNT[0] = bCOUNT[0] + 3*len(re.findall("b",sFEN))
    wCOUNT[0] = wCOUNT[0] + 3*len(re.findall("B",sFEN))
    bCOUNT[1] = bCOUNT[1] + 3*len(re.findall("b",eFEN))
    wCOUNT[1] = wCOUNT[1] + 3*len(re.findall("B",eFEN))
    # Counting crows, I mean queens : q = 9, Q = 9
    bCOUNT[0] = bCOUNT[0] + 9*len(re.findall("q",sFEN))
    wCOUNT[0] = wCOUNT[0] + 9*len(re.findall("Q",sFEN))
    bCOUNT[1] = bCOUNT[1] + 9*len(re.findall("q",eFEN))
    wCOUNT[1] = wCOUNT[1] + 9*len(re.findall("Q",eFEN))

    # Initialize a risk score of 0.00 for both White and Black
    wRISK = 0.00
    bRISK = 0.00

    if (wCOUNT[1] - wCOUNT[0])<0:
        wRISK = (1 - wCOUNT[1]/39) - (1 - wCOUNT[0]/39)
        
    if (bCOUNT[1] - bCOUNT[0])<0:
        bRISK = (1 - bCOUNT[1]/39) - (1 - bCOUNT[0]/39)

# Output: Floating point number between [0.00, 1.00]; 0.00 is No Risk, 1.00 maximal risk (assuming checking for Check and Checkmate has already been done)      
    print("White Risk Score : %1.2f" % (wRISK))
    print("Black Risk Score : %1.2f" % (bRISK)) 

pseudo_risk(sFEN,eFEN)
#%%
    # Switch case alert!
    # If the difference between starting points and ending points is 0, you have lost no pieces--> Risk=0.00
    # If the difference between starting points and ending points is 1, you have lost a pawn-->Risk>0.00
    # If the difference is 3, you have lost either a knight or bishop --> Risk>0.00 & greater than the Risk value for losing a pawn
    # If the difference is 5, you have lost a rook --> Risk >0.00 & greater than the risk value for losing a knight or bishop
    # If the difference is 9, you have lost your queen --> Risk=1.00
#%%
# Input thoughts: do I need to have the two FENs or simply "instructions on where to find each FEN" (pointer); for now I can start with two FENs but this is an opportunity to make the input for agnostic in the future

# Output thoughts: I scale down between 0 and 1 because that's what you do in ML for weights; integer numbers also could be fine but who cares

# Output thoughts: You could also use riskbot in "reverse"; if you count your opponenets points before and after a legal move, the move that produces the biggest change in points (i.e., taking their queen), would have a high anti-risk score --> you would want to do this over another move

# Output thoughts: See previous, you could also count your points before and after a move and pick a move that increases your own points. Promoting a paw earns you 9 points and would have a high anti-risk score --> you would want to do this over another move

# Output though: There's weighting to be done. If promoting a pawn is not possible from the list of legal moves that anti-riskbot is applied to (e.g., riskbot([start_FEN], [end_FEN_1]) through riskbot([start_FEN], [end_FEN_*]) does not ever contain a pawn promotion, then you don't give a shit), then you only care about moves that minimize your opponents points. If promoting a pawn is possible AND taking a queen is possible, then anti-riskbot would give both equal weights. If promoting a pawn is possible AND you cannot take a queen AND you can take a rook, then anti-riskbot would rank promoting the pawn higher than taking the rook etc. etc. 

# SWITCH thoughts:
    # Obviously this is limitted only to piece value; space for more robust execution could include a pieces point value with some scaling factor assocaited with position. An opponent moving a knight from the center to the edge would have a lower risk value than an opponent moving a knight to the center, regardless of total point value
    # Losing a pawn: "How" the lost pawn affects the board could be a place for depth --> losing a pawn that opens up a bishop might be beneficial, losing a pawn that protects your king would not be beneficial etc. etc. 
    # Losing a knight or bishop: since these pieces have the same points, you could play with where the lost piece was (a knight lost from the center is worse than a knight lost at the edge) OR you could play with style. A true chess bot might play a knight heavy game and losing a knight might be worth 4 points rather than 3, making a bishop worth only 2 or vise versa. 3-3 assumes that the player relies and implements knights and bishops equitably
