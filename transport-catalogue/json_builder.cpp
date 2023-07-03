#include "json_builder.h"
#include <algorithm>

namespace json {

Builder& Builder::Value(NodeValue&& value)
{
    if (root && nodes_stack_.empty()) {
        throw std::logic_error("Value error: invalid method call context");
    }

    if (std::holds_alternative<std::string>(value)) {
        root = Node(std::get<std::string>(value));
    }
    else if (std::holds_alternative<int>(value)) {
        root = Node(std::get<int>(value));
    }
    else if (std::holds_alternative<bool>(value)) {
        root = Node(std::get<bool>(value));
    }
    else if (std::holds_alternative<double>(value)) {
        root = Node(std::get<double>(value));
    }
    else if (std::holds_alternative<Array>(value)) {
        root = Node(std::get<Array>(value));
    }
    else if (std::holds_alternative<Dict>(value)) {
        root = Node(std::get<Dict>(value));
    }
    else {
        root = Node(nullptr);
    }


    if (!nodes_stack_.empty())
    {
        if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetValue()).push_back(std::move(*root));
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
            std::get<Dict>(nodes_stack_.back()->GetValue())[last_key.back()] = *root;
            last_key.pop_back();
            key_opened_ = false;
        }
        else {
            throw std::logic_error("Value error: Unknown structure on building stack");
        }
    }

    return *this;
}

BaseContext Builder::StartDict()
{
    root = Dict();
    nodes_stack_.push_back(std::make_unique<Node>(*root));
    key_opened_ = false;

    return BaseContext(*this);
}

DictContext Builder::Key(std::string&& key)
{
    key_opened_ = true;

    std::get<Dict>(nodes_stack_.back()->GetValue())[key];
    last_key.push_back(std::move(key));

    return DictContext(*this);
}

Builder& Builder::EndDict()
{

    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw std::logic_error("EndDict error: invalid method call context");
    }

    if (nodes_stack_.size() != 1) {
        auto node_back = std::move(*nodes_stack_.back());
        nodes_stack_.pop_back();
        if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetValue()).push_back(node_back);
        }
        else {
            std::get<Dict>(nodes_stack_.back()->GetValue())[last_key.back()] = node_back;
            last_key.pop_back();
            key_opened_ = false;
        }
    }
    else {
        root = std::move(*nodes_stack_.back());
        nodes_stack_.clear();
    }

    return *this;
}

ArrayContext Builder::StartArray()
{
    if (root && nodes_stack_.empty()) {
        throw std::logic_error("StartArray error: invalid method call context");
    }

    root = Array();
    nodes_stack_.push_back(std::make_unique<Node>(*root));

    return ArrayContext(*this);
}

Builder& Builder::EndArray()
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("EndArray error: invalid method call context");
    }

    if (nodes_stack_.size() != 1) {
        auto node_back = std::move(*nodes_stack_.back());
        nodes_stack_.pop_back();
        if (nodes_stack_.back()->IsDict()) {
            std::get<Dict>(nodes_stack_.back()->GetValue())[last_key.back()] = std::move(node_back);
            last_key.pop_back();
            key_opened_ = false;
        }
        else {
            std::get<Array>(nodes_stack_.back()->GetValue()).push_back(std::move(node_back));
        }
    }
    else {
        root = std::move(*nodes_stack_.back());
        nodes_stack_.clear();
    }

    return *this;
}

Node Builder::Build()
{
    if (nodes_stack_.empty() && !root) {
        throw std::logic_error("Build error: no item to build Node");
    }

    if (!nodes_stack_.empty()) {
        root = std::move(*nodes_stack_.back());
    }

    return *root;
}

BaseContext::BaseContext(Builder& b)
    : builder_(b) {
}

DictContext BaseContext::Key(std::string key)
{
    return builder_.Key(std::move(key));
}

Builder& BaseContext::EndDict()
{
    return builder_.EndDict();
}

DictContext::DictContext(Builder& b)
    : builder_(b) {
}

BaseContext DictContext::Value(NodeValue&& value)
{
    return BaseContext(builder_.Value(std::move(value)));
}

BaseContext DictContext::StartDict()
{
    return builder_.StartDict();
}

ArrayContext DictContext::StartArray()
{
    return builder_.StartArray();
}

ArrayContext::ArrayContext(Builder& b)
    : builder_(b) {
}

ArrayContext json::ArrayContext::Value(NodeValue&& value)
{
    return ArrayContext(builder_.Value(std::move(value)));
}

BaseContext ArrayContext::StartDict()
{
    return builder_.StartDict();
}

ArrayContext ArrayContext::StartArray()
{
    return builder_.StartArray();
}

Builder& ArrayContext::EndArray()
{
    return builder_.EndArray();
}

}