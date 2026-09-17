/*// ============================================================
// Bug-check tests for the Calypso16/32 disassembler.
// NEW FILE — does not modify any existing project source.
//
// Bugs under test:
//   Bug 1: Disassembler::disassemble(const Decoded&) indexes
//          names[d.funct] with NO bounds check, while the raw-
//          instruction APIs (disassembleCS/CI/R) DO check.
//          CS funct is 4 bits (0..15) vs an 8-entry table,
//          CI funct is 3 bits (0..7) vs a 4-entry table,
//          R  funct is 4 bits (0..15) vs an 8-entry table.
//          Compile with -fsanitize=address,undefined
//          -fno-sanitize-recover=all : the UBSan/ASan abort IS
//          the evidence for the out-of-bounds read.
//
//   Bug 2: the two API entry points contradict each other on
//          the same legality rule (raw path -> "UNKNOWN",
//          Decoded path -> OOB read / garbage).
// ============================================================

#include <gtest/gtest.h>

#include <string>

#include "decode.h"
#include "disassembler.h"

// ------------------------------------------------------------
// Bug 1 — unchecked names[d.funct] in the Decoded dispatcher
// ------------------------------------------------------------

// CS: funct field is bits [13:10] (4 bits) but the table has 8
// entries. funct = 12 is a real, encodable value.
TEST(Bug1_OOB, CS_Funct12_ReadsPastTable) {
    Decoded d{};
    d.format  = FMT_CS;
    d.funct   = 12;      // legal bit-pattern, illegal architecture value
    d.rd  = 1;
    d.rs2 = 1;
    d.illegal = true;

    // Under -fsanitize=address,undefined -fno-sanitize-recover=all
    // this line aborts with global-buffer-overflow / index-out-of-
    // bounds BEFORE the assertion is ever evaluated.
    const std::string s = Disassembler::disassemble(d);
    // If sanitizers are off we at least prove it does not return
    // "UNKNOWN" like the raw API does for the same input.
    EXPECT_EQ(s, "UNKNOWN");
}

// CI: funct field is bits [13:11] (3 bits, 0..7) vs a 4-entry table.
TEST(Bug1_OOB, CIFunct7ReadsPastTable) {
    Decoded d{};
    d.format  = FMT_CI;
    d.funct   = 7;       // encodable, table only has 0..3
    d.rd  = 1;
    d.illegal = true;

    const std::string s = Disassembler::disassemble(d);
    EXPECT_EQ(s, "UNKNOWN");
}

// R: funct field is bits [29:26] (4 bits, 0..15) vs an 8-entry table.
TEST(Bug1_OOB, R_Funct15_ReadsPastTable) {
    Decoded d{};
    d.format  = FMT_R;
    d.funct   = 15;      // encodable, table only has 0..7
    d.rd  = 1;
    d.rs1 = 1;
    d.rs2 = 1;
    d.illegal = true;

    const std::string s = Disassembler::disassemble(d);
    EXPECT_EQ(s, "UNKNOWN");
}

// ------------------------------------------------------------
// Bug 2 — the two APIs contradict each other
// ------------------------------------------------------------

TEST(Bug2_ApiDivergence, SameHalfwordDifferentBehavior) {
    // 0x3001: CS prefix (00), funct = 12 -> raw API says UNKNOWN.
    EXPECT_EQ(Disassembler::disassembleCS(0x3001), "UNKNOWN");

    // End-to-end through the real decoder: the decoder DOES flag it
    // illegal, so the dispatcher is shielded in the full pipeline.
    const Decoded d = Decoder::decode(0x3001, 2);
    EXPECT_TRUE(d.illegal);
    EXPECT_EQ(Disassembler::disassemble(d), "UNKNOWN");

    // But disassemble()'s CONTRACT is "give me a Decoded, I give you
    // a string". Nothing in the type system or the code stops a
    // caller from handing in a legal-looking Decoded (e.g. produced
    // by any future decoder change, or a hand-built one) — and then
    // the two APIs demonstrably disagree on the same legality rule.
    // That disagreement is proven by Bug1_OOB.CS_Funct12 above vs
    // the "UNKNOWN" asserted here.
}
*/

#include <gtest/gtest.h>
#include <string>

#include "decode.h"
#include "disassembler.h"


// ============================================================
// Helpers
// ============================================================

// Encode 16-bit CS instruction
static uint16_t encodeCS(int funct, int rd, int rs2)
{
    return static_cast<uint16_t>(
        (0b00u << 14) |
        (static_cast<uint16_t>(funct) << 10) |
        (static_cast<uint16_t>(rd) << 5) |
        static_cast<uint16_t>(rs2)
    );
}


// Encode 16-bit CI instruction
static uint16_t encodeCI(int funct, int rd, int imm)
{
    // imm is represented using 6 bits
    return static_cast<uint16_t>(
        (0b01u << 14) |
        (static_cast<uint16_t>(funct) << 11) |
        (static_cast<uint16_t>(rd) << 6) |
        (static_cast<uint16_t>(imm) & 0x3F)
    );
}



