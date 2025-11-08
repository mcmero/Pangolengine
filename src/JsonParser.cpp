#include "JsonParser.h"
#include <sstream>
#include <fstream>

//------------------------------------------------------------------------------
// JsonTokeniser Implementation
//------------------------------------------------------------------------------

JsonTokeniser::JsonTokeniser(std::istream &in) : in(in), line(1), column(1) {}

JsonToken JsonTokeniser::getToken() {
  if (lookahead_) {
    JsonToken t = *lookahead_;
    lookahead_.reset();
    return t;
  }
  return getTokenImpl();
}

JsonToken JsonTokeniser::peekToken() {
  if (!lookahead_)
    lookahead_ = getTokenImpl();
  return *lookahead_;
}

JsonToken JsonTokeniser::getTokenImpl() {
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

void JsonTokeniser::skipWhitespace() {
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

int JsonTokeniser::getChar() {
  int ch = in.get();
  if (ch == '\n') {
    line++;
    column = 1;
  } else if (ch != EOF) {
    column++;
  }
  return ch;
}

JsonToken JsonTokeniser::makeToken(std::string value, JsonToken::Type type,
                    uint32_t startLine, uint32_t startCol) {
  JsonToken token {type, value};
  token.line = startLine;
  token.column = startCol;
  return token;
}

std::string JsonTokeniser::parseString() {
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

std::string JsonTokeniser::parseAlpha() {
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
    else if (ch == ',' || ch == '}' || ch == ']' || std::isspace(static_cast<unsigned char>(ch)))
      break;
    else
      raiseError(errorMsg);
  }
  return result.str();
}

std::string JsonTokeniser::parseNumber() {
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

std::string JsonTokeniser::parseDigits() {
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

void JsonTokeniser::raiseError(std::string message) {
  std::stringstream msg;
  msg << message << " at line " << line << ", column " << column;
  throw std::runtime_error(msg.str());
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

  JsonToken token = tokeniser.getToken();
  if (token.type != JsonToken::Type::LeftBrace)
      throw std::runtime_error(
      "Unexpected character found at start of object. Expected brace"
      );

  while (true) {
    token = tokeniser.peekToken();
    if (token.type == JsonToken::Type::RightBrace)
      break; // empty object

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
        token = tokeniser.getToken();
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
      case JsonToken::Type::LeftBrace: // nested object
        object[string] = parseObject(tokeniser);
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

JsonValue JsonParser::parseArray(JsonTokeniser &tokeniser) {
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
    else if (token.type != JsonToken::Type::Comma) {
      raiseError(token,
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
