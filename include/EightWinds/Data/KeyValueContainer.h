#pragma once

#include "EightWinds/Preprocessor.h"

#include <type_traits>
#include <utility>
#include <initializer_list>

#include <vector>

//#include "EWGraphics/Data/OptionalMutex.h"

#ifndef INITIAL_SIZE_LIMIT
#define INITIAL_SIZE_LIMIT 64
#endif

#define TO_STRING(str) #str
#define INITIAL_SIZE_STR(str) TO_STRING(str)


namespace EWE {
	namespace KV_Helper {
		template <typename, typename = void>
		struct is_container : std::false_type {};

		template <typename T>
		struct is_container<T, std::void_t<
			decltype(std::declval<T>().begin()),
			decltype(std::declval<T>().end()),
			decltype(std::declval<T>().size())
			>> : std::true_type {};

		constexpr void CheckSizeCondition(const std::size_t size) {
#if defined(__GNUC__) || defined(__clang__)
#if size > INITIAL_SIZE_LIMIT
			#warning "Warning: initialSize exceeds the INITIAL_SIZE_LIMIT (" INITIAL_SIZE_STR(INITIAL_SIZE_LIMIT) "), consider using a hash table"
#endif
#elif defined (_MSC_VER)
#if size > INITIAL_SIZE_LIMIT
#pragma message("Warning: initialSize exceeds the INITIAL_SIZE_LIMIT (" INITIAL_SIZE_STR(INITIAL_SIZE_LIMIT) "), consider using a hash table")
#endif
#endif
		}
	}

	template<typename Key, typename Value>
	struct KeyValuePair {
		Key key;
		Value value;

		using KeyParamType = typename std::conditional_t<
			std::is_pointer_v<Key> || std::is_lvalue_reference_v<Key>,
			Key,
			typename std::conditional_t<
				sizeof(Key) <= sizeof(void*) && !KV_Helper::is_container<Key>::value,
				Key,
				const Key&
			>
		>;
		using ValueParamType = typename std::conditional_t<
			std::is_pointer_v<Value> || std::is_lvalue_reference_v<Value>,
			Value,
			typename std::conditional_t<
				sizeof(Value) <= sizeof(void*) && !KV_Helper::is_container<Value>::value,
				Value,
				const Value&
			>
		>;


		KeyValuePair(KeyParamType _key, ValueParamType _value) : key{ _key }, value{ _value } {}

		template<typename = std::enable_if_t<std::is_default_constructible_v<Value>>>
		explicit KeyValuePair(KeyParamType _key) : key{ _key }, value{ } {}

		template<typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_default_constructible_v<K> && std::is_default_constructible_v<V>>>
		KeyValuePair() : key{}, value{} {}

		~KeyValuePair() = default;

		template <typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_copy_constructible_v<K> && std::is_copy_constructible_v<V>>>
		KeyValuePair(const KeyValuePair& copySource)
			: key{ copySource.key }, value{ copySource.value } {
		}

		template <typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_move_constructible_v<K> && std::is_move_constructible_v<V>>>
		KeyValuePair(KeyValuePair&& moveSource) noexcept
			: key{ std::move(moveSource.key) }, value{ std::move(moveSource.value) } {
		}

		template <typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_copy_assignable_v<K> && std::is_copy_assignable_v<V>>>
		KeyValuePair& operator=(const KeyValuePair& copySource) {
			EWE_ASSERT(this != &copySource);
			key = copySource.key;
			value = copySource.value;
			return *this;
		}

		template <typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_move_assignable_v<K>&& std::is_move_assignable_v<V>>>
		KeyValuePair& operator=(KeyValuePair&& moveSource) noexcept {
			EWE_ASSERT(this != &moveSource);
			key = std::move(moveSource.key);
			value = std::move(moveSource.value);
			
			return *this;
		}

	};

	template<typename Key, typename Value>
	class KeyValueContainer {
	private:

		using KVPair = KeyValuePair<Key, Value>;
		std::vector<KVPair> inner_data;
		using iterator = std::vector<KVPair>::iterator;
	public:
		constexpr KeyValueContainer() : inner_data{} {}
		constexpr KeyValueContainer(std::size_t count) : inner_data{ count } { KV_Helper::CheckSizeCondition(count); }
		constexpr KeyValueContainer(std::size_t count, KVPair const& value) : inner_data{ count, value } { KV_Helper::CheckSizeCondition(count); }
		constexpr KeyValueContainer(std::initializer_list<KVPair> init) : inner_data{ init } { KV_Helper::CheckSizeCondition(init.size()); }

