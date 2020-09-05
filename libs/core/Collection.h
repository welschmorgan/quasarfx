//
// Created by darkboss on 8/2/20.
//

#ifndef QUASARFX_COLLECTION_H
#define QUASARFX_COLLECTION_H

#include <vector>
#include <list>
#include <queue>
#include <deque>
#include <stack>

#include <functional>
#include <map>

namespace quasar {
	namespace core {
		template<typename Value, typename Container = std::vector<Value>>
		class BasicCollection {
		public:
			using value_type = Value;
			using container_type = Container;
			using self_type = BasicCollection<Value, Container>;

			using iter_type = typename container_type::iterator;
			using riter_type = typename container_type::reverse_iterator;

			using citer_type = typename container_type::const_iterator;
			using criter_type = typename container_type::const_reverse_iterator;

			using cfilter_predicate = std::function<bool(const value_type&)>;
			using cfilter_predicate_with_id = std::function<bool(size_t i, const value_type&)>;

			using filter_predicate = std::function<bool(value_type&)>;
			using filter_predicate_with_id = std::function<bool(size_t i, value_type&)>;

			using map_fn = std::function<value_type(const value_type&)>;
			using map_fn_with_id = std::function<value_type(size_t, const value_type&)>;

			template<typename Carry = value_type>
			using fold_fn = std::function<Carry(Carry, const Value &)>;

			template<typename Carry = value_type>
			using fold_fn_with_id = std::function<Carry(size_t id, const Carry &, const Value &)>;

		protected:
			container_type      mData;

		public:
			BasicCollection() = default;
			explicit BasicCollection(const container_type &data): mData(data) {}
			BasicCollection(const std::initializer_list<Value> &data): mData(data) {}
			BasicCollection(const BasicCollection &rhs) = default;
			virtual ~BasicCollection() = default;

			BasicCollection          &operator=(const BasicCollection &rhs) = default;
			BasicCollection          &operator=(const std::initializer_list<Value> &rhs) {
				mData = rhs.mData;
				return *this;
			}

			container_type      &&operator*() noexcept { return mData; }
			const container_type&&operator*() const noexcept { return mData; }

			container_type      *operator->() noexcept { return &mData; }
			const container_type*operator->() const noexcept { return &mData; }

			const container_type&&data() const noexcept { return mData; }
			container_type      &&data() noexcept { return mData; }

			void                clear() noexcept { mData.clear(); }
			bool                empty() const noexcept { return mData.empty(); }
			size_t              size() const noexcept { return mData.size(); }
//			virtual void        add(Value &&v) { mData.push_back(v); }
//			virtual void        add(const Value &v) { mData.push_back(v); }

			bool                includes(const Value &wanted) const noexcept {
				return find([&](const Value &it) {
					return it == wanted;
				}) != nullptr;
			}

			iter_type           begin() {
				return mData.begin();
			}
			citer_type          begin() const {
				return mData.begin();
			}
			citer_type          cbegin() const {
				return mData.cbegin();
			}
			riter_type          rbegin() {
				return mData.rbegin();
			}
			criter_type         rbegin() const {
				return mData.rbegin();
			}
			criter_type         crbegin() const {
				return mData.crbegin();
			}


			iter_type           end() {
				return mData.end();
			}
			citer_type          end() const {
				return mData.end();
			}
			citer_type          cend() const {
				return mData.end();
			}
			riter_type          rend() {
				return mData.end();
			}
			criter_type         rend() const {
				return mData.end();
		}
			criter_type         crend() const {
				return mData.end();
			}

			iter_type           iter(const filter_predicate &p) {
				for (iter_type it = mData.begin(); it != mData.end(); ++it) {
					if (p(*it)) {
						return it;
					}
				}
				return mData.end();
			}

			citer_type          iter(const cfilter_predicate &p) const {
				for (citer_type it = mData.begin(); it != mData.end(); ++it) {
					if (p(*it)) {
						return it;
					}
				}
				return mData.end();
			}

			iter_type           iter(const filter_predicate_with_id &p) {
				size_t id = 0;
				for (iter_type it = mData.begin(); it != mData.end(); ++it, ++id) {
					if (p(id, *it)) {
						return it;
					}
				}
				return mData.end();
			}

			citer_type          iter(const cfilter_predicate_with_id &p) const {
				size_t id = 0;
				for (iter_type it = mData.begin(); it != mData.end(); ++it, ++id) {
					if (p(id, *it)) {
						return it;
					}
				}
				return mData.end();
			}

			/**
			 * Iterate over each element. If true is returned from the predicate, stop the iteration.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			Value returned from predicate
			 */
			bool                forEach(filter_predicate p) noexcept {
				for (iter_type it = mData.begin(); it != mData.end(); ++it) {
					if (p(*it)) {
						return true;
					}
				}
				return false;
			}

