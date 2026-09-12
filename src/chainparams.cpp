// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <hash.h> // for signet block challenge hash
#include <tinyformat.h>
#include <util/system.h>
#include <util/strencodings.h>
#include <versionbitsinfo.h>
#include <script/standard.h>
#include <key_io.h>

#include <assert.h>
#include <stdexcept>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

static void ValidateOpcodeActivationOrdering(const Consensus::Params& consensus)
{
    if (consensus.TaprootHeight < consensus.DisabledScriptReactivationHeight) {
        throw std::runtime_error("TaprootHeight must not precede DisabledScriptReactivationHeight");
    }
}

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
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
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
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = CBaseChainParams::MAIN;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 100000; // Not used in Junkcoin (custom schedule)
        consensus.BIP16Height = 0; // Always enforce P2SH
        consensus.BIP34Height = 0x210c; // 8460
        consensus.BIP34Hash = uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"); // genesis
        consensus.BIP65Height = 0x210c; // 8460
        consensus.BIP66Height = 0x210c; // 8460
        consensus.CSVHeight = 1145000; // Activate CSV at block 1,145,000
        consensus.SegwitHeight = 1145000; // Activate SegWit at block 1,145,000
        // Junkcoin: Re-enable legacy disabled opcodes (OP_CAT, OP_MUL, etc.)
        consensus.DisabledScriptReactivationHeight = 1155000; // Re-enable opcodes at block 1,155,000
        consensus.TaprootHeight = 1155000; // Activate Taproot at block 1,155,000
        ValidateOpcodeActivationOrdering(consensus);
        consensus.MWEBHeight = 1165000; // Activate MWEB at block 1,165,000
        consensus.mweb_pegout_feature_activation_height = consensus.MWEBHeight;
        consensus.mweb_extradata_feature_activation_height = consensus.MWEBHeight;
        consensus.MinBIP9WarningHeight = 10080 + 10080; // miner confirmation window
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // 1 day
        consensus.nPowTargetSpacing = 60; // 1 minute
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 9576; // 95% of 10,080
        consensus.nMinerConfirmationWindow = 10080; // 60 * 24 * 7 = 10,080 blocks, or one week
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nStartTime = 1534490155;   // 2023-12-25 00:00:00 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nTimeout = 1764490155;    // 2025-12-25 18:00:00 UTC

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nStartTime = 1534490155;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nTimeout = 1764490155;

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nStartTime = 1534490155;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nTimeout = 1764490155;

        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1724732207;     // 2024-09-26 05:16:47 UTC
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1764490155;      // 2025-12-25 18:00:00 UTC

        // Junkcoin: SegWit disabled - compatible with legacy core
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartHeight = std::numeric_limits<int>::max();
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeoutHeight = std::numeric_limits<int>::max();

        // Junkcoin: Taproot disabled - compatible with legacy core
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartHeight = std::numeric_limits<int>::max();
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeoutHeight = std::numeric_limits<int>::max();

        // Junkcoin: MWEB disabled - compatible with legacy core
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartHeight = std::numeric_limits<int>::max();
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeoutHeight = std::numeric_limits<int>::max();

        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.defaultAssumeValid = uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"); // genesis
        consensus.nCoinbaseMaturity = 70; // Junkcoin mainnet: 70 blocks

        // AuxPoW parameters
        consensus.nAuxpowChainId = 0x2020; // 8224 in decimal
        consensus.fStrictChainId = true;
        consensus.nAuxpowStartHeight = 173000;
        consensus.nBlockAfterAuxpowRewardThreshold = 5; // Match junkcoin-core
        consensus.nLegacyBlocksBefore = -1; // Always allow legacy blocks

        // Base consensus is effective from genesis. It is the left leaf of the
        // consensus tree; digishield (the root) never activates. Matches legacy
        // junkcoin-core, which leaves the base consensus nHeightEffective at 0.
        consensus.nHeightEffective = 0;

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
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;

        genesis = CreateGenesisBlock(1367394064, 112158625, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"));
        assert(genesis.hashMerkleRoot == uint256S("0x3de124b0274307911fe12550e96bf76cb92c12835db6cb19f82658b8aca1dbc8"));

        // DNS seeds for dynamic peer discovery (resolve to current seed nodes)
        // Fixed seeds (hardcoded IPs) are in chainparamsseeds.h as fallback
        vSeeds.emplace_back("mainnet.junk-coin.com");
        vSeeds.emplace_back("junk-seed.s3na.xyz");
        vSeeds.emplace_back("jkc-seed.junkiewally.xyz");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,16);  // Legacy addresses start with '7'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);   // Script addresses start with '3'
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,5);  // Script addresses
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,144); // WIF private keys start with 'N'
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "jc";
        mweb_hrp = "jcmweb";

        // Fixed seeds - hardcoded IPs from DNS seeds
        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_main), std::end(chainparams_seed_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {      0, uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6")},
                {      1, uint256S("0xca55073a54775a1ef78294f53f38a3e02d0654d7417f3cbbe4d28d17d50e07d0")},
                {     53, uint256S("0xb623a39a5a0534990a59916d5803fa2bd6a6d52d8e594546936a42a2cc9b0441")},
                {    117, uint256S("0x6cab49bd69fcce2bb48793cc064bb49e75f068e7029b5173db83654fbcb5953d")},
                {    200, uint256S("0x45257b0f2ee6d5c55ac16a76817d7151b776d6452ae6f21426eaa42345b831f8")},
                {   6452, uint256S("0x506562c2172d9f10e86d2b467ed3bb7b9eba40148d18d1e660c1ff692604f3fc")},
                {  10978, uint256S("0x1c9f7f7a4702f8225df430b259ac58c387de99439be8a8789841a1c011ead7fc")},
                {  17954, uint256S("0x6036051659e92a17cb7488040e05a94483b7a7f88b184156c136d51ff0390a7d")},
                {  23978, uint256S("0x7924154aa896363ec9be3ca5f939602f72cf4a5396e6e1cd9139335dd1819487")},
                {  33212, uint256S("0x448040ac454da8654d9c58ad79386aa1a88fd113be0fcc5ca39ecd3eae8c8618")},
                {  45527, uint256S("0xf2420d964001d4d2c8bc0d9283f3f684d4d91a509a50985888458a68e08e1c82")},
                {  57484, uint256S("0xc3e95c6fb35f4b39006c89538415b4f50a253a3ac1cad0e583fb287f6bd91be1")},
                {  69240, uint256S("0xc34f5d113fe92f3206ef8855caf51cd6252286e3381b253bbc1237211198c22b")},
                {  73892, uint256S("0xd05129c2d9f3e99565bf84fbceabbc61728e4d644173e194823b639f7c406b04")},
                { 168312, uint256S("0xdeea2bcecb1146ae9cd74d67b29b4d0161e9bb63beb9022ca10f3625dda6c0e6")},
                { 172000, uint256S("0x6208026838cd18d3f401f3c5c2305105a3aafeb63164827d3e341f356cb3529a")},
            }
        };

        chainTxData = ChainTxData{
            /* nTime    */ 0,
            /* nTxCount */ 0,
            /* dTxRate  */ 0
        };

        // Development Fund - 20% of block reward goes to dev fund
        vDevelopmentFundAddress = {
            "3P3UvT6vdDJVrbB2mn6WrP8gywpu2Knx8C",
            "34cGTrxRD4VvfbDri6RhQDKPBokfLTNJse",
            "37NpTG2p6gjVeZDmAiPLKNs6Nhj5EfTR55"
        };
        vDevelopmentFundStartHeight = 365000;
        vDevelopmentFundLastHeight = 3547800;
        vDevelopmentFundPercent = 0.2; // 20% development fund
        assert(static_cast<int>(vDevelopmentFundAddress.size()) <= GetLastDevelopmentFundBlockHeight());
        
        // Junkcoin: Setup consensus tree for height-based consensus rules
        // This allows different consensus parameters at different heights
        
        // Digishield consensus (not activated - set to never)
        digishieldConsensus = consensus;
        digishieldConsensus.nHeightEffective = 0xFFFFFFFF; // Never activated
        digishieldConsensus.fSimplifiedRewards = true;
        digishieldConsensus.fDigishieldDifficultyCalculation = true;
        digishieldConsensus.nPowTargetTimespan = 60; // 1 minute
        
        // Min difficulty consensus (not activated)
        minDifficultyConsensus = digishieldConsensus;
        minDifficultyConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();
        minDifficultyConsensus.fPowAllowDigishieldMinDifficultyBlocks = true;
        minDifficultyConsensus.fPowAllowMinDifficultyBlocks = false;
        
        // AuxPoW consensus (not activated as separate fork)
        auxpowConsensus = digishieldConsensus;
        auxpowConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();
        
        // Assemble the binary search tree of consensus parameters
        // Root is digishield, left is base consensus, right is auxpow
        pConsensusRoot = &digishieldConsensus;
        digishieldConsensus.pLeft = &consensus;
        digishieldConsensus.pRight = &auxpowConsensus;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = CBaseChainParams::TESTNET;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 100000; // Junkcoin (not Litecoin's 840000)
        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on testnet
        // After deployments are activated we can change it
        consensus.BIP34Hash = uint256S("0x00"); // unused for now.
        consensus.BIP65Height = 99999999;
        consensus.BIP66Height = 99999999;
        consensus.CSVHeight = 120000;      // Activate CSV at block 120,000
        consensus.SegwitHeight = 140000;   // Activate SegWit at block 140,000 (+20k)
        consensus.TaprootHeight = 160000;  // Activate Taproot at block 160,000 (+20k)
        consensus.DisabledScriptReactivationHeight = 160000; // Re-enable opcodes (OP_CAT, OP_MUL, etc.) (+20k)
        ValidateOpcodeActivationOrdering(consensus);
        consensus.MWEBHeight = 180000;     // Activate MWEB at block 180,000 (+20k)
        consensus.mweb_pegout_feature_activation_height = consensus.MWEBHeight;
        consensus.mweb_extradata_feature_activation_height = consensus.MWEBHeight;
        consensus.MinBIP9WarningHeight = 111440;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 4 * 60 * 60; // 4H - match junkcoin-core
        consensus.nPowTargetSpacing = 60; // Junkcoin: 1 minute
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 9576; // 95% of 10,080 - match junkcoin-core
        consensus.nMinerConfirmationWindow = 10080; // 60 * 24 * 7 = 10,080 blocks - match junkcoin-core
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // AuxPoW parameters
        consensus.nAuxpowChainId = 0x2021; // Match junkcoin-core testnet
        consensus.fStrictChainId = true;
        consensus.nAuxpowStartHeight = 0;
        consensus.nBlockAfterAuxpowRewardThreshold = 5; // Match junkcoin-core
        consensus.nLegacyBlocksBefore = -1; // Always allow legacy blocks

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nTimeout   = 1735084800;   // 2024-12-25 00:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nTimeout   = 1735084800;   // 2024-12-25 18:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nTimeout   = 1735084800;   // 2024-12-25 18:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1703462400; // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1735084800;   // 2024-12-25 18:00:00

        // Junkcoin testnet: SegWit scheduled activation
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartHeight = 140000;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeoutHeight = std::numeric_limits<int>::max();

        // Junkcoin testnet: Taproot scheduled activation
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartHeight = 160000;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeoutHeight = std::numeric_limits<int>::max();

        // Junkcoin testnet: MWEB scheduled activation
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartHeight = 180000;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeoutHeight = std::numeric_limits<int>::max();

        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.defaultAssumeValid = uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"); // Match junkcoin-core
        consensus.nCoinbaseMaturity = 30; // Junkcoin testnet: 30 blocks (matches old core)

        // Junkcoin testnet magic bytes and port
        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdc;
        nDefaultPort = 19771;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 4;
        m_assumed_chain_state_size = 1;

        // Junkcoin: Testnet uses same genesis as mainnet
        genesis = CreateGenesisBlock(1367394064, 112158625, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6"));
        assert(genesis.hashMerkleRoot == uint256S("0x3de124b0274307911fe12550e96bf76cb92c12835db6cb19f82658b8aca1dbc8"));

        vFixedSeeds.clear();
        // DNS seeds for testnet dynamic peer discovery
        vSeeds.emplace_back("testnet.junk-coin.com");
        vSeeds.emplace_back("junk-testnet.s3na.xyz");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);   // Match junkcoin-core
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x02, 0xfa, 0xca, 0xfd};  // Match junkcoin-core
        base58Prefixes[EXT_SECRET_KEY] = {0x02, 0xfa, 0xc3, 0x98};  // Match junkcoin-core

        bech32_hrp = "tjc";
        mweb_hrp = "tjcmweb";

        // Fixed seeds from chainparamsseeds.h
        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {0, uint256S("0xa2effa738145e377e08a61d76179c21703e13e48910b30a2a87f0dfe794b64c6")},
            }
        };

        chainTxData = ChainTxData{
            /* nTime    */ 0,
            /* nTxCount */ 0,
            /* dTxRate  */ 0,
        };

        // Development Fund - match junkcoin-core testnet
        vDevelopmentFundAddress = {
            "3PM5bKKhggNFzYjLnLsbUF7XHNSuA4bSVY",
            "39Ak9GuMHfpWL3VoTm9NigyELzPC5toiE4",
            "35tfGskRDxU3tWU3n5n2uvCqeHmGDKorVN"
        };
        vDevelopmentFundStartHeight = 90650;
        vDevelopmentFundLastHeight = 3547800;
        vDevelopmentFundPercent = 0.2;

        // Junkcoin: Setup consensus tree for height-based consensus rules
        digishieldConsensus = consensus;
        digishieldConsensus.nHeightEffective = 0xFFFFFFFF; // Never activated on testnet
        digishieldConsensus.fSimplifiedRewards = true;
        digishieldConsensus.fDigishieldDifficultyCalculation = true;
        digishieldConsensus.nPowTargetTimespan = 60; // 1 minute timespan when enabled

        minDifficultyConsensus = digishieldConsensus;
        minDifficultyConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();
        minDifficultyConsensus.fPowAllowDigishieldMinDifficultyBlocks = true;
        minDifficultyConsensus.fPowAllowMinDifficultyBlocks = true; // Match junkcoin-core

        auxpowConsensus = digishieldConsensus;
        auxpowConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();

        pConsensusRoot = &digishieldConsensus;
        digishieldConsensus.pLeft = &consensus;
        digishieldConsensus.pRight = &auxpowConsensus;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    explicit CRegTestParams(const ArgsManager& args) {
        strNetworkID =  CBaseChainParams::REGTEST;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 100000; // Match junkcoin-core
        consensus.BIP16Height = 0;
        consensus.BIP34Hash = uint256S("0x00"); // unused for now.
        consensus.BIP65Height = 99999999; // Disabled - match junkcoin-core
        consensus.BIP66Height = 99999999; // Disabled - match junkcoin-core
        consensus.CSVHeight = 0;
        consensus.SegwitHeight = 0;
        consensus.TaprootHeight = 0;
        consensus.DisabledScriptReactivationHeight = 0;
        ValidateOpcodeActivationOrdering(consensus);
        consensus.MWEBHeight = 1; // 0 breaks regtest mining: genesis would be MWEB-enabled without a HogEx tx, so the first mined block can never validate
        consensus.mweb_pegout_feature_activation_height = 0;
        consensus.mweb_extradata_feature_activation_height = 0;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // Match junkcoin-core
        consensus.nPowTargetTimespan = 4 * 60 * 60; // 4 hours - match junkcoin-core
        consensus.nPowTargetSpacing = 60; // Junkcoin: 1 minute block target
        consensus.fPowAllowMinDifficultyBlocks = false; // Match junkcoin-core base consensus
        consensus.fPowNoRetargeting = true; // Enable MineBlocksOnDemand() for regtest (new code derives this from fPowNoRetargeting)
        consensus.nRuleChangeActivationThreshold = 9576; // 95% of 10,080 - match junkcoin-core
        consensus.nMinerConfirmationWindow = 10080; // 60 * 24 * 7 = 10,080 blocks - match junkcoin-core

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // AuxPoW parameters - match junkcoin-core
        consensus.nAuxpowChainId = 0x2021; // 98 - Josh Wise!
        consensus.fStrictChainId = true;
        consensus.nAuxpowStartHeight = 0; // -1 will always allow legacy blocks
        consensus.nBlockAfterAuxpowRewardThreshold = 5; // Match junkcoin-core
        consensus.nLegacyBlocksBefore = -1; // Always allow legacy blocks

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP34].nTimeout   = 1735084800;   // 2024-12-25 00:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP66].nTimeout   = 1735084800;   // 2024-12-25 18:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nStartTime = 1703462400;   // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_BIP65].nTimeout   = 1735084800;   // 2024-12-25 18:00:00

        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1703462400; // 2023-12-25 00:00:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1735084800;   // 2024-12-25 18:00:00

        // Junkcoin: SegWit disabled - compatible with junkcoin-core
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartHeight = std::numeric_limits<int>::max();
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeoutHeight = std::numeric_limits<int>::max();

        // Junkcoin: Taproot disabled - compatible with junkcoin-core
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartHeight = std::numeric_limits<int>::max();
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeoutHeight = std::numeric_limits<int>::max();

        // Junkcoin: MWEB disabled - compatible with junkcoin-core
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nStartHeight = std::numeric_limits<int>::max();
        consensus.vDeployments[Consensus::DEPLOYMENT_MWEB].nTimeoutHeight = std::numeric_limits<int>::max();

        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000003e3c33bc605e5d"); // Match junkcoin-core
        consensus.defaultAssumeValid = uint256S("0x2c05ea6918e28ca2d216c6518940c8782c09bebfe705d792155465662e275351"); // Match junkcoin-core
        consensus.nCoinbaseMaturity = 30; // Junkcoin regtest: 30 blocks (matches old core)

        pchMessageStart[0] = 0xfc;  // Match junkcoin-core
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdc;
        nDefaultPort = 19771;
        nPruneAfterHeight = 100000; // Match junkcoin-core
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        UpdateActivationParametersFromArgs(args);

        // Junkcoin regtest genesis - match junkcoin-core
        genesis = CreateGenesisBlock(1296688602, 12097647, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        // Junkcoin: No assertion for regtest (allows flexible testing)

        vFixedSeeds.clear();
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.
        // Fixed seeds from chainparamsseeds.h for regtest testing
        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_regtest), std::end(chainparams_seed_regtest));

        fDefaultConsistencyChecks = true;
        fRequireStandard = false; // Match junkcoin-core
        m_is_test_chain = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {
                // No checkpoints for regtest
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,47);  // Regtest specific - match junkcoin-core
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);   // Script addresses - match junkcoin-core
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,153); // Regtest specific - match junkcoin-core
        base58Prefixes[EXT_PUBLIC_KEY] = {0x02, 0xfa, 0xca, 0xfd};  // Match junkcoin-core
        base58Prefixes[EXT_SECRET_KEY] = {0x02, 0xfa, 0xc3, 0x98};  // Match junkcoin-core

        bech32_hrp = "rjc";
        mweb_hrp = "rjcmweb";

        // Development Fund - match junkcoin-core regtest
        vDevelopmentFundAddress = {
            "3PSpnu5Fdt34u2EEdHZjfaogcVCMC72h24"
        };
        vDevelopmentFundStartHeight = 1;
        vDevelopmentFundLastHeight = 150;
        vDevelopmentFundPercent = 0.2; // 20% development fund

        // Junkcoin: Setup consensus tree for height-based consensus rules
        digishieldConsensus = consensus;
        digishieldConsensus.nHeightEffective = 0xFFFFFFFF; // Never activated on regtest
        digishieldConsensus.fSimplifiedRewards = true;
        digishieldConsensus.fDigishieldDifficultyCalculation = true;
        digishieldConsensus.nPowTargetTimespan = 60; // 1 minute placeholder

        minDifficultyConsensus = digishieldConsensus;
        minDifficultyConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();
        minDifficultyConsensus.fPowAllowDigishieldMinDifficultyBlocks = true;
        minDifficultyConsensus.fPowAllowMinDifficultyBlocks = true; // Match junkcoin-core

        auxpowConsensus = digishieldConsensus;
        auxpowConsensus.nHeightEffective = std::numeric_limits<uint32_t>::max();

        pConsensusRoot = &digishieldConsensus;
        digishieldConsensus.pLeft = &consensus;
        digishieldConsensus.pRight = &auxpowConsensus;
    }

    /**
     * Allows modifying the Version Bits regtest parameters.
     */
    void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout, int64_t nStartHeight, int64_t nTimeoutHeight)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
        consensus.vDeployments[d].nStartHeight = nStartHeight;
        consensus.vDeployments[d].nTimeoutHeight = nTimeoutHeight;
    }
    void UpdateActivationParametersFromArgs(const ArgsManager& args);
};

