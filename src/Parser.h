#pragma once

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
class JsonNode : IJsonNode {
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

    // TODO: Parse JSON

    return Json{};
  }
};