			/**
			 * Iterate over each const element. If true is returned from the predicate, stop the iteration.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			Value returned from predicate
			 */
			bool                forEach(cfilter_predicate p) const noexcept {
				for (citer_type it = mData.begin(); it != mData.end(); ++it) {
					if (p(*it)) {
						return true;
					}
				}
				return false;
			}

			/**
			 * Find a specific element by iterating over all elements and calling the predicate.
			 * If the predicate returns true, the element at the corresponding position is returned.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			The element found or null
			 */
			const Value         *find(cfilter_predicate p) const noexcept {
				for (citer_type it = mData.begin(); it != mData.end(); ++it) {
					if (p(*it)) {
						return &*it;
					}
				}
				return nullptr;
			}


			/**
			 * Find a specific element by iterating over all elements and calling the predicate.
			 * If the predicate returns true, the element at the corresponding position is returned.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			The element found or null
			 */
			const Value         *find(cfilter_predicate_with_id p) const noexcept {
				size_t id = 0;
				for (citer_type it = mData.begin(); it != mData.end(); ++it, ++id) {
					if (p(id, *it)) {
						return &*it;
					}
				}
				return nullptr;
			}


			/**
			 * Find a specific element by iterating over all elements and calling the predicate.
			 * If the predicate returns true, the element at the corresponding position is returned.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			The element found or null
			 */
			Value               *find(cfilter_predicate p) noexcept {
				for (iter_type it = mData.begin(); it != mData.end(); ++it) {
					if (p(*it)) {
						return &*it;
					}
				}
				return nullptr;
			}


			/**
			 * Find a specific element by iterating over all elements and calling the predicate.
			 * If the predicate returns true, the element at the corresponding position is returned.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			The element found or null
			 */
			Value               *find(cfilter_predicate_with_id p) noexcept {
				size_t id = 0;
				for (iter_type it = mData.begin(); it != mData.end(); ++it, ++id) {
					if (p(id, *it)) {
						return &*it;
					}
				}
				return nullptr;
			}

			/**
			 * Reverse-find a specific element by iterating over all elements (starting from the end)
			 * and calling the predicate. If the predicate returns true, the element at the corresponding
			 * position is returned.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			The element found or null
			 */
			const Value         *rfind(cfilter_predicate p) const noexcept {
				for (criter_type it = mData.rbegin(); it != mData.rend(); ++it) {
					if (p(*it)) {
						return &*it;
					}
				}
				return nullptr;
			}

			/**
			 * Reverse-find a specific element by iterating over all elements (starting from the end)
			 * and calling the predicate. If the predicate returns true, the element at the corresponding
			 * position is returned.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			The element found or null
			 */
			const Value         *rfind(cfilter_predicate_with_id p) const noexcept {
				size_t id = 0;
				for (criter_type it = mData.rbegin(); it != mData.rend(); ++it, ++id) {
					if (p(id, *it)) {
						return &*it;
					}
				}
				return nullptr;
			}

			/**
			 * Reverse-find a specific element by iterating over all elements (starting from the end)
			 * and calling the predicate. If the predicate returns true, the element at the corresponding
			 * position is returned.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			The element found or null
			 */
			Value               *rfind(cfilter_predicate p) noexcept {
				for (riter_type it = mData.rbegin(); it != mData.rend(); ++it) {
					if (p(*it)) {
						return &*it;
					}
				}
				return nullptr;
			}

			/**
			 * Reverse-find a specific element by iterating over all elements (starting from the end)
			 * and calling the predicate. If the predicate returns true, the element at the corresponding
			 * position is returned.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			The element found or null
			 */
			Value               *rfind(cfilter_predicate_with_id p) noexcept {
				size_t id = 0;
				for (riter_type it = mData.rbegin(); it != mData.rend(); ++it, ++id) {
					if (p(id, *it)) {
						return *it;
					}
				}
				return nullptr;
			}

			/**
			 * Aggregate all elements matching the predicate by.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			A new basicCollection representing this aggregation
			 */
			BasicCollection          filter(cfilter_predicate p) const noexcept {
				BasicCollection ret;
				for (citer_type it = mData.begin(); it != mData.end(); ++it) {
					if (p(*it)) {
						ret.mData.push_back(*it);
					}
				}
				return ret;
			}

			/**
			 * Aggregate all elements matching the predicate by.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			A new basicCollection representing this aggregation
			 */
			BasicCollection          filter(cfilter_predicate_with_id p) const noexcept {
				BasicCollection      ret;
				size_t          id = 0;
				for (citer_type it = mData.begin(); it != mData.end(); ++it, ++id) {
					if (p(id, *it)) {
						ret.mData.push_back(*it);
					}
				}
				return ret;
			}


