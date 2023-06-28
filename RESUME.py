#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Jun 25 16:43:55 2023

@author: ejgipson
"""

from Board import Board

while True:
    try:
        resume = input("Enter any letter: ")
        if resume.isalpha():
            pass
        else:
            print("Sorry, I didn't understand that.")
            raise ValueError
    except ValueError:
            continue
    else:
                if resume == 'N' or resume == 'n':
                    # NEW should overwrite a previous save.log file 
                    # NEW should execute board_test.py as written
                    print("Starting NEW game, previous saved data destroyed.")
                    board = Board()
                    print(board)
                    break
                elif resume == 'R' or resume == 'r':
                    # RESUME Sr. board = Board([Last line FEN])
                    # RESUME Jr. board = Board('rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1')
                    # RESUME should open the existing save.log file and append information
                    print("RESUMing previous game.")
                    break
                elif resume == 'Q' or resume == 'q':
                    print("Eat glass, soy boy.")
                    break
                elif resume != 'N' and resume != 'n' and resume != 'R' and resume != 'r':
                    print("Sorry, I didn't understand that.")
                    continue
    