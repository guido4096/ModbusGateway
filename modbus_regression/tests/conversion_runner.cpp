#include "convert_em24_e1_to_wattnode.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
using namespace modbus_gateway;
int main(int argc,char** argv){
 int mode=argc>1?std::atoi(argv[1]):0;
 Client<EM24_E1> meter; Server<WattNode> wattnode; Server<Optimizer> optimizer;
 Client<SolarEdge> solaredge; Client<Alfen> alfen;
 // Populate every source measurement with deterministic, signed raw values.
 // This provides coverage beyond the registers used in the hand-picked scenarios.
 for(const auto& d:EM24_E1::metadata.registers){
    uint32_t seed=uint32_t(d.address)*37u+193u;
    if(d.type==DataType::int32){ int32_t value=(int32_t(seed%400)-200)*1000; uint32_t bits=uint32_t(value);
      uint16_t raw[2]{uint16_t(bits),uint16_t(bits>>16)};
      if(d.high_word_first)std::swap(raw[0],raw[1]);
      meter._device.writeHoldingRegisters(d.address,2,raw);
    } else if(d.type==DataType::int16||d.type==DataType::uint16){
      uint16_t raw=uint16_t(seed%1000);
      meter._device.writeHoldingRegisters(d.address,1,&raw);
    }
 }
 optimizer._device.set<Optimizer::optimizer_strategy>(mode%3);
 // Explicitly cover physically meaningful power / photovoltaic inputs.
 meter._device.set<EM24_E1::power_active>(mode==4?-12340:20000);
 meter._device.set<EM24_E1::l1_power_active>(-12340);
 meter._device.set<EM24_E1::export_energy_active>(12340);
 alfen._device.set<Alfen::real_power_sum>(mode==5?2750.5f:3000.0f);
 solaredge._device.set<SolarEdge::ac_power>(4000);
 solaredge._device.set<SolarEdge::ac_power_sf>(-1);
 solaredge._device.set<SolarEdge::b1_instantaneous_power>(mode==5?615.5f:600.0f);
 ConvertEM24_E1ToWattNode conv(meter,wattnode,optimizer,solaredge,alfen);
 conv.CopyDataFromMasterToSlave();
 for(const auto& block:WattNode::metadata.blocks){
   for(unsigned i=0;i<block.words;++i){uint16_t word=0;
      if(!wattnode._device.readHoldingRegisters(block.address+i,1,&word)) return 2;
      std::cout<<std::hex<<std::setfill('0')<<std::setw(4)<<word<<' ';
   }
 }
 std::cout<<'\n';
}
