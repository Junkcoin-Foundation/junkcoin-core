// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include <uint256.h>
#include <limits>

namespace Consensus {

enum DeploymentPos
{
    DEPLOYMENT_TESTDUMMY,
    DEPLOYMENT_BIP34,
    DEPLOYMENT_BIP66,
    DEPLOYMENT_BIP65,
    DEPLOYMENT_CSV,
    DEPLOYMENT_SEGWIT,
    DEPLOYMENT_TAPROOT, // Deployment of Schnorr/Taproot (BIPs 340-342)
    DEPLOYMENT_MWEB, // Deployment of MWEB (LIPs 0002-0004)
    // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp
    MAX_VERSION_BITS_DEPLOYMENTS
};

/**
 * Struct for each individual consensus rule change using BIP9.
 */
struct BIP9Deployment {
    /** Bit position to select the particular bit in nVersion. */
    int bit;
    /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
    int64_t nStartTime = 0;
    /** Timeout/expiry MedianTime for the deployment attempt. */
    int64_t nTimeout = 0;
    /** Start block height for version bits miner confirmation. Should be a retarget block, can be in the past */
    int64_t nStartHeight = 0;
    /** Timeout/expiry block height for the deployment attempt. Should be a retarget block. */
    int64_t nTimeoutHeight = 0;

    /** Constant for nTimeout very far in the future. */
    static constexpr int64_t NO_TIMEOUT = std::numeric_limits<int64_t>::max();

    /** Special value for nStartTime indicating that the deployment is always active.
     *  This is useful for testing, as it means tests don't need to deal with the activation
     *  process (which takes at least 3 BIP9 intervals). Only tests that specifically test the
     *  behaviour during activation cannot use this. */
    static constexpr int64_t ALWAYS_ACTIVE = -1;

    /** Special value for nStartTime indicating that the deployment is never active.
     *  This is useful for integrating the code changes for a new feature
     *  prior to deploying it on some or all networks. */
    static constexpr int64_t NEVER_ACTIVE = -2;
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    int nSubsidyHalvingInterval;
    /** Block height at which BIP16 becomes active */
    int BIP16Height;
    /** Block height and hash at which BIP34 becomes active */
    int BIP34Height;
    uint256 BIP34Hash;
    /** Block height at which BIP65 becomes active */
    int BIP65Height;
    /** Block height at which BIP66 becomes active */
    int BIP66Height;
    /** Block height at which CSV (BIP68, BIP112 and BIP113) becomes active */
    int CSVHeight;
    /** Block height at which Segwit (BIP141, BIP143 and BIP147) becomes active.
     * Note that segwit v0 script rules are enforced on all blocks except the
     * BIP 16 exception blocks. */
    int SegwitHeight;
    /** Block height at which Taproot (BIP340, BIP341, BIP342) becomes active.
     * Set to std::numeric_limits<int>::max() to disable. */
    int TaprootHeight;
    /** Block height at which MWEB becomes active.
     * Set to std::numeric_limits<int>::max() to disable. */
    int MWEBHeight;
    /** MWEB kernels signaling pegouts must contain at least one pegout at and after this height. */
    int mweb_pegout_feature_activation_height{0};
    /** MWEB kernels signaling extra data must contain non-empty extra data at and after this height. */
    int mweb_extradata_feature_activation_height{0};
    /** Block height at which legacy disabled opcodes (OP_CAT, OP_MUL, etc.) are re-enabled. */
    int DisabledScriptReactivationHeight;
    /** Don't warn about unknown BIP 9 activations below this height.
     * This prevents us from warning about the CSV and segwit activations. */
    int MinBIP9WarningHeight;
    /**
     * Minimum blocks including miner confirmation of the total of 2016 blocks in a retargeting period,
     * (nPowTargetTimespan / nPowTargetSpacing) which is also used for BIP9 deployments.
     * Examples: 1916 for 95%, 1512 for testchains.
     */
    uint32_t nRuleChangeActivationThreshold;
    uint32_t nMinerConfirmationWindow;
    BIP9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];
    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;
    int64_t nPowTargetTimespan;
    int64_t DifficultyAdjustmentInterval() const { return nPowTargetTimespan / nPowTargetSpacing; }
    /** The best chain should have at least this much work */
    uint256 nMinimumChainWork;
    /** By default assume that the signatures in ancestors of this block are valid */
    uint256 defaultAssumeValid;
    
    /** Junkcoin: Number of blocks that coinbase transactions must mature before they can be spent */
    uint32_t nCoinbaseMaturity;

    /**
     * If true, witness commitments contain a payload equal to a Bitcoin Script solution
     * to the signet challenge. See BIP325.
     */
    bool signet_blocks{false};
    std::vector<uint8_t> signet_challenge;
    
    /** Junkcoin: AuxPoW parameters */
    int nAuxpowChainId;
    bool fStrictChainId;
    int nAuxpowStartHeight;
    int nBlockAfterAuxpowRewardThreshold;
    int nLegacyBlocksBefore; // -1 for "always allow"
    
    /** Junkcoin: Digishield parameters */
    bool fDigishieldDifficultyCalculation{false};
    bool fPowAllowDigishieldMinDifficultyBlocks{false};
    bool fSimplifiedRewards{false};
    
    /** Junkcoin: Height-aware consensus parameters (for consensus tree) */
    uint32_t nHeightEffective{0}; // When these parameters come into use
    struct Params *pLeft{nullptr};  // Left hand branch
    struct Params *pRight{nullptr}; // Right hand branch
    
    /**
     * Get the consensus parameters for a given height.
     * Uses binary search tree to find the correct parameters.
     * @param nTargetHeight Height to query
     * @return Consensus parameters for that height
     */
    const Consensus::Params *GetConsensus(uint32_t nTargetHeight) const;
    
    /**
     * Check whether or not to allow legacy blocks at the given height.
     * @param nHeight Height of the block to check.
     * @return True if it is allowed to have a legacy version.
     */
    bool AllowLegacyBlocks(unsigned nHeight) const
    {
        if(nAuxpowStartHeight < 0) // To always allow legacy blocks, set nAuxpowStartHeight to -1
            return true;
        return static_cast<int>(nHeight) < nAuxpowStartHeight;
    }
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
