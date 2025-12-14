#include "JsonParser.h"
#include "Tokeniser.h"
#include <memory>
#include <sstream>
#include <fstream>

JsonToken::JsonToken(Type tokenType, const std::string& value, uint32_t line,
                     uint32_t col) : type(tokenType) {
    this->value = value;
    this->line = line;
    this->column = col;
}

//------------------------------------------------------------------------------
// JsonTokeniser Implementation
//------------------------------------------------------------------------------

JsonTokeniser::JsonTokeniser(std::istream &in) : Tokeniser(in) {}

std::unique_ptr<IToken> JsonTokeniser::getTokenImpl() {
  skipWhitespace();

  int ch = in.peek();
  if (ch == EOF) {
    return makeToken<JsonToken>(JsonToken::Type::EndOfFile, "", line, column);
  }

  uint32_t startLine = line;
  uint32_t startCol  = column;

  switch(ch) {
    case '{': {
      getChar();
      return makeToken<JsonToken>(JsonToken::Type::LeftBrace, "{",
                       startLine, startCol);
    }
    case '}': {
      getChar();
      return makeToken<JsonToken>(JsonToken::Type::RightBrace, "}",
                       startLine, startCol);
    }
    case ':': {
      getChar();
      return makeToken<JsonToken>(JsonToken::Type::Colon, ":",
                       startLine, startCol);
    }
    case '[': {
      getChar();
      return makeToken<JsonToken>(JsonToken::Type::LeftBracket, "[",
                       startLine, startCol);
    }
    case ']': {
      getChar();
      return makeToken<JsonToken>(JsonToken::Type::RightBracket, "]",
                       startLine, startCol);
    }
    case ',': {
      getChar();
      return makeToken<JsonToken>(JsonToken::Type::Comma, ",",
                       startLine, startCol);
    }
    case '"': {
      std::string str = parseString();
      return makeToken<JsonToken>(JsonToken::Type::String, str, startLine, startCol);
    }
    default:
      if (isalpha(static_cast<unsigned char>(ch))) {
        std::string str = parseAlpha();
        if (str == "true")
          return makeToken<JsonToken>(JsonToken::Type::True, str,
                           startLine, startCol);
        else if (str == "false")
          return makeToken<JsonToken>(JsonToken::Type::False, str,
                           startLine, startCol);
        else if (str == "null")
          return makeToken<JsonToken>(JsonToken::Type::Null, str,
                           startLine, startCol);
        else
          return makeToken<JsonToken>(JsonToken::Type::Error, str,
                           startLine, startCol);

      } else {
        std::string str = parseNumber();
        return makeToken<JsonToken>(JsonToken::Type::Number, str,
                         startLine, startCol); // handle number
      }
  }
  return makeToken<JsonToken>(
    JsonToken::Type::Error,
    std::string{static_cast<char>(ch)},
    startLine, startCol
  );
}

//------------------------------------------------------------------------------
// JsonValue Implementation
//------------------------------------------------------------------------------

JsonValue::JsonValue() : value(std::monostate{}) {}
JsonValue::JsonValue(double d) : value(d) {}
JsonValue::JsonValue(bool b) : value(b) {}
JsonValue::JsonValue(std::string s) : value(std::move(s)) {}
JsonValue::JsonValue(JsonArray a) : value(std::move(a)) {}
JsonValue::JsonValue(JsonObject o) : value(std::move(o)) {}

bool JsonValue::isNull() const {
  return std::holds_alternative<std::monostate>(value);
}

bool JsonValue::isBool() const {
  return std::holds_alternative<bool>(value);
}

bool JsonValue::isNumber() const {
  return std::holds_alternative<double>(value);
}

bool JsonValue::isString() const {
  return std::holds_alternative<std::string>(value);
}

bool JsonValue::isArray() const {
  return std::holds_alternative<JsonArray>(value);
}

bool JsonValue::isObject() const {
  return std::holds_alternative<JsonObject>(value);
}

bool JsonValue::getBool() {
  return std::get<bool>(value);
}

double JsonValue::getNumber() {
  return std::get<double>(value);
}

std::string& JsonValue::getString() {
  return std::get<std::string>(value);
}

JsonArray& JsonValue::getArray() {
  return std::get<JsonArray>(value);
}

JsonObject& JsonValue::getObject() {
  return std::get<JsonObject>(value);
}

JsonValue& JsonValue::at(const std::string &key) {
  if (!isObject())
    throw std::runtime_error("JsonValue is not an object.");

  JsonObject &obj = getObject();
  auto it = obj.find(key);
  if (it == obj.end())
    throw std::out_of_range("Key not found: " + key);

  return it->second;
}

bool JsonValue::getBool() const {
  return std::get<bool>(value);
}

double JsonValue::getNumber() const {
  return std::get<double>(value);
}

const std::string& JsonValue::getString() const {
  return std::get<std::string>(value);
}

const JsonArray& JsonValue::getArray() const {
  return std::get<JsonArray>(value);
}

const JsonObject& JsonValue::getObject() const {
  return std::get<JsonObject>(value);
}

