#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

// Oшибка выбрасываeться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node;
    
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

class Node final : private NodeValue {
public:

    using NodeValue::NodeValue;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    bool operator==(const Node& rhs) const;

    const Node& GetValue() const;
       
};

inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}



struct PrintNode{

    void operator()(const Node& node) const;

    void operator()(nullptr_t value) const;
    void operator()(Array value) const;
    void operator()(Dict value) const;
    void operator()(bool value) const;
    void operator()(int value) const;
    void operator()(double value) const;
    void operator()(std::string value) const;

    std::ostream& out;
};



class Document {
public:

    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs)
{
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs)
{
    return !(lhs == rhs);
}

Document Load(std::istream& input);


void Print(const Document& doc, std::ostream& output);

}  // namespace json