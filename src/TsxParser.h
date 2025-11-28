#pragma once

#include <optional>
#include <string>
#include <unordered_map>

// This is a simple, minimal XML parser that will handle only the features found
// in the tileset tsx file format

//------------------------------------------------------------------------------
// Tokenizer
//------------------------------------------------------------------------------
struct TsxToken {
  enum class Type {
    ElementStart,   // <abc123
    ElementClose,   // >
    ElementEnd,     // />
    PIStart,        // <?xml
    PIEnd,          // ?>
    Attribute,      // abc123 within a tag
    Equals,         // =
    String,         // ".."
    EndOfFile,
    Error
  } type;

  std::string value = "";

  uint32_t line = 1;
  uint32_t column = 1;
};

class TsxTokeniser {
public:
  TsxTokeniser(std::istream &in);

  TsxToken getToken();
  TsxToken peekToken();

private:
  std::istream &in;
  std::optional<TsxToken> lookahead_;
  uint32_t line = 1;
  uint32_t column = 1;

  TsxToken getTokenImpl();
  void skipWhitespace();
  int getChar();
  TsxToken makeToken(std::string value, TsxToken::Type type,
                         uint32_t startLine, uint32_t startCol);
  std::string parseString();
  std::string parseAlpha();
  [[noreturn]] void raiseError(std::string message);
};

//------------------------------------------------------------------------------
// Tsx value definitions
//------------------------------------------------------------------------------

class TsxNode {
public:
  TsxNode *subNode = nullptr;

  TsxNode(std::string name);

  void addAttribute(std::string name, std::string value);

  // Const getters
  int queryGetInt(std::string attribute) const;
  double queryGetDouble(std::string attribute) const;
  const std::string& getValue(std::string attribute) const;

private:
  std::unordered_map<std::string, std::string> element;
};

//------------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------------

class TsxParser {
public:
  static TsxNode parseTsx(const std::string &file);

private:
  static void raiseError(const TsxToken &token, const std::string &message);
};
