#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <policy/policy.h>
#include <arith_uint256.h>
#include <junkcoin.h>
#include <txmempool.h>
#include <util/system.h>
#include <validation.h>
#include <junkcoin-fees.h>
#include <pow.h>
#include <auxpow.h>

// Generate random number using Mersenne Twister
int static generateMTRandom(unsigned int s, int range) {
    boost::mt19937 gen(s);
    boost::uniform_int<> dist(1, range);
    return dist(gen);
}

bool AllowDigishieldMinDifficultyForBlock(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params) {
    // Check if minimum difficulty blocks are allowed
    if (!params.fPowAllowMinDifficultyBlocks) {
        return false;
    }

    // Check height requirement for minimum difficulty blocks
    if ((unsigned)pindexLast->nHeight < params.nHeightEffective) {
        return false;
    }

    // Allow minimum difficulty if block time > 2 * target spacing
    return (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 2);
}

unsigned int CalculateJunkcoinNextWorkRequired(bool fNewDifficultyProtocol, const int64_t nTargetTimespanCurrent, const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params) {
    int nHeight = pindexLast->nHeight + 1;

    // Calculate timespan bounds based on protocol version
    int64_t nActualTimespanMin = fNewDifficultyProtocol ? 
        (nTargetTimespanCurrent - nTargetTimespanCurrent/4) : 
        (nTargetTimespanCurrent/4);
    int64_t nActualTimespanMax = fNewDifficultyProtocol ? 
        (nTargetTimespanCurrent + nTargetTimespanCurrent/4) : 
        (nTargetTimespanCurrent*4);

    // Calculate actual timespan
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;

    // Apply timespan limits based on block height
    if (nHeight > 10000) {
        nActualTimespan = std::max(nActualTimespanMin, std::min(nActualTimespan, nActualTimespanMax));
    } else if (nHeight > 5000) {
        nActualTimespan = std::max(nActualTimespanMin/2, std::min(nActualTimespan, nActualTimespanMax));
    } else {
        nActualTimespan = std::max(nActualTimespanMin/4, std::min(nActualTimespan, nActualTimespanMax));
    }

    // Calculate new target
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespanCurrent;

    // Ensure target is below pow limit
    if (bnNew > bnPowLimit) {
        bnNew = bnPowLimit;
    }

    return bnNew.GetCompact();
}

bool CheckAuxPowProofOfWork(const CBlockHeader& block, const Consensus::Params& params) {
    // Verify chain ID for non-legacy blocks
    if (!block.IsLegacy() && params.fStrictChainId && block.GetChainId() != params.nAuxpowChainId) {
        return error("%s: block does not have our chain ID (got %d, expected %d, full nVersion %d)",
                    __func__, block.GetChainId(), params.nAuxpowChainId, block.nVersion);
    }

    // Handle non-auxpow blocks
    if (!block.auxpow) {
        if (block.IsAuxpow()) {
            return error("%s: no auxpow on block with auxpow version", __func__);
        }

        // Special case for genesis block
        if (block.GetHash() == params.hashGenesisBlock && block.IsLegacy()) {
            return true;
        }

        return CheckProofOfWork(block.GetPoWHash(), block.nBits, params);
    }

    // Verify auxpow blocks
    if (!block.IsAuxpow()) {
        return error("%s: auxpow on block with non-auxpow version", __func__);
    }

    if (!block.auxpow->check(block.GetHash(), block.GetChainId(), params)) {
        return error("%s: AUX POW is not valid", __func__);
    }

    return CheckProofOfWork(block.auxpow->getParentBlockPoWHash(), block.nBits, params);
}

CAmount GetJunkcoinBlockSubsidy(int nHeight, CAmount nFees, const Consensus::Params& consensusParams, uint256 prevHash) {
    CAmount nSubsidy;

    // Calculate base subsidy based on network type and height
    if (consensusParams.fPowAllowMinDifficultyBlocks) {
        // Testnet rewards
        if (nHeight < 101) nSubsidy = 1000 * COIN;
        else if (nHeight < 201) nSubsidy = 500 * COIN;  // 1st day
        else if (nHeight < 401) nSubsidy = 200 * COIN;  // 2nd day
        else if (nHeight < 601) nSubsidy = 100 * COIN;  // 3rd and 4th days
        else if (nHeight < 801) nSubsidy = 50 * COIN;
        else if (nHeight < 1001) nSubsidy = 25 * COIN;
        else if (nHeight < 657000) nSubsidy = 12.5 * COIN;
        else if (nHeight < 1182600) nSubsidy = 6.25 * COIN;
        else if (nHeight < 1971000) nSubsidy = 3.125 * COIN;
        else if (nHeight < 2759400) nSubsidy = 1.5625 * COIN;
        else if (nHeight < 3547800) nSubsidy = 0.78125 * COIN;
        else nSubsidy = 0.390625 * COIN;
    } else {
        // Mainnet rewards
        if (nHeight < 101) nSubsidy = 1000 * COIN;
        else if (nHeight < 1541) nSubsidy = 500 * COIN;  // 1st day
        else if (nHeight < 2981) nSubsidy = 200 * COIN;  // 2nd day
        else if (nHeight < 5861) nSubsidy = 100 * COIN;  // 3rd and 4th days
        else if (nHeight < 262800) nSubsidy = 50 * COIN;
        else if (nHeight < 394200) nSubsidy = 25 * COIN;
        else if (nHeight < 657000) nSubsidy = 12.5 * COIN;
        else if (nHeight < 1182600) nSubsidy = 6.25 * COIN;
        else if (nHeight < 1971000) nSubsidy = 3.125 * COIN;
        else if (nHeight < 2759400) nSubsidy = 1.5625 * COIN;
        else if (nHeight < 3547800) nSubsidy = 0.78125 * COIN;
        else nSubsidy = 0.390625 * COIN;
    }

    // Apply random bonus
    int rand = generateMTRandom(nHeight, 100000);
    if (rand > 99990) {
        nSubsidy = 1000 * COIN;
    } else if (rand < 1001) {
        nSubsidy *= 3;
    }

    return nSubsidy + nFees;
}

bool ValidateBlockSubsidy(const CAmount nSubsidy, int nHeight, const Consensus::Params& consensusParams) {
    CAmount expectedSubsidy = GetJunkcoinBlockSubsidy(nHeight, 0, consensusParams, uint256());
    
    // Strict validation at halving heights
    bool strictCheck = false;
    if (consensusParams.fPowAllowMinDifficultyBlocks) {
        strictCheck = (nHeight == 801 || nHeight == 1001 || nHeight == 657000 || 
                      nHeight == 1182600 || nHeight == 1971000 || nHeight == 2759400 || 
                      nHeight == 3547800);
    } else {
        strictCheck = (nHeight == 262800 || nHeight == 394200 || nHeight == 657000 || 
                      nHeight == 1182600 || nHeight == 1971000 || nHeight == 2759400 || 
                      nHeight == 3547800);
    }

    // Strict check at halving heights, lenient otherwise
    return strictCheck ? (nSubsidy == expectedSubsidy) : (nSubsidy <= expectedSubsidy);
}