#ifndef FETCH_TEST_HH
#define FETCH_TEST_HH


class FetchTest : public ::testing::Test {
public: 
    CPUState cpu_state;
    Fetcher fetcher_obj{cpu_state};

    const unsigned int  RAW_SENTINEL  = 0xDEADBEEFu;
    const int      SIZE_SENTINEL = -1;

    void SetUp() override;
    void TearDown() override;
 
};

#endif