void CRegTestParams::UpdateActivationParametersFromArgs(const ArgsManager& args)
{
    if (args.IsArgSet("-segwitheight")) {
        int64_t height = args.GetArg("-segwitheight", consensus.SegwitHeight);
        if (height < -1 || height >= std::numeric_limits<int>::max()) {
            throw std::runtime_error(strprintf("Activation height %ld for segwit is out of valid range. Use -1 to disable segwit.", height));
        } else if (height == -1) {
            LogPrintf("Segwit disabled for testing\n");
            height = std::numeric_limits<int>::max();
        }
        consensus.SegwitHeight = static_cast<int>(height);
    }

    if (!args.IsArgSet("-vbparams")) return;

    for (const std::string& strDeployment : args.GetArgs("-vbparams")) {
        std::vector<std::string> vDeploymentParams;
        boost::split(vDeploymentParams, strDeployment, boost::is_any_of(":"));
        if (vDeploymentParams.size() < 3 || 5 < vDeploymentParams.size()) {
            throw std::runtime_error("Version bits parameters malformed, expecting deployment:start:end[:heightstart:heightend]");
        }
        int64_t nStartTime, nTimeout, nStartHeight, nTimeoutHeight;
        if (!ParseInt64(vDeploymentParams[1], &nStartTime)) {
            throw std::runtime_error(strprintf("Invalid nStartTime (%s)", vDeploymentParams[1]));
        }
        if (!ParseInt64(vDeploymentParams[2], &nTimeout)) {
            throw std::runtime_error(strprintf("Invalid nTimeout (%s)", vDeploymentParams[2]));
        }
        if (vDeploymentParams.size() > 3 && !ParseInt64(vDeploymentParams[3], &nStartHeight)) {
            throw std::runtime_error(strprintf("Invalid nStartHeight (%s)", vDeploymentParams[3]));
        }
        if (vDeploymentParams.size() > 4 && !ParseInt64(vDeploymentParams[4], &nTimeoutHeight)) {
            throw std::runtime_error(strprintf("Invalid nTimeoutHeight (%s)", vDeploymentParams[4]));
        }
        bool found = false;
        for (int j=0; j < (int)Consensus::MAX_VERSION_BITS_DEPLOYMENTS; ++j) {
            if (vDeploymentParams[0] == VersionBitsDeploymentInfo[j].name) {
                UpdateVersionBitsParameters(Consensus::DeploymentPos(j), nStartTime, nTimeout, nStartHeight, nTimeoutHeight);
                found = true;
                LogPrintf("Setting version bits activation parameters for %s to start=%ld, timeout=%ld, start_height=%d, timeout_height=%d\n", vDeploymentParams[0], nStartTime, nTimeout, nStartHeight, nTimeoutHeight);
                break;
            }
        }
        if (!found) {
            throw std::runtime_error(strprintf("Invalid deployment (%s)", vDeploymentParams[0]));
        }
    }
}

