#pragma once

#include <fstream>
#include <third_party/nlohmann/json.hpp>

using namespace nlohmann;

struct Response {
  std::string response;
  int next;
};

struct DialogueNode {
  int id;
  std::string speaker;
  std::string line;
  std::vector<Response> responses;
};

class Dialogue {
public:
  std::vector<DialogueNode> dialogueTree = {};
  DialogueNode currentNode;
  bool active = false;

  Dialogue(const char *dialogueFile) {
    // load dialogue
    std::ifstream f(dialogueFile);
    json dialogueJson = json::parse(f);

    // parse into dialogue tree
    for (auto jnode : dialogueJson) {
      std::vector<Response> responses = {};
      if (jnode["responses"].size() > 0) {
        for (auto jresp : jnode["responses"]) {
          responses.push_back({jresp["response"], jresp["next"]});
        }
      }
      dialogueTree.push_back(
          {jnode["id"], jnode["speaker"], jnode["line"], responses});
    }
  }

  void beginDialogue() {
    if (dialogueTree.size() > 0)
      currentNode = dialogueTree.front();
  }

  std::string getLine() { return (currentNode.line); }

  std::vector<Response> getResponses() { return (currentNode.responses); }
};