		template <typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_copy_constructible_v<K>&& std::is_copy_constructible_v<V>>>
		KeyValueContainer(KeyValueContainer& copySource) : inner_data{copySource.inner_data.begin(), copySource.inner_data.end() } {}
		template <typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_move_constructible_v<K>&& std::is_move_constructible_v<V>>>
		KeyValueContainer(KeyValueContainer&& moveSource) : inner_data{ std::move(moveSource.inner_data) } {}
		template <typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_copy_assignable_v<K>&& std::is_copy_assignable_v<V>>>
		KeyValueContainer& operator=(KeyValueContainer& other) = delete;
		template <typename K = Key, typename V = Value, typename = std::enable_if_t<std::is_move_assignable_v<K>&& std::is_move_assignable_v<V>>>
		KeyValueContainer& operator=(KeyValueContainer&& other) = delete;
		~KeyValueContainer() = default;

		KVPair& at(KVPair::KeyParamType _key) {
			for (auto& point : inner_data) {
				if (point.key == _key) {
					return point;
				}
			}
			EWE_UNREACHABLE;
		}
		KVPair const& at(KVPair::KeyParamType _key) const {
			for (auto& point : inner_data) {
				if (point.key == _key) {
					return point;
				}
			}
			EWE_UNREACHABLE;
		}

		auto operator[](std::size_t i) {
			return inner_data[i];
		}
		Value& GetValue(KVPair::KeyParamType _key) {
			return at(_key).value;
		}
		Value const& GetValue(KVPair::KeyParamType _key) const {
			return at(_key).value;
		}

		void* data() {
			return inner_data.data();
		}
		auto begin() { return inner_data.begin();}
		auto end() { return inner_data.end(); }
		auto cbegin() const { return inner_data.cbegin(); }
		auto cend() const { return inner_data.cend(); }
		auto begin() const { return inner_data.begin();}
		auto end() const { return inner_data.end();}

		std::size_t size() {
			return inner_data.size();
		}
		void reserve(std::size_t res) {
			inner_data.reserve(res);
		}

		void erase(iterator iter) {
			inner_data.erase(iter);
		}
		auto find(KVPair::KeyParamType _key) const {
			for(auto iter = begin(); iter != end(); iter++){
				if(iter->key == _key){
					return iter;
				}
			}
			return end();
		}

		void clear() {
			inner_data.clear();
		}
		template<typename = std::enable_if_t<std::is_default_constructible_v<Value>>>
		Value& push_back(KVPair::KeyParamType _key) {

			return inner_data.emplace_back(_key).value;
		}
		template<typename = std::enable_if_t<std::is_default_constructible_v<Value>>>
		Value& emplace_back(KVPair::KeyParamType _key) {

			return inner_data.emplace_back(_key).value;
		}
		void push_back(KVPair const& kvPair) {
			inner_data.push_back(kvPair);
		}


		void push_back(KVPair::KeyParamType _key, Value _value) {
			inner_data.push_back(KVPair(_key, _value));
		}

		void emplace_back(Key&& _key, Value&& _value) {
			inner_data.emplace_back(KVPair(_key, _value));
		}
		void emplace_back(KVPair&& kvPair) {
			inner_data.emplace_back(kvPair);
		}

		void Remove(iterator iter) {
			inner_data.erase(iter);
		}
		void Remove(KVPair::KeyParamType _key) {
			for (auto iter = inner_data.begin(); iter != inner_data.end(); iter++) {
				if (iter->key == _key) {
					Remove(iter);
					return;
				}
			}
			EWE_ASSERT(false);
		}
		bool Contains(KVPair::KeyParamType _key) {
			for (auto iter = inner_data.begin(); iter != inner_data.end(); iter++) {
				if (iter->key == _key) {
					return true;
				}
			}
			return false;
		}
		bool Contains(KVPair::KeyParamType _key) const {
			for (auto iter = inner_data.begin(); iter != inner_data.end(); iter++) {
				if (iter->key == _key) {
					return true;
				}
			}
			return false;
		}
	};

}