#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <istream>
#include <optional>
#include <string>
#include <map>
#include <variant>
#include <vector>

//------------------------------------------------------------------------------
// Tokenizer
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
  JsonTokeniser(std::istream &in);

  JsonToken getToken();
  JsonToken peekToken();

private:
  std::istream &in;
  std::optional<JsonToken> lookahead_;
  uint32_t line = 1;
  uint32_t column = 1;

  JsonToken getTokenImpl();
  void skipWhitespace();
  int getChar();
  JsonToken makeToken(std::string value, JsonToken::Type type,
                      uint32_t startLine, uint32_t startCol);
  std::string parseString();
  std::string parseAlpha();
  std::string parseNumber();
  std::string parseDigits();
  [[noreturn]] void raiseError(std::string message);
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

  JsonValue();
  JsonValue(double d);
  JsonValue(bool b);
  JsonValue(std::string s);
  JsonValue(JsonArray a);
  JsonValue(JsonObject o);

  bool isNull() const;
  bool isBool() const;
  bool isNumber() const;
  bool isString() const;
  bool isArray() const;
  bool isObject() const;

  // Mutable getters
  bool getBool();
  double getNumber();
  std::string& getString();
  JsonArray& getArray();
  JsonObject& getObject();
  JsonValue& at(const std::string &key);

  // Const getters
  bool getBool() const;
  double getNumber() const;
  const std::string& getString() const;
  const JsonArray& getArray() const;
  const JsonObject& getObject() const;
  const JsonValue& at(const std::string &key) const;
};

//------------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------------

class JsonParser {
public:
  static JsonObject parseJson(const std::string &file);

private:
  static JsonObject parseObject(JsonTokeniser &tokeniser);
  static JsonValue parseArray(JsonTokeniser &tokeniser);
  static void raiseError(const JsonToken &token, const std::string &message);
};
