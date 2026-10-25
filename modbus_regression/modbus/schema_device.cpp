#include "fixture.h"
#include <cassert>
#include <iostream>
using namespace modbus_gateway;
enum class Id { a, b, last };
constexpr auto a = field<DataType::uint16>(Id::a, Scaling::none, WordOrder::HighFirst, "a", "", 0);
constexpr auto b = field<DataType::int32>(Id::b, Scaling::ten, WordOrder::LowFirst, "b", "W", 0);
constexpr auto good = makeDeviceSchema<Id>(block("x", 65533, a, b));
static_assert(good.valid);
static_assert(good.registers[1].address == 65534);
constexpr auto duplicate = makeDeviceSchema<Id>(block("x", 0, a, a));
static_assert(!duplicate.valid && duplicate.error == SchemaError::duplicate_id);
constexpr auto missing = makeDeviceSchema<Id>(block("x", 0, a));
static_assert(!missing.valid && missing.error == SchemaError::missing_id);
constexpr auto invalid_id = makeDeviceSchema<Id>(block("x", 0,
    field<DataType::uint16>(Id::last, Scaling::none, WordOrder::HighFirst, "bad", "", 0), b));
static_assert(!invalid_id.valid && invalid_id.error == SchemaError::invalid_id);
constexpr auto overlap = makeDeviceSchema<Id>(block("x", 0, a), block("y", 0, b));
static_assert(!overlap.valid && overlap.error == SchemaError::overlapping_blocks);
constexpr auto overflow = makeDeviceSchema<Id>(block("x", 65534, a, b));
static_assert(!overflow.valid && overflow.error == SchemaError::address_overflow);
constexpr auto truncated = makeDeviceSchema<Id>(block("x", 65536, a, b));
static_assert(!truncated.valid && truncated.error == SchemaError::address_overflow);
constexpr auto invalid_scale = makeDeviceSchema<Id>(block("x", 0,
    field<DataType::uint16>(Id::a, static_cast<Scaling>(0), WordOrder::HighFirst, "bad", "", 0), b));
static_assert(!invalid_scale.valid && invalid_scale.error == SchemaError::invalid_field);
constexpr auto invalid_order = makeDeviceSchema<Id>(block("x", 0,
    field<DataType::uint16>(Id::a, Scaling::none, static_cast<WordOrder>(9), "bad", "", 0), b));
static_assert(!invalid_order.valid && invalid_order.error == SchemaError::invalid_field);
constexpr auto invalid_type = makeDeviceSchema<Id>(block("x", 0,
    field<static_cast<DataType>(99)>(Id::a, Scaling::none, WordOrder::HighFirst, "bad", "", 0), b));
static_assert(!invalid_type.valid && invalid_type.error == SchemaError::invalid_field);
// A declared extent that cannot be represented must not be silently narrowed.
struct WideField : TypedField<Id, DataType::uint16> { static constexpr std::size_t word_count = 65536; };
constexpr WideField wide{{Id::a, Scaling::none, WordOrder::HighFirst, "wide", "", 0}};
static_assert(!makeDeviceSchema<Id>(block("x", 0, wide, b)).valid);

struct Reordered : SchemaApi<Reordered> {
    enum RegisterId { low, high, last };
    inline static constexpr auto metadata = makeDeviceSchema<RegisterId>(
        block("high", 11, field<DataType::uint16>(high, Scaling::none, WordOrder::HighFirst, "high", "", 2)),
        block("low", 10, field<DataType::uint16>(low, Scaling::none, WordOrder::HighFirst, "low", "", 1)));
};
int main() {
    Device<TestSchema> device;
    device.set<TestSchema::value>(-14795);
    assert(device.allValuesAsString() ==
           "Block first\r\n  value=-1479.5 W\r\n"
           "Block next\r\n  adjacent=7\r\n"
           "Block last\r\n  distant=9\r\n");
    const auto& descriptor = TestSchema::description<TestSchema::value>();
    assert(decode<int32_t>(device.rawWords(), descriptor) == -14795);
    assert(decode<float>(device.rawWords(), descriptor) == -1479.5f);
    uint16_t encoded[2]{};
    encode<int32_t>(encoded, descriptor, -14795);
    assert(encoded[0] == 0xc635 && encoded[1] == 0xffff);
    uint16_t words[4] = {0xaaaa, 0xbbbb, 0xcccc, 0xdddd};
    assert(device.readHoldingRegisters(101, 2, words)); // half field and adjacent block
    assert(words[0] == 0xffff && words[1] == 7);
    const uint16_t update[] = {11, 12, 13, 14};
    assert(!device.writeHoldingRegisters(102, 4, update)); // gap after first valid word
    assert(device.get<TestSchema::adjacent>() == 7 && device.get<TestSchema::distant>() == 9);
    words[0] = 0xaaaa; words[1] = 0xbbbb;
    assert(!device.readHoldingRegisters(102, 2, words));
    assert(words[0] == 0xaaaa && words[1] == 0xbbbb);
    assert(!device.readHoldingRegisters(100, 0, words));
    assert(!device.readHoldingRegisters(100, 1, nullptr));
    assert(!device.writeHoldingRegisters(100, 1, nullptr));
    assert(!device.writeHoldingRegisters(65535, 2, update));
    Device<Reordered> reordered;
    assert(reordered.readHoldingRegisters(10, 2, words));
    assert(words[0] == 1 && words[1] == 2);
    assert(reordered.writeHoldingRegisters(10, 2, update));
    assert(reordered.get<Reordered::low>() == 11 && reordered.get<Reordered::high>() == 12);
    std::cout << "PASS: negative schemas, boundary addresses, partial fields and atomic ranges\n";
}
