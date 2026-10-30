// Copyright (c) 2015-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arith_uint256.h>
#include <chain.h>
#include <cmath>
#include <chainparams.h>
#include <pow.h>
#include <test/util/random.h>
#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/* Test calculation of next difficulty target with no constraints applying */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1261130161; // Block #30240
    CBlockIndex pindexLast;
    pindexLast.nHeight = 32255;
    pindexLast.nTime = 1262152739;  // Block #32255
    pindexLast.nBits = 0x1d00ffff;

    // Here (and below): expected_nbits is calculated in
    // CalculateNextWorkRequired(); redoing the calculation here would be just
    // reimplementing the same code that is written in pow.cpp. Rather than
    // copy that code, we just hardcode the expected result.
    unsigned int expected_nbits = 0x1d00d86aU;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), expected_nbits);
    BOOST_CHECK(PermittedDifficultyTransition(chainParams->GetConsensus(), pindexLast.nHeight+1, pindexLast.nBits, expected_nbits));
}

/* Test the constraint on the upper bound for next work */
BOOST_AUTO_TEST_CASE(get_next_work_pow_limit)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1231006505; // Block #0
    CBlockIndex pindexLast;
    pindexLast.nHeight = 2015;
    pindexLast.nTime = 1233061996;  // Block #2015
    pindexLast.nBits = 0x1d00ffff;
    unsigned int expected_nbits = 0x1d00ffffU;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), expected_nbits);
    BOOST_CHECK(PermittedDifficultyTransition(chainParams->GetConsensus(), pindexLast.nHeight+1, pindexLast.nBits, expected_nbits));
}

