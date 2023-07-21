// ArduinoJson - https://arduinojson.org
// Copyright © 2014-2023, Benoit BLANCHON
// MIT License

#include <ArduinoJson/Memory/ResourceManager.hpp>
#include <ArduinoJson/Memory/VariantPoolImpl.hpp>
#include <catch.hpp>

#include "Allocators.hpp"

using namespace ArduinoJson::detail;

TEST_CASE("ResourceManager::shrinkToFit()") {
  TimebombAllocator allocator(100);
  SpyingAllocator spyingAllocator(&allocator);
  ResourceManager resources(&spyingAllocator);

  SECTION("empty") {
    resources.shrinkToFit();

    REQUIRE(spyingAllocator.log() == AllocatorLog());
  }

  SECTION("only one pool") {
    resources.allocSlot();

    resources.shrinkToFit();

    REQUIRE(spyingAllocator.log() ==
            AllocatorLog() << AllocatorLog::Allocate(sizeofPool())
                           << AllocatorLog::Reallocate(sizeofPool(),
                                                       sizeof(VariantSlot)));
  }

  SECTION("more pools than initial count") {
    for (size_t i = 0;
         i < ARDUINOJSON_POOL_CAPACITY * ARDUINOJSON_INITIAL_POOL_COUNT + 1;
         i++)
      resources.allocSlot();
    REQUIRE(spyingAllocator.log() ==
            AllocatorLog() << AllocatorLog::Allocate(sizeofPool()) *
                                  ARDUINOJSON_INITIAL_POOL_COUNT
                           << AllocatorLog::Allocate(sizeofPoolList(
                                  ARDUINOJSON_INITIAL_POOL_COUNT * 2))
                           << AllocatorLog::Allocate(sizeofPool()));

    spyingAllocator.clearLog();
    resources.shrinkToFit();

    REQUIRE(spyingAllocator.log() ==
            AllocatorLog()
                << AllocatorLog::Reallocate(sizeofPool(), sizeof(VariantSlot))
                << AllocatorLog::Reallocate(
                       sizeofPoolList(ARDUINOJSON_INITIAL_POOL_COUNT * 2),
                       sizeofPoolList(ARDUINOJSON_INITIAL_POOL_COUNT + 1)));
  }
}
