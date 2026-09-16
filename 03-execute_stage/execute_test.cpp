#include "execute_test.hpp"
 
Decoded ExecuteTest::make_alu_immediate(int funct, int rd, int rs1, int64_t imm) {
	Decoded instruction;
	instruction.format = FMT_I;
	instruction.funct = funct;
	instruction.rd = rd;
	instruction.rs1 = rs1;
	instruction.imm = imm;
	return instruction;
}

Decoded ExecuteTest::make_alu_register(int funct, int rd, int rs1, int rs2) {
	Decoded instruction;
	instruction.format = FMT_R;
	instruction.funct = funct;
	instruction.rd = rd;
	instruction.rs1 = rs1;
	instruction.rs2 = rs2;
	return instruction;
}

Decoded ExecuteTest::make_memory_instruction(int funct, int rd, int rs1, int64_t imm) {
	Decoded instruction;
	instruction.format = FMT_I;
	instruction.funct = funct;
	instruction.rd = rd;
	instruction.rs1 = rs1;
	instruction.imm = imm;
	return instruction;
}

void ExecuteTest::SetUp() {
	cpu.reset();
}



// Verifies that an ADDI instruction adds a positive immediate to a source register.
TEST_F(ExecuteTest, ExecutesImmediateAddition) {
	cpu.rf.write(1, 7);

	executor.execute(make_alu_immediate(ALU_ADDI, 2, 1, 5));

	EXPECT_EQ(cpu.rf.read(2), 12u);
	EXPECT_TRUE(cpu.running);
}

// Verifies that an ADDI instruction sign-extends and adds a negative immediate.
TEST_F(ExecuteTest, ExecutesNegativeImmediateAddition) {
	cpu.rf.write(1, 3);

	executor.execute(make_alu_immediate(ALU_ADDI, 2, 1, -5));

	EXPECT_EQ(static_cast<int64_t>(cpu.rf.read(2)), -2);
	EXPECT_TRUE(cpu.running);
}

// Verifies that an ADD instruction adds the values of two source registers.
TEST_F(ExecuteTest, ExecutesRegisterAddition) {
	cpu.rf.write(1, 7);
	cpu.rf.write(2, 5);

	executor.execute(make_alu_register(ALU_ADD, 3, 1, 2));

	EXPECT_EQ(cpu.rf.read(3), 12u);
}

// Verifies that a stored 64-bit value can be loaded back unchanged.
TEST_F(ExecuteTest, StoresAndLoadsAWord) {
	cpu.rf.write(1, 0x1122334455667788ULL);

	executor.execute(make_memory_instruction(OP_SW, 1, 0, 0));
	executor.execute(make_memory_instruction(OP_LW, 2, 0, 0));

	EXPECT_EQ(cpu.rf.read(2), 0x1122334455667788ULL);
	EXPECT_TRUE(cpu.running);
}

// Verifies that BEQ updates the program counter when both registers are equal.
TEST_F(ExecuteTest, TakesEqualBranch) {
	cpu.PC = 0x1000;
	cpu.rf.write(1, 42);
	cpu.rf.write(2, 42);

	executor.execute(make_memory_instruction(OP_BEQ, 2, 1, 8));

	EXPECT_EQ(cpu.PC, 0x1008u);
}

// Verifies that the JAL halt convention stops CPU execution.
TEST_F(ExecuteTest, HaltJalStopsExecution) {
	executor.execute(make_memory_instruction(OP_JAL, 0, 0, 0));

	EXPECT_FALSE(cpu.running);
}

// Verifies that an illegal instruction stops CPU execution.
TEST_F(ExecuteTest, IllegalInstructionStopsExecution) {
	Decoded instruction;
	instruction.illegal = true;

	executor.execute(instruction);

	EXPECT_FALSE(cpu.running);
}

// Verifies that a load outside memory stops execution instead of writing a result.
TEST_F(ExecuteTest, InvalidLoadStopsExecution) {
	cpu.rf.write(1, MEM_SIZE);

	executor.execute(make_memory_instruction(OP_LW, 2, 1, 0));

	EXPECT_FALSE(cpu.running);
}

// Verifies that register x0 remains zero after an attempted write.
TEST_F(ExecuteTest, ZeroRegisterCannotBeWritten) {
	executor.execute(make_alu_immediate(ALU_ADDI, 0, 0, 5));

	EXPECT_EQ(cpu.rf.read(0), 0u);
}