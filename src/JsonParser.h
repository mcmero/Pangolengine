#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <istream>
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
    skipWhitespace();

    char ch = in.peek();
    if (static_cast<int>(ch) == EOF) {
      return JsonToken{JsonToken::Type::EndOfFile};
    }

    switch(ch) {
      case '{': {
        getChar();
        return makeToken("{", JsonToken::Type::LeftBrace);
      }
      case '}': {
        getChar();
        return makeToken("}", JsonToken::Type::RightBrace);
      }
      case ':': {
        getChar();
        return makeToken(":", JsonToken::Type::Colon);
      }
      case '[': {
        getChar();
        return makeToken("[", JsonToken::Type::LeftBracket);
      }
      case ']': {
        getChar();
        return makeToken("]", JsonToken::Type::RightBracket);
      }
      case ',': {
        getChar();
        return makeToken(",", JsonToken::Type::Comma);
      }
      case '"': {
        std::string str = parseString();
        return makeToken(str, JsonToken::Type::String);
      }
      default:
        if (isalpha(ch)) {
          std::string str = parseAlpha();
          if (str == "true")
            return makeToken(str, JsonToken::Type::True);
          else if (str == "false")
            return makeToken(str, JsonToken::Type::False);
          else if (str == "null")
            return makeToken(str, JsonToken::Type::Null);
          else
            return makeToken(str, JsonToken::Type::Error);
        } else {
          std::string str = parseNumber();
          return makeToken(str, JsonToken::Type::Number); // handle number
        }
    }
    return makeToken(std::string{ch}, JsonToken::Type::Error);
  }

  JsonToken peekToken() {
    skipWhitespace();

    char ch = in.peek();
    if (static_cast<int>(ch) == EOF) {
      return JsonToken{JsonToken::Type::EndOfFile};
    }

    switch(ch) {
      case '{': {
        return makeToken("{", JsonToken::Type::LeftBrace);
      }
      case '}': {
        return makeToken("}", JsonToken::Type::RightBrace);
      }
      case ':': {
        return makeToken(":", JsonToken::Type::Colon);
      }
      case '[': {
        return makeToken("[", JsonToken::Type::LeftBracket);
      }
      case ']': {
        return makeToken("]", JsonToken::Type::RightBracket);
      }
      case ',': {
        return makeToken(",", JsonToken::Type::Comma);
      }
      case '"': {
        std::string str = parseString();
        putBackString(str);
        return makeToken(str, JsonToken::Type::String);
      }
      default:
        if (isalpha(ch)) {
          std::string str = parseAlpha();
          putBackString(str);
          if (str == "true")
            return makeToken(str, JsonToken::Type::True);
          else if (str == "false")
            return makeToken(str, JsonToken::Type::False);
          else if (str == "null")
            return makeToken(str, JsonToken::Type::Null);
          else
            return makeToken(str, JsonToken::Type::Error);
        } else {
          std::string str = parseNumber();
          putBackString(str);
          return makeToken(str, JsonToken::Type::Number); // handle number
        }
    }
    return makeToken(std::string{ch}, JsonToken::Type::Error);
  }

private:
  std::istream &in;
  uint32_t line = 1;
  uint32_t column = 1;

  void putBackString(std::string str) {
    for (int i = int(str.size() - 1); i >= 0; i--) {
      in.putback(str[i]);
      column--;
    }
  }

  void skipWhitespace() {
     while (true) {
        char ch = in.peek();
        if (static_cast<int>(ch) == EOF)
          return;
        if (std::isspace(static_cast<unsigned char>(ch))) {
            getChar();
            continue;
        }
        break;
    }
  }

  char getChar() {
    char ch = in.get();
    if (ch == '\n') {
      line++;
      column = 1;
    } else if (static_cast<int>(ch) != EOF) {
      column++;
    }
    return ch;
  }

  JsonToken makeToken(std::string value, JsonToken::Type type) {
    JsonToken token {type, value};
    token.line = line;
    token.column = column;
    return token;
  }

  /*
  * Parse string in format "mystring" from file stream. Must include quotes.
  */
  std::string parseString() {
    char ch = getChar();

    // We first need a quote character
    if (ch != '"')
      raiseError("Expected quote at start of string");

    std::stringstream result;
    while (true) {
      ch = getChar();

      if (static_cast<int>(ch) == EOF) {
        raiseError("Unexpected end of file while parsing string");
      }

      if (ch == '"')
        break; // end of string

      if (ch == '\\') {
          int esc = in.get();
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
        result << ch;
    }
    return result.str();
  }

  /*
  * Parse alphabetical values (not strings)
  */
  std::string parseAlpha() {
    char ch;

    std::stringstream result;
    std::string errorMsg = "Unexpected character while parsing alphabetical value";
    while (true) {
      ch = in.peek();

      if (static_cast<int>(ch) == EOF) {
        raiseError(errorMsg);
      }

      if (isalpha(ch))
        result << getChar();
      else if (ch == ',' || std::isspace(ch))
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
    char ch = in.peek();
    std::string errorMsg = "Unexpected character while parsing number value";

    if (static_cast<int>(ch) == EOF)
      raiseError(errorMsg);

    std::stringstream result;

    // Handle negative numbers
    if (ch == '-') {
      result << getChar();
      ch = in.peek();
    }

    if (ch == '0') { // leading zero
      result << getChar();
      ch = in.peek();
    } else if (std::isdigit(ch))
      result << parseDigits();
    else
      raiseError(errorMsg);

    ch = in.peek();
    if (ch == '.') { // decimal
      result << getChar();
      ch = in.peek();
      if (isdigit(ch))
        result << parseDigits();
      else
        raiseError(errorMsg);
    }

    ch = in.peek();
    if (ch == 'e' || ch == 'E') {
      result << getChar();
      ch = in.peek();

      if (ch == '+' || ch == '-') {
        result << getChar();
        ch = in.peek();
      }

      if (isdigit(ch))
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
    char ch = in.peek();
    std::stringstream result;

    while (true) {
      if (isdigit(ch)) {
        result << getChar();
        ch = in.peek();
      } else
        break;
    }

    return result.str();
  }

  /*
   * Raise exception with message and report line and column numbers.
   */
  void raiseError(std::string message) {
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
        case JsonToken::Type::LeftBracket:
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
        "Unexpected character found at start of array. Expected bracket"
        );

    JsonArray array {};
    while (true) {
      token = tokeniser.peekToken();
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
        default:
          raiseError(token, "Unexpected token found");
          break;
      }

      token = tokeniser.getToken();
      if (token.type == JsonToken::Type::RightBracket)
        break; // end of array
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
