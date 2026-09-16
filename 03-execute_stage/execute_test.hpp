#ifndef EXECUTE_TEST_HPP
#define EXECUTE_TEST_HPP

#include "gtest/gtest.h"
#include "execute.h"

class ExecuteTest : public ::testing::Test {
protected:
	CPUState cpu;
	Memory memory{cpu};
	Executor executor{cpu, memory};

	void SetUp() override;

	static Decoded make_alu_immediate(int funct, int rd, int rs1, int64_t imm);
	static Decoded make_alu_register(int funct, int rd, int rs1, int rs2);
	static Decoded make_memory_instruction(int funct, int rd, int rs1, int64_t imm);
};
#endif