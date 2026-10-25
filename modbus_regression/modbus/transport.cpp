#include "modbus/client.h"
#include "modbus/server.h"
#include "fixture.h"
#include <cassert>
#include <iostream>
using namespace modbus_gateway;

void expect(const ModbusMessage& actual, std::initializer_list<uint8_t> bytes) {
    assert(actual.bytes == std::vector<uint8_t>(bytes));
}
int main() {
    ModbusServer transport;
    {
        Server<TestSchema> first(transport, 1);
        Server<TestSchema> second(transport, 2);
        assert(first.registered() && second.registered());
        {
            Server<TestSchema> duplicate(transport, 1);
            assert(!duplicate.registered());
        }
        expect(transport.request({1,3,0,100,0,3}), {1,3,6,0,0,0,0,0,7});
        expect(transport.request({1,6,0,100,0x12,0x34}), {1,6,0,100,0x12,0x34});
        expect(transport.request({2,3,0,100,0,1}), {2,3,2,0,0}); // instances isolated
        expect(transport.request({1,16,0,100,0,2,4,0xff,0xff,0xff,0xff}), {1,16,0,100,0,2});
        { DataAccess<Server<TestSchema>> access(first); assert(access.get<TestSchema::value>() == -1); }
        expect(transport.request({1,3,0,100,0,2}), {1,3,4,0xff,0xff,0xff,0xff});
        expect(transport.request({1,3,0,100,0,0}), {1,0x83,3});
        expect(transport.request({1,3,0,100,0,126}), {1,0x83,3});
        expect(transport.request({1,3,0,100}), {1,0x83,3});
        expect(transport.request({1,3,0,100,0,1,0}), {1,0x83,3});
        expect(transport.request({1,6,0,100,0}), {1,0x86,3});
        expect(transport.request({1,6,0,100,0,1,0}), {1,0x86,3});
        expect(transport.request({1,16,0,100,0,2,2,0,0}), {1,0x90,3});
        expect(transport.request({1,16,0,100,0,124,0}), {1,0x90,3});
        expect(transport.request({1,16,0,102,0,2,4,0x12,0x34,0x56,0x78}), {1,0x90,2});
        expect(transport.request({1,3,0,102,0,1}), {1,3,2,0,7}); // rejected write did not touch first word
        expect(transport.request({1,3,0xff,0xff,0,2}), {1,0x83,2});
    }
    assert(transport.workers.empty());
    Server<TestSchema> replacement(transport, 1);
    assert(replacement.registered());

    ModbusClientTCP detached;
    { Client<TestSchema> temporary(detached, IPAddress{}, 502, 1); }
    assert(!detached.data_handler && !detached.error_handler);

    ModbusClientTCP tcp;
    Client<TestSchema> client(tcp, IPAddress{}, 502, 1);
    client.connect();
    assert(!client.consumeUpdated());
    assert(client.readFromMeter(TestSchema::value, TestSchema::value));
    assert(tcp.pending.front().address == 100 && tcp.pending.front().count == 2);
    const auto old_token = tcp.pending.front().token;
    tcp.respond({1,3,4,0xc6,0x35,0xff,0xff}); // -14795, low first
    assert(client.consumeUpdated() && !client.consumeUpdated());
    { DataAccess<Client<TestSchema>> access(client); assert(access.get<TestSchema::value>() == -14795); }
    tcp.data_handler({1,3,4,0,0,0,0}, old_token); // duplicate terminal callback ignored
    assert(!client.consumeUpdated());
    for (const ModbusMessage bad : {ModbusMessage{2,3,4,0,0,0,0}, {1,6,4,0,0,0,0},
                                   {1,3,2,0,0}, {1,3,3,0,0,0,0}, {1,3}}) {
        assert(client.readBlockFromMeter(std::size_t(0)));
        tcp.respond(bad);
        assert(!client.consumeUpdated());
        DataAccess<Client<TestSchema>> access(client);
        assert(access.get<TestSchema::value>() == -14795);
    }
    // New data after consumption remains pending, including during processing.
    assert(client.readBlockFromMeter("first"));
    tcp.respond({1,3,4,0,1,0,0});
    assert(client.consumeUpdated());
    assert(client.readBlockFromMeter("first"));
    tcp.respond({1,3,4,0,2,0,0});
    assert(client.consumeUpdated());
    assert(!client.readBlockFromMeter("missing"));
    assert(!client.readBlockFromMeter(std::size_t(99)));
    auto forged = TestSchema::description<TestSchema::value>();
    forged.block = 65535;
    assert(!client.readFromMeter(forged, forged));
    tcp.enqueue_error = REQUEST_QUEUE_FULL;
    for (unsigned n=0; n<100; ++n) assert(!client.readBlockFromMeter("first"));
    tcp.enqueue_error = SUCCESS;
    for (std::size_t n=0; n<client.request_capacity; ++n) assert(client.readBlockFromMeter("first"));
    assert(!client.readBlockFromMeter("first"));
    while (!tcp.pending.empty()) tcp.fail();
    assert(client.readBlockFromMeter("first"));
    tcp.respond({1,3,4,0,3,0,0});
    assert(client.consumeUpdated());
    std::cout << "PASS: production client/server frames, isolation, callbacks and request capacity\n";
}
