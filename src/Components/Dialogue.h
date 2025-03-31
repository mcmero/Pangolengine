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
  int currentNode = -1;
  bool active = false;

  Dialogue(const char *dialogueFile) {
    try {
      // Load dialogue file
      std::ifstream f(dialogueFile);
      if (!f.is_open()) {
        throw std::runtime_error("Failed to open dialogue file.");
      }
      json dialogueJson = json::parse(f);

      // Parse into dialogue tree
      for (const auto &jnode : dialogueJson) {
        std::vector<Response> responses = {};
        if (jnode.contains("responses") && jnode["responses"].is_array()) {
          for (const auto &jresp : jnode["responses"]) {
            responses.push_back({jresp["response"], jresp["next"]});
          }
        }
        dialogueTree.push_back(
            {jnode["id"], jnode["speaker"], jnode["line"], responses});
      }
    } catch (const std::exception &e) {
      std::cerr << "Error loading dialogue: " << e.what() << std::endl;
      throw;
    }
  }

  void beginDialogue() {
    if (!dialogueTree.empty()) {
      currentNode = dialogueTree.front().id;
      active = true;
    } else {
      throw std::runtime_error("Dialogue tree is empty.");
    }
  }
  std::string getLine() {
    DialogueNode *node = getNodeFromId(currentNode);
    if (node != nullptr) {
      return node->line;
    }
    throw std::runtime_error("Current node is not valid.");
  }

  std::vector<Response> getResponses() {
    DialogueNode *node = getNodeFromId(currentNode);
    if (node != nullptr) {
      return node->responses;
    }
    throw std::runtime_error("Current node is not valid.");
  }

  // Returns true if dialogue continues and false otherwise
  bool progressToNode(int nextNodeId) {
    DialogueNode *node = getNodeFromId(nextNodeId);
    if (node != nullptr) {
      currentNode = node->id;
      std::cout << "Progressed to node " << currentNode << std::endl;
      return true;
    }
    active = false;
    return false; // End dialogue
  }

private:
  DialogueNode *getNodeFromId(int id) {
    for (auto &node : dialogueTree) {
      if (node.id == id)
        return &node;
    }
    return nullptr;
  }
};
