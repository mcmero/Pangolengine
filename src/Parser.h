#pragma once

#include <cassert>
#include <execution>
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

  bool is_value() const { return std::holds_alternative<Value>(data);}

  bool is_json() const { return std::holds_alternative<Json>(data);}

  const Value& get_value() const { return std::get<Value>(data);}

  const Json& get_json() const { return std::get<Json>(data);}

};

class Parser{
public:
  static Json parse_json(std::string file) {

    std::ifstream f(file);
    if (!f.is_open()) {
      std::stringstream ss;
      ss << "Failed to open file: " << file << std::endl;
      throw std::runtime_error(ss.str());
    }

    Json json = {};
    State state = State::Begin;

    bool isObject = parseObjectStart(f);
    if (!isObject)
      state = State::Error;
    else {
      std::string key = parseString(f);
      json[key] = std::unique_ptr<IJsonNode>();
    }

    if (state == State::Error) {
      std::cerr << "Unexpected state reading file " <<
        file << "." << std::endl;
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
    Digit,
    Escape,
    Other
  };

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
        if (isdigit(ch))
          return CharType::Digit;
        else
          return CharType::Other;
    }
  }

  // Move to first key of object, return false if we don't match
  // object start, whitespace then quote. Handles empty object by
  // moving to the next object, where it starts parsing again.
  static bool parseObjectStart(std::ifstream &f) {
    char ch;

    f.get(ch);
    // First char should be {
    if (getCharType(ch) != CharType::ObjectStart)
      return false;

    // Next char is whitespace
    f.get(ch);
    if (getCharType(ch) != CharType::Whitespace)
      return false;

    // Check for object end, in which case, parse again
    f.get(ch);
    if (getCharType(ch) == CharType::ObjectEnd) {
      f.get(ch);
      if (getCharType(ch) == CharType::Separator) {
        f.get(ch);
        if (getCharType(ch) == CharType::Whitespace) {
          return parseObjectStart(f);
        }
      }
    }

    // If we haven't reached a quote, something has gone wrong
    if (getCharType(ch) != CharType::Quote)
      return false;

    f.putback(ch); // return to quote char, ready to parse string
    return true;
  }

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