/* Test the constraint on the lower bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_lower_limit_actual)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1279008237; // Block #66528
    CBlockIndex pindexLast;
    pindexLast.nHeight = 68543;
    pindexLast.nTime = 1279297671;  // Block #68543
    pindexLast.nBits = 0x1c05a3f4;
    unsigned int expected_nbits = 0x1c0168fdU;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), expected_nbits);
    BOOST_CHECK(PermittedDifficultyTransition(chainParams->GetConsensus(), pindexLast.nHeight+1, pindexLast.nBits, expected_nbits));
    // Test that reducing nbits further would not be a PermittedDifficultyTransition.
    unsigned int invalid_nbits = expected_nbits-1;
    BOOST_CHECK(!PermittedDifficultyTransition(chainParams->GetConsensus(), pindexLast.nHeight+1, pindexLast.nBits, invalid_nbits));
}

/* Test the constraint on the upper bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_upper_limit_actual)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1263163443; // NOTE: Not an actual block time
    CBlockIndex pindexLast;
    pindexLast.nHeight = 46367;
    pindexLast.nTime = 1269211443;  // Block #46367
    pindexLast.nBits = 0x1c387f6f;
    unsigned int expected_nbits = 0x1d00e1fdU;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), expected_nbits);
    BOOST_CHECK(PermittedDifficultyTransition(chainParams->GetConsensus(), pindexLast.nHeight+1, pindexLast.nBits, expected_nbits));
    // Test that increasing nbits further would not be a PermittedDifficultyTransition.
    unsigned int invalid_nbits = expected_nbits+1;
    BOOST_CHECK(!PermittedDifficultyTransition(chainParams->GetConsensus(), pindexLast.nHeight+1, pindexLast.nBits, invalid_nbits));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_negative_target)
{
    const auto consensus = CreateChainParams(*m_node.args, CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    nBits = UintToArith256(consensus.powLimit).GetCompact(true);
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_overflow_target)
{
    const auto consensus = CreateChainParams(*m_node.args, CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits{~0x00800000U};
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_too_easy_target)
{
    const auto consensus = CreateChainParams(*m_node.args, CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 nBits_arith = UintToArith256(consensus.powLimit);
    nBits_arith *= 2;
    nBits = nBits_arith.GetCompact();
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_biger_hash_than_target)
{
    const auto consensus = CreateChainParams(*m_node.args, CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 hash_arith = UintToArith256(consensus.powLimit);
    nBits = hash_arith.GetCompact();
    hash_arith *= 2; // hash > nBits
    hash = ArithToUint256(hash_arith);
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_zero_target)
{
    const auto consensus = CreateChainParams(*m_node.args, CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 hash_arith{0};
    nBits = hash_arith.GetCompact();
    hash = ArithToUint256(hash_arith);
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        blocks[i].nTime = 1269211443 + i * chainParams->GetConsensus().nPowTargetSpacing;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = &blocks[InsecureRandRange(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, chainParams->GetConsensus());
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

void sanity_check_chainparams(const ArgsManager& args, std::string chainName)
{
    const auto chainParams = CreateChainParams(args, chainName);
    const auto consensus = chainParams->GetConsensus();

    // hash genesis is correct
    BOOST_CHECK_EQUAL(consensus.hashGenesisBlock, chainParams->GenesisBlock().GetHash());

    // target timespan is an even multiple of spacing
    BOOST_CHECK_EQUAL(consensus.nPowTargetTimespan % consensus.nPowTargetSpacing, 0);

    // genesis nBits is positive, doesn't overflow and is lower than powLimit
    arith_uint256 pow_compact;
    bool neg, over;
    pow_compact.SetCompact(chainParams->GenesisBlock().nBits, &neg, &over);
    BOOST_CHECK(!neg && pow_compact != 0);
    BOOST_CHECK(!over);
    BOOST_CHECK(UintToArith256(consensus.powLimit) >= pow_compact);

    // check max target * 4*nPowTargetTimespan doesn't overflow -- see pow.cpp:CalculateNextWorkRequired()
    if (!consensus.fPowNoRetargeting) {
        arith_uint256 targ_max("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
        targ_max /= consensus.nPowTargetTimespan*4;
        BOOST_CHECK(UintToArith256(consensus.powLimit) < targ_max);
    }
}

BOOST_AUTO_TEST_CASE(ChainParams_MAIN_sanity)
{
    sanity_check_chainparams(*m_node.args, CBaseChainParams::MAIN);
}

BOOST_AUTO_TEST_CASE(ChainParams_REGTEST_sanity)
{
    sanity_check_chainparams(*m_node.args, CBaseChainParams::REGTEST);
}

BOOST_AUTO_TEST_CASE(ChainParams_TESTNET_sanity)
{
    sanity_check_chainparams(*m_node.args, CBaseChainParams::TESTNET);
}

BOOST_AUTO_TEST_CASE(ChainParams_SIGNET_sanity)
{
    sanity_check_chainparams(*m_node.args, CBaseChainParams::SIGNET);
}


/* HANBIT: ASERT basics */
BOOST_AUTO_TEST_CASE(hanbit_asert_basics)
{
    const arith_uint256 lim = UintToArith256(uint256S("00000ffff0000000000000000000000000000000000000000000000000000000"));
    const arith_uint256 ref = lim >> 12;
    const int64_t sp = 600, hl = 7200;
    // on schedule -> unchanged
    BOOST_CHECK(CalculateASERT(ref, sp, sp * 11, 10, lim, hl) == ref);
    // one half-life late -> x2, early -> /2
    BOOST_CHECK(CalculateASERT(ref, sp, sp * 11 + hl, 10, lim, hl) == ref * 2);
    BOOST_CHECK(CalculateASERT(ref, sp, sp * 11 - hl, 10, lim, hl) == ref / 2);
    // clamps
    BOOST_CHECK(CalculateASERT(ref, sp, 1000000000, 0, lim, hl) == lim);
    BOOST_CHECK(CalculateASERT(ref, sp, -1000000000, 0, lim, hl) == arith_uint256(1));
    // monotonic in time, and fractional factor close to 2^x
    arith_uint256 prev = 0;
    for (int64_t dt = -hl; dt <= hl; dt += 97) {
        arith_uint256 t = CalculateASERT(ref, sp, sp + dt, 0, lim, hl);
        BOOST_CHECK(t >= prev);
        prev = t;
        double want = ref.getdouble() * std::pow(2.0, double(dt) / hl);
        BOOST_CHECK(std::abs(t.getdouble() / want - 1.0) < 0.0002);
    }
}

