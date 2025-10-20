#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <istream>
#include <stdexcept>
#include <string>
#include <map>
#include <memory>
#include <variant>
#include <fstream>
#include <sstream>

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
        if (isalpha(ch)) // handle boolean
          return makeToken("true", JsonToken::Type::True);
        else
          return makeToken("", JsonToken::Type::Number); // handle number
    }

    return makeToken("", JsonToken::Type::Error);
  }

  JsonToken peekToken() {
    JsonToken token = JsonToken{JsonToken::Type::Error, ""};
    return token;
  }

private:
  std::istream &in;
  uint32_t line = 1;
  uint32_t column = 1;

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
    // TODO: make error function that prints line and col numbers
    if (ch != '"')
      std::runtime_error("Expected quote at start of string.");

    std::stringstream result;
    while (true) {
      ch = getChar();

      if (static_cast<int>(ch) == EOF) {
        std::runtime_error("Unexpected end of file while parsing string.");
      }

      if (ch == '"')
        break; // end of string

      if (ch == '\\') {
          int esc = in.get();
          if (esc == EOF)
            throw std::runtime_error(
            "Unexpected end of file in escape sequence."
          );

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
                throw std::runtime_error("\\u escapes not implemented");
            default:
                throw std::runtime_error(
                  std::string("Invalid escape: \\") + ec
                );
          }
      } else
        result << ch;
    }
    return result.str();
  }
};

class IJsonNode {
public:
    virtual ~IJsonNode() = default;
};

using Json = std::map<std::string, std::unique_ptr<IJsonNode>>;

// TODO: make separate class for each type
template<typename T>
class JsonNode : public IJsonNode {
public:
  using Value = T;
  using Content = std::variant<Value, Json>;

  Content data;

  bool is_value() const { return std::holds_alternative<Value>(data); }

  bool is_json() const { return std::holds_alternative<Json>(data); }

  const Value& get_value() const { return std::get<Value>(data); }

  const Json& get_json() const { return std::get<Json>(data); }
};

class JsonParser {
public:
  /*
   * Parse JSON file, returning a map of JSON objects
   */
  static Json parseJson(std::string file) {

    std::ifstream f(file);
    if (!f.is_open()) {
      std::stringstream ss;
      ss << "Failed to open file: " << file << std::endl;
      throw std::runtime_error(ss.str());
    }

    Json json = {};

    JsonTokeniser tokeniser {f};

    while (true) {
      JsonToken token = tokeniser.getToken();

      if (token.type == JsonToken::Type::EndOfFile)
        break;

      if (token.type == JsonToken::Type::Error) {
        // report error
        break;
      }
    }

    /*
    while(matchObjectStart(f) || matchNextItem(f)) {
      // TODO: handle object end case
      std::string key = parseString(f);

      // Make sure next char is a colon
      if (!expectNextChar(f,CharType::Colon)) {
        // TODO: better error handling/error state?
        std::stringstream ss;
        ss << "Colon not found after string for " << key << "." << std::endl;
        throw std::runtime_error(ss.str());
      }

      // Optional whitespace
      expectNextChar(f, CharType::Whitespace);

      json[key] = parseJsonNode(f);
    }
    */

    return json;
  }

private:
  enum class State { Begin, Name, Value, End, Error };
  enum class CharType {
    ObjectStart,
    ObjectEnd,
    Whitespace,
    Colon,
    ArrayStart,
    ArrayEnd,
    Separator,
    Quote,
    Numeric,
    Escape,
    Alpha,
    Other
  };

  /*
   * Returns a pointer to a Json Node given a file stream. Will recursively keep
   * parsing until it reaches the bottom of the hierarchy.
   */
  static std::unique_ptr<IJsonNode> parseJsonNode(std::ifstream &f) {
    char ch; f.get(ch);

    // TODO: error handling for parse functions
    if (getCharType(ch) == CharType::ArrayStart) {
      // TODO: Special logic handling arrays, recursively
    } else if (getCharType(ch) == CharType::Quote) {
      f.putback(ch); // need full string with quote
      auto node = std::make_unique<JsonNode<std::string>>();
      node->data = parseString(f);
      return node;
    } else if (getCharType(ch) == CharType::Numeric) {
      f.putback(ch);
      auto node = std::make_unique<JsonNode<float>>();
      node->data = parseNumber(f);
      return node;
    } else if (getCharType(ch) == CharType::Alpha) {
      f.putback(ch);
      auto node = std::make_unique<JsonNode<bool>>();
      node->data = parseBool(f);
      return node;
    }

    // Nothing matched, return null pointer
    return std::unique_ptr<IJsonNode>();
  }

