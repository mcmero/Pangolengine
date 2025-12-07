#pragma once

#include <string>
#include <unordered_map>
#include "Tokeniser.h"

// This is a simple, minimal XML parser that will handle only the features found
// in the tileset tsx file format

//------------------------------------------------------------------------------
// Tokenizer
//------------------------------------------------------------------------------
class TsxToken : public IToken {
public:
  enum class Type {
    ElementStart,   // <abc123
    ElementClose,   // >
    ElementEnd,     // /> or </abc123>
    PIStart,        // <?xml
    PIEnd,          // ?>
    Attribute,      // abc123 within a tag
    Equals,         // =
    String,         // ".."
    EndOfFile,
    Error
  };

  TsxToken(
    Type tokenType,
    const std::string& value = "",
    uint32_t line = 1,
    uint32_t col = 1
  );

  Type type;

  std::unique_ptr<IToken> clone() const override {
      return std::make_unique<TsxToken>(*this);
  }
};

class TsxTokeniser : public Tokeniser {
public:
  TsxTokeniser(std::istream &in);
  std::unique_ptr<IToken> getTokenImpl() override;
};

//------------------------------------------------------------------------------
// Tsx value definitions
//------------------------------------------------------------------------------

class TsxNode {
public:
  std::string name = "";
  std::vector<TsxNode> subNodes = {};

  TsxNode();
  TsxNode(std::string name);

  void addAttribute(std::string name, std::string value);

  // Const getters
  int queryGetInt(std::string attribute) const;
  double queryGetDouble(std::string attribute) const;
  const std::string& getValue(std::string attribute) const;

private:
  std::unordered_map<std::string, std::string> attributes = {};
};

//------------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------------

class TsxParser {
public:
  static std::vector<TsxNode> parseTsx(const std::string &file);

private:
  static TsxNode parseNode(TsxTokeniser &tokeniser);
  static void raiseError(const TsxToken &token, const std::string &message);
};
