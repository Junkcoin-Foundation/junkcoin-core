// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2022 The Junkcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1386325540, nBits=0x1e0ffff0, nNonce=99943, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Wed May 1, 2013: Spot gold fell 1.3 percent to $1,457.90 an ounce by 3:11 p.m. EDT (1911 GMT)";
    const CScript genesisOutputScript = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
private:
    Consensus::Params digishieldConsensus;
    Consensus::Params auxpowConsensus;
    Consensus::Params minDifficultyConsensus;
public:
    CMainParams() {
        strNetworkID = "main";

        // Halving schedule
        consensus.nFirstHalvingHeight = 262800;      // January 2024
        consensus.nSecondHalvingHeight = 394200;     // After 3 months
        consensus.nThirdHalvingHeight = 657000;      // After 10 months
        consensus.nFourthHalvingHeight = 1182600;    // After 22 months
        consensus.nSubsidyHalvingInterval = 788400;  // Every 1.5 years after fourth halving

        consensus.nMajorityEnforceBlockUpgrade = 1500;
        consensus.nMajorityRejectBlockOutdated = 1900;
        consensus.nMajorityWindow = 2000;

        // After deployments are activated we can change it
        consensus.BIP34Hash = uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"); //genesis
        consensus.BIP65Height = 0x210c; //set
        consensus.BIP66Height = 0x210c;

        consensus.powLimit = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 20;
        consensus.nPowTargetTimespan = 24 * 60 * 60; // pre-digishield: 1 day
        consensus.nPowTargetSpacing = 60; // 1 minute
        consensus.nCoinbaseMaturity = 70; //confirm
        consensus.fPowNoRetargeting = false;
        consensus.nStrictBlockTimeActivationHeight = 262800; // Activation height for strict block timing

        consensus.nRuleChangeActivationThreshold = 9576; // 95% of 10,080
        consensus.nMinerConfirmationWindow = 10080; // 60 * 24 * 7 = 10,080 blocks, or one week

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nStartTime = 1534490155;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nTimeout   = 1764490155;   // 2024-12-25 00:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nStartTime = 1534490155;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nTimeout   = 1764490155;   // 2024-12-25 18:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nStartTime = 1534490155;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nTimeout   = 1764490155;   // 2024-12-25 18:00:00

        // Deployment of BIP68, BIP112, and BIP113.
        // XXX: BIP heights and hashes all need to be updated to JunkCoin values
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1724732207; // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1764490155;   // 2024-12-25 18:00:00

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1724732207; // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1764490155;   // 2024-12-25 18:00:00

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); // 4,303,965

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"); //check

        // AuxPoW parameters
        consensus.nAuxpowChainId = 0x2020;
        consensus.fStrictChainId = true;  //changed to true
        consensus.nBlockAfterAuxpowRewardThreshold = 5;
        consensus.nAuxpowStartHeight = 173000;

        //consensus.fAllowLegacyBlocks = true;

        // We do not activate digishield in this consensus
        digishieldConsensus = consensus;

        digishieldConsensus.nHeightEffective = 0xFFFFFFFF; // like never

        digishieldConsensus.fSimplifiedRewards = true;
        digishieldConsensus.fDigishieldDifficultyCalculation = true;
        digishieldConsensus.nPowTargetTimespan = 60; // post-digishield: 1 minute
        digishieldConsensus.nCoinbaseMaturity = 70;

        // Not implementing digishield yet
        minDifficultyConsensus = digishieldConsensus;
        minDifficultyConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();;
        minDifficultyConsensus.fPowAllowDigishieldMinDifficultyBlocks = true;
        minDifficultyConsensus.fPowAllowMinDifficultyBlocks = false;

        // Not implementing AuxPow hardfork yet
        auxpowConsensus = digishieldConsensus;
        auxpowConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();
        //auxpowConsensus.fAllowLegacyBlocks = true;

        // Assemble the binary search tree of consensus parameters
        pConsensusRoot = &digishieldConsensus;
        digishieldConsensus.pLeft = &consensus;
        digishieldConsensus.pRight = &auxpowConsensus;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xfb;
        pchMessageStart[1] = 0xc0;
        pchMessageStart[2] = 0xb6;
        pchMessageStart[3] = 0xdb;
        nDefaultPort = 9771;
        nPruneAfterHeight = 518400;

        genesis = CreateGenesisBlock(1367394064, 112158625, 0x1e0ffff0, 1, 50 * COIN); // 12097647

        consensus.hashGenesisBlock = genesis.GetHash();
        digishieldConsensus.hashGenesisBlock = consensus.hashGenesisBlock;
        auxpowConsensus.hashGenesisBlock = consensus.hashGenesisBlock;

        assert(consensus.hashGenesisBlock == uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"));
        assert(genesis.hashMerkleRoot == uint256S("0x3de124b0274307911fe12550e96bf76cb92c12835db6cb19f82658b8aca1dbc8"));

        // DNS seeds
        vSeeds.push_back(CDNSSeedData("junk-coin.com", "mainnet.junk-coin.com", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,16);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,144);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true; //change back
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        /*checkpointData = (CCheckpointData) {
                boost::assign::map_list_of
                        (      0, uint256S("0x1a91e3dace36e2be3bf030a65679fe821aa1d6ef92e7c9902eb318182c355691"))
                        ( 14000, uint256S("0x0f3bec6968ad87171063c28f696f133c0e4bdf3a0bd56d369be658367dd21ee9"))
        };

        chainTxData = ChainTxData{
                // Data as of block ed7d266dcbd8bb8af80f9ccb8deb3e18f9cc3f6972912680feeb37b090f8cee0 (height 4303965).
                // Tx estimate based on average between 2021-07-01 (3793538) and 2022-07-01 (4288126)
                1702700138, // * UNIX timestamp of last checkpoint block
                15423,   // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
                0.1        // * estimated number of transactions per second after checkpoint
        };*/

        checkpointData = (CCheckpointData) {
                boost::assign::map_list_of
                (      0, uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"))
                (     1, uint256S("0xca55073a54775a1ef78294f53f38a3e02d0654d7417f3cbbe4d28d17d50e07d0"))
                (    53, uint256S("0xb623a39a5a0534990a59916d5803fa2bd6a6d52d8e594546936a42a2cc9b0441"))
                (   117, uint256S("0x6cab49bd69fcce2bb48793cc064bb49e75f068e7029b5173db83654fbcb5953d"))
                (   200, uint256S("0x45257b0f2ee6d5c55ac16a76817d7151b776d6452ae6f21426eaa42345b831f8"))
                (  6452, uint256S("0x506562c2172d9f10e86d2b467ed3bb7b9eba40148d18d1e660c1ff692604f3fc"))
                ( 10978, uint256S("0x1c9f7f7a4702f8225df430b259ac58c387de99439be8a8789841a1c011ead7fc"))
                ( 17954, uint256S("0x6036051659e92a17cb7488040e05a94483b7a7f88b184156c136d51ff0390a7d"))
                ( 23978, uint256S("0x7924154aa896363ec9be3ca5f939602f72cf4a5396e6e1cd9139335dd1819487"))
                ( 33212, uint256S("0x448040ac454da8654d9c58ad79386aa1a88fd113be0fcc5ca39ecd3eae8c8618"))
                ( 45527, uint256S("0xf2420d964001d4d2c8bc0d9283f3f684d4d91a509a50985888458a68e08e1c82"))
                ( 57484, uint256S("0xc3e95c6fb35f4b39006c89538415b4f50a253a3ac1cad0e583fb287f6bd91be1"))
                ( 69240, uint256S("0xc34f5d113fe92f3206ef8855caf51cd6252286e3381b253bbc1237211198c22b"))
                ( 73892, uint256S("0xd05129c2d9f3e99565bf84fbceabbc61728e4d644173e194823b639f7c406b04"))
                ( 168312, uint256S("0xdeea2bcecb1146ae9cd74d67b29b4d0161e9bb63beb9022ca10f3625dda6c0e6"))
        };

        chainTxData = ChainTxData{
                // Data as of block ed7d266dcbd8bb8af80f9ccb8deb3e18f9cc3f6972912680feeb37b090f8cee0 (height 4303965).
                // Tx estimate based on average between 2021-07-01 (3793538) and 2022-07-01 (4287924154aa896363ec9be3ca5f939602f72cf4a5396e6e1cd9139335dd18194878126)
                0, // * UNIX timestamp of last checkpoint block
                0,   // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
                0        // * estimated number of transactions per second after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
private:
    Consensus::Params digishieldConsensus;
    Consensus::Params auxpowConsensus;
    Consensus::Params minDifficultyConsensus;
public:
    CTestNetParams() {
        strNetworkID = "test";

        // Testnet halving schedule: starts at 2880 with subsequent halvings every 28800 blocks
        consensus.nFirstHalvingHeight = 2880;        // First halving
        consensus.nSecondHalvingHeight = 31680;      // 2880 + (10 * 2880)
        consensus.nThirdHalvingHeight = 60480;       // 31680 + (10 * 2880)
        consensus.nFourthHalvingHeight = 89280;      // 60480 + (10 * 2880)
        consensus.nSubsidyHalvingInterval = 28800;   // 10 * 2880 blocks between halvings

        consensus.nMajorityEnforceBlockUpgrade = 1500;
        consensus.nMajorityRejectBlockOutdated = 1900;
        consensus.nMajorityWindow = 2000;


        // After deployments are activated we can change it
        consensus.BIP34Hash = uint256S("0x00"); // unused for now.
        consensus.BIP65Height = 99999999;
        consensus.BIP65Height = 99999999;
        consensus.BIP66Height = 99999999;


        consensus.powLimit = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 20;
        consensus.nPowTargetTimespan = 4 * 60 * 60; // pre-digishield: 4 hours
        consensus.nPowTargetSpacing = 60; // 1 minute
        consensus.nCoinbaseMaturity = 30;
        consensus.fPowNoRetargeting = false;
        consensus.nStrictBlockTimeActivationHeight = 2880; // Activation height for strict block timing on testnet

        consensus.nRuleChangeActivationThreshold = 9576; // 95% of 10,080
        consensus.nMinerConfirmationWindow = 10080; // 60 * 24 * 7 = 10,080 blocks, or one week

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nTimeout   = 1735084800;   // 2024-12-25 00:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nTimeout   = 1735084800;   // 2024-12-25 18:00:00


        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nTimeout   = 1735084800;   // 2024-12-25 18:00:00


        // Deployment of BIP68, BIP112, and BIP113.
        // XXX: BIP heights and hashes all need to be updated to JunkCoin values
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1703462400; // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1735084800;   // 2024-12-25 18:00:00

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1703462400; // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1735084800;   // 2024-12-25 18:00:00

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); // 4,303,965

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"); // 10000

        // AuxPoW parameters
        consensus.nAuxpowChainId =  0x2021; // 98 - Josh Wise!
        //consensus.fAllowLegacyBlocks = true;
        consensus.nAuxpowStartHeight = 0; // -1 will always allow legacy blocks
        consensus.nBlockAfterAuxpowRewardThreshold = 5;
        consensus.fStrictChainId = true;

        // We do not activate digishield in this consensus
        digishieldConsensus = consensus;

        digishieldConsensus.nHeightEffective = 0xFFFFFFFF; // like never

        digishieldConsensus.fSimplifiedRewards = true;
        digishieldConsensus.fDigishieldDifficultyCalculation = true;
        digishieldConsensus.nPowTargetTimespan = 60; // post-digishield: 1 minute
        digishieldConsensus.nCoinbaseMaturity = 240;

        // Not implementing digishield yet
        minDifficultyConsensus = digishieldConsensus;
        minDifficultyConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();;
        minDifficultyConsensus.fPowAllowDigishieldMinDifficultyBlocks = true;
        minDifficultyConsensus.fPowAllowMinDifficultyBlocks = true;

        // Not implementing AuxPow hardfork yet
        auxpowConsensus = digishieldConsensus;
        auxpowConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();
        //auxpowConsensus.fAllowLegacyBlocks = true;

        // Assemble the binary search tree of consensus parameters
        pConsensusRoot = &digishieldConsensus;
        digishieldConsensus.pLeft = &consensus;
        digishieldConsensus.pRight = &auxpowConsensus;

        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdc;

        nDefaultPort = 19771;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1367394064, 112158625, 0x1e0ffff0, 1, 50 * COIN);

        consensus.hashGenesisBlock = genesis.GetHash();
        digishieldConsensus.hashGenesisBlock = consensus.hashGenesisBlock;
        auxpowConsensus.hashGenesisBlock = consensus.hashGenesisBlock;

        assert(consensus.hashGenesisBlock == uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"));
        assert(genesis.hashMerkleRoot == uint256S("0x3de124b0274307911fe12550e96bf76cb92c12835db6cb19f82658b8aca1dbc8"));

        //assert(consensus.hashGenesisBlock == uint256S("0x324635c8e36f663b0adb126a21ad0bd7fa43cc5c5f15aec992bf4dde650bc0ea"));
        //assert(genesis.hashMerkleRoot == uint256S("0x6f80efd038566e1e3eab3e1d38131604d06481e77f2462235c6a9a94b1f8abf9"));

        // DNS seeds
        vSeeds.push_back(CDNSSeedData("junk-coin.com", "testnet.junk-coin.com", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111); // Testnet addresses start with 'm' or 'n'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196); // Testnet script addresses start with '2'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239); // Testnet private keys start with '9' or 'c'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x02)(0xfa)(0xca)(0xfd).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x02)(0xfa)(0xc3)(0x98).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"))
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
private:
    Consensus::Params digishieldConsensus;
    Consensus::Params auxpowConsensus;
    Consensus::Params minDifficultyConsensus;
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        // Testnet halving schedule: starts at 2880 with subsequent halvings every 28800 blocks
        consensus.nFirstHalvingHeight = 2880;        // First halving
        consensus.nSecondHalvingHeight = 31680;      // 2880 + (10 * 2880)
        consensus.nThirdHalvingHeight = 60480;       // 31680 + (10 * 2880)
        consensus.nFourthHalvingHeight = 89280;      // 60480 + (10 * 2880)
        consensus.nSubsidyHalvingInterval = 28800;   // 10 * 2880 blocks between halvings

        consensus.nMajorityEnforceBlockUpgrade = 1500;
        consensus.nMajorityRejectBlockOutdated = 1900;
        consensus.nMajorityWindow = 2000;


        // After deployments are activated we can change it
        consensus.BIP34Hash = uint256S("0x00"); // unused for now.
        consensus.BIP65Height = 99999999;
        consensus.BIP65Height = 99999999;
        consensus.BIP66Height = 99999999;


        consensus.powLimit = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~uint256(0) >> 20;
        consensus.nPowTargetTimespan = 4 * 60 * 60; // pre-digishield: 4 hours
        consensus.nPowTargetSpacing = 60; // 1 minute
        consensus.nCoinbaseMaturity = 30;
        consensus.fPowNoRetargeting = false;


        consensus.nRuleChangeActivationThreshold = 9576; // 95% of 10,080
        consensus.nMinerConfirmationWindow = 10080; // 60 * 24 * 7 = 10,080 blocks, or one week

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nTimeout   = 1735084800;   // 2024-12-25 00:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nTimeout   = 1735084800;   // 2024-12-25 18:00:00


        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nTimeout   = 1735084800;   // 2024-12-25 18:00:00


        // Deployment of BIP68, BIP112, and BIP113.
        // XXX: BIP heights and hashes all need to be updated to JunkCoin values
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1703462400; // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1735084800;   // 2024-12-25 18:00:00

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1703462400; // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1735084800;   // 2024-12-25 18:00:00

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000003e3c33bc605e5d"); // 4,303,965

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x2c05ea6918e28ca2d216c6518940c8782c09bebfe705d792155465662e275351"); // 10000

        // AuxPoW parameters
        consensus.nAuxpowChainId = 0x2021; // 98 - Josh Wise!
        consensus.nAuxpowStartHeight = 0; // -1 will always allow legacy blocks
        consensus.nBlockAfterAuxpowRewardThreshold = 5;
        consensus.fStrictChainId = true;

        // We do not activate digishield in this consensus
        digishieldConsensus = consensus;

        digishieldConsensus.nHeightEffective = 0xFFFFFFFF; // like never

        digishieldConsensus.fSimplifiedRewards = true;
        digishieldConsensus.fDigishieldDifficultyCalculation = true;
        digishieldConsensus.nPowTargetTimespan = 60; // post-digishield: 1 minute
        digishieldConsensus.nCoinbaseMaturity = 240;

        // Not implementing digishield yet
        minDifficultyConsensus = digishieldConsensus;
        minDifficultyConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();;
        minDifficultyConsensus.fPowAllowDigishieldMinDifficultyBlocks = true;
        minDifficultyConsensus.fPowAllowMinDifficultyBlocks = true;

        // Not implementing AuxPow hardfork yet
        auxpowConsensus = digishieldConsensus;
        auxpowConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();
        //auxpowConsensus.fAllowLegacyBlocks = false;

        // Assemble the binary search tree of consensus parameters
        pConsensusRoot = &digishieldConsensus;
        digishieldConsensus.pLeft = &consensus;
        digishieldConsensus.pRight = &auxpowConsensus;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdc;
        nDefaultPort = 19771;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1369199888, 12097647, 0x1e0ffff0, 1, 50 * COIN);

        consensus.hashGenesisBlock = genesis.GetHash();
        digishieldConsensus.hashGenesisBlock = consensus.hashGenesisBlock;
        auxpowConsensus.hashGenesisBlock = consensus.hashGenesisBlock;

        //assert(consensus.hashGenesisBlock == uint256S("0x324635c8e36f663b0adb126a21ad0bd7fa43cc5c5f15aec992bf4dde650bc0ea"));
        //assert(genesis.hashMerkleRoot == uint256S("0x6f80efd038566e1e3eab3e1d38131604d06481e77f2462235c6a9a94b1f8abf9"));

        // nodes with support for servicebits filtering should be at the top
        //vSeeds.push_back(CDNSSeedData("belscan.io", "testnetseed.belscan.io", true));
        //vSeeds.push_back(CDNSSeedData("belscan.io", "testnetseeder.belscan.io", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,47);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,153);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x02)(0xfa)(0xca)(0xfd).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x02)(0xfa)(0xc3)(0x98).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = (CCheckpointData){
                boost::assign::map_list_of
                        ( 0, uint256S("0x324635c8e36f663b0adb126a21ad0bd7fa43cc5c5f15aec992bf4dde650bc0ea"))
        };

        chainTxData = ChainTxData{
                0,
                0,
                0
        };
    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

const Consensus::Params *Consensus::Params::GetConsensus(uint32_t nTargetHeight) const {
    if (nTargetHeight < this -> nHeightEffective && this -> pLeft != NULL) {
        return this -> pLeft -> GetConsensus(nTargetHeight);
    } else if (nTargetHeight > this -> nHeightEffective && this -> pRight != NULL) {
        const Consensus::Params *pCandidate = this -> pRight -> GetConsensus(nTargetHeight);
        if (pCandidate->nHeightEffective <= nTargetHeight) {
            return pCandidate;
        }
    }

    // No better match below the target height
    return this;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
        return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
        return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}