// Encode 32-bit R instruction
static uint32_t encodeR(int funct, int rd, int rs1, int rs2)
{
    return
        (0b10u << 30) |
        (static_cast<uint32_t>(funct) << 26) |
        (static_cast<uint32_t>(rd) << 21) |
        (static_cast<uint32_t>(rs1) << 16) |
        (static_cast<uint32_t>(rs2) << 11);
}


// Encode 32-bit I instruction
//
// For BEQ:
//     rdOrRs2 = rs2
//
// For SW:
//     rdOrRs2 = rs2
//
// For the other I instructions:
//     rdOrRs2 = rd
//
static uint32_t encodeI(int funct, int rdOrRs2, int rs1, int imm)
{
    return
        (0b11u << 30) |
        (static_cast<uint32_t>(funct) << 26) |
        (static_cast<uint32_t>(rdOrRs2) << 21) |
        (static_cast<uint32_t>(rs1) << 16) |
        (static_cast<uint32_t>(imm) & 0xFFFFu);
}


TEST(Bug1_OOB, R_Funct15_ReadsPastTable) {
    Decoded d{};
    d.format  = FMT_R;
    d.funct   = 15;      // encodable, table only has 0..7
    d.rd  = 1;
    d.rs1 = 1;
    d.rs2 = 1;
    d.illegal = false;

    const std::string s = Disassembler::disassemble(d);
    EXPECT_EQ(s, "UNKNOWN");
}

// ============================================================
// 1. Compressed CS instructions
// ============================================================

TEST(DisassemblerCS, AllValidInstructions)
{
    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(0, 1, 2)),
        "C.ADD x1, x2"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(1, 3, 4)),
        "C.SUB x3, x4"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(2, 5, 6)),
        "C.AND x5, x6"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(3, 7, 8)),
        "C.OR x7, x8"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(4, 9, 10)),
        "C.SLT x9, x10"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(5, 11, 12)),
        "C.MUL x11, x12"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(6, 13, 14)),
        "C.DIV x13, x14"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(7, 15, 16)),
        "C.REM x15, x16"
    );
}


// ============================================================
// 2. Compressed CS invalid funct
// ============================================================

TEST(DisassemblerCS, InvalidFunctReturnsUnknown)
{
    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(8, 1, 2)),
        "UNKNOWN"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(15, 1, 2)),
        "UNKNOWN"
    );
}


// ============================================================
// 3. Compressed CS register boundaries
// ============================================================

TEST(DisassemblerCS, RegisterBoundaries)
{
    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(0, 0, 0)),
        "C.ADD x0, x0"
    );

    EXPECT_EQ(
        Disassembler::disassembleCS(encodeCS(7, 31, 31)),
        "C.REM x31, x31"
    );
}


// ============================================================
// 4. Compressed CI instructions
// ============================================================

TEST(DisassemblerCI, AllValidInstructions)
{
    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(0, 1, 5)),
        "C.ADDI x1, 5"
    );

    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(1, 2, 5)),
        "C.ANDI x2, 5"
    );

    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(2, 3, 5)),
        "C.ORI x3, 5"
    );

    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(3, 4, 5)),
        "C.SLTI x4, 5"
    );
}


// ============================================================
// 5. Compressed CI invalid funct
// ============================================================

TEST(DisassemblerCI, InvalidFunctReturnsUnknown)
{
    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(4, 1, 5)),
        "UNKNOWN"
    );

    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(7, 1, 5)),
        "UNKNOWN"
    );
}


// ============================================================
// 6. Compressed CI immediate sign extension
// ============================================================

TEST(DisassemblerCI, ImmediateSignExtension)
{
    // +31
    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(0, 1, 31)),
        "C.ADDI x1, 31"
    );

    // -1 = 0b111111 in 6-bit two's complement
    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(0, 1, -1)),
        "C.ADDI x1, -1"
    );

    // -32 = minimum 6-bit signed value
    EXPECT_EQ(
        Disassembler::disassembleCI(encodeCI(0, 1, -32)),
        "C.ADDI x1, -32"
    );
}


// ============================================================
// 7. R-type instructions
// ============================================================

TEST(DisassemblerR, AllValidInstructions)
{
    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(0, 1, 2, 3)),
        "ADD x1, x2, x3"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(1, 1, 2, 3)),
        "SUB x1, x2, x3"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(2, 1, 2, 3)),
        "AND x1, x2, x3"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(3, 1, 2, 3)),
        "OR x1, x2, x3"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(4, 1, 2, 3)),
        "SLT x1, x2, x3"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(5, 1, 2, 3)),
        "MUL x1, x2, x3"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(6, 1, 2, 3)),
        "DIV x1, x2, x3"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(7, 1, 2, 3)),
        "REM x1, x2, x3"
    );
}


// ============================================================
// 8. R-type invalid funct
// ============================================================

TEST(DisassemblerR, InvalidFunctReturnsUnknown)
{
    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(8, 1, 2, 3)),
        "UNKNOWN"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(15, 1, 2, 3)),
        "UNKNOWN"
    );
}


// ============================================================
// 9. R-type register boundaries
// ============================================================