  static CharType getCharType(char ch) {
    switch(ch) {
      case '{':
        return CharType::ObjectStart;
      case '}':
        return CharType::ObjectEnd;
      case ' ': case '\n':
        return CharType::Whitespace;
      case ':':
        return CharType::Colon;
      case '[':
        return CharType::ArrayStart;
      case ']':
        return CharType::ArrayEnd;
      case ',':
        return CharType::Separator;
      case '"':
        return CharType::Quote;
      case '\\':
        return CharType::Escape;
      case '-': case '.':
        return CharType::Numeric;
      default:
        if (isdigit(ch))
          return CharType::Numeric;
        else if (isalpha(ch))
          return CharType::Alpha;
        else
          return CharType::Other;
    }
  }

  /*
  * Return true if stream matches end-separator-whitespace pattern
  * denoting end of object
  */
  static bool matchObjectEnd(std::ifstream &f, char &ch) {
    f.get(ch);
    if (getCharType(ch) != CharType::ObjectEnd)
        return false;
    f.get(ch);
    if (getCharType(ch) != CharType::Separator)
        return false;
    f.get(ch);
    return getCharType(ch) == CharType::Whitespace;
  }

  /*
  * Move to first key of object, return false if we don't match
  * object start, whitespace then quote. Handles empty object by
  * moving to the next object, where it starts parsing again.
  */
  static bool matchObjectStart(std::ifstream &f) {
    char ch;

    f.get(ch);
    // First char should be {
    if (getCharType(ch) != CharType::ObjectStart) {
      f.putback(ch);
      return false;
    }

    // Next char is whitespace
    f.get(ch);
    if (getCharType(ch) != CharType::Whitespace) {
      f.putback(ch);
      return false;
    }

    // Check for object end, in which case, parse again
    // This case means it is an empty object.
    if (matchObjectEnd(f, ch))
      return matchObjectStart(f);

    // If we haven't reached a quote, something has gone wrong
    if (getCharType(ch) != CharType::Quote) {
      f.putback(ch);
      return false;
    }

    f.putback(ch); // return to quote char, ready to parse string
    return true;
  }

  /*
  * Return true if there's another item next, typically comma then whitespace
  */
  static bool matchNextItem(std::ifstream &f) {
    char ch;

    f.get(ch);
    // First char should be {
    if (getCharType(ch) != CharType::Separator) {
      f.putback(ch);
      return false;
    }
    // Next chars can be whitespace
    while(f.get(ch)) {
      if (getCharType(ch)!= CharType::Whitespace) {
        break;
      }
    }

    // If we haven't reached a quote, something has gone wrong
    if (getCharType(ch) != CharType::Quote) {
      f.putback(ch);
      return false;
    }

    f.putback(ch); // return to quote char, ready to parse string
    return true;
  }

  /*
   * Returns true if the next character is the expected one, otherwise false.
   * If the character is not expected, it will put it back in the stream.
   */
  static bool expectNextChar(std::ifstream &f, CharType charType) {
    char ch;
    f.get(ch);
    if (getCharType(ch) == charType) {
      return true;
    }
    f.putback(ch);
    return false;
  }

  /*
  * Parse string in format "mystring" from file stream. Must include quotes.
  */
  static std::string parseString(std::ifstream &f) {
    char ch; f.get(ch);

    // We first need a quote character
    assert(getCharType(ch) == CharType::Quote &&
           "String does not start with a quote.");

    bool end = false;
    bool escapeNext = false;
    // TODO: fix escape handing
    std::stringstream result;
    while (f.get(ch)) {
      switch(getCharType(ch)) {
        case CharType::Escape:
          result << ch;
          escapeNext = true;
          break;
        case CharType::Quote:
          if (!escapeNext) {
            end = true;
          } else {
            result << ch;
            escapeNext = false;
          }
          break;
        default:
          result << ch;
          break;
      }

      if (end)
        break;

    }
    return result.str();
  }

  /*
  * Parse stream for numeric values
  */
  static float parseNumber(std::ifstream &f) {
    char ch;
    std::stringstream result;
    while (f.get(ch)) {
      if (getCharType(ch) == CharType::Numeric)
        result << ch;
      else {
        f.putback(ch);
        break;
      }
    }
    return std::stof(result.str());
  }

  /*
  * Parse stream for boolean
  */
  static bool parseBool(std::ifstream &f) {
    char ch;
    std::stringstream result;
    while (f.get(ch)) {
      if (getCharType(ch) == CharType::Alpha)
        result << ch;
      else {
        f.putback(ch);
        break;
      }
    }
    // TODO: This is obviously not right -- it should only be false if we find the
    // value 'false'. Need to fix this with proper error return values.
    return result.str() == "true";
  }
};