/* HANBIT: simulate a 143,000x miner joining and leaving */
BOOST_AUTO_TEST_CASE(hanbit_asert_hashrate_shock)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    Consensus::Params p = chainParams->GetConsensus();
    BOOST_CHECK_EQUAL(p.nAsertActivationHeight, 64512);
    p.nAsertActivationHeight = 100;
    const arith_uint256 lim = UintToArith256(p.powLimit);

    const int N = 6000;
    std::vector<CBlockIndex> b(N);
    const double cpu = 8.4e6; // H/s, today's network
    auto expected_secs = [](uint32_t nBits, double hps) {
        arith_uint256 t; t.SetCompact(nBits);
        return std::ldexp(1.0, 256) / (t.getdouble() + 1.0) / hps;
    };
    arith_uint256 start; start.SetCompact(0x1d00ffff); // ~ today's difficulty
    int64_t now = 1790000000;
    int flood_blocks = -1, asic_on = 1000, asic_off = 3000;
    double stall_secs = -1, sum_after = 0; int n_after = 0;
    for (int h = 0; h < N; ++h) {
        b[h].nHeight = h;
        b[h].pprev = h ? &b[h - 1] : nullptr;
        double hps = (h >= asic_on && h < asic_off) ? cpu * 143000 : cpu;
        if (h <= 100) {
            b[h].nBits = start.GetCompact();
        } else {
            CBlockHeader hdr; hdr.nTime = now + 1;
            b[h].nBits = GetNextWorkRequired(&b[h - 1], &hdr, p);
        }
        double secs = expected_secs(b[h].nBits, hps);
        if (h > 100 && secs > 6 * 3600) {
            // stall relief: find the time at which relaxed target makes the block expected now
            int64_t gap = 0;
            while (true) {
                gap += 60;
                CBlockHeader hdr; hdr.nTime = now + gap;
                uint32_t bits = GetNextWorkRequired(&b[h - 1], &hdr, p);
                if (expected_secs(bits, hps) <= gap) { b[h].nBits = bits; secs = gap; break; }
            }
        }
        if (h == asic_off) stall_secs = secs;
        if (h >= asic_on && flood_blocks < 0 && secs >= 300) flood_blocks = h - asic_on;
        if (h > asic_off + 300 && h < asic_off + 2300) { sum_after += secs; ++n_after; }
        now += int64_t(secs < 1 ? 1 : secs);
        b[h].nTime = now;
    }
    BOOST_TEST_MESSAGE("flood blocks " << flood_blocks << ", stall hours " << stall_secs / 3600 << ", avg after " << sum_after / n_after);
    BOOST_CHECK(flood_blocks >= 0 && flood_blocks < 400);
    BOOST_CHECK(stall_secs < 30 * 3600);
    BOOST_CHECK(std::abs(sum_after / n_after - 600) < 60);
    // fork not active before activation: old rule untouched
    BOOST_CHECK(!p.AsertActiveAt(99));
    BOOST_CHECK(p.AsertActiveAt(100));
}


/* HANBIT: stall relief steps are exact; a header rolled across a step needs new nBits */
BOOST_AUTO_TEST_CASE(hanbit_stall_relief_step)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    Consensus::Params p = chainParams->GetConsensus();
    p.nAsertActivationHeight = 10;
    std::vector<CBlockIndex> b(20);
    for (int h = 0; h < 20; ++h) {
        b[h].nHeight = h; b[h].pprev = h ? &b[h - 1] : nullptr;
        b[h].nTime = 1790000000 + 600 * h; b[h].nBits = 0x1d00ffff;
    }
    auto bits_at = [&](int64_t gap) { CBlockHeader hdr; hdr.nTime = b[19].nTime + gap; return GetNextWorkRequired(&b[19], &hdr, p); };
    const uint32_t normal = bits_at(600);
    BOOST_CHECK_EQUAL(bits_at(6 * 3600), normal);           // not yet
    BOOST_CHECK_EQUAL(bits_at(7 * 3600 - 60), normal);      // first full step not reached
    arith_uint256 a, c; a.SetCompact(normal); c.SetCompact(bits_at(7 * 3600 + 60));
    BOOST_CHECK(bits_at(7 * 3600 + 60) != normal);          // rolled across a step -> different nBits
    BOOST_CHECK(c.getdouble() / a.getdouble() > 1.99 && c.getdouble() / a.getdouble() < 2.01);
}

BOOST_AUTO_TEST_SUITE_END()
