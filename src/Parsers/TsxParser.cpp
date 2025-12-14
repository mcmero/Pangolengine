#include "TsxParser.h"
#include <memory>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using std::ifstream;

TsxToken::TsxToken(Type tokenType, const std::string &value, uint32_t line,
                   uint32_t col)
    : type(tokenType) {
  this->value = value;
  this->line = line;
  this->column = col;
}

//------------------------------------------------------------------------------
// TsxTokeniser Implementation
//------------------------------------------------------------------------------

TsxTokeniser::TsxTokeniser(std::istream &in) : Tokeniser(in) {}

std::unique_ptr<IToken> TsxTokeniser::getTokenImpl() {
  skipWhitespace();

  int ch = in.peek();
  if (ch == EOF) {
    return makeToken<TsxToken>(TsxToken::Type::EndOfFile, "", line, column);
  }

  uint32_t startLine = line;
  uint32_t startCol  = column;
  const char terminators[] = "=>";

  if (ch == '<') {
    getChar();
    ch = in.peek();

    if (ch == '/') {
      getChar();
      std::string str = parseAlpha(terminators);
      ch = in.peek();
      if (ch == '>') {
        getChar();
        return makeToken<TsxToken>(
          TsxToken::Type::ElementEnd, "</" + str + ">", line, column
        );
      }
    } else if (ch == '?') {
      getChar();
      std::string str = parseAlpha(terminators);
      if (str == "xml")
        return makeToken<TsxToken>(
          TsxToken::Type::PIStart, "xml", line, column
        );
    } else {
      std::string str = parseAlpha(terminators);
      return makeToken<TsxToken>(
        TsxToken::Type::ElementStart, str, line, column
      );
    }
  } else if (ch == '?') {
    getChar();
    ch = in.peek();
    if (ch == '>') {
      getChar();
      return makeToken<TsxToken>(
        TsxToken::Type::PIEnd, "?>", line, column
      );
    }
  } else if (ch == '/') {
    getChar();
    ch = in.peek();
    if (ch == '>') {
      getChar();
      return makeToken<TsxToken>(
        TsxToken::Type::ElementEnd, "/>", line, column
      );
    }
  } else if (ch == '=') {
    getChar();
    return makeToken<TsxToken>(
      TsxToken::Type::Equals, "=", line, column
    );
  } else if (ch == '"') {
    std::string str = parseString();
    return makeToken<TsxToken>(
      TsxToken::Type::String, str, line, column
    );
  } else if (ch == '>') {
    getChar();
    return makeToken<TsxToken>(
      TsxToken::Type::ElementClose, ">", line, column
    );
  } else {
    std::string str = parseAlpha(terminators);
    return makeToken<TsxToken>(
      TsxToken::Type::Attribute, str, line, column
    );
  }

  return makeToken<TsxToken>(
    TsxToken::Type::Error,
    std::string{static_cast<char>(ch)},
    startLine, startCol
  );
}

//------------------------------------------------------------------------------
// TsxValue Implementation
//------------------------------------------------------------------------------

TsxNode::TsxNode() {}
TsxNode::TsxNode(std::string name) : name(name){}

void TsxNode::addAttribute(std::string name, std::string value) {
  attributes[name] = value;
}

int TsxNode::getInt(std::string attribute) const {
  if (!attributes.contains(attribute))
    throw std::runtime_error("Attribute " + attribute + " not found!");

  std::string value = attributes.at(attribute);
  int result = std::stoi(value);
  return result;
}

float TsxNode::getFloat(std::string attribute) const {
  if (!attributes.contains(attribute))
    throw std::runtime_error("Attribute " + attribute + " not found!");

  std::string value = attributes.at(attribute);
  float result = std::stof(value);
  return result;
}

const std::string& TsxNode::getValue(std::string attribute) const {
  if (!attributes.contains(attribute))
    throw std::runtime_error("Attribute " + attribute + " not found!");

  return attributes.at(attribute);
}

//------------------------------------------------------------------------------
// TxsParser Implementation
//------------------------------------------------------------------------------

