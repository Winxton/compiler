#ifndef __LEXER_H__
#define __LEXER_H__
#include <string>
#include <vector>
#include "kind.h"

namespace ASM{
  // The different states the the MIPS recognizer uses
  // Judicious use of the pimpl idiom or the Bridge pattern
  // would allow us to place this in the implementation file
  // However, that's more complexity than is necessary
  enum State {
    ST_ERR,
    ST_START,
    ST_ID,
    ST_NUM,  // a string consisting of a single digit (in the range 0-9) or two or more digits the first of which is not 0
    ST_LPAREN,  // the string "("
    ST_RPAREN,  // the string ")"
    ST_LBRACE,  // the string "{"
    ST_RBRACE,  // the string "}"
    ST_BECOMES,  // the string "="
    ST_EQ,  // the string "=="
    ST_NE,  // the string "!="
    ST_LT,  // the string "<"
    ST_GT,  // the string ">"
    ST_LE,  // the string "<="
    ST_GE,  // the string ">="
    ST_PLUS,  // the string "+"
    ST_MINUS,  // the string "-"
    ST_STAR,  // the string "*"
    ST_SLASH,  // the string "/"
    ST_PCT,  // the string "%"
    ST_COMMA,  // the string ","
    ST_SEMI,  // the string ";"
    ST_LBRACK,  // the string "["
    ST_RBRACK,  // the string "]"
    ST_AMP,  // the string "&"
    ST_ZERO, // just the number zero
    ST_WHITESPACE,
    ST_COMMENT,
    ST_NOT
  };
  // Forward declare the Token class to reduce compilation dependencies
  class Token;

  // Class representing a MIPS recognizer
  class Lexer {
    // At most 21 states and 256 transitions (max number of characters in ASCII)
    static const int maxStates = 29;
    static const int maxTrans = 256;

    // Transition function
    State delta[maxStates][maxTrans];
    // Private method to set the transitions based upon characters in the
    // given string
    void setTrans(State from, const std::string& chars, State to);
    bool inArr(ASM::Kind arr[], ASM::Kind kind);
    void assertWhiteSpace(std::vector<Token*> &ret, ASM::Kind currKind, bool hasPrecedingWhitespace);
    // deletes created vectors upon error
    void cleanUp(std::vector<Token*> &ret);
  public:
    Lexer();
    // Output a vector of Tokens representing the Tokens present in the
    // given line
    std::vector<Token*> scan(const std::string& line);
  };
}

#endif
