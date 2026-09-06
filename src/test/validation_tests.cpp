// Copyright (c) 2014-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <net.h>
#include <signet.h>
#include <validation.h>

#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(validation_tests, TestingSetup)

// Junkcoin does not use Bitcoin/Litecoin's halving model. It emits a fixed,
// custom step schedule (see GetJunkcoinBlockSubsidy in junkcoin.cpp), on top of
// which a deterministic per-height random bonus may either set the reward to
// 1000 JKC or triple it. This reference reproduces the *base* mainnet schedule
// (no bonus) so the tests can assert the schedule has not regressed.
static CAmount ExpectedMainnetBaseSubsidy(int nHeight)
{
    if (nHeight < 101)      return 1000 * COIN;
    if (nHeight < 1541)     return 500 * COIN;   // 1st day
    if (nHeight < 2981)     return 200 * COIN;   // 2nd day
    if (nHeight < 5861)     return 100 * COIN;   // 3rd and 4th days
    if (nHeight < 262800)   return 50 * COIN;
    if (nHeight < 394200)   return 25 * COIN;
    if (nHeight < 657000)   return 12.5 * COIN;
    if (nHeight < 1182600)  return 6.25 * COIN;
    if (nHeight < 1971000)  return 3.125 * COIN;
    if (nHeight < 2759400)  return 1.5625 * COIN;
    if (nHeight < 3547800)  return 0.78125 * COIN;
    return 0.390625 * COIN;
}

BOOST_AUTO_TEST_CASE(block_subsidy_test)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    const Consensus::Params& consensus = chainParams->GetConsensus();

    // Sample heights straddling every step boundary of Junkcoin's schedule. At
    // each height the reward must be the scheduled base value, or one of the two
    // bonus variants (tripled, or pinned to 1000 JKC).
    const int heights[] = {
        0, 100, 101, 1540, 1541, 2980, 2981, 5860, 5861, 100000,
        262799, 262800, 394199, 394200, 656999, 657000, 1182599, 1182600,
        1970999, 1971000, 2759399, 2759400, 3547799, 3547800, 4000000, 10000000,
    };
    for (int nHeight : heights) {
        const CAmount base = ExpectedMainnetBaseSubsidy(nHeight);
        const CAmount nSubsidy = GetBlockSubsidy(nHeight, consensus);
        const bool ok = (nSubsidy == base) || (nSubsidy == base * 3) || (nSubsidy == 1000 * COIN);
        BOOST_CHECK_MESSAGE(ok, "unexpected subsidy " << nSubsidy << " at height " << nHeight
                                << " (scheduled base " << base << ")");
        BOOST_CHECK(nSubsidy > 0);
        BOOST_CHECK(MoneyRange(nSubsidy));
    }
}

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    const auto chainParams = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    const Consensus::Params& consensus = chainParams->GetConsensus();

    // The per-block reward is never more than 3000 JKC (3x the 1000 JKC opening
    // reward). Walk the emission curve and verify the running supply stays well
    // within MoneyRange (MAX_MONEY = 54,000,000 JKC).
    CAmount nSum = 0;
    for (int nHeight = 0; nHeight < 5000000; nHeight += 1000) {
        const CAmount nSubsidy = GetBlockSubsidy(nHeight, consensus);
        BOOST_CHECK(nSubsidy > 0);
        BOOST_CHECK(nSubsidy <= 3000 * COIN);
        nSum += nSubsidy * 1000;
        BOOST_CHECK(MoneyRange(nSum));
    }
    BOOST_CHECK(nSum > 0);
}

BOOST_AUTO_TEST_SUITE_END()
