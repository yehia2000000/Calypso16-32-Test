#include "decode_test.hpp"
#include "decode.h"

unsigned int DecodeTest::cs_instruction(int funct, int rd, int rs2) {
	return (static_cast<unsigned int>(funct) << 10) |
		   (static_cast<unsigned int>(rd) << 5) |
		   static_cast<unsigned int>(rs2);
}

unsigned int DecodeTest::ci_instruction(int funct, int rd, int imm) {
	return (1u << 14) |
		   (static_cast<unsigned int>(funct) << 11) |
		   (static_cast<unsigned int>(rd) << 6) |
		   (static_cast<unsigned int>(imm) & 0x3Fu);
}

unsigned int DecodeTest::r_instruction(int funct, int rd, int rs1, int rs2) {
	return (2u << 30) |
		   (static_cast<unsigned int>(funct) << 26) |
		   (static_cast<unsigned int>(rd) << 21) |
		   (static_cast<unsigned int>(rs1) << 16) |
		   (static_cast<unsigned int>(rs2) << 11);
}

unsigned int DecodeTest::i_instruction(int funct, int rd, int rs1, int imm) {
	return (3u << 30) |
		   (static_cast<unsigned int>(funct) << 26) |
		   (static_cast<unsigned int>(rd) << 21) |
		   (static_cast<unsigned int>(rs1) << 16) |
		   (static_cast<unsigned int>(imm) & 0xFFFFu);
}

// Decodes a valid 16-bit CS instruction and extracts its function and registers.
TEST_F(DecodeTest, TC_DES_01) {
	const Decoded decoded = Decoder::decode(cs_instruction(ALU_SUB, 9, 17), 2);

	EXPECT_EQ(decoded.format, FMT_CS);
	EXPECT_EQ(decoded.funct, ALU_SUB);
	EXPECT_EQ(decoded.rd, 9);
	EXPECT_EQ(decoded.rs1, 9);
	EXPECT_EQ(decoded.rs2, 17);
	EXPECT_FALSE(decoded.illegal);
}

// Decodes a valid 16-bit CI instruction and extracts its register and immediate.
TEST_F(DecodeTest, TC_DES_02) {
	const Decoded decoded = Decoder::decode(ci_instruction(ALU_ADDI, 6, 12), 2);

	EXPECT_EQ(decoded.format, FMT_CI);
	EXPECT_EQ(decoded.funct, ALU_ADDI);
	EXPECT_EQ(decoded.rd, 6);
	EXPECT_EQ(decoded.rs1, 6);
	EXPECT_EQ(decoded.imm, 12);
	EXPECT_FALSE(decoded.illegal);
}

// Sign-extends a negative 6-bit immediate from a 16-bit CI instruction.
TEST_F(DecodeTest, TC_DES_03) {
	const Decoded decoded = Decoder::decode(ci_instruction(ALU_ADDI, 6, -1), 2);

	EXPECT_EQ(decoded.imm, -1);
	EXPECT_FALSE(decoded.illegal);
}

// Decodes a valid 32-bit R instruction and extracts all register fields.
TEST_F(DecodeTest, TC_DES_04) {
	const Decoded decoded = Decoder::decode(r_instruction(ALU_MUL, 3, 12, 21), 4);

	EXPECT_EQ(decoded.format, FMT_R);
	EXPECT_EQ(decoded.funct, ALU_MUL);
	EXPECT_EQ(decoded.rd, 3);
	EXPECT_EQ(decoded.rs1, 12);
	EXPECT_EQ(decoded.rs2, 21);
	EXPECT_FALSE(decoded.illegal);
}

// Decodes a valid 32-bit I instruction and extracts its registers and immediate.
TEST_F(DecodeTest, TC_DES_05) {
	const Decoded decoded = Decoder::decode(i_instruction(OP_LW, 4, 7, 24), 4);

	EXPECT_EQ(decoded.format, FMT_I);
	EXPECT_EQ(decoded.funct, OP_LW);
	EXPECT_EQ(decoded.rd, 4);
	EXPECT_EQ(decoded.rs1, 7);
	EXPECT_EQ(decoded.imm, 24);
	EXPECT_FALSE(decoded.illegal);
}

// Sign-extends a negative 16-bit immediate from a 32-bit I instruction.
TEST_F(DecodeTest, TC_DES_06) {
	const Decoded decoded = Decoder::decode(i_instruction(OP_LW, 4, 7, -2), 4);

	EXPECT_EQ(decoded.imm, -2);
	EXPECT_FALSE(decoded.illegal);
}

// Marks a 16-bit CS instruction illegal when its operation code is unsupported.
TEST_F(DecodeTest, TC_DES_07) {
	EXPECT_TRUE(Decoder::decode(cs_instruction(15, 1, 2), 2).illegal);
}

// Marks a 16-bit CI instruction illegal when its operation code is unsupported.
TEST_F(DecodeTest, TC_DES_08) {
	EXPECT_TRUE(Decoder::decode(ci_instruction(7, 1, 2), 2).illegal);
}

// Marks a 32-bit R instruction illegal when its operation code is unsupported.
TEST_F(DecodeTest, TC_DES_09) {
	EXPECT_TRUE(Decoder::decode(r_instruction(15, 1, 2, 3), 4).illegal);
}

// Marks a 32-bit I instruction illegal when its operation code is unsupported.
TEST_F(DecodeTest, TC_DES_10) {
	EXPECT_TRUE(Decoder::decode(i_instruction(8, 1, 2, 3), 4).illegal);
}

// Rejects a 16-bit instruction with a reserved compressed-format prefix.
TEST_F(DecodeTest, TC_DES_11) {
	const Decoded decoded = Decoder::decode(0x8000u, 2);

	EXPECT_TRUE(decoded.illegal);
	EXPECT_EQ(decoded.format, -1);
}

// Rejects a 32-bit instruction with a prefix that does not select R or I format.
TEST_F(DecodeTest, TC_DES_12) {
	const Decoded decoded = Decoder::decode(0, 4);

	EXPECT_TRUE(decoded.illegal);
	EXPECT_EQ(decoded.format, -1);
}

// Rejects instruction sizes other than the supported 2-byte and 4-byte sizes.
TEST_F(DecodeTest, TC_DES_13) {
	const Decoded decoded = Decoder::decode(0, 3);

	EXPECT_TRUE(decoded.illegal);
	EXPECT_EQ(decoded.format, -1);
}
