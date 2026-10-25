#pragma once
#include "modbus/definitions.h"
namespace modbus_gateway {
struct TestSchema : SchemaApi<TestSchema> {
    enum RegisterId { value, adjacent, distant, last };
    static constexpr const char* name = "test";
    inline static constexpr auto metadata = makeDeviceSchema<RegisterId>(
        block("first", 100, field<DataType::int32>(value, Scaling::ten, WordOrder::LowFirst, "value", "W", 0)),
        block("next", 102, field<DataType::uint16>(adjacent, Scaling::none, WordOrder::HighFirst, "adjacent", "", 7)),
        block("last", 105, field<DataType::uint16>(distant, Scaling::none, WordOrder::HighFirst, "distant", "", 9)));
    static DeviceType makeDevice() { return {}; }
};
}
