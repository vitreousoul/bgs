# -*- coding: utf-8 -*-
"""
Created on Sun May 26 09:44:55 2024

@author: zachg
"""

# -*- coding: utf-8 -*-
"""
Created on Mon May 20 18:50:05 2024

@author: zachg
"""

# https://python.plainenglish.io/data-structures-in-python-tree-410255b87107

from Board import Board

# TODO: When the first move isn't valid, need to make it not able to continue
# Find a way to actually delete the board states from memory as you go

class TreeNode:
    def __init__(self,board,parent,move):
        self.DEPTH = 3
        self.board = board
        self.parent = parent
        self.move = move
        
        if self.move:
            self.board.move(self.move)
        self.responses = self.board.get_valid_moves()
        self.children = []
        # If there are legal moves in the position
        if self.responses:
            if self.get_level() < self.DEPTH:
                for i in self.responses:
                    child = TreeNode(board,self,i)
                    self.children.append(child)
                #self.board = None
        # If there aren't legal moves, it's either checkmate or stalemate
        #    so need to figure out what to do there
        
    def get_level(self):
        level = 0
        p = self.parent
        while p:
            level += 1
            p = p.parent
        return level
    
    def print_tree(self):
        space = ' ' * self.get_level() * 3
        prefix = space + '|__' if self.parent else ''
        print(prefix + self.move)
        if self.children:
            for child in self.children:
                child.print_tree()

if __name__ == "__main__":
    board = Board('4k3/8/8/p7/P7/8/8/4K3 w - - 0 1')
    tree = TreeNode(board,None,"Root")
    tree.print_tree()
    
    