const JsonValue& JsonValue::at(const std::string &key) const {
  if (!isObject())
    throw std::runtime_error("JsonValue is not an object.");

  const JsonObject &obj = getObject();
  auto it = obj.find(key);
  if (it == obj.end())
    throw std::out_of_range("Key not found: " + key);

  return it->second;
}

//------------------------------------------------------------------------------
// JsonParser Implementation
//------------------------------------------------------------------------------

JsonObject JsonParser::parseJson(const std::string &file) {
  std::ifstream f(file);
  if (!f.is_open()) {
    std::stringstream ss;
    ss << "Failed to open file: " << file << std::endl;
    throw std::runtime_error(ss.str());
  }

  JsonTokeniser tokeniser {f};
  return parseObject(tokeniser);
}

JsonObject JsonParser::parseObject(JsonTokeniser &tokeniser) {
  JsonObject object = {};

  std::unique_ptr<IToken> token = tokeniser.getToken();
  JsonToken* jsonToken = dynamic_cast<JsonToken*>(token.get());
  if (jsonToken->type != JsonToken::Type::LeftBrace)
      throw std::runtime_error(
      "Unexpected character found at start of object. Expected brace"
      );

  while (true) {
    token = tokeniser.peekToken();
    jsonToken = dynamic_cast<JsonToken*>(token.get());
    if (jsonToken->type == JsonToken::Type::RightBrace)
      break; // empty object

    token = tokeniser.getToken();
    jsonToken = dynamic_cast<JsonToken*>(token.get());
    if (jsonToken->type != JsonToken::Type::String)
      raiseError(*jsonToken, "No string found at start of object");

    std::string string = jsonToken->value;

    token = tokeniser.getToken();
    jsonToken = dynamic_cast<JsonToken*>(token.get());
    if (jsonToken->type != JsonToken::Type::Colon)
      raiseError(*jsonToken,
                 "No colon found between string and object value");

    token = tokeniser.peekToken();
    jsonToken = dynamic_cast<JsonToken*>(token.get());
    switch (jsonToken->type) {
      case JsonToken::Type::Error:
        raiseError(*jsonToken, "Error token found");
        break;
      case JsonToken::Type::Null:
        token = tokeniser.getToken();
        object[string] = JsonValue{};
        break;
      case JsonToken::Type::String: {
        token = tokeniser.getToken();
        object[string] = JsonValue{token->value};
        break;
      }
      case JsonToken::Type::Number: {
        token = tokeniser.getToken();
        object[string] = JsonValue{std::stod(token->value)};
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
      case JsonToken::Type::LeftBrace: // nested object
        object[string] = parseObject(tokeniser);
        break;
      default:
        raiseError(*jsonToken, "Unexpected token found");
        break;
    }

    token = tokeniser.getToken();
    jsonToken = dynamic_cast<JsonToken*>(token.get());
    if (jsonToken->type == JsonToken::Type::RightBrace)
      break; // end of object
    else if (jsonToken->type != JsonToken::Type::Comma) {
      raiseError(*jsonToken,
                 "No comma found to indicate next value in object");
      break;
    }
  }
  return object;
}

JsonValue JsonParser::parseArray(JsonTokeniser &tokeniser) {
  std::unique_ptr<IToken> token = tokeniser.getToken();
  JsonToken *jsonToken = dynamic_cast<JsonToken*>(token.get());
  if (jsonToken->type != JsonToken::Type::LeftBracket)
      throw std::runtime_error(
      "Unexpected character found at start of array. Expected '['"
      );

  JsonArray array {};

  while (true) {
    token = tokeniser.peekToken();
    jsonToken = dynamic_cast<JsonToken*>(token.get());
    if (jsonToken->type == JsonToken::Type::RightBracket) {
      tokeniser.getToken();
      jsonToken = dynamic_cast<JsonToken*>(token.get());
      return JsonValue(array);
    }
    switch (jsonToken->type) {
      case JsonToken::Type::Error:
        raiseError(*jsonToken, "Error token found");
        break;
      case JsonToken::Type::Null: {
        token = tokeniser.getToken();
        array.push_back(JsonValue{});
        break;
      }
      case JsonToken::Type::String: {
        token = tokeniser.getToken();
        array.push_back(JsonValue{token->value});
        break;
      }
      case JsonToken::Type::Number: {
        token = tokeniser.getToken();
        array.push_back(JsonValue{std::stod(token->value)});
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
        raiseError(*jsonToken, "Unexpected token found");
        break;
    }

    token = tokeniser.getToken();
    jsonToken = dynamic_cast<JsonToken*>(token.get());
    if (jsonToken->type == JsonToken::Type::RightBracket)
      break; // end of array
    else if (jsonToken->type != JsonToken::Type::Comma) {
      raiseError(*jsonToken,
                 "No comma found to indicate next value in array");
      break;
    }
  }
  return JsonValue(array);
}

void JsonParser::raiseError(const JsonToken &token, const std::string &message) {
  std::stringstream msg;
  msg << message << " at line " << token.line << ", column " << token.column;
  throw std::runtime_error(msg.str());
}
