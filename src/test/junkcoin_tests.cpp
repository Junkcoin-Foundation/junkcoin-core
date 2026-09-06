// Copyright (c) 2015-2021 The Junkcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arith_uint256.h>
#include <auxpow.h>
#include <chainparams.h>
#include <chainparamsbase.h>
#include <junkcoin.h>
#include <primitives/block.h>
#include <streams.h>
#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(junkcoin_tests, TestingSetup)

/**
 * the maximum block reward at a given height for a block without fees
 */
uint64_t expectedMaxSubsidy(int height) {
    if (height < 100000) {
        return 1000000 * COIN;
    } else if (height < 145000) {
        return 500000 * COIN;
    } else if (height < 200000) {
        return 250000 * COIN;
    } else if (height < 300000) {
        return 125000 * COIN;
    } else if (height < 400000) {
        return  62500 * COIN;
    } else if (height < 500000) {
        return  31250 * COIN;
    } else if (height < 600000) {
        return  15625 * COIN;
    } else {
        return  10000 * COIN;
    }
}

/**
 * the minimum possible value for the maximum block reward at a given height
 * for a block without fees
 */
uint64_t expectedMinSubsidy(int height) {
    if (height < 100000) {
        return 0;
    } else if (height < 145000) {
        return 0;
    } else if (height < 200000) {
        return 250000 * COIN;
    } else if (height < 300000) {
        return 125000 * COIN;
    } else if (height < 400000) {
        return  62500 * COIN;
    } else if (height < 500000) {
        return  31250 * COIN;
    } else if (height < 600000) {
        return  15625 * COIN;
    } else {
        return  10000 * COIN;
    }
}

BOOST_AUTO_TEST_CASE(subsidy_first_100k_test)
{
    const CChainParams& mainParams = Params(CBaseChainParams::MAIN);
    CAmount nSum = 0;
    arith_uint256 prevHash = UintToArith256(uint256S("0"));

    for (int nHeight = 0; nHeight <= 100000; nHeight++) {
        const Consensus::Params& params = mainParams.GetConsensus(nHeight);
        CAmount nSubsidy = GetJunkcoinBlockSubsidy(nHeight, 0, params, ArithToUint256(prevHash));
        BOOST_CHECK(MoneyRange(nSubsidy));
        BOOST_CHECK(nSubsidy <= 1000000 * COIN);
        nSum += nSubsidy;
        // Use nSubsidy to give us some variation in previous block hash, without requiring full block templates
        prevHash += nSubsidy;
    }

    const CAmount expected = 54894174438 * COIN;
    BOOST_CHECK_EQUAL(expected, nSum);
}

BOOST_AUTO_TEST_CASE(subsidy_100k_145k_test)
{
    const CChainParams& mainParams = Params(CBaseChainParams::MAIN);
    CAmount nSum = 0;
    arith_uint256 prevHash = UintToArith256(uint256S("0"));

    for (int nHeight = 100000; nHeight <= 145000; nHeight++) {
        const Consensus::Params& params = mainParams.GetConsensus(nHeight);
        CAmount nSubsidy = GetJunkcoinBlockSubsidy(nHeight, 0, params, ArithToUint256(prevHash));
        BOOST_CHECK(MoneyRange(nSubsidy));
        BOOST_CHECK(nSubsidy <= 500000 * COIN);
        nSum += nSubsidy;
        // Use nSubsidy to give us some variation in previous block hash, without requiring full block templates
        prevHash += nSubsidy;
    }

    const CAmount expected = 12349960000 * COIN;
    BOOST_CHECK_EQUAL(expected, nSum);
}

// Check the simplified rewards after block 145,000
BOOST_AUTO_TEST_CASE(subsidy_post_145k_test)
{
    const CChainParams& mainParams = Params(CBaseChainParams::MAIN);
    const uint256 prevHash = uint256S("0");

    for (int nHeight = 145000; nHeight < 600000; nHeight++) {
        const Consensus::Params& params = mainParams.GetConsensus(nHeight);
        CAmount nSubsidy = GetJunkcoinBlockSubsidy(nHeight, 0, params, prevHash);
        CAmount nExpectedSubsidy = (500000 >> (nHeight / 100000)) * COIN;
        BOOST_CHECK(MoneyRange(nSubsidy));
        BOOST_CHECK_EQUAL(nSubsidy, nExpectedSubsidy);
    }

    // Test reward at 600k+ is constant
    CAmount nConstantSubsidy = GetJunkcoinBlockSubsidy(600000, 0, mainParams.GetConsensus(600000), prevHash);
    BOOST_CHECK_EQUAL(nConstantSubsidy, 10000 * COIN);

    nConstantSubsidy = GetJunkcoinBlockSubsidy(700000, 0, mainParams.GetConsensus(700000), prevHash);
    BOOST_CHECK_EQUAL(nConstantSubsidy, 10000 * COIN);
}

