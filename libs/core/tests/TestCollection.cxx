//
// Created by darkboss on 8/1/20.
//

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <core/Collection.h>
#include <core/String.h>
#include "doctest.h"

using quasar::core::Collection;
using quasar::core::Map;
using quasar::core::String;
using quasar::core::BasicString;

TEST_CASE("Collection iter works") {
	auto collec = Collection<int>({1, 3, 6});
	REQUIRE(collec.iter(1) == collec->begin());
	REQUIRE(collec.iter(6) == --collec->end());
}

TEST_CASE("Collection riter works") {
	auto collec = Collection<int>({1, 3, 6});
	REQUIRE(collec.riter(1) == --collec->rend());
	REQUIRE(collec.riter(6) == collec->rbegin());
}

TEST_CASE("Collection<vector>'s take works") {
	auto collec = Collection<int>({1, 3, 6});
	auto filtered = collec.take(2);
	REQUIRE(collec.size() == 1);
	REQUIRE(filtered.size() == 2);
	REQUIRE(collec->at(0) == 6);
	REQUIRE(filtered->at(0) == 1);
}

TEST_CASE("Collection<map>'s take(keys) works") {
	auto collec = Map<String, int>({{"key1", 1}, {"key2", 3}, {"key3", 6}});
	auto filtered = collec.take(std::vector<String>{"key1", "key3"});
	REQUIRE(collec.size() == 1);
	REQUIRE(filtered.size() == 2);
	REQUIRE(collec->at("key2") == 3);
	REQUIRE(filtered->at("key1") == 1);
	REQUIRE(filtered->at("key3") == 6);
}

TEST_CASE("Collection<vector> can be searched") {
	auto collec = Collection<int>({1, 3, 6});
	auto found = collec.find([](const int &i) {
		return i == 3;
	});
	REQUIRE(found != collec.end());
	REQUIRE(*found == 3);
}

TEST_CASE("Collection<vector> can be searched (reverse)") {
	auto const collec = Collection<int>({1, 3, 6});
	int num_iters = 0;
	auto found = collec.rfind([&num_iters](const int &i) {
		num_iters++;
		return i == 6;
	});
	REQUIRE(found != collec.rend());
	REQUIRE(*found == 6);
	REQUIRE(num_iters == 1);
}

TEST_CASE("Collection<vector> can be filtered") {
	auto collec = Collection<int>({1, 3, 6});
	auto filtered = collec.filter([](const int &i) {
		return i == 1 || i == 6;
	});
	REQUIRE(collec->size() == 3); // old collection untouched
	REQUIRE(collec->at(0) == 1);
	REQUIRE(collec->at(1) == 3);
	REQUIRE(collec->at(2) == 6);

	REQUIRE(filtered->size() == 2); // new collection filtered
	REQUIRE(filtered->at(0) == 1);
	REQUIRE(filtered->at(1) == 6);
}

TEST_CASE("Collection<vector> can be filtered with ids") {
	auto collec = Collection<int>({1, 3, 6});
	auto filtered = collec.filter([](size_t id, const int &i) {
		return i == 1 || i == 6;
	});
	REQUIRE(collec->size() == 3); // old collection untouched
	REQUIRE(collec->at(0) == 1);
	REQUIRE(collec->at(1) == 3);
	REQUIRE(collec->at(2) == 6);

	REQUIRE(filtered->size() == 2); // new collection filtered
	REQUIRE(filtered->at(0) == 1);
	REQUIRE(filtered->at(1) == 6);
}

TEST_CASE("Collection<vector> can be mapped") {
	auto collec = Collection<int>({1, 3, 6});
	auto mapped = collec.map([](const int &i) {
		return i * 2;
	});
	REQUIRE(collec->size() == 3); // old collection untouched
	REQUIRE(collec->at(0) == 1);
	REQUIRE(collec->at(1) == 3);
	REQUIRE(collec->at(2) == 6);

	REQUIRE(mapped->size() == 3); // new collection mapped
	REQUIRE(mapped->at(0) == 2);
	REQUIRE(mapped->at(1) == 6);
	REQUIRE(mapped->at(2) == 12);
}

TEST_CASE("Collection<vector> can be mapped with id") {
	auto collec = Collection<int>({1, 3, 6});
	auto mapped = collec.map([](size_t id, const int &i) {
		return (int)id;
	});

	REQUIRE(collec->size() == 3); // old collection untouched
	REQUIRE(collec->at(0) == 1);
	REQUIRE(collec->at(1) == 3);
	REQUIRE(collec->at(2) == 6);

	REQUIRE(mapped->size() == 3); // new collection mapped
	REQUIRE(mapped->at(0) == 0);
	REQUIRE(mapped->at(1) == 1);
	REQUIRE(mapped->at(2) == 2);
}

TEST_CASE("Collection<vector> can be reduced") {
	auto collec = Collection<int>({1, 3, 6});
	auto reduced = collec.fold<int>([](int carry, const int &val) {
		return carry + val;
	});
	REQUIRE(reduced == 10);
}

TEST_CASE("Collection<vector> can be reduced with id") {
	auto collec = Collection<int>({1, 3, 6});
	auto reduced = collec.fold<int>([](size_t id, const int &carry, const int &val) {
		if (id == 1) {
			return carry + val;
		}
		return carry;
	});
	REQUIRE(reduced == 3);
}

TEST_CASE("Collection<vector> can be stringified when small") {
	auto collec = Collection<int>({3, 2, 1});
	std::stringstream buf;
	buf << collec;
	REQUIRE(buf.str() == "[3, 2, 1]");
}

TEST_CASE("Collection<map> can be stringified when small") {
	auto collec = Map<String, int>({{"a", 3}, {"b", 2}, {"c", 1}});
	std::stringstream buf;
	buf << collec;
	REQUIRE(buf.str() == "{a: 3, b: 2, c: 1}");
}

TEST_CASE("Collection<vector> can be stringified when large") {
	auto collec = Collection<int>({3, 2, 1, 4, 5, 6, 7});
	std::stringstream buf;
	buf << collec;
	REQUIRE(buf.str() == R"([
	3,
	2,
	1,
	4,
	5,
	6,
	7
])");
}

TEST_CASE("Collection<map> can be stringified when large") {
	auto collec = Map<String, int>({{"a", 3}, {"b", 2}, {"c", 1}, {"d", 4}, {"e", 5}, {"f", 6}, {"g", 7}});
	std::stringstream buf;
	buf << collec;
	REQUIRE(buf.str() == R"({
	a: 3,
	b: 2,
	c: 1,
	d: 4,
	e: 5,
	f: 6,
	g: 7
})");
}


TEST_CASE("Collection<vector> can be stringified when small on wchar streams") {
	auto collec = Collection<int>({3, 2, 1});
	std::wstringstream buf;
	buf << collec;
	REQUIRE(buf.str() == L"[3, 2, 1]");
}

TEST_CASE("Collection<map> can be stringified when small on wchar streams") {
	auto collec = Map<BasicString<wchar_t>, int>({{L"a", 3}, {L"b", 2}, {L"c", 1}});
	std::wstringstream buf;
	buf << collec;
	REQUIRE(buf.str() == L"{a: 3, b: 2, c: 1}");
}
