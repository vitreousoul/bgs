#!/usr/bin/env python3

"""
Run this file to see if things are working
"""

import Board
import pgn

DIVIDER_TEXT = '=' * 32

def test_all():
    total_tests = 0
    failed_tests = 0
    # run the tests
    Board.test()
    pgn.test()
    # show the test results
    passed_tests = total_tests - failed_tests
    print(f"\n{DIVIDER_TEXT}")
    print("Tests Complete")
    print(f"({passed_tests}/{total_tests}) passed tests")
    print(f"{failed_tests} failed tests")
    print(DIVIDER_TEXT)

if __name__ == "__main__":
    # we can check command-line args here if we want more control over _which_ tests we run
    test_all()
