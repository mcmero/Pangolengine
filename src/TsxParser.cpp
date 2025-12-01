#include "TsxParser.h"
#include <memory>
#include <sstream>

TsxToken::TsxToken(Type tokenType, const std::string& value, uint32_t line,
                    uint32_t col) : type(tokenType) {
    this->value = value;
    this->line = line;
    this->column = col;
}

//------------------------------------------------------------------------------
// TsxTokeniser Implementation
//------------------------------------------------------------------------------

TsxTokeniser::TsxTokeniser(std::istream &in) : Tokeniser(in) {}

std::unique_ptr<IToken> TsxTokeniser::getTokenImpl() {
  skipWhitespace();

  int ch = in.peek();
  if (ch == EOF) {
    return makeToken<TsxToken>(TsxToken::Type::EndOfFile, "", line, column);
  }

  uint32_t startLine = line;
  uint32_t startCol  = column;

  if (ch == '<') {
    getChar();
    std::string str = parseAlpha();
    if (str == "?xml")
      return makeToken<TsxToken>(
        TsxToken::Type::PIStart, "<?xml", line, column
      );
    else
      return makeToken<TsxToken>(
        TsxToken::Type::ElementStart, "<" + str, line, column
      );
  }

  return makeToken<TsxToken>(
    TsxToken::Type::Error,
    std::string{static_cast<char>(ch)},
    startLine, startCol
  );
}