static std::unique_ptr<const CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<const CChainParams> CreateChainParams(const ArgsManager& args, const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN) {
        return std::unique_ptr<CChainParams>(new CMainParams());
    } else if (chain == CBaseChainParams::TESTNET) {
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    } else if (chain == CBaseChainParams::SIGNET) {
        return std::unique_ptr<CChainParams>(new CTestNetParams()); // TODO: Support SigNet
    } else if (chain == CBaseChainParams::REGTEST) {
        return std::unique_ptr<CChainParams>(new CRegTestParams(args));
    }
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(gArgs, network);
}

// Development Fund helper methods implementation
// Block height must be >= vDevelopmentFundStartHeight and <= vDevelopmentFundLastHeight
// Index variable i ranges from 0 - (vDevelopmentFundAddress.size()-1)
std::string CChainParams::GetDevelopmentFundAddressAtHeight(int nHeight) const {
    assert(nHeight >= vDevelopmentFundStartHeight && nHeight <= vDevelopmentFundLastHeight);
    size_t addressChangeInterval = (vDevelopmentFundLastHeight - vDevelopmentFundStartHeight + 1) / vDevelopmentFundAddress.size();
    size_t i = (nHeight - vDevelopmentFundStartHeight) / addressChangeInterval;
    return vDevelopmentFundAddress[i];
}