std::vector<TsxNode> TsxParser::parseTsx(const std::string &file) {
  ifstream f(file);
  if (!f.is_open()) {
    std::stringstream ss;
    ss << "Failed to open file: " << file << std::endl;
    throw std::runtime_error(ss.str());
  }
  TsxTokeniser tokeniser {f};

  // Parse <?xml tags at start of file
  std::unique_ptr<IToken> token = tokeniser.getToken();
  TsxToken* tsxToken = dynamic_cast<TsxToken*>(token.get());
  if (tsxToken->type != TsxToken::Type::PIStart)
    throw std::runtime_error(
    "Unexpected character found at start of object. Expected PI start (<?xml)."
    );

  std::vector<TsxNode> nodes = {};
  TsxNode node = {"xml"};
  while(true) {
    token = tokeniser.getToken();
    tsxToken = dynamic_cast<TsxToken*>(token.get());
    if (tsxToken->type == TsxToken::Type::EndOfFile)
      throw std::runtime_error(
      "Unexpected end of file encountered."
      );
    else if (tsxToken->type == TsxToken::Type::Attribute) {
      std::string attribute = std::string(token->value);

      token = tokeniser.getToken(); // Now check for equals
      tsxToken = dynamic_cast<TsxToken*>(token.get());
      if (tsxToken->type != TsxToken::Type::Equals)
        throw std::runtime_error(
        "Expected equals (=) after attribute."
        );

      token = tokeniser.getToken(); // Get value
      tsxToken = dynamic_cast<TsxToken*>(token.get());
      if (tsxToken->type != TsxToken::Type::String)
        throw std::runtime_error(
        "Expected string value after attribute equals sign."
        );

      node.addAttribute(attribute, token->value);
    } else if (tsxToken->type == TsxToken::Type::PIEnd)
      break;
    else {
      throw std::runtime_error(
      "Unexpected token: " + token->value
      );
    }
  }
  nodes.push_back(node);

  while(true) {
    token = tokeniser.peekToken();
    tsxToken = dynamic_cast<TsxToken*>(token.get());

    if (tsxToken->type == TsxToken::Type::EndOfFile)
      break;

    node = parseNode(tokeniser);
    nodes.push_back(node);
  }

  return nodes;
}

std::vector<TsxNode> TsxParser::getChildElements(
  const std::vector<TsxNode> &nodes,
  std::string name
) {
  std::vector<TsxNode> childNodes = {};
  for(const TsxNode &node : nodes) {
    if (node.name == name)
      childNodes.push_back(node);
  }

  return childNodes;
}

TsxNode TsxParser::parseNode(TsxTokeniser &tokeniser) {
  TsxNode node = {};

  std::unique_ptr<IToken> token = tokeniser.getToken();
  TsxToken* tsxToken = dynamic_cast<TsxToken*>(token.get());
  if (tsxToken->type != TsxToken::Type::ElementStart)
    throw std::runtime_error(
    "Unexpected character found at start of object. Expected element start."
    );

  node.name = token->value;
  while(true) {
    token = tokeniser.peekToken();
    tsxToken = dynamic_cast<TsxToken*>(token.get());
    if (tsxToken->type == TsxToken::Type::EndOfFile)
      throw std::runtime_error(
      "Unexpected end of file encountered."
      );
    else if (tsxToken->type == TsxToken::Type::Attribute) {
      token = tokeniser.getToken(); // Consume
      std::string attribute = std::string(token->value);

      token = tokeniser.getToken(); // Now check for equals
      tsxToken = dynamic_cast<TsxToken*>(token.get());
      if (tsxToken->type != TsxToken::Type::Equals)
        throw std::runtime_error(
        "Expected equals (=) after attribute."
        );

      token = tokeniser.getToken(); // Get value
      tsxToken = dynamic_cast<TsxToken*>(token.get());
      if (tsxToken->type != TsxToken::Type::String)
        throw std::runtime_error(
        "Expected string value after attribute equals sign."
        );

      node.addAttribute(attribute, token->value);
    } else if (tsxToken->type == TsxToken::Type::ElementStart ||
               tsxToken->type == TsxToken::Type::ElementClose) {
      if (tsxToken->type == TsxToken::Type::ElementClose)
        token = tokeniser.getToken(); // Consume if we have a close

      // Parse subnode
      TsxNode subnode = parseNode(tokeniser);
      node.subNodes.push_back(subnode);
    } else if (tsxToken->type == TsxToken::Type::ElementEnd) {
      token = tokeniser.getToken(); // Consume
      break;
    } else {
      throw std::runtime_error(
      "Unexpected token: " + token->value
      );
    }
  }

  return node;
}
