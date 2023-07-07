#include "json_builder.h"
#include <algorithm>

namespace json {

Builder& Builder::Value(NodeValue value)
{
    if (root && nodes_stack_.empty()) {
        throw std::logic_error("Value error: invalid method call context");
    }


    root = Node(value);

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

Builder::BaseContext Builder::StartDict()
{
    root = Dict();
    nodes_stack_.push_back(std::make_unique<Node>(*root));
    key_opened_ = false;

    return BaseContext(*this);
}

Builder::DictContext Builder::Key(std::string&& key)
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
            if (!last_key.empty()) {
                std::get<Dict>(nodes_stack_.back()->GetValue())[last_key.back()] = node_back;
                last_key.pop_back();
                key_opened_ = false;
            }
            else {
                throw std::logic_error("EndDict error: invalid method call context");
            }
        }
    }
    else {
        root = std::move(*nodes_stack_.back());
        nodes_stack_.clear();
    }

    return *this;
}

Builder::ArrayContext Builder::StartArray()
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
            if (!last_key.empty()) {
                std::get<Dict>(nodes_stack_.back()->GetValue())[last_key.back()] = std::move(node_back);
                last_key.pop_back();
                key_opened_ = false;
            }
            else {
                throw std::logic_error("EndArray error: invalid method call context");
            }
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

Builder::BaseContext::BaseContext(Builder& b)
    : builder_(b) {
}

Builder::DictContext Builder::BaseContext::Key(std::string key)
{
    return builder_.Key(std::move(key));
}

Builder& Builder::BaseContext::EndDict()
{
    return builder_.EndDict();
}

Builder::DictContext::DictContext(Builder& b)
    : builder_(b) {
}

Builder::BaseContext Builder::DictContext::Value(NodeValue&& value)
{
    return BaseContext(builder_.Value(std::move(value)));
}

Builder::BaseContext Builder::DictContext::StartDict()
{
    return builder_.StartDict();
}

Builder::ArrayContext Builder::DictContext::StartArray()
{
    return builder_.StartArray();
}

Builder::ArrayContext::ArrayContext(Builder& b)
    : builder_(b) {
}

Builder::ArrayContext Builder::ArrayContext::Value(NodeValue&& value)
{
    return Builder::ArrayContext(builder_.Value(std::move(value)));
}

Builder::BaseContext Builder::ArrayContext::StartDict()
{
    return builder_.StartDict();
}

Builder::ArrayContext Builder::ArrayContext::StartArray()
{
    return builder_.StartArray();
}

Builder& Builder::ArrayContext::EndArray()
{
    return builder_.EndArray();
}

}