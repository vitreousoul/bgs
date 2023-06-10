#!/usr/bin/env python3

"""
Run this file to see if things are working
"""

import sys

import Board
import pgn

DIVIDER_TEXT = '=' * 32

ARGV_MAP = {
    '-v': 'verbose',
    '--verbose': 'verbose',
}

# Define test functions statically so that we can loop through them.
# This means all test functions must accept the same arguments.
test_functions = [
    (Board.test, 'Board'),
    (pgn.test, 'pgn')
]

def test_all(arg_object):
    total_tests = 0
    failed_tests = 0
    # run the tests
    for test_function, test_title in test_functions:
        print(f"Testing {test_title}...")
        total_tests += 1
        failure_code = test_function(arg_object)
        if failure_code:
            print(f"Error in test {test_title} with code {failure_code}")
            failed_tests += 1
    # show the test results
    passed_tests = total_tests - failed_tests
    print(f"\n{DIVIDER_TEXT}")
    print("Tests Complete")
    print(f"({passed_tests}/{total_tests}) passed tests")
    print(f"{failed_tests} failed tests")
    print(DIVIDER_TEXT)

if __name__ == "__main__":
    args = sys.argv[1:]
    arg_object = {}
    # quick and dirty arg parsing for setting basic flags
    for arg in args:
        if (ARGV_MAP[arg]):
            arg_object[ARGV_MAP[arg]] = True
    test_all(arg_object)
