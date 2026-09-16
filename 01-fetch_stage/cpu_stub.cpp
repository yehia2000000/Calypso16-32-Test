
#include <cstdint>
#include <cstring>
#include "cpu_stub.hpp"

CPUState :: CPUState() {
    std::memset(memory, 0, sizeof(memory));
    PC      = PROGRAM_START;
    running = false;
}

void CPUState :: reset() {
    std::memset(memory, 0, sizeof(memory));
    PC      = PROGRAM_START;
    running = true;
}

uint16_t CPUState::encCS(int funct4, int rd, int rs2) {
    return static_cast<uint16_t>((0b00u << 14) |
                                 ((static_cast<uint16_t>(funct4) & 0xFu) << 10) |
                                 ((static_cast<uint16_t>(rd)     & 0x1Fu) << 5) |
                                 (static_cast<uint16_t>(rs2)     & 0x1Fu));
}

 uint16_t CPUState::encCI(int funct3, int rd, int imm6) {
    return static_cast<uint16_t>((0b01u << 14) |
                                 ((static_cast<uint16_t>(funct3) & 0x7u) << 11) |
                                 ((static_cast<uint16_t>(rd)     & 0x1Fu) << 6) |
                                 (static_cast<uint16_t>(imm6)   & 0x3Fu));
}

 uint32_t CPUState::encI(int funct4, int rd_or_rs2, int rs1, int imm16) {
    return (0b11u << 30) |
           ((static_cast<uint32_t>(funct4)    & 0xFu)  << 26) |
           ((static_cast<uint32_t>(rd_or_rs2) & 0x1Fu) << 21) |
           ((static_cast<uint32_t>(rs1)       & 0x1Fu) << 16) |
           (static_cast<uint32_t>(imm16) & 0xFFFFu);
}

// ============================================================
// Big-Endian memory write helpers (program loader)
// ============================================================
void CPUState :: write16BE(uint64_t addr, uint16_t value) {
    this->memory[addr]     = static_cast<uint8_t>(value >> 8);
    this->memory[addr + 1] = static_cast<uint8_t>(value);
}

void CPUState :: write32BE(uint64_t addr, uint32_t value) {
    this->memory[addr]     = static_cast<uint8_t>(value >> 24);
    this->memory[addr + 1] = static_cast<uint8_t>(value >> 16);
    this->memory[addr + 2] = static_cast<uint8_t>(value >> 8);
    this->memory[addr + 3] = static_cast<uint8_t>(value);
}

// =========FetchTest===================================================
// Demo program loader
// ============================================================
void CPUState::load_demo_program() {
    // --- Program listing ---
    //   0x1000  C.ADDI  x1, 5      ; x1 = 5
    //   0x1002  C.ADD   x1, x1     ; x1 = 10
    //   0x1004  SW      x1, 0(x0)  ; mem[0x0000] = 10
    //   0x1008  LW      x3, 0(x0)  ; x3 = mem[0x0000] = 10
    //   0x100C  JAL     x0, 0      ; HALT
   this->write16BE(PROGRAM_START + 0,  this->encCI(ALU_ADDI, 1, 5));
   this->write16BE(PROGRAM_START + 2,  this->encCS(ALU_ADD, 1, 1));
   this->write32BE(PROGRAM_START + 4,  this->encI(OP_SW, 1, 0, 0));
   this->write32BE(PROGRAM_START + 8,  this->encI(OP_LW, 3, 0, 0));
   this->write32BE(PROGRAM_START + 12, this->encI(OP_JAL, 0, 0, 0));
}
