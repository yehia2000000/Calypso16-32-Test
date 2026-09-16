#include "gtest/gtest.h"
#include "fetch.h"
#include "cpu_stub.hpp"
#include "fetch_test.hpp"

void FetchTest::SetUp() {
    std ::  cout << "Setting up the test environment..." << std :: endl;
    cpu_state.reset();
}
 
void FetchTest::TearDown() {
    std ::  cout << "Cleaning up the test environment..." << std :: endl;
}

TEST_F(FetchTest, TC_FES_01) {
    uint32_t raw_inst;
    int inst_size;
    bool result = fetcher_obj.fetch(raw_inst, inst_size);
    EXPECT_TRUE(result);
    EXPECT_EQ (raw_inst, 0b0100000001000101);
    EXPECT_EQ(inst_size, 2);
}


TEST_F(FetchTest, TC_FES_02) {
    cpu_state.PC = PROGRAM_START; 

    unsigned int raw_inst  = RAW_SENTINEL;
    int      inst_size = SIZE_SENTINEL;
    bool     result    = fetcher_obj.fetch(raw_inst, inst_size);
 
    // Expected output
    EXPECT_TRUE(result);
    EXPECT_EQ(raw_inst, 0x00000000u);  
    EXPECT_EQ(inst_size, 2);
}

TEST_F(FetchTest, TC_FES_03) {
    // Input
    cpu_state.write32BE(0x0000, 0xDC000000u);  // JAL x0, 0
    cpu_state.PC = 0x0000;
 
    // Call
    uint32_t raw_inst  = RAW_SENTINEL;
    int      inst_size = SIZE_SENTINEL;
    bool     result    = fetcher_obj.fetch(raw_inst, inst_size);
 
    // Expected output
    EXPECT_TRUE(result);
    EXPECT_EQ(raw_inst, 0xDC000000u);
    EXPECT_EQ(inst_size, 4);
}
 
TEST_F(FetchTest, TC_FES_04) {
    // Input
    const uint64_t pc = MEM_SIZE - 2;
    cpu_state.write16BE(pc, 0x4045);
    cpu_state.PC = pc;
 
    // Call
    uint32_t raw_inst  = RAW_SENTINEL;
    int      inst_size = SIZE_SENTINEL;
    bool     result    = fetcher_obj.fetch(raw_inst, inst_size);
 
    // Expected output
    EXPECT_TRUE(result);
    EXPECT_EQ(raw_inst, 0x00004045u);
    EXPECT_EQ(inst_size, 2);
}
 
TEST_F(FetchTest, TC_FES_05) {
    // Input
    const uint64_t pc = MEM_SIZE - 4;
    cpu_state.write32BE(pc, 0xD0600000u);
    cpu_state.PC = pc;
 
    // Call
    uint32_t raw_inst  = RAW_SENTINEL;
    int      inst_size = SIZE_SENTINEL;
    bool     result    = fetcher_obj.fetch(raw_inst, inst_size);
 
    // Expected output
    EXPECT_TRUE(result);
    EXPECT_EQ(raw_inst, 0xD0600000u);
    EXPECT_EQ(inst_size, 4);
}
