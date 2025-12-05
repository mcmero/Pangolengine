#include "TsxParser.h"
#include <memory>
#include <sstream>
#include <fstream>

using std::ifstream;

TsxToken::TsxToken(Type tokenType, const std::string &value, uint32_t line,
                   uint32_t col)
    : type(tokenType) {
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
  const char terminators[] = "=>";

  if (ch == '<') {
    getChar();
    ch = in.peek();

    if (ch == '/') {
      getChar();
      std::string str = parseAlpha(terminators);
      ch = in.peek();
      if (ch == '>') {
        getChar();
        return makeToken<TsxToken>(
          TsxToken::Type::ElementEnd, "</" + str + ">", line, column
        );
      }
    } else if (ch == '?') {
      getChar();
      std::string str = parseAlpha(terminators);
      if (str == "xml")
        return makeToken<TsxToken>(
          TsxToken::Type::PIStart, "<?xml", line, column
        );
    } else {
      std::string str = parseAlpha(terminators);
      return makeToken<TsxToken>(
        TsxToken::Type::ElementStart, "<" + str, line, column
      );
    }
  } else if (ch == '?') {
    getChar();
    ch = in.peek();
    if (ch == '>') {
      getChar();
      return makeToken<TsxToken>(
        TsxToken::Type::PIEnd, "?>", line, column
      );
    }
  } else if (ch == '/') {
    getChar();
    ch = in.peek();
    if (ch == '>') {
      getChar();
      return makeToken<TsxToken>(
        TsxToken::Type::ElementEnd, "/>", line, column
      );
    }
  } else if (ch == '=') {
    getChar();
    return makeToken<TsxToken>(
      TsxToken::Type::Equals, "=", line, column
    );
  } else if (ch == '"') {
    std::string str = parseString();
    return makeToken<TsxToken>(
      TsxToken::Type::String, str, line, column
    );
  } else if (ch == '>') {
    getChar();
    return makeToken<TsxToken>(
      TsxToken::Type::ElementEnd, ">", line, column
    );
  } else {
    std::string str = parseAlpha(terminators);
    return makeToken<TsxToken>(
      TsxToken::Type::Attribute, str, line, column
    );
  }

  return makeToken<TsxToken>(
    TsxToken::Type::Error,
    std::string{static_cast<char>(ch)},
    startLine, startCol
  );
}

//------------------------------------------------------------------------------
// TsxValue Implementation
//------------------------------------------------------------------------------

TsxNode::TsxNode() : name("") {}
TsxNode::TsxNode(std::string name) : name(name) {}

//------------------------------------------------------------------------------
// TxsParser Implementation
//------------------------------------------------------------------------------

TsxNode TsxParser::parseTsx(const std::string &file) {
  ifstream f(file);
  if (!f.is_open()) {
    std::stringstream ss;
    ss << "Failed to open file: " << file << std::endl;
    throw std::runtime_error(ss.str());
  }

  TsxTokeniser tokeniser {f};
  return parseNode(tokeniser);
}

TsxNode TsxParser::parseNode(TsxTokeniser &tokeniser) {
  TsxNode node = {};

  std::unique_ptr<IToken> token = tokeniser.peekToken();
  TsxToken* tsxToken = dynamic_cast<TsxToken*>(token.get());
  if (tsxToken->type != TsxToken::Type::PIStart)
      throw std::runtime_error(
      "Unexpected character found at start of object. Expected PI start (<?xml)."
      );
  token = tokeniser.getToken();

  while (true) {
    token = tokeniser.getToken();
    tsxToken = dynamic_cast<TsxToken*>(token.get());
    if (tsxToken->type == TsxToken::Type::EndOfFile)
      break;
  }

  return node;
}