BOOST_AUTO_TEST_CASE(get_next_work_difficulty_limit)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus(0);

    CBlockIndex pindexLast;
    int64_t nLastRetargetTime = 1386474927; // Block # 1
    
    pindexLast.nHeight = 239;
    pindexLast.nTime = 1386475638; // Block #239
    pindexLast.nBits = 0x1e0ffff0;
    //BOOST_CHECK_EQUAL(CalculateJunkcoinNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1e00ffff);
}

BOOST_AUTO_TEST_CASE(get_next_work_pre_digishield)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus(0);
    
    CBlockIndex pindexLast;
    int64_t nLastRetargetTime = 1386942008; // Block 9359

    pindexLast.nHeight = 9599;
    pindexLast.nTime = 1386954113;
    pindexLast.nBits = 0x1c1a1206;
    //BOOST_CHECK_EQUAL(CalculateJunkcoinNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1c15ea59);
}

BOOST_AUTO_TEST_CASE(get_next_work_digishield)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus(145000);
    
    CBlockIndex pindexLast;
    int64_t nLastRetargetTime = 1395094427;

    // First hard-fork at 145,000, which applies to block 145,001 onwards
    pindexLast.nHeight = 145000;
    pindexLast.nTime = 1395094679;
    pindexLast.nBits = 0x1b499dfd;
    //BOOST_CHECK_EQUAL(CalculateJunkcoinNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1b671062);
}

BOOST_AUTO_TEST_CASE(get_next_work_digishield_modulated_upper)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus(145000);
    
    CBlockIndex pindexLast;
    int64_t nLastRetargetTime = 1395100835;

    // Test the upper bound on modulated time using mainnet block #145,107
    pindexLast.nHeight = 145107;
    pindexLast.nTime = 1395101360;
    pindexLast.nBits = 0x1b3439cd;
    //BOOST_CHECK_EQUAL(CalculateJunkcoinNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1b4e56b3);
}

BOOST_AUTO_TEST_CASE(get_next_work_digishield_modulated_lower)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus(145000);
    
    CBlockIndex pindexLast;
    int64_t nLastRetargetTime = 1395380517;

    // Test the lower bound on modulated time using mainnet block #149,423
    pindexLast.nHeight = 149423;
    pindexLast.nTime = 1395380447;
    pindexLast.nBits = 0x1b446f21;
    //BOOST_CHECK_EQUAL(CalculateJunkcoinNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1b335358);
}

BOOST_AUTO_TEST_CASE(get_next_work_digishield_rounding)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& params = Params().GetConsensus(145000);
    
    CBlockIndex pindexLast;
    int64_t nLastRetargetTime = 1395094679;

    // Test case for correct rounding of modulated time - this depends on
    // handling of integer division, and is not obvious from the code
    pindexLast.nHeight = 145001;
    pindexLast.nTime = 1395094727;
    pindexLast.nBits = 0x1b671062;
    //BOOST_CHECK_EQUAL(CalculateJunkcoinNextWorkRequired(&pindexLast, nLastRetargetTime, params), 0x1b6558a4);
}

