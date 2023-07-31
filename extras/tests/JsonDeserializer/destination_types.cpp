// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#include <ArduinoJson.h>

#include <catch.hpp>
#include <string>

#include "Allocators.hpp"

using ArduinoJson::detail::sizeofArray;
using ArduinoJson::detail::sizeofObject;

TEST_CASE("deserializeJson(JsonDocument&)") {
  SpyingAllocator spy;
  JsonDocument doc(&spy);
  doc.add(std::string("hello"));
  spy.clearLog();

  auto err = deserializeJson(doc, "[42]");

  REQUIRE(err == DeserializationError::Ok);
  REQUIRE(doc.as<std::string>() == "[42]");
  REQUIRE(spy.log() == AllocatorLog{
                           Deallocate(sizeofPool()),
                           Deallocate(sizeofString("hello")),
                           Allocate(sizeofPool()),
                           Reallocate(sizeofPool(), sizeofArray(1)),
                       });
}

TEST_CASE("deserializeJson(JsonVariant)") {
  SECTION("variant is bound") {
    SpyingAllocator spy;
    JsonDocument doc(&spy);
    doc.add(std::string("hello"));
    spy.clearLog();

    JsonVariant variant = doc[0];

    auto err = deserializeJson(variant, "[42]");

    REQUIRE(err == DeserializationError::Ok);
    REQUIRE(doc.as<std::string>() == "[[42]]");
    REQUIRE(spy.log() ==
            AllocatorLog{
                Deallocate(sizeofString("hello")),
                Reallocate(sizeofPool(), sizeofArray(1) + sizeofArray(1)),
            });
  }

  SECTION("variant is unbound") {
    JsonVariant variant;

    auto err = deserializeJson(variant, "[42]");

    REQUIRE(err == DeserializationError::NoMemory);
  }
}

TEST_CASE("deserializeJson(ElementProxy)") {
  SpyingAllocator spy;
  JsonDocument doc(&spy);
  doc.add(std::string("hello"));
  spy.clearLog();

  SECTION("element already exists") {
    auto err = deserializeJson(doc[0], "[42]");

    REQUIRE(err == DeserializationError::Ok);
    REQUIRE(doc.as<std::string>() == "[[42]]");
    REQUIRE(spy.log() ==
            AllocatorLog{
                Deallocate(sizeofString("hello")),
                Reallocate(sizeofPool(), sizeofArray(1) + sizeofArray(1)),
            });
  }

  SECTION("element must be created") {
    auto err = deserializeJson(doc[1], "[42]");

    REQUIRE(err == DeserializationError::Ok);
    REQUIRE(doc.as<std::string>() == "[\"hello\",[42]]");
    REQUIRE(spy.log() ==
            AllocatorLog{
                Reallocate(sizeofPool(), sizeofArray(2) + sizeofArray(1)),
            });
  }
}

TEST_CASE("deserializeJson(MemberProxy)") {
  SpyingAllocator spy;
  JsonDocument doc(&spy);
  doc[std::string("hello")] = std::string("world");
  spy.clearLog();

  SECTION("member already exists") {
    auto err = deserializeJson(doc["hello"], "[42]");

    REQUIRE(err == DeserializationError::Ok);
    REQUIRE(doc.as<std::string>() == "{\"hello\":[42]}");
    REQUIRE(spy.log() ==
            AllocatorLog{
                Deallocate(sizeofString("world")),
                Reallocate(sizeofPool(), sizeofObject(1) + sizeofArray(1)),
            });
  }

  SECTION("member must be created exists") {
    auto err = deserializeJson(doc["value"], "[42]");

    REQUIRE(err == DeserializationError::Ok);
    REQUIRE(doc.as<std::string>() == "{\"hello\":\"world\",\"value\":[42]}");
    REQUIRE(spy.log() ==
            AllocatorLog{
                Reallocate(sizeofPool(), sizeofObject(2) + sizeofArray(1)),
            });
  }
}
