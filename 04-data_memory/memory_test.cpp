#include "memory_test.hpp"

void MemoryTest::SetUp() {
    std::cout << "Setting up test: " << std::endl;  
}

void MemoryTest::TearDown() {
    std::cout << "Tearing down test: " << std::endl;
}

//read function 

// Verifies that freshly constructed memory reads back as zero.
TEST_F(MemoryTest, TC_DAM_01) {
    int64_t out = 0x42;

    EXPECT_TRUE(mem.read64(0, out));
    EXPECT_EQ(out, 0);
}

// Verifies that a read starting exactly at MEM_SIZE is rejected.
TEST_F(MemoryTest, TC_DAM_02) {
    int64_t out = 0x1234;

    EXPECT_FALSE(mem.read64(static_cast<uint64_t>(MEM_SIZE), out));
    EXPECT_EQ(out, 0x1234);
}

// Verifies that the largest possible address is rejected without overflowing addr + WORD_BYTES.
TEST_F(MemoryTest, TC_DAM_03) {
    int64_t out = 0x1234;

    EXPECT_FALSE(mem.read64(static_cast<uint64_t>(MEM_SIZE) +100, out));
    EXPECT_EQ(out, 0x1234);
}

//write function test cases

// Verifies that a write at address zero succeeds and is readable back.
TEST_F(MemoryTest, TC_DAM_04) {
    EXPECT_TRUE(mem.write64(0, 0x1122334455667788));

    int64_t out = 0;
    EXPECT_TRUE(mem.read64(0, out));
    EXPECT_EQ(out, 0x1122334455667788);
}


// Verifies that a write ending exactly on the last data byte (DATA_END) succeeds.
TEST_F(MemoryTest, TC_DAM_05) {
    uint64_t addr = DATA_END - WORD_BYTES + 1;

    EXPECT_TRUE(mem.write64(addr, 0x7EADBEEF7EADBEEF));

    int64_t out = 0;
    EXPECT_TRUE(mem.read64(addr, out));
    EXPECT_EQ(out, 0x7EADBEEF7EADBEEF);
}

// Verifies that a write whose byte range crosses into program memory is rejected.
TEST_F(MemoryTest, TC_DAM_06) {
    uint64_t addr = DATA_END - WORD_BYTES + 2;

    EXPECT_FALSE(mem.write64(addr, 0x1122334455667788));
}

// Verifies that boundary values storage , try  to store and read back  min  anf max signed 64 bit values and zero. 
TEST_F(MemoryTest, TC_DAM_07) {
    EXPECT_TRUE(mem.write64(0, INT64_MIN));
    int64_t out_min = 0;
    EXPECT_TRUE(mem.read64(0, out_min));
    EXPECT_EQ(out_min, INT64_MIN);

    EXPECT_TRUE(mem.write64(0, INT64_MAX));
    int64_t out_max = 0;
    EXPECT_TRUE(mem.read64(0, out_max));
    EXPECT_EQ(out_max, INT64_MAX);

    EXPECT_TRUE(mem.write64(0, 0));
    int64_t out_zero = 0x1234;
    EXPECT_TRUE(mem.read64(0, out_zero));
    EXPECT_EQ(out_zero, 0);
}
