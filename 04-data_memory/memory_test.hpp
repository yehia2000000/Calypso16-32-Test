#ifndef MEMORY_TEST_HPP
#define MEMORY_TEST_HPP

#include "memory.h"

#include <gtest/gtest.h>

class MemoryTest : public ::testing::Test {
public:
    CPUState cpu;
    Memory mem{cpu};

    void SetUp() override;
    void TearDown() override;
};

#endif
