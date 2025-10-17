#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <variant>
#include <fstream>
#include <sstream>

class IJsonNode {
public:
    virtual ~IJsonNode() = default;
};

using Json = std::map<std::string, std::unique_ptr<IJsonNode>>;

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

class Parser{
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
    while(matchObjectStart(f) || matchNextItem(f)) {
      // TODO: handle object end case
      std::string key = parseString(f);

      // TODO: maybe a generic expectNext function?
      // Make sure next char is a colon
      char ch; f.get(ch);
      if (getCharType(ch) != CharType::Colon) {
        std::stringstream ss;
        ss << "Colon not found after string for " << key <<
          ". Instead found " << ch << std::endl;
        throw std::runtime_error(ss.str());
      }

      // Optional whitespace -- put the character back if not
      f.get(ch);
      if (getCharType(ch) != CharType::Whitespace)
        f.putback(ch);

      json[key] = parseJsonNode(f);
    }

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
    Other
  };

  /*
   * Returns a pointer to a Json Node given a file stream. Will recursively keep
   * parsing until it reaches the bottom of the hierarchy.
   */
  static std::unique_ptr<IJsonNode> parseJsonNode(std::ifstream &f) {
    char ch; f.get(ch);

    if (getCharType(ch) == CharType::ArrayStart) {
      // Special logic handling arrays, recursively
    }

    if (getCharType(ch) == CharType::Quote) {
      f.putback(ch); // need full string with quote
      auto node = std::make_unique<JsonNode<std::string>>();
      node->data = parseString(f);
      return node;
    }

    // Handle case bool
    // Handle case int

    return std::unique_ptr<IJsonNode>();
  }

  static CharType getCharType(char ch) {
    switch(ch) {
      case '{':
        return CharType::ObjectStart;
      case '}':
        return CharType::ObjectEnd;
      case ' ':
        return CharType::Whitespace;
      case '\n':
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
      default:
        if (isdigit(ch) || ch == '-' || ch =='.')
          return CharType::Numeric;
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
    // Next char is whitespace
    f.get(ch);
    if (getCharType(ch) != CharType::Whitespace) {
      f.putback(ch);
      return false;
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
  * Parse string in format "mystring" from file stream . Must include quotes.
  */
  static std::string parseString(std::ifstream &f) {
    char ch; f.get(ch);

    // We first need a quote character
    assert(getCharType(ch) == CharType::Quote &&
           "String does not start with a quote.");

    bool end = false;
    bool escapeNext = false;
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
};
