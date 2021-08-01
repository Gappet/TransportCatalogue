#include "json_builder.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "json.h"

using namespace std::string_literals;

namespace json {

SetKey Builder::Key(std::string key) {
  if (node_stack_.back()->IsDict()) {
    node_stack_.emplace_back(std::make_unique<Node>(key));
  } else {
    throw std::logic_error("Key cant be applied"s);
  }
  return *this;
}

Builder& Builder::Value(Node::Value value) {
  if (node_stack_.empty()) {
    auto a =
        std::make_unique<Node>((std::visit(NodeGetter{}, std::move(value))));
    node_stack_.emplace_back(std::move(a));
  } else if (node_stack_.back()->IsArray() && cnt_build_array_ != 0) {
    auto a = std::move(node_stack_.back()->AsArray());
    a.emplace_back(std::visit(NodeGetter{}, std::move(value)));
    node_stack_.pop_back();
    node_stack_.emplace_back(std::make_unique<Node>(std::move(a)));
  } else if (node_stack_.back()->IsString() && cnt_build_dict_ != 0) {
    auto key = std::move(node_stack_.back());
    node_stack_.pop_back();
    Dict a = std::move(node_stack_.back()->AsDict());
    a.emplace(key->AsString(), std::visit(NodeGetter{}, std::move(value)));
    node_stack_.pop_back();
    node_stack_.emplace_back(std::make_unique<Node>(std::move(a)));
  } else {
    throw std::logic_error("Value can't be applied"s);
  }
  return *this;
}

StartArrayC Builder::StartArray() {
  if (node_stack_.empty()) {
    ++cnt_build_array_;
    node_stack_.push_back(std::make_unique<Node>(Array{}));
    return *this;
  }
  if (node_stack_.back()->IsString() || cnt_build_array_ != 0 ||
      cnt_build_dict_ != 0) {
    ++cnt_build_array_;
    node_stack_.emplace_back(std::make_unique<Node>(Array{}));
  } else {
    throw std::logic_error("Cant create array"s);
  }
  return *this;
}

StartDictC Builder::StartDict() {
  if (node_stack_.empty()) {
    ++cnt_build_dict_;
    node_stack_.push_back(std::make_unique<Node>(Dict{}));
    return *this;
  }
  if (node_stack_.back()->IsString() || cnt_build_array_ != 0 ||
      cnt_build_dict_ != 0) {
    ++cnt_build_dict_;
    node_stack_.push_back(std::make_unique<Node>(Dict{}));
  } else {
    throw std::logic_error("Cant create array"s);
  }
  return *this;
}

Builder& Builder::EndArray() {
  if (node_stack_.back()->IsArray()) {
    --cnt_build_array_;
    Array end_array = std::move(node_stack_.back()->AsArray());
    node_stack_.pop_back();
    this->Value(std::move(end_array));
  } else {
    throw std::logic_error("Cant end array"s);
  }
  return *this;
}

Builder& Builder::EndDict() {
  if (node_stack_.back()->IsDict()) {
    --cnt_build_dict_;
    Dict end_dict = std::move(node_stack_.back()->AsDict());
    node_stack_.pop_back();
    this->Value(std::move(end_dict));

  } else {
    throw std::logic_error("Cant end dictionary"s);
  }
  return *this;
}

Node Builder::Build() {
  if (cnt_build_array_ != 0) {
    throw std::logic_error("Array build can't be applied "s);
  }

  if (cnt_build_dict_ != 0) {
    throw std::logic_error("Dictionary Build can't be applied 2"s);
  }

  if (node_stack_.size() == 1) {
    root_ = std::move(*node_stack_.back());

  } else {
    throw std::logic_error("Build can't be applied"s);
  }
  return root_;
}

StartDictC SetKey::Value(Node::Value value) { return builder_.Value(value); }

StartDictC SetKey::StartDict() { return builder_.StartDict(); }

StartArrayC SetKey::StartArray() { return builder_.StartArray(); }

SetKey StartDictC::Key(std::string key) { return builder_.Key(key); }

Builder& StartDictC::EndDict() { return builder_.EndDict(); }

StartArrayC StartArrayC::Value(Node::Value value) {
  return builder_.Value(value);
}

StartDictC StartArrayC::StartDict() {
  builder_.StartDict();
  return builder_;
}

StartArrayC StartArrayC::StartArray() {
  builder_.StartArray();
  return builder_;
}

Builder& StartArrayC::EndArray() { return builder_.EndArray(); }

}  // namespace json
