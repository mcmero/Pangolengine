#pragma once

#include <memory>
#include <string>

class IToken {
public:
  virtual ~IToken() = default;
  virtual std::unique_ptr<IToken> clone() const = 0;

  std::string value = "";

  uint32_t line = 1;
  uint32_t column = 1;
};

class Tokeniser {
public:
  Tokeniser(std::istream &in);

  std::unique_ptr<IToken> getToken();
  std::unique_ptr<IToken> peekToken();

protected:
  std::istream &in;
  uint32_t line = 1;
  uint32_t column = 1;

  virtual std::unique_ptr<IToken> getTokenImpl() = 0;

  template<typename TokenType, typename...  Args>
  std::unique_ptr<IToken> makeToken(Args&&... args) {
    return std::make_unique<TokenType>(std::forward<Args>(args)... );
  }

  void skipWhitespace();
  int getChar();

  std::string parseString();
  std::string parseAlpha(const char terminators[] = ",]}");
  std::string parseNumber();
  std::string parseDigits();

  [[noreturn]] void raiseError(std::string message);

private:
  std::unique_ptr<IToken> lookahead_;
};

