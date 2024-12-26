// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "auxpow.h"
#include "arith_uint256.h"
#include "chain.h"
#include "junkcoin.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

// Determine if the for the given block, a min difficulty setting applies
bool AllowMinDifficultyForBlock(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    // Check if strict block timing is active
    if (pindexLast->nHeight + 1 >= params.nStrictBlockTimeActivationHeight) {
        // Enforce strict 1-minute block spacing
        int64_t expectedTime = pindexLast->GetBlockTime() + params.nPowTargetSpacing;
        int64_t actualTime = pblock->GetBlockTime();
        
        // Only allow blocks exactly at the target spacing
        if (actualTime != expectedTime) {
            LogPrintf("Block time %d not at expected time %d\n", actualTime, expectedTime);
            return false;
        }
    }

    return params.fPowAllowMinDifficultyBlocks;
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // junkcoin: Special rules for minimum difficulty blocks with Digishield
    if (AllowDigishieldMinDifficultyForBlock(pindexLast, pblock, params))
    {
        // Special difficulty rule for testnet:
        // If the new block's timestamp is more than 2* nTargetSpacing minutes
        // then allow mining of a min-difficulty block.
        return nProofOfWorkLimit;
    }

    // Check if strict block timing is active
    if (pindexLast->nHeight + 1 >= params.nStrictBlockTimeActivationHeight) {
        // Enforce strict 1-minute block spacing
        int64_t expectedTime = pindexLast->GetBlockTime() + params.nPowTargetSpacing;
        int64_t actualTime = pblock->GetBlockTime();
        
        // Return maximum difficulty if block time is not exactly at target spacing
        if (actualTime != expectedTime) {
            LogPrintf("Block time %d not at expected time %d, enforcing high difficulty\n", actualTime, expectedTime);
            return nProofOfWorkLimit;
        }
    }

    // Only change difficulty on interval
    bool fNewDifficultyProtocol = (pindexLast->nHeight+1 >= 69360);
    const int64_t nTargetTimespanCurrent = fNewDifficultyProtocol ? params.nPowTargetTimespan : (params.nPowTargetTimespan*12);
    const int64_t difficultyAdjustmentInterval = nTargetTimespanCurrent / params.nPowTargetSpacing;
    
    if ((pindexLast->nHeight+1) % difficultyAdjustmentInterval != 0) {
        return pindexLast->nBits;
    }

    // Litecoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = difficultyAdjustmentInterval-1;
    if ((pindexLast->nHeight+1) != difficultyAdjustmentInterval)
        blockstogoback = difficultyAdjustmentInterval;

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - blockstogoback;
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateJunkcoinNextWorkRequired(fNewDifficultyProtocol, nTargetTimespanCurrent, pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit)) {
        return false;
    }

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget) {
        LogPrintf("WARN: Check proof of work matches claimed amount %s | %s > %s\n", hash.ToString().c_str(), UintToArith256(hash).ToString().c_str(), bnTarget.ToString().c_str());
        return false;
    }

    return true;
}
