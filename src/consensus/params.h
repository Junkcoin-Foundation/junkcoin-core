// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2021 The Junkcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include "uint256.h"
#include <map>
#include <string>

namespace Consensus {

enum DeploymentPos
{
    DEPLOYMENT_TESTDUMMY,
    // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp   
    DEPLOYMENT_BIP34, // Deployment of BIP34
    DEPLOYMENT_BIP66, // Deployment of BIP66
    DEPLOYMENT_BIP65, // Deployment of BIP65
    DEPLOYMENT_CSV, // Deployment of BIP68, BIP112, and BIP113.
    DEPLOYMENT_SEGWIT, // Deployment of BIP141, BIP143, and BIP147.
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
    int64_t nStartTime;
    /** Timeout/expiry MedianTime for the deployment attempt. */
    int64_t nTimeout;
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    /** Block heights for the new halving schedule */
    int32_t nFirstHalvingHeight;    // First halving at block 262,800 (January)
    int32_t nSecondHalvingHeight;   // Second halving after 131,400 blocks (3 months)
    int32_t nThirdHalvingHeight;    // Third halving after 525,600 blocks (10 months)
    int32_t nFourthHalvingHeight;   // Fourth halving after 788,400 blocks (22 months)
    int32_t nSubsidyHalvingInterval; // Regular halving interval after fourth halving (788,400 blocks)
    /** Used to check majorities for block version upgrade */
    int nMajorityEnforceBlockUpgrade;
    int nMajorityRejectBlockOutdated;
    int nMajorityWindow;
    /** Block height and hash at which BIP34 becomes active */
    int BIP34Height;
    uint256 BIP34Hash;
    /** Block height at which BIP65 becomes active */
    int BIP65Height;
    /** Block height at which BIP66 becomes active */
    int BIP66Height;
    /**
     * Minimum blocks including miner confirmation of the total of 2016 blocks in a retargeting period,
     * (nPowTargetTimespan / nPowTargetSpacing) which is also used for BIP9 deployments.
     * Examples: 1916 for 95%, 1512 for testchains.
     */
    uint32_t nRuleChangeActivationThreshold;
    uint32_t nMinerConfirmationWindow;
    BIP9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];
    uint32_t nCoinbaseMaturity;
    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;
    int64_t nPowTargetTimespan;
    int64_t DifficultyAdjustmentInterval() const { return nPowTargetTimespan / nPowTargetSpacing; }

    /** Height at which strict block timing is enforced */
    int32_t nStrictBlockTimeActivationHeight;

    /** Junkcoin-specific parameters */
    bool fDigishieldDifficultyCalculation;
    bool fPowAllowDigishieldMinDifficultyBlocks; // Allow minimum difficulty blocks where a retarget would normally occur
    bool fSimplifiedRewards; // Use block height derived rewards rather than previous block hash derived

    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    /** Auxpow parameters */
    int32_t nAuxpowChainId;
    int nAuxpowStartHeight;
    int nBlockAfterAuxpowRewardThreshold;
    bool fStrictChainId;
    //bool fAllowLegacyBlocks;
    int nLegacyBlocksBefore; // -1 for "always allow"

    /** Height-aware consensus parameters */
    uint32_t nHeightEffective; // When these parameters come into use
    struct Params *pLeft = nullptr;      // Left hand branch
    struct Params *pRight = nullptr;     // Right hand branch
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
        return static_cast<int> (nHeight) < nAuxpowStartHeight;
    }
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
