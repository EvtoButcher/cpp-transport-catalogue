#pragma once
#include "json.h"
#include <optional>
#include <vector>
#include <string>
#include <memory>

namespace json {

	class Builder
	{
	public:
		class BaseContext;
		class ArrayContext;
		class DictContext;

		Builder() = default;

		Builder& Value(NodeValue value);

		BaseContext StartDict();
		DictContext Key(std::string key);
		Builder& EndDict();

		ArrayContext StartArray();
		Builder& EndArray();

		Node Build();

	private:
		std::optional<Node> root{};
		std::vector<std::unique_ptr<Node>> nodes_stack_{};
		std::vector<std::string> last_key{};
		bool key_opened_ = false;
	};


	class Builder::BaseContext
	{
	public:
		BaseContext(Builder& b);
		DictContext Key(std::string key);
		Builder& EndDict();
	private:
		Builder& builder_;
	};

	class Builder::DictContext
	{
	public:
		DictContext(Builder& b);
		BaseContext Value(NodeValue value);
		BaseContext StartDict();
		ArrayContext StartArray();
	private:
		Builder& builder_;
	};

	class Builder::ArrayContext
	{
	public:
		ArrayContext(Builder& b);
		ArrayContext Value(NodeValue value);
		BaseContext StartDict();
		ArrayContext StartArray();
		Builder& EndArray();
	private:
		Builder& builder_;
	};

}

