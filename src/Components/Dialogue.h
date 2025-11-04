#pragma once

#include <iostream>
#include <vector>
#include <filesystem>
#include "../JsonParser.h"

namespace fs = std::filesystem;

struct Response {
  std::string line;
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
  bool canRespond = false;

  Dialogue(const char *dialogueFile) {
    try {
      // Load dialogue file
      JsonObject dialogueJson = JsonParser::parseJson(
        std::string(dialogueFile)
      );

      // Get name of dialogue file, which will be the first object name
      std::string dialogueName = fs::path(dialogueFile).stem().string();

      // Parse into dialogue tree
      JsonArray dialogueArray = dialogueJson[dialogueName].getArray();
      for (const auto &jnode : dialogueArray) {
        std::vector<Response> responses = {};
        JsonArray responseArray = jnode.at("responses").getArray();
        for (const auto &jresp : responseArray) {
          int next = static_cast<int>(jresp.at("next").getNumber());
          responses.push_back({jresp.at("response").getString(),
                               next});
        }
        dialogueTree.push_back({
          static_cast<int>(jnode.at("id").getNumber()),
          jnode.at("speaker").getString(),
          jnode.at("line").getString(),
          responses
        });
      }
    } catch (const std::exception &e) {
      std::cerr << "Error loading dialogue: " << e.what() << std::endl;
      throw;
    }
  }

  ~Dialogue() {}

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

  std::string getSpeaker() {
    DialogueNode *node = getNodeFromId(currentNode);
    if (node != nullptr) {
      return node->speaker;
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
