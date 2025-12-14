#pragma once

#include "Tokeniser.h"
#include <cassert>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

//------------------------------------------------------------------------------
// Tokenizer
//------------------------------------------------------------------------------
class JsonToken : public IToken {
public:
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
  };

  JsonToken(
    Type tokenType,
    const std::string& value = "",
    uint32_t line = 1,
    uint32_t col = 1
  );

  Type type;

  std::unique_ptr<IToken> clone() const override {
      return std::make_unique<JsonToken>(*this);
  }
};

class JsonTokeniser : public Tokeniser {
public:
  JsonTokeniser(std::istream &in);
  std::unique_ptr<IToken> getTokenImpl() override;
};

//------------------------------------------------------------------------------
// Json value definitions
//------------------------------------------------------------------------------

class JsonValue; // Forward definition
using JsonArray  = std::vector<JsonValue>;
using JsonObject = std::unordered_map<std::string, JsonValue>;

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
