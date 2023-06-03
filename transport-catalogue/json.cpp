#include "json.h"
#include <math.h>


using namespace std;

namespace json {

namespace {

// ---------- Loaders ------------------
Node LoadNode(istream& input);

std::string LoadLiteral(std::istream& input)
{
    std::string s;
    while (std::isalpha(input.peek()))
    {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}

Node LoadArray(istream& input) {
    std::vector<Node> result;

    for (char c; input >> c && c != ']';)
    {
        if (c != ',')
        {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input)
    {
        throw ParsingError("Array parsing error"s);
    }
    return Node(std::move(result));
}

Node LoadString(istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true)
    {
        if (it == end)
        {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"')
        {
            ++it;
            break;
        }
        else if (ch == '\\')
        {
            ++it;
            if (it == end)
            {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r')
        {
            throw ParsingError("Unexpected end of line"s);
        }
        else
        {
            s.push_back(ch);
        }
        ++it;
    }
    return Node(std::move(s));
}

Node LoadDict(istream& input) {
    Dict dict;

    for (char c; input >> c && c != '}';)
    {
        if (c == '"')
        {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':')
            {
                if (dict.find(key) != dict.end())
                {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                dict.emplace(std::move(key), LoadNode(input));
            }
            else
            {
                throw ParsingError(": is expected but '"s + c + "' has been found"s);
            }
        }
        else if (c != ',')
        {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input)
    {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}

Node LoadBool(istream& input) {
    auto line = LoadLiteral(input);
    if (line == "true"sv)
    {
        return Node{ true };
    }
    else if (line == "false"sv)
    {
        return Node{ false };
    }
    else
    {
        throw ParsingError("Failed to parse '"s + line + "' as bool"s);
    }
}

Node LoadNull(istream& input) {

    if (auto literal = LoadLiteral(input); literal == "null"sv)
    {
        return Node();
    }
    else
    {
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
    }
}

Node LoadNumber(std::istream& input)
{
    std::string parsed_num;

    auto read_char = [&parsed_num, &input]
    {
        parsed_num += static_cast<char>(input.get());
        if (!input)
        {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char]
    {
        if (!std::isdigit(input.peek()))
        {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek()))
        {
            read_char();
        }
    };

    if (input.peek() == '-')
    {
        read_char();
    }
    if (input.peek() == '0')
    {
        read_char();
    }
    else
    {
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.')
    {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E')
    {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-')
        {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try
    {
        if (is_int) {
            try {
                return std::stoi(parsed_num);
            }
            catch (...)
            {

            }
        }
        return std::stod(parsed_num);
    }
    catch (...)
    {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(istream& input) {
    char c;
    if (!(input >> c))
    {
        throw ParsingError("Unexpected EOF"s);
    }
    switch (c)
    {
    case '[':
        return LoadArray(input);
    case '{':
        return LoadDict(input);
    case '"':
        return LoadString(input);
    case 't':
        [[fallthrough]];
    case 'f':
        input.putback(c);
        return LoadBool(input);
    case 'n':
        input.putback(c);
        return LoadNull(input);
    default:
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

// ---------- Node ------------------

bool Node::IsInt() const
{
    return holds_alternative<int>(*this);
}

bool Node::IsDouble() const
{
    return IsPureDouble() || IsInt();
}

bool Node::IsPureDouble() const
{
    return holds_alternative<double>(*this);
}

bool Node::IsBool() const
{
    return holds_alternative<bool>(*this);
}

bool Node::IsString() const
{
    return holds_alternative<std::string>(*this);
}

bool Node::IsNull() const
{
    return holds_alternative<nullptr_t>(*this);
}

bool Node::IsArray() const
{
    return holds_alternative<Array>(*this);
}

bool Node::IsMap() const
{
    return holds_alternative<Dict>(*this);
}

const Array& Node::AsArray() const {
    return IsArray() ? std::get<Array>(*this) : throw std::logic_error("Not a Array");
}

const Dict& Node::AsMap() const {
    return IsMap() ? std::get<Dict>(*this) : throw std::logic_error("Not a map");
}

bool Node::operator==(const Node& rhs) const
{
    return GetValue() == rhs.GetValue();
}

const Node& Node::GetValue() const
{
    return *this;
}

int Node::AsInt() const {
    return IsInt() ? std::get<int>(*this) : throw std::logic_error("Not a int");
}

bool Node::AsBool() const
{
    return IsBool() ? std::get<bool>(*this) : throw std::logic_error("Not a bool");
}

double Node::AsDouble() const
{
    if (!IsDouble()) {
        throw std::logic_error("Not a double");
    }
    else if (IsPureDouble()) {
        return std::get<double>(*this);
    }
    else {
        return AsInt();
    }
}

const string& Node::AsString() const {
    return IsString() ? std::get<string>(*this) : throw std::logic_error("Not a string");
}

// ---------- Document ------------------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode{ output }.operator()(doc.GetRoot().GetValue());
}

// ---------- PrintNode ------------------

void PrintNode::operator()(const Node& node) const
{
    if (node.IsMap()) {
        this->operator()(node.AsMap());
        return;
    }
    if (node.IsArray()) {
        this->operator()(node.AsArray());
        return;
    }
    if (node.IsString()) {
        this->operator()(node.AsString());
        return;
    }
    if (node.IsInt()) {
        this->operator()(node.AsInt());
        return;
    }
    if (node.IsDouble()) {
        this->operator()(node.AsDouble());
        return;
    }
    if (node.IsBool()) {
        this->operator()(node.AsBool());
        return;
    }
    if (node.IsNull()) {
        this->operator()(nullptr);
        return;
    }
}

void PrintNode::operator()(nullptr_t) const
{
        out << std::string("null");
}

void PrintNode::operator()(Array value) const
{
    using namespace std::literals;
    out << "[\n"sv;
    bool not_first = false;
    for (const Node& node : value)
    {
        if (not_first) {
            out << ",\n"sv;
        }
        this->operator()(node.GetValue());
        not_first = true;
    }
    out << "\n]"sv;
}

void PrintNode::operator()(Dict value) const
{
    using namespace std::literals;
    out << "{\n"sv;
    bool first = true;

    for (const auto& [key, node] : value)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ",\n"sv;
        }

        out << "\""sv << key;
        out << "\": "sv;
        this->operator()(node.GetValue());
    }
    out << "\n}"sv;
}

void PrintNode::operator()(bool value) const
{
    using namespace std::literals;
    value ? out << "true" : out << "false"sv;
}

void PrintNode::operator()(int value) const
{
    out << value;
}

void PrintNode::operator()(double value) const
{
    out << value;
}

void PrintNode::operator()(std::string value) const
{
    using namespace std::literals;
    out.put('"');
    for (const char c : value)
    {
        switch (c)
        {
        case '\r':
            out << "\\r"sv;
            break;
        case '\n':
            out << "\\n"sv;
            break;
        case '"':
            [[fallthrough]];
        case '\\':
            out.put('\\');
            [[fallthrough]];
        default:
            out.put(c);
            break;
        }
    }
    out.put('"');
}

}  // namespace json