			/**
			 * Aggregate all elements matching the predicate by, starting  at the end.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			A new basicCollection representing this aggregation
			 */
			BasicCollection          rfilter(cfilter_predicate p) const noexcept {
				BasicCollection ret;
				for (criter_type it = mData.rbegin(); it != mData.rend(); ++it) {
					if (p(*it)) {
						ret.mData.push_back(*it);
					}
				}
				return ret;
			}

			/**
			 * Aggregate all elements matching the predicate by, starting at the end.
			 *
			 * @param p			Predicate called on each element.
			 *
			 * @return			A new basicCollection representing this aggregation
			 */
			BasicCollection          rfilter(cfilter_predicate_with_id p) const noexcept {
				BasicCollection      ret;
				size_t          id = mData.size() - 1;
				for (criter_type it = mData.rbegin(); it != mData.rend(); ++it, --id) {
					if (p(id, *it)) {
						ret.mData.push_back(*it);
					}
				}
				return ret;
			}

			/**
			 * Apply a transformation to each element.
			 *
			 * @param mapper	Transformation function called on each element.
			 *
			 * @return			A new basicCollection representing this transformation
			 */
			BasicCollection          map(map_fn mapper) const noexcept {
				BasicCollection      ret;
				for (citer_type it = mData.begin(); it != mData.end(); ++it) {
					ret.mData.push_back(mapper(*it));
				}
				return ret;
			}

			/**
			 * Apply a transformation to each element.
			 *
			 * @param mapper	Transformation function called on each element.
			 *
			 * @return			A new basicCollection representing this transformation
			 */
			BasicCollection          map(map_fn_with_id mapper) const noexcept {
				BasicCollection      ret;
				size_t          id = 0;
				for (citer_type it = mData.begin(); it != mData.end(); ++it, ++id) {
					ret.mData.push_back(mapper(id, *it));
				}
				return ret;
			}

			/**
			 * Apply a transformation to each element starting from the end.
			 *
			 * @param mapper	Transformation function called on each element.
			 *
			 * @return			A new basicCollection representing this transformation
			 */
			BasicCollection          rmap(map_fn mapper) const noexcept {
				BasicCollection      ret;
				for (criter_type it = mData.rbegin(); it != mData.rend(); ++it) {
					ret.mData.push_back(mapper(*it));
				}
				return ret;
			}

			/**
			 * Apply a transformation to each element starting from the end.
			 *
			 * @param mapper	Transformation function called on each element.
			 *
			 * @return			A new basicCollection representing this transformation
			 */
			BasicCollection          rmap(map_fn_with_id mapper) const noexcept {
				BasicCollection      ret;
				size_t id = mData.size() - 1;
				for (criter_type it = mData.rbegin(); it != mData.rend(); ++it, --id) {
					ret.mData.push_back(mapper(id, *it));
				}
				return ret;
			}

			/**
			 * Reduce this basicCollection to a simple value.
			 *
			 * @param r			Transformation function called on each element.
			 *
			 * @return			A value representing this transformation
			 */
			template<typename Carry = value_type>
			Carry               fold(fold_fn<Carry> r, Carry c = Carry()) {
				for (citer_type it = mData.begin(); it != mData.end(); ++it) {
					c = r(c, *it);
				}
				return c;
			}

			/**
			 * Reduce this basicCollection to a simple value.
			 *
			 * @param r			Transformation function called on each element.
			 *
			 * @return			A value representing this transformation
			 */
			template<typename Carry = value_type>
			Carry               fold(fold_fn_with_id<Carry> r, Carry c = Carry()) {
				size_t id = 0;
				for (citer_type it = mData.begin(); it != mData.end(); ++it, ++id) {
					c = r(id, c, *it);
				}
				return c;
			}

			/**
			 * Reduce this basicCollection to a simple value starting at the end.
			 *
			 * @param r			Transformation function called on each element.
			 *
			 * @return			A value representing this transformation
			 */
			template<typename Carry = value_type>
			Carry               rfold(fold_fn<Carry> r, Carry c = Carry()) {
				for (criter_type it = mData.rbegin(); it != mData.rend(); ++it) {
					c = r(c, *it);
				}
				return c;
			}

			/**
			 * Reduce this basicCollection to a simple value starting at the end.
			 *
			 * @param r			Transformation function called on each element.
			 *
			 * @return			A value representing this transformation
			 */
			template<typename Carry = value_type>
			Carry               rfold(fold_fn_with_id<Carry> r, Carry c = Carry()) {
				size_t id = mData.size() - 1;
				for (criter_type it = mData.rbegin(); it != mData.rend(); ++it, --id) {
					c = r(id, c, *it);
				}
				return c;
			}

//
//			self_type           take(size_t n) {
//				self_type       ret;
//				while (n--) {
//					ret.mData.push_back(mData.front());
//					mData.erase(mData.begin());
//				}
//				return ret;
//			}

