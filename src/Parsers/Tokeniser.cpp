#include "Tokeniser.h"
#include <memory>
#include <sstream>

Tokeniser::Tokeniser(std::istream &in) : in(in), line(1), column(1) {}

std::unique_ptr<IToken> Tokeniser::getToken() {
  if (lookahead_) {
    auto token = std::move(lookahead_);
    lookahead_ = nullptr;
    return token;
  }
  return getTokenImpl();
}

std::unique_ptr<IToken> Tokeniser::peekToken() {
  if (!lookahead_)
    lookahead_ = getTokenImpl();
  return lookahead_->clone();
}

void Tokeniser::skipWhitespace() {
   while (true) {
      int ch = in.peek();
      if (ch == EOF)
        return;
      if (std::isspace(static_cast<unsigned char>(ch))) {
          getChar();
          continue;
      }
      break;
  }
}

int Tokeniser::getChar() {
  int ch = in.get();
  if (ch == '\n') {
    line++;
    column = 1;
  } else if (ch != EOF) {
    column++;
  }
  return ch;
}

std::string Tokeniser::parseString() {
  int ch = getChar();

  // We first need a quote character
  if (static_cast<char>(ch) != '"')
    raiseError("Expected quote at start of string");

  std::stringstream result;
  while (true) {
    ch = getChar();

    if (ch == EOF) {
      raiseError("Unexpected end of file while parsing string");
    }

    if (static_cast<char>(ch) == '"')
      break; // end of string

    if (static_cast<char>(ch) == '\\') {
        int esc = getChar();
        if (esc == EOF)
          raiseError("Unexpected end of file in escape sequence");

        char ec = static_cast<char>(esc);
        switch (ec) {
          case '"':  result << '"' ; break;
          case '\\': result << '\\'; break;
          case '/':  result << '/' ; break;
          case 'b':  result << '\b'; break;
          case 'f':  result << '\f'; break;
          case 'n':  result << '\n'; break;
          case 'r':  result << '\r'; break;
          case 't':  result << '\t'; break;
          case 'u':
              raiseError("\\u escapes not implemented");
          default:
              raiseError(
                std::string("Invalid escape: \\") + ec
              );
        }
    } else
      result << static_cast<char>(ch);
  }
  return result.str();
}

std::string Tokeniser::parseAlpha(const char terminators[]) {
  int ch;

  std::stringstream result;
  std::string errorMsg = "Unexpected character while parsing alphabetical value";
  while (true) {
    ch = in.peek();

    if (ch == EOF) {
      raiseError(errorMsg);
    }

    if (isalpha(static_cast<unsigned char>(ch)))
      result << static_cast<char>(getChar());
    else if (std::isspace(static_cast<unsigned char>(ch)) ||
             memchr(terminators, static_cast<unsigned char>(ch),
                    strlen(terminators)))
      break;
    else
      raiseError(errorMsg);
  }
  return result.str();
}

std::string Tokeniser::parseNumber() {
  int ch = in.peek();
  std::string errorMsg = "Unexpected character while parsing number value";

  if (static_cast<int>(ch) == EOF)
    raiseError(errorMsg);

  std::stringstream result;

  // Handle negative numbers
  if (ch == '-') {
    result << static_cast<char>(getChar());
    ch = in.peek();
  }

  if (ch == '0') { // leading zero
    result << static_cast<char>(getChar());
    ch = in.peek();
  } else if (std::isdigit(static_cast<unsigned char>(ch)))
    result << parseDigits();
  else
    raiseError(errorMsg);

  ch = in.peek();
  if (ch == '.') { // decimal
    result << static_cast<char>(getChar());
    ch = in.peek();
    if (isdigit(static_cast<unsigned char>(ch)))
      result << parseDigits();
    else
      raiseError(errorMsg);
  }

  ch = in.peek();
  if (ch == 'e' || ch == 'E') {
    result << static_cast<char>(getChar());
    ch = in.peek();

    if (ch == '+' || ch == '-') {
      result << static_cast<char>(getChar());
      ch = in.peek();
    }

    if (isdigit(static_cast<unsigned char>(ch)))
      result << parseDigits();
    else
      raiseError("Expected digits after exponent");
  }

  return result.str();
}

std::string Tokeniser::parseDigits() {
  int ch = in.peek();
  std::stringstream result;

  while (true) {
    if (isdigit(static_cast<unsigned char>(ch))) {
      result << static_cast<char>(getChar());
      ch = in.peek();
    } else
      break;
  }

  return result.str();
}

void Tokeniser::raiseError(std::string message) {
  std::stringstream msg;
  msg << message << " at line " << line << ", column " << column;
  throw std::runtime_error(msg.str());
}
