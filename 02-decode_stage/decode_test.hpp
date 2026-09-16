#ifndef DECODE_TEST_HPP
#define DECODE_TEST_HPP

#include "decode.h"

#include <gtest/gtest.h>

class DecodeTest : public ::testing::Test {
    public:
    // readable test helpers to construct raw instruction words for each format
	static unsigned int cs_instruction(int funct, int rd, int rs2);
	static unsigned int ci_instruction(int funct, int rd, int imm);
	static unsigned int r_instruction(int funct, int rd, int rs1, int rs2);
	static unsigned int i_instruction(int funct, int rd, int rs1, int imm);
};

#endif