// Block height must be >0 and <=last development fund block height
// The development fund address is expected to be a multisig (P2SH) address
CScript CChainParams::GetDevelopmentFundScriptAtHeight(int nHeight) const {
    assert(nHeight > 0 && nHeight <= GetLastDevelopmentFundBlockHeight());

    // Use *this to decode with correct network params (not global Params())
    CTxDestination dest = DecodeDestination(GetDevelopmentFundAddressAtHeight(nHeight), *this);
    assert(IsValidDestination(dest));
    
    // v0.21: CScriptID renamed to ScriptHash
    const ScriptHash* scriptHash = boost::get<ScriptHash>(&dest);
    assert(scriptHash != nullptr);
    
    CScript script = CScript() << OP_HASH160 << ToByteVector(*scriptHash) << OP_EQUAL;
    return script;
}

std::string CChainParams::GetDevelopmentFundAddressAtIndex(int i) const {
    assert(i >= 0 && i < static_cast<int>(vDevelopmentFundAddress.size()));
    return vDevelopmentFundAddress[i];
}

/**
 * Junkcoin: Get consensus parameters for a given height using binary search tree.
 * This allows different consensus rules at different heights (e.g., digishield, auxpow).
 */
const Consensus::Params *Consensus::Params::GetConsensus(uint32_t nTargetHeight) const {
    if (nTargetHeight < this->nHeightEffective && this->pLeft != nullptr) {
        return this->pLeft->GetConsensus(nTargetHeight);
    } else if (nTargetHeight > this->nHeightEffective && this->pRight != nullptr) {
        const Consensus::Params *pCandidate = this->pRight->GetConsensus(nTargetHeight);
        if (pCandidate->nHeightEffective <= nTargetHeight) {
            return pCandidate;
        }
    }

    // No better match below the target height
    return this;
}
