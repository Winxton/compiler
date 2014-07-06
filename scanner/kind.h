#ifndef __KIND_H__
#define __KIND_H__
#include <string>
#include <iostream>
namespace ASM{
  // The different kinds of Tokens that are part of different MIPS instructions
  // Used for determining the correct Token to create in the MIPS recognizer
  enum Kind{
    ID,  // a string consisting of a letter (in the range a-z or A-Z) followed by zero or more letters and digits (in the range 0-9)
    NUM,  // a string consisting of a single digit (in the range 0-9) or two or more digits the first of which is not 0
    LPAREN,  // the string "("
    RPAREN,  // the string ")"
    LBRACE,  // the string "{"
    RBRACE,  // the string "}"
    RETURN,  // the string "return" (in lower case)
    IF,  // the string "if"
    ELSE,  // the string "else"
    WHILE,  // the string "while"
    PRINTLN,  // the string "println"
    WAIN,  // the string "wain"
    BECOMES,  // the string "="
    INT,  // the string "int"
    EQ,  // the string "=="
    NE,  // the string "!="
    LT,  // the string "<"
    GT,  // the string ">"
    LE,  // the string "<="
    GE,  // the string ">="
    PLUS,  // the string "+"
    MINUS,  // the string "-"
    STAR,  // the string "*"
    SLASH,  // the string "/"
    PCT,  // the string "%"
    COMMA,  // the string ","
    SEMI,  // the string ";"
    NEW,  // the string "new"
    DELETE,  // the string "delete"
    LBRACK,  // the string "["
    RBRACK,  // the string "]"
    AMP,  // the string "&"
    NULLSTRING,  // the string "NULL"
    WHITESPACE,   // WHITESPACE
    ERR           // ERR
  };

  // A Token class representing the concrete functions we
  // might want to apply to a MIPS Token

  class Token{
  protected:
    // The kind of the Token
    Kind kind;
    // The actual string representing the Token
    std::string lexeme;
  public:
    // A Factory Method that allows us to make specific Tokens
    // based upon the Kind
    static Token* makeToken(Kind kind, std::string lexeme);
    Token(Kind kind, std::string lexeme);
    // Convenience functions for operations we might like to
    // use on a Token
    virtual int toInt() const;
    std::string toString() const;
    std::string getLexeme() const;
    Kind getKind() const;
  };

  // Overload the output operator for Tokens
  std::ostream& operator<<(std::ostream& out, const Token& t);
}
#endif