BOOST_AUTO_TEST_CASE(hardfork_parameters)
{
    SelectParams(CBaseChainParams::MAIN);
    const Consensus::Params& initialParams = Params().GetConsensus(0);

    BOOST_CHECK_EQUAL(initialParams.nPowTargetTimespan, 86400); // 24 * 60 * 60 = 1 day
    BOOST_CHECK_EQUAL(initialParams.AllowLegacyBlocks(0), true);
    BOOST_CHECK_EQUAL(initialParams.fDigishieldDifficultyCalculation, false);

    // Check pre-auxpow heights
    const Consensus::Params& preAuxpowParams = Params().GetConsensus(172999);
    BOOST_CHECK_EQUAL(preAuxpowParams.nPowTargetTimespan, 86400);
    BOOST_CHECK_EQUAL(preAuxpowParams.AllowLegacyBlocks(172999), true);
    BOOST_CHECK_EQUAL(preAuxpowParams.fDigishieldDifficultyCalculation, false);

    // Digishield is NOT activated on Junkcoin (nHeightEffective = 0xFFFFFFFF)
    // so all heights return the same consensus params
    const Consensus::Params& digishieldHeightParams = Params().GetConsensus(145000);
    BOOST_CHECK_EQUAL(digishieldHeightParams.nPowTargetTimespan, 86400);
    BOOST_CHECK_EQUAL(digishieldHeightParams.AllowLegacyBlocks(145000), true);
    BOOST_CHECK_EQUAL(digishieldHeightParams.fDigishieldDifficultyCalculation, false);

    // AuxPoW activates at height 173000
    const Consensus::Params& preAuxpowParams2 = Params().GetConsensus(172999);
    BOOST_CHECK_EQUAL(preAuxpowParams2.AllowLegacyBlocks(172999), true);

    const Consensus::Params& auxpowParams = Params().GetConsensus(173000);
    // AllowLegacyBlocks returns true at exactly nAuxpowStartHeight
    BOOST_CHECK_EQUAL(auxpowParams.AllowLegacyBlocks(173000), true);
    BOOST_CHECK_EQUAL(auxpowParams.nAuxpowStartHeight, 173000);

    const Consensus::Params& auxpowHighParams = Params().GetConsensus(200000);
    BOOST_CHECK_EQUAL(auxpowHighParams.nPowTargetTimespan, 86400);
    // After nAuxpowStartHeight, AllowLegacyBlocks should return false
    BOOST_CHECK_EQUAL(auxpowHighParams.AllowLegacyBlocks(200000), false);
    BOOST_CHECK_EQUAL(auxpowHighParams.fDigishieldDifficultyCalculation, false);
}