TEST(DisassemblerR, RegisterBoundaries)
{
    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(0, 0, 0, 0)),
        "ADD x0, x0, x0"
    );

    EXPECT_EQ(
        Disassembler::disassembleR(encodeR(7, 31, 31, 31)),
        "REM x31, x31, x31"
    );
}


// ============================================================
// 10. I-type ALU instructions
// ============================================================

TEST(DisassemblerI, ALUInstructions)
{
    EXPECT_EQ(
        Disassembler::disassembleI(encodeI(ALU_ADDI, 1, 2, 10)),
        "ADDI x1, x2, 10"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(encodeI(ALU_ANDI, 3, 4, 10)),
        "ANDI x3, x4, 10"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(encodeI(ALU_ORI, 5, 6, 10)),
        "ORI x5, x6, 10"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(encodeI(ALU_SLTI, 7, 8, 10)),
        "SLTI x7, x8, 10"
    );
}


// ============================================================
// 11. I-type load/store
// ============================================================

TEST(DisassemblerI, LoadStoreInstructions)
{
    EXPECT_EQ(
        Disassembler::disassembleI(encodeI(OP_LW, 5, 10, 16)),
        "LW x5, 16(x10)"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(encodeI(OP_SW, 5, 10, 16)),
        "SW x5, 16(x10)"
    );
}


// ============================================================
// 12. I-type control flow
// ============================================================

TEST(DisassemblerI, BranchAndJump)
{
    EXPECT_EQ(
        Disassembler::disassembleI(encodeI(OP_BEQ, 2, 1, 20)),
        "BEQ x1, x2, 20"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(encodeI(OP_JAL, 5, 0, 100)),
        "JAL x5, 100"
    );
}


// ============================================================
// 13. I-type immediate sign extension
// ============================================================

TEST(DisassemblerI, ImmediateSignExtension)
{
    EXPECT_EQ(
        Disassembler::disassembleI(
            encodeI(ALU_ADDI, 1, 2, 0)
        ),
        "ADDI x1, x2, 0"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(
            encodeI(ALU_ADDI, 1, 2, 32767)
        ),
        "ADDI x1, x2, 32767"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(
            encodeI(ALU_ADDI, 1, 2, -1)
        ),
        "ADDI x1, x2, -1"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(
            encodeI(ALU_ADDI, 1, 2, -32768)
        ),
        "ADDI x1, x2, -32768"
    );
}


// ============================================================
// 14. I-type invalid funct
// ============================================================

TEST(DisassemblerI, InvalidFunctReturnsUnknown)
{
    EXPECT_EQ(
        Disassembler::disassembleI(
            encodeI(8, 1, 2, 10)
        ),
        "UNKNOWN"
    );

    EXPECT_EQ(
        Disassembler::disassembleI(
            encodeI(15, 1, 2, 10)
        ),
        "UNKNOWN"
    );
}


// ============================================================
// 15. Decoder + Disassembler integration
// ============================================================

TEST(DisassemblerIntegration, CS)
{
    uint16_t raw = encodeCS(0, 1, 2);

    Decoded d = Decoder::decode(raw, 2);

    EXPECT_FALSE(d.illegal);
    EXPECT_EQ(Disassembler::disassemble(d), "C.ADD x1, x2");
}


TEST(DisassemblerIntegration, CI)
{
    uint16_t raw = encodeCI(0, 1, -1);

    Decoded d = Decoder::decode(raw, 2);

    EXPECT_FALSE(d.illegal);
    EXPECT_EQ(Disassembler::disassemble(d), "C.ADDI x1, -1");
}


TEST(DisassemblerIntegration, R)
{
    uint32_t raw = encodeR(0, 1, 2, 3);

    Decoded d = Decoder::decode(raw, 4);

    EXPECT_FALSE(d.illegal);
    EXPECT_EQ(Disassembler::disassemble(d), "ADD x1, x2, x3");
}


TEST(DisassemblerIntegration, I)
{
    uint32_t raw = encodeI(ALU_ADDI, 1, 2, -10);

    Decoded d = Decoder::decode(raw, 4);

    EXPECT_FALSE(d.illegal);
    EXPECT_EQ(Disassembler::disassemble(d), "ADDI x1, x2, -10");
}


// ============================================================
// 16. Illegal decoded instruction
// ============================================================

TEST(DisassemblerDecoded, IllegalInstructionReturnsUnknown)
{
    Decoded d{};

    d.illegal = true;

    EXPECT_EQ(
        Disassembler::disassemble(d),
        "UNKNOWN"
    );
}


// ============================================================
// 17. Invalid instruction sizes
// ============================================================

TEST(DecoderIntegration, InvalidInstructionSize)
{
    Decoded d2 = Decoder::decode(0x12345678, 1);

    EXPECT_TRUE(d2.illegal);

    EXPECT_EQ(
        Disassembler::disassemble(d2),
        "UNKNOWN"
    );
}


TEST(DecoderIntegration, InvalidInstructionSize3)
{
    Decoded d = Decoder::decode(0x12345678, 3);

    EXPECT_TRUE(d.illegal);

    EXPECT_EQ(
        Disassembler::disassemble(d),
        "UNKNOWN"
    );
}