#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "json.h"

namespace json {

//  class Builder;
class SetKey;

class StartDictC;

class StartArrayC;

class SetValue;

struct NodeGetter {
  Node operator()(std::nullptr_t) const { return Node(); }
  Node operator()(std::string&& value) const { return Node(std::move(value)); }
  Node operator()(bool&& value) const { return Node(value); }
  Node operator()(int&& value) const { return Node(value); }
  Node operator()(double&& value) const { return Node(value); }
  Node operator()(Array&& value) const { return Node(std::move(value)); }
  Node operator()(Dict&& value) const { return Node(std::move(value)); }
};

class Builder {
 public:
  SetKey Key(std::string key);

  Builder& Value(Node::Value value);

  StartDictC StartDict();

  StartArrayC StartArray();

  Builder& EndDict();

  Builder& EndArray();

  Node Build();

 private:
  Node root_;
  std::vector<std::unique_ptr<Node>> node_stack_;
  int cnt_build_array_ = 0;
  int cnt_build_dict_ = 0;
};

class SetKey {
 public:
  SetKey(Builder& builder) : builder_(builder) {}

  StartDictC Value(Node::Value value);

  StartDictC StartDict();

  StartArrayC StartArray();

 protected:
  Builder& builder_;
};

class StartDictC {
 public:
  StartDictC(json::Builder& builder) : builder_(builder) {}

  SetKey Key(std::string key);

  Builder& EndDict();

 protected:
  Builder& builder_;
};

class StartArrayC {
 public:
  StartArrayC(json::Builder& builder) : builder_(builder) {}

  StartArrayC Value(Node::Value value);

  StartDictC StartDict();

  StartArrayC StartArray();

  Builder& EndArray();

 private:
  Builder& builder_;
};

}  // namespace json