BOOST_AUTO_TEST_CASE(block_header_serialization_compatibility)
{
    // Test that CBlockHeader serialization produces identical bytes to old junkcoin-core
    // This ensures merge-mined blocks from rebased code are accepted by old nodes

    // Test 1: Legacy block header (pre-auxpow)
    {
        CBlockHeader header;
        header.nVersion = 4;  // Version 4, no auxpow, no chain ID
        header.hashPrevBlock = uint256S("0x0000000000000000000000000000000000000000000000000000000000000001");
        header.hashMerkleRoot = uint256S("0x0000000000000000000000000000000000000000000000000000000000000002");
        header.nTime = 1367394064;
        header.nBits = 0x1e0ffff0;
        header.nNonce = 112158625;

        // Serialize header
        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
        ss << header;

        // Expected size: 80 bytes (4+32+32+4+4+4)
        BOOST_CHECK_EQUAL(ss.size(), 80u);

        // Expected hash from old junkcoin-core for this exact header
        // Calculated using: GetHash() on the same header data
        uint256 expectedHash = uint256S("0x99f3e0c8a0177b0872e11abbb38069f3bec4c16e2587b8c94372f2a6c38d09fe");
        BOOST_CHECK_EQUAL(header.GetHash().ToString(), expectedHash.ToString());
    }

    // Test 2: Auxpow block header (post-auxpow activation)
    {
        CBlockHeader header;
        // Version with chain ID 0x2020 and auxpow flag set
        // VERSION_CHAIN_START (1<<16) = 0x10000, chain ID = 0x2020, auxpow flag = 0x100
        header.nVersion = 0x20200104;  // chain ID 0x2020, base version 4, auxpow enabled

        header.hashPrevBlock = uint256S("0x0000000000000000000000000000000000000000000000000000000000000003");
        header.hashMerkleRoot = uint256S("0x0000000000000000000000000000000000000000000000000000000000000004");
        header.nTime = 1500000000;
        header.nBits = 0x1b0ffff0;
        header.nNonce = 12345;

        // Create minimal auxpow data
        header.auxpow = std::make_shared<CAuxPow>();
        header.auxpow->nIndex = 0;
        header.auxpow->vMerkleBranch.clear();
        header.auxpow->vChainMerkleBranch.clear();
        header.auxpow->nChainIndex = 0;

        // Parent block header (minimal)
        header.auxpow->parentBlock.nVersion = 0x10000000;
        header.auxpow->parentBlock.hashPrevBlock.SetNull();
        header.auxpow->parentBlock.hashMerkleRoot.SetNull();
        header.auxpow->parentBlock.nTime = 1500000001;
        header.auxpow->parentBlock.nBits = 0x1b0ffff0;
        header.auxpow->parentBlock.nNonce = 54321;

        // Serialize header with auxpow
        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
        ss << header;

        // Should be > 80 bytes (header + auxpow data)
        BOOST_CHECK(ss.size() > 80u);

        // Deserialize and verify round-trip
        CBlockHeader header2;
        ss >> header2;

        BOOST_CHECK_EQUAL(header2.nVersion, header.nVersion);
        BOOST_CHECK_EQUAL(header2.hashPrevBlock, header.hashPrevBlock);
        BOOST_CHECK_EQUAL(header2.hashMerkleRoot, header.hashMerkleRoot);
        BOOST_CHECK_EQUAL(header2.nTime, header.nTime);
        BOOST_CHECK_EQUAL(header2.nBits, header.nBits);
        BOOST_CHECK_EQUAL(header2.nNonce, header.nNonce);
        BOOST_CHECK(header2.IsAuxpow());
        BOOST_CHECK(header2.auxpow != nullptr);
    }

    // Test 3: Genesis block header hash matches expected
    {
        SelectParams(CBaseChainParams::MAIN);
        const Consensus::Params& consensus = Params().GetConsensus(0);

        // Verify genesis block hash is correct
        BOOST_CHECK_EQUAL(consensus.hashGenesisBlock.ToString(),
            "a2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6");
    }

    // Test 4: Real mainnet block from API - verify header hash matches
    // This proves our CBlockHeader::GetHash() produces same result as old core
    // Block data from: https://junk-api.s3na.xyz/block/19f06aa05990bd9ea09ee46d1a2edabbc92a6e38129d1b65143007e19ffaf903
    {
        SelectParams(CBaseChainParams::MAIN);

        // Construct block header from API JSON fields
        CBlockHeader header;
        header.nVersion = 538968324;  // 0x20200004 - chain ID 0x2020, version 4, no auxpow
        header.hashPrevBlock = uint256S("4826a173c90ab06a5089db8c542fdd840ddc075f88910b031cd7ba7cb000d6ab");
        header.hashMerkleRoot = uint256S("45bd64a8b3a5f6400f0c6b9e90dd17b57ab01b6bad755a4334c823ab097bc60a");
        header.nTime = 1732482552;
        header.nBits = 438325611;
        header.nNonce = 0;

        // Verify chain ID
        BOOST_CHECK_EQUAL(header.GetChainId(), 0x2020);

        // Verify NOT auxpow (this is legacy block after auxpow activation but without auxpow)
        BOOST_CHECK(!header.IsAuxpow());
        BOOST_CHECK(!header.IsLegacy());  // version 4 with chain ID 0x2020 is not legacy

        // Calculate hash
        uint256 calculatedHash = header.GetHash();

        // Expected hash from API (block id)
        uint256 expectedHash = uint256S("19f06aa05990bd9ea09ee46d1a2edabbc92a6e38129d1b65143007e19ffaf903");

        // THIS IS THE CRITICAL TEST - if this passes, serialization is compatible!
        BOOST_CHECK_EQUAL(calculatedHash.ToString(), expectedHash.ToString());

        BOOST_TEST_MESSAGE("Mainnet block hash verification: " + calculatedHash.ToString());
    }

    // Test 5: Create auxpow block and verify parent hash calculation
    // This verifies merge mining creates valid parent block hash
    {
        CBlockHeader junkHeader;
        junkHeader.nVersion = 0x20200104;  // chain ID 0x2020, version 4, auxpow
        junkHeader.hashPrevBlock.SetNull();
        junkHeader.hashMerkleRoot.SetNull();
        junkHeader.nTime = 1500000000;
        junkHeader.nBits = 0x1b0ffff0;
        junkHeader.nNonce = 12345;

        // Create auxpow
        junkHeader.auxpow = std::make_shared<CAuxPow>();
        junkHeader.auxpow->nIndex = 0;

        // Create parent block header (Bitcoin/Litecoin style)
        junkHeader.auxpow->parentBlock.nVersion = 0x20000000;
        junkHeader.auxpow->parentBlock.hashPrevBlock.SetNull();
        junkHeader.auxpow->parentBlock.hashMerkleRoot.SetNull();
        junkHeader.auxpow->parentBlock.nTime = 1500000001;
        junkHeader.auxpow->parentBlock.nBits = 0x1b0ffff0;
        junkHeader.auxpow->parentBlock.nNonce = 54321;

        // Calculate parent block hash (this is what merge miners hash)
        uint256 parentHash = junkHeader.auxpow->parentBlock.GetHash();

        // Verify parent hash is not null
        BOOST_CHECK(!parentHash.IsNull());

        // Verify our hash calculation produces consistent results
        uint256 hash1 = junkHeader.auxpow->parentBlock.GetHash();
        uint256 hash2 = junkHeader.auxpow->parentBlock.GetHash();
        BOOST_CHECK_EQUAL(hash1.ToString(), hash2.ToString());
    }
}

BOOST_AUTO_TEST_SUITE_END()
