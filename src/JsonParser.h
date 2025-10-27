#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <istream>
#include <optional>
#include <stdexcept>
#include <string>
#include <map>
#include <variant>
#include <fstream>
#include <sstream>
#include <vector>

//------------------------------------------------------------------------------
// Tokeiser
//------------------------------------------------------------------------------
struct JsonToken {
  enum class Type {
    LeftBrace,      // {
    RightBrace,     // }
    LeftBracket,    // [
    RightBracket,   // ]
    Colon,          // :
    Comma,          // ,
    String,         // ".."
    Number,         // 0-9, -1.23, 1e10
    True,           // true
    False,          // false
    Null,           // null
    EndOfFile,
    Error
  } type;

  std::string value = "";

  uint32_t line = 1;
  uint32_t column = 1;
};

class JsonTokeniser {
public:
  JsonTokeniser(std::istream &in) : in(in), line(1), column(1) {};

  JsonToken getToken() {
    if (lookahead_) {
      JsonToken t = *lookahead_;
      lookahead_.reset();
      return t;
    }
    return getTokenImpl();
  }

  JsonToken peekToken() {
    if (!lookahead_)
      lookahead_ = getTokenImpl();
    return *lookahead_;
  }

private:
  std::istream &in;
  std::optional<JsonToken> lookahead_;
  uint32_t line = 1;
  uint32_t column = 1;

  JsonToken getTokenImpl() {
    skipWhitespace();

    int ch = in.peek();
    if (ch == EOF) {
      return JsonToken{JsonToken::Type::EndOfFile};
    }

    uint32_t startLine = line;
    uint32_t startCol  = column;

    switch(ch) {
      case '{': {
        getChar();
        return makeToken("{", JsonToken::Type::LeftBrace,
                         startLine, startCol);
      }
      case '}': {
        getChar();
        return makeToken("}", JsonToken::Type::RightBrace,
                         startLine, startCol);
      }
      case ':': {
        getChar();
        return makeToken(":", JsonToken::Type::Colon,
                         startLine, startCol);
      }
      case '[': {
        getChar();
        return makeToken("[", JsonToken::Type::LeftBracket,
                         startLine, startCol);
      }
      case ']': {
        getChar();
        return makeToken("]", JsonToken::Type::RightBracket,
                         startLine, startCol);
      }
      case ',': {
        getChar();
        return makeToken(",", JsonToken::Type::Comma,
                         startLine, startCol);
      }
      case '"': {
        std::string str = parseString();
        return makeToken(str, JsonToken::Type::String, startLine, startCol);
      }
      default:
        if (isalpha(static_cast<unsigned char>(ch))) {
          std::string str = parseAlpha();
          if (str == "true")
            return makeToken(str, JsonToken::Type::True,
                             startLine, startCol);
          else if (str == "false")
            return makeToken(str, JsonToken::Type::False,
                             startLine, startCol);
          else if (str == "null")
            return makeToken(str, JsonToken::Type::Null,
                             startLine, startCol);
          else
            return makeToken(str, JsonToken::Type::Error,
                             startLine, startCol);

        } else {
          std::string str = parseNumber();
          return makeToken(str, JsonToken::Type::Number,
                           startLine, startCol); // handle number
        }
    }
    return makeToken(std::string{static_cast<char>(ch)},
                     JsonToken::Type::Error, startLine, startCol);
  }

  void skipWhitespace() {
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

  int getChar() {
    int ch = in.get();
    if (ch == '\n') {
      line++;
      column = 1;
    } else if (ch != EOF) {
      column++;
    }
    return ch;
  }

  JsonToken makeToken(std::string value, JsonToken::Type type,
                      uint32_t startLine, uint32_t startCol) {
    JsonToken token {type, value};
    token.line = startLine;
    token.column = startCol;
    return token;
  }

  /*
  * Parse string in format "mystring" from filegetChar(getChar()) stream. Must include quotes.
  */
  std::string parseString() {
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

  /*
  * Parse alphabetical values (not strings)
  */
  std::string parseAlpha() {
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
      else if (ch == ',' || std::isspace(static_cast<unsigned char>(ch)))
        break;
      else
        raiseError(errorMsg);
    }
    return result.str();
  }

  /*
   * Parse numeric values
   */
  std::string parseNumber() {
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
        raiseError(errorMsg);
    }

    return result.str();
  }

  /*
   * Return digits as string while input stream consists of them.
   */
  std::string parseDigits() {
    int ch = in.peek();
    std::stringstream result;

    while (true) {
      if (isdigit(static_cast<char>(ch))) {
        result << static_cast<char>(getChar());
        ch = in.peek();
      } else
        break;
    }

    return result.str();
  }

  /*
   * Raise exception with message and report line and column numbers.
   */
  [[noreturn]] void raiseError(std::string message) {
    std::stringstream msg;
    msg << message << " at line " << line << ", column " << column;
    throw std::runtime_error(msg.str());
  }
};

//------------------------------------------------------------------------------
// Json value definitions
//------------------------------------------------------------------------------

class JsonValue; // Forward definition
using JsonArray  = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;

