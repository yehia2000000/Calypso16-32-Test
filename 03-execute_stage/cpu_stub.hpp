#ifndef CPU_STUB_HPP
#define CPU_STUB_HPP    

#include <cstdint>
#include <cstring>

static constexpr int      XLEN           = 64;
static constexpr int      NUM_REGS       = 32;
static constexpr int      MEM_SIZE       = 65536;      // 64 KiB
static constexpr uint64_t PROGRAM_START  = 0x1000;    // first program byte
static constexpr uint64_t DATA_END       = PROGRAM_START - 1; // 0x0FFF
static constexpr int      WORD_BYTES     = 8;          // LW/SW move 64 bits
static constexpr int      MAX_CYCLES     = 1000;       // execution backstop

class RegisterFile {
public:
    RegisterFile() { std::memset(regs_, 0, sizeof(regs_)); }

    // Architectural write — x0 writes are silently discarded.
    void write(int idx, uint64_t value) {
        if (idx != 0) {
            regs_[idx] = value;
        }
    }

    // Architectural read — x0 always reads as 0.
    uint64_t read(int idx) const {
        return (idx == 0) ? 0ULL : regs_[idx];
    }

    // Raw storage access needed by legacy test helpers that poke/peek
    // register physical storage directly (x0 hardwire bypassed).
    uint64_t& raw(int idx)             { return regs_[idx]; }
    uint64_t  raw(int idx) const       { return regs_[idx]; }

private:
    uint64_t regs_[NUM_REGS];
};
class CPUState {
public:

    uint16_t encCS(int funct4, int rd, int rs2);
    uint16_t encCI(int funct3, int rd, int imm6);
    uint32_t encI(int funct4, int rd_or_rs2, int rs1, int imm16);
    void write16BE(uint64_t addr, uint16_t value);
    void write32BE(uint64_t addr, uint32_t value) ;
    void load_demo_program() ;

    CPUState() ; 

    void reset();
     RegisterFile rf;
    uint8_t      memory[MEM_SIZE];
    uint64_t     PC;
    bool         running;
};
enum { FMT_CS = 0, FMT_CI = 1, FMT_R = 2, FMT_I = 3 };

// Register-register funct4 values used by CS and R.
enum {
    ALU_ADD = 0,
    ALU_SUB = 1,
    ALU_AND = 2,
    ALU_OR  = 3,
    ALU_SLT = 4,
    ALU_MUL = 5,
    ALU_DIV = 6,
    ALU_REM = 7
};

// Register-immediate funct3 values for CI and funct4 low values for I.
enum {
    ALU_ADDI = 0,
    ALU_ANDI = 1,
    ALU_ORI  = 2,
    ALU_SLTI = 3
};

// I-format non-ALU operations.
enum {
    OP_LW   = 4,
    OP_SW   = 5,
    OP_BEQ  = 6,
    OP_JAL  = 7
};


#endif 