"""
PGN Parser
http://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm
"""

from enum import Enum
from dataclasses import dataclass

# TODO: we haven't parsed yet, but once we do, delete ParserState if it is not used
class ParserState(Enum):
    TAG_PAIR = 0   # DOCS 8.1: Tag pair section
    MOVE_TEXT = 1  # DOCS 8.2: Movetext section

class TokenKind(Enum):
    NONE = 0
    # use ASCII value for single-char kinds to make single-char tokenizing easier
    PERIOD = ord('.')
    ASTERISK = ord('*')
    BRACKET_OPEN = ord('[')
    BRACKET_CLOSE = ord(']')
    PAREN_OPEN = ord('(')
    PAREN_CLOSE = ord(')')
    ANGLE_OPEN = ord('<')
    ANGLE_CLOSE = ord('>')
    # multi-char token values must be outside of ASCII upper-range (127)
    STRING = 128
    NUMERIC_ANNOTATION_GLYPH = 129
    SYMBOL = 130
    INTEGER = 131
    END_OF_STREAM = 256

@dataclass
class Token:
    kind: TokenKind = TokenKind.NONE
    text: str = ''
    number: int = 0

def tokenize_pgn(pgn_source):
    tokens = []
    index = 0
    error = False
    while index < len(pgn_source):
        token, index = get_next_token(pgn_source, index)
        token_success = token and token.kind != TokenKind.NONE
        if token_success and index >= 0:
            tokens.append(token)
        else:
            error = True
            print(f"Token error!\n    token={token}\n    index={index}")
            break
    # Return an empty array if any errors. We may want a better way to signal an error...
    return [] if error else tokens

def get_next_token(pgn_source, index):
    token = Token(kind=TokenKind.NONE)
    index = eat_space(pgn_source, index)
    if index >= len(pgn_source):
        # signal the end of pgn_source and end of token stream
        return Token(kind=TokenKind.END_OF_STREAM), index
    char = pgn_source[index]
    if is_single_char_token(char):
        # single-char token trick only works because we set the TokenKind value equal to its ASCII value
        token = Token(kind=ord(char))
        index += 1
    elif char == '"':
        token, index = parse_string_token(pgn_source, index)
    elif char == '$':
        token, index = parse_numeric_annotation_glyph_token(pgn_source, index)
    elif is_alphanum(char):
        token, index = parse_symbol_token(pgn_source, index)
    else:
        print(f"Unrecognized token begin-char: {ord(char)}    index={index}")
        return Token(kind=TokenKind.NONE), -1
    return token, index

def eat_space(pgn_source, index):
    while index < len(pgn_source):
        char = pgn_source[index]
        if is_space(char):
            index += 1
        else:
            break
    return index

def parse_string_token(pgn_source, index):
    # assume quote char was matched to get to this function, so increment index
    index += 1
    start_index = index
    while index < len(pgn_source):
        char = pgn_source[index]
        if char == '\\':
            # backslash escape char, ignore next char
            index += 2
        elif char == '"':
            # end of string
            index += 1
            break
        else:
            index += 1
    token = Token(kind=TokenKind.STRING,
                  text=pgn_source[start_index:index-1])
    return token, index

def parse_numeric_annotation_glyph_token(pgn_source, index):
    # assume the dollar ($) character was matched to get to this functions, so increment index
    index += 1
    start_index = index
    while True:
        char = pgn_source[index]
        if is_digit(char):
            index += 1
        else:
            break
    numeric_value = int(pgn_source[start_index:index]) if index > start_index else -1
    if numeric_value >= 0 and numeric_value <= 255:
        token = Token(kind=TokenKind.NUMERIC_ANNOTATION_GLYPH, number=numeric_value)
        return token, index
    else:
        print(f"Numeric Annotation Glyph value out of range {numeric_value}")
        return Token(kind=TokenKind.NONE), -1

def parse_symbol_token(pgn_source, index):
    start_index = index
    is_integer = is_digit(pgn_source[index])
    # assume we matched the symbol start-char to get to this function, so increment index
    index += 1
    token = Token(kind=TokenKind.NONE)
    while index < len(pgn_source):
        char = pgn_source[index]
        if is_symbol_continuation_char(char):
            if not is_digit(char):
                is_integer = False
            index += 1
        else:
            break
    text = pgn_source[start_index:index]
    if is_integer:
        token = Token(kind=TokenKind.INTEGER, number=int(text))
    else:
        token = Token(kind=TokenKind.SYMBOL, text=text)
    return token, index

def parse_pgn(pgn_source):
    tokens = tokenize_pgn(pgn_source)
    return tokens

def is_space(char):
    return (char == ' ' or char == '\n' or char == '\r' or char == '\t')

def is_single_char_token(char):
    return (char == '.' or char == '*' or char == '[' or char == ']' or
            char == '(' or char == ')' or char == '<' or char == '>')

def is_symbol_continuation_char(char):
    # NOTE: '/' is not really a symbol-continuation char according to the docs, but
    # it's easier to include it, and parse out game-termination-markers in a later stage.
    return (is_alphanum(char) or
            char == '_' or char == '+' or char == '#' or
            char == '=' or char == ':' or char == '-' or
            char == '/')

def is_digit(char):
    return char >= '0' and char <= '9'

def is_uppercase(char):
    return char >= 'A' and char <= 'Z'

def is_lowercase(char):
    return char >= 'a' and char <= 'z'

def is_alpha(char):
    return is_lowercase(char) or is_uppercase(char)

def is_alphanum(char):
    return is_alpha(char) or is_digit(char)

def test(arg_object):
    pgn_file = open('./assets/1992_11_04_fischer_spassky.pgn', 'r')
    pgn_source = pgn_file.read()
    pgn_file.close()
    tokens = parse_pgn(pgn_source)
    if 'verbose' in arg_object:
        for token in tokens:
            print(token)
    return 0 if len(tokens) > 0 else 1
