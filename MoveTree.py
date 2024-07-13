# -*- coding: utf-8 -*-
"""
Created on Sun May 26 09:44:55 2024

@author: zachg
"""

from Board import Board
import copy

# TODO: Find a way to actually delete the board states from memory as you go
# TODO: Find ways to prune reduant branches
# TODO: Why is the initial board state changing by the end of the tree creationn?
#       Do I need to use deepcopy()? The tree seems to be generating ok recursively...
# TODO: # If there aren't legal moves, it's either checkmate or stalemate
#       so need to figure out what to do there
# 6/9/24, ok I got it to actually traverse the list and find the leaf nodes.
# I think this is a good start.
# TODO: for some reason it's not counting material correctly...
# Need to figure out if I eed to use this deepcopy thing. I have a feeling it's going to be slow.

class TreeNode:
    def __init__(self,board,parent,move):
        self.DEPTH = 3
        #self.board = board
        self.board = copy.deepcopy(board)
        self.parent = parent
        self.move = move
        
        if self.move:
            self.board.move(self.move)
        self.material = self.board.count_material()
        self.board_state = self.board.board_state.copy()
        self.responses = self.board.get_valid_moves()
        self.children = []
        
        # If there are legal moves in the position
        if self.responses and (self.get_level() < self.DEPTH):
            for i in self.responses:
                child = TreeNode(board,self,i)
                self.children.append(child)
        
        
    def get_level(self):
        level = 0
        p = self.parent
        while p:
            level += 1
            p = p.parent
        return level
    
    def get_leaf_nodes(self,node,leafs):
        for child in node.children:
            if len(child.children) == 0:
                leafs.append(child)
            else:
                self.get_leaf_nodes(child,leafs)
        return leafs
        
    
    def print_tree(self):
        space = ' ' * self.get_level() * 3
        prefix = space + '|__' if self.parent else ''
        print(prefix + self.move)
        if self.children:
            for child in self.children:
                child.print_tree()

if __name__ == "__main__":
    board = Board('4k3/8/2p5/1p6/P7/8/8/4K3 w - - 0 1')
    print(board)
    #board = Board('8/8/8/8/8/5KQ1/8/4k3 b - - 1 1')
    tree = TreeNode(board,None,"Root")
    tree.print_tree()
    
    leafs = tree.get_leaf_nodes(tree,[])
    