class JsonValue {
public:
  using Variant = std::variant<
    double,
    bool,
    std::string,
    std::monostate, // null type
    JsonArray,
    JsonObject
  >;
  Variant value;

  JsonValue()                 : value(std::monostate{}) {}
  JsonValue(double d)         : value(d) {}
  JsonValue(bool b)           : value(b) {}
  JsonValue(std::string s)    : value(std::move(s)) {}
  JsonValue(JsonArray a)      : value(std::move(a)) {}
  JsonValue(JsonObject o)     : value(std::move(o)) {}
};

//------------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------------

class JsonParser {
public:
  /*
   * Parse JSON file, returning a map of JSON objects
   */
  static JsonObject parseJson(std::string file) {

    std::ifstream f(file);
    if (!f.is_open()) {
      std::stringstream ss;
      ss << "Failed to open file: " << file << std::endl;
      throw std::runtime_error(ss.str());
    }

    JsonTokeniser tokeniser {f};
    return parseObject(tokeniser);
  }

private:
  /*
  * Parse JSON object
  */
  static JsonObject parseObject(JsonTokeniser &tokeniser) {
    JsonObject object = {};

    JsonToken token = tokeniser.getToken();
    if (token.type != JsonToken::Type::LeftBrace)
        throw std::runtime_error(
        "Unexpected character found at start of object. Expected brace"
        );

    while (true) {
      token = tokeniser.getToken();
      if (token.type != JsonToken::Type::String)
        raiseError(token, "No string found at start of object");

      std::string string = token.value;

      token = tokeniser.getToken();
      if (token.type != JsonToken::Type::Colon)
        raiseError(token,
                   "No colon found between string and object value");

      token = tokeniser.peekToken();
      switch (token.type) {
        case JsonToken::Type::Error:
          raiseError(token, "Error token found");
          break;
        case JsonToken::Type::Null:
          object[string] = JsonValue{};
          break;
        case JsonToken::Type::String: {
          token = tokeniser.getToken();
          object[string] = JsonValue{token.value};
          break;
        }
        case JsonToken::Type::Number: {
          token = tokeniser.getToken();
          object[string] = JsonValue{std::stod(token.value)};
          break;
        }
        case JsonToken::Type::True: {
          token = tokeniser.getToken();
          object[string] = JsonValue{true};
          break;
        }
        case JsonToken::Type::False: {
          token = tokeniser.getToken();
          object[string] = JsonValue{false};
          break;
        }
        case JsonToken::Type::LeftBracket: // nested array
          object[string] = parseArray(tokeniser);
          break;
        default:
          raiseError(token, "Unexpected token found");
          break;
      }

      token = tokeniser.getToken();
      if (token.type == JsonToken::Type::RightBrace)
        break; // end of object
      else if (token.type != JsonToken::Type::Comma) {
        raiseError(token,
                   "No comma found to indicate next value in object");
        break;
      }
    }
    return object;
  }

/*
  * Parse JSON array
  */
  static JsonValue parseArray(JsonTokeniser &tokeniser) {
    JsonToken token = tokeniser.getToken();
    if (token.type != JsonToken::Type::LeftBracket)
        throw std::runtime_error(
        "Unexpected character found at start of array. Expected '['"
        );

    JsonArray array {};

    while (true) {
      token = tokeniser.peekToken();
      if (token.type == JsonToken::Type::RightBracket) {
        tokeniser.getToken();
        return JsonValue(array);
      }
      switch (token.type) {
        case JsonToken::Type::Error:
          raiseError(token, "Error token found");
          break;
        case JsonToken::Type::Null: {
          token = tokeniser.getToken();
          array.push_back(JsonValue{});
          break;
        }
        case JsonToken::Type::String: {
          token = tokeniser.getToken();
          array.push_back(JsonValue{token.value});
          break;
        }
        case JsonToken::Type::Number: {
          token = tokeniser.getToken();
          array.push_back(JsonValue{std::stod(token.value)});
          break;
        }
        case JsonToken::Type::True: {
          token = tokeniser.getToken();
          array.push_back(JsonValue{true});
          break;
        }
        case JsonToken::Type::False: {
          token = tokeniser.getToken();
          array.push_back(JsonValue{false});
          break;
        }
        case JsonToken::Type::LeftBracket:
          array.push_back(parseArray(tokeniser));
          break;
        case JsonToken::Type::LeftBrace:
          array.push_back(parseObject(tokeniser));
          break;
        default:
          raiseError(token, "Unexpected token found");
          break;
      }

      token = tokeniser.getToken();
      if (token.type == JsonToken::Type::RightBracket)
        break; // end of array
      else if (token.type == JsonToken::Type::Comma)
        continue; // go to next element
      else if (token.type != JsonToken::Type::Comma) {
        raiseError(token,
                   "No comma found to indicate next value in array");
        break;
      }
    }
    return JsonValue(array);
  }

  /*
   * Raise exception with message and report line and column numbers.
   */
  static void raiseError(JsonToken token, std::string message) {
    std::stringstream msg;
    msg << message << " at line " << token.line << ", column " << token.column;
    throw std::runtime_error(msg.str());
  }
};
