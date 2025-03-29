#pragma once

#include <fstream>
#include <iostream>
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
  int currentNode = 0;
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
      currentNode = dialogueTree.front().id;
  }

  std::string getLine() {
    DialogueNode *node = getNodeFromId(currentNode);
    if (!(node == nullptr)) {
      // std::cout << "Node ID " << currentNode << std::endl;
      return node->line;
    }
    // TODO: handle case/error
    return "";
  }

  std::vector<Response> getResponses() {
    DialogueNode *node = getNodeFromId(currentNode);
    if (!(node == nullptr)) {
      return node->responses;
    }
    // TODO: handle case/error
    return std::vector<Response>();
  }

  // Returns true if dialogue continues and false otherwise
  bool progressToNode(int nextNodeId) {
    for (auto &node : dialogueTree) {
      if (node.id == nextNodeId) {
        currentNode = node.id;
        std::cout << "Progressed to node " << currentNode << std::endl;
        return true;
      }
    }
    active = false;
    return false; // end dialogue
  }

private:
  DialogueNode *getNodeFromId(int id) {
    for (auto &node : dialogueTree) {
      if (node.id == id) {
        return &node;
      }
    }
    return nullptr;
  }
};
