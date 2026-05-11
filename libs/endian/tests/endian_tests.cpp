#include <obz/endian.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

TEST_CASE("endian writes and reads little-endian integers") {
    std::array<std::byte, 4> buffer{};

    obz::endian::write_le<std::uint32_t>(buffer, 0, 0x12345678u);

    REQUIRE(std::to_integer<std::uint8_t>(buffer[0]) == 0x78);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[1]) == 0x56);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[2]) == 0x34);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[3]) == 0x12);

    REQUIRE(obz::endian::read_le<std::uint32_t>(buffer, 0) == 0x12345678u);
}

TEST_CASE("endian writes and reads big-endian integers") {
    std::array<std::byte, 4> buffer{};

    obz::endian::write_be<std::uint32_t>(buffer, 0, 0x12345678u);

    REQUIRE(std::to_integer<std::uint8_t>(buffer[0]) == 0x12);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[1]) == 0x34);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[2]) == 0x56);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[3]) == 0x78);

    REQUIRE(obz::endian::read_be<std::uint32_t>(buffer, 0) == 0x12345678u);
}

TEST_CASE("endian supports offsets") {
    std::array<std::byte, 6> buffer{};

    obz::endian::write_be<std::uint16_t>(buffer, 2, 0xabcd);

    REQUIRE(std::to_integer<std::uint8_t>(buffer[0]) == 0x00);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[1]) == 0x00);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[2]) == 0xab);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[3]) == 0xcd);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[4]) == 0x00);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[5]) == 0x00);

    REQUIRE(obz::endian::read_be<std::uint16_t>(buffer, 2) == 0xabcd);
}

TEST_CASE("endian supports single-byte integer values") {
    std::array<std::byte, 1> buffer{};

    obz::endian::write_be<std::uint8_t>(buffer, 0, 0xab);

    REQUIRE(std::to_integer<std::uint8_t>(buffer[0]) == 0xab);
    REQUIRE(obz::endian::read_le<std::uint8_t>(buffer, 0) == 0xab);
    REQUIRE(obz::endian::read_be<std::uint8_t>(buffer, 0) == 0xab);
}

TEST_CASE("endian writes and reads 64-bit integers") {
    std::array<std::byte, 8> buffer{};

    obz::endian::write_be<std::uint64_t>(buffer, 0, 0x0123456789abcdefULL);

    REQUIRE(std::to_integer<std::uint8_t>(buffer[0]) == 0x01);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[1]) == 0x23);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[2]) == 0x45);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[3]) == 0x67);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[4]) == 0x89);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[5]) == 0xab);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[6]) == 0xcd);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[7]) == 0xef);

    REQUIRE(obz::endian::read_be<std::uint64_t>(buffer, 0) == 0x0123456789abcdefULL);
}

TEST_CASE("endian preserves signed integer byte representation") {
    std::array<std::byte, 2> buffer{};

    obz::endian::write_le<std::int16_t>(buffer, 0, -2);

    REQUIRE(std::to_integer<std::uint8_t>(buffer[0]) == 0xfe);
    REQUIRE(std::to_integer<std::uint8_t>(buffer[1]) == 0xff);
    REQUIRE(obz::endian::read_le<std::int16_t>(buffer, 0) == -2);
}

TEST_CASE("endian throws when operations exceed buffer bounds") {
    std::array<std::byte, 2> buffer{};

    REQUIRE_THROWS_AS(obz::endian::write_le<std::uint32_t>(buffer, 0, 1u), std::out_of_range);
    REQUIRE_THROWS_AS(obz::endian::read_be<std::uint16_t>(buffer, 2), std::out_of_range);
}