			value_type          remove(const filter_predicate &p) {
				auto it = iter(p);
				if (it != mData.end()) {
					mData.erase(it);
				}
				return *it;
			}

			value_type          remove(const filter_predicate_with_id &p) {
				auto it = iter(p);
				if (it != mData.end()) {
					mData.erase(it);
				}
				return *it;
			}
		};

		template<typename Value, typename Container = std::vector<Value>>
		class Collection: public BasicCollection<Value, std::vector<Value>> {
		public:
			using container_type = std::vector<Value>;
			using base_type = BasicCollection<Value, container_type>;
			using self_type = Collection<Value, container_type>;
			using value_type = typename base_type::value_type;

			Collection() = default;
			explicit Collection(const container_type &data): base_type(data) {}
			Collection(const std::initializer_list<Value> &data): base_type(data) {}
			Collection(const Collection &rhs) = default;
			virtual ~Collection() = default;

			Collection          &operator=(const Collection &rhs) = default;

			value_type          &operator[](size_t pos) {
				return base_type::mData[pos];
			}
			const value_type    &operator[](size_t pos) const {
				return base_type::mData[pos];
			}

			value_type          &at(size_t pos) {
				return base_type::mData.at(pos);
			}
			const value_type    &at(size_t pos) const {
				return base_type::mData.at(pos);
			}

			self_type           take(size_t n) {
				self_type       ret;
				while (n--) {
					ret.mData.push_back(base_type::mData.front());
					base_type::mData.erase(base_type::mData.begin());
				}
				return ret;
			}

			virtual void        add(value_type &&v) { base_type::mData.push_back(v); }
			virtual void        add(const value_type &v) { base_type::mData.push_back(v); }
		};

		template<typename Value>
		class Collection<Value, std::map<typename Value::first_type, typename Value::second_type>>
			: public BasicCollection<Value, std::map<typename Value::first_type, typename Value::second_type>> {
		public:
			using key_type = typename Value::first_type;
			using value_type = typename Value::second_type;
			using item_type = std::pair<typename Value::first_type, typename Value::second_type>;
			using container_type = std::map<key_type, value_type>;

			using base_type = BasicCollection<item_type, container_type>;
			using self_type = Collection<item_type, container_type>;

			Collection() = default;
			explicit Collection(const container_type &data): base_type(data) {}
			Collection(std::initializer_list<std::pair<const key_type, value_type>> data): base_type(data) {}
			Collection(const Collection &rhs) = default;
			virtual ~Collection() = default;

			Collection          &operator=(const Collection &rhs) = default;

			value_type          &operator[](const key_type pos) {
				return base_type::mData[pos];
			}

			const value_type    &operator[](const key_type pos) const {
				return base_type::mData[pos];
			}

			value_type          &at(const key_type pos) {
				return base_type::mData.at(pos);
			}

			const value_type    &at(const key_type pos) const {
				return base_type::mData.at(pos);
			}

			template<template<class, class> class Container = std::vector>
			self_type           take(const Container<key_type, std::allocator<key_type>> &keys) {
				self_type       ret;
				for (auto it = keys.begin(); it != keys.end(); it++) {
					auto found = base_type::mData.find(*it);
					ret.put(found->first, found->second);
					base_type::mData.erase(found);
				}
				return ret;
			}

			self_type           take(size_t n) {
				self_type       ret;
				while (n--) {
					ret.mData.insert(*base_type::mData.begin());
					base_type::mData.erase(base_type::mData.begin());
				}
				return ret;
			}

			virtual void        add(item_type &&v) {
				auto it = base_type::mData.insert(v);
				if (!it.second) {
					throw std::runtime_error("Failed to insert pair!");
				}
			}
			virtual void        add(const item_type &v) {
				auto it = base_type::mData.insert(v);
				if (!it.second) {
					throw std::runtime_error("Failed to insert pair!");
				}
			}

			virtual self_type   &put(const key_type &k, const value_type &v) {
				add(std::make_pair(k, v));
				return *this;
			}

			virtual self_type   &put(const key_type &k, value_type &&v) {
				add(std::make_pair(k, v));
				return *this;
			}

		};

		template<typename V>
		using Vector = Collection<V, std::vector<V>>;

		template<typename V>
		using List = Collection<V, std::list<V>>;

		template<typename V>
		using Queue = Collection<V, std::queue<V>>;

		template<typename V>
		using Deque = Collection<V, std::deque<V>>;

		template<typename V>
		using Stack = Collection<V, std::stack<V>>;

		template<typename K, typename V>
		using Map = Collection<std::pair<K, V>, std::map<K, V>>;
	}
}

#endif //QUASARFX_COLLECTION_H
