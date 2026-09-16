#ifndef EXECUTE_H
#define EXECUTE_H

#include "cpu_stub.hpp"
#include "mem_stub.hpp"
struct Decoded {
    int     format  = -1;
    int     funct   = 0;
    int     rd      = 0;
    int     rs1     = 0;
    int     rs2     = 0;
    int64_t imm     = 0;
    bool    illegal = false;
};
// ============================================================
// Executor — executes one already-decoded instruction.
// Holds references to CPUState (registers, PC, running flag)
// and Memory (for load/store operations).
// ============================================================
class Executor {
public:
    Executor(CPUState& cpu, Memory& mem) : cpu_(cpu), mem_(mem) {}

    // Execute exactly one already-decoded instruction.
    // May update registers, memory, PC, and the running flag.
    void execute(const Decoded& d);

private:
    bool is_alu_op(const Decoded& d) const;
    void execute_alu(const Decoded& d);
    void execute_mem_branch_jump(const Decoded& d);

    CPUState& cpu_;
    Memory&   mem_;
};

#endif
