# Implementation Changes for Block Timing and Rewards

## 1. Chain Parameters (chainparams.cpp)

### Mainnet Changes
```cpp
// In CMainParams constructor
consensus.nStrictBlockTimeActivationHeight = 262800;  // January 2024
consensus.nFirstHalvingHeight = 262800;      // January 2024
consensus.nSecondHalvingHeight = 394200;     // After 3 months
consensus.nThirdHalvingHeight = 919800;      // After 10 months
consensus.nFourthHalvingHeight = 1708200;    // After 22 months
consensus.nSubsidyHalvingInterval = 788400;  // Every 1.5 years after fourth halving
```

### Testnet Changes
```cpp
// In CTestNetParams constructor
consensus.nStrictBlockTimeActivationHeight = 2880;
consensus.nFirstHalvingHeight = 2880;        // First halving
consensus.nSubsidyHalvingInterval = 28800;   // Subsequent halvings
```

## 2. Block Timing (miner.cpp)

### UpdateTime Function
```cpp
int64_t UpdateTime(CBlockHeader* pblock, const Consensus::Params& consensusParams, const CBlockIndex* pindexPrev)
{
    int64_t nOldTime = pblock->nTime;
    int64_t nNewTime;

    // After activation height: Force exact 1-minute spacing between blocks
    if (pindexPrev->nHeight + 1 >= consensusParams.nStrictBlockTimeActivationHeight) {
        nNewTime = pindexPrev->GetBlockTime() + consensusParams.nPowTargetSpacing;
    } else {
        // Before activation: Use adjusted time
        nNewTime = GetAdjustedTime();
        // Ensure it's greater than median time
        if (nNewTime <= pindexPrev->GetMedianTimePast())
            nNewTime = pindexPrev->GetMedianTimePast() + 1;
        // Ensure it's not too far in the future
        if (nNewTime > GetAdjustedTime() + 2 * 60 * 60)
            nNewTime = GetAdjustedTime() + 2 * 60 * 60;
    }
    
    pblock->nTime = nNewTime;

    // Updating time can change work required on testnet:
    if (consensusParams.fPowAllowMinDifficultyBlocks)
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock, consensusParams);

    return nNewTime - nOldTime;
}
```

## 3. Block Rewards (junkcoin.cpp)

### Random Number Generation
```cpp
int static generateMTRandom(unsigned int s, int range)
{
    boost::mt19937 gen(s);
    boost::uniform_int<> dist(1, range);
    return dist(gen);
}
```

### Block Subsidy Function
```cpp
CAmount GetJunkcoinBlockSubsidy(int nHeight, int nFees, const Consensus::Params& consensusParams, uint256 prevHash)
{
    CAmount nSubsidy = 50 * COIN;

    // No mining rewards after October 2025 (~block 1,707,600) for mainnet
    // For testnet, no mining rewards after block 89,280
    if ((consensusParams.nAuxpowChainId == 0x2020 && nHeight > 1707600) ||  // Mainnet
        (consensusParams.nAuxpowChainId == 0x2021 && nHeight > 89280))      // Testnet
        return nFees;

    // Initial distribution period
    if(nHeight < 101) {
        nSubsidy = 1000 * COIN;  // First 100 blocks
    }
    else if(nHeight < 1541) {
        nSubsidy = 500 * COIN;   // 1st day
    }
    else if(nHeight < 2981) {
        nSubsidy = 200 * COIN;   // 2nd day
    }
    else if(nHeight < 5861) {
        nSubsidy = 100 * COIN;   // 3rd and 4th days
    }
    else {
        nSubsidy = 50 * COIN;
        
        if (consensusParams.nAuxpowChainId == 0x2020) {  // Mainnet
            // First halving at block 262,800
            if (nHeight >= consensusParams.nFirstHalvingHeight) {
                nSubsidy = 25 * COIN;
                
                // Second halving after 131,400 blocks
                if (nHeight >= consensusParams.nSecondHalvingHeight) {
                    nSubsidy = 12.5 * COIN;
                    
                    // Third halving after 525,600 blocks
                    if (nHeight >= consensusParams.nThirdHalvingHeight) {
                        nSubsidy = 6.25 * COIN;
                        
                        // Fourth halving after 788,400 blocks
                        if (nHeight >= consensusParams.nFourthHalvingHeight) {
                            nSubsidy = 3.125 * COIN;
                            
                            // Further halvings every 788,400 blocks
                            int additionalHalvings = (nHeight - consensusParams.nFourthHalvingHeight) / 
                                                   consensusParams.nSubsidyHalvingInterval;
                            nSubsidy >>= additionalHalvings;
                        }
                    }
                }
            }
        }
        else {  // Testnet
            // Halving every 28800 blocks starting at block 2880
            if (nHeight >= consensusParams.nFirstHalvingHeight) {
                int halvings = 1 + (nHeight - consensusParams.nFirstHalvingHeight) / 
                              consensusParams.nSubsidyHalvingInterval;
                nSubsidy >>= halvings;
            }
        }

        // Apply random bonus rewards
        int rand = generateMTRandom(nHeight, 100000);
        if(rand > 99990) {       // 0.01% chance for 20x reward
            nSubsidy *= 20;
        }
        else if (rand < 1001) {  // 1% chance for 3x reward
            nSubsidy *= 3;
        }
    }

    return nSubsidy + nFees;
}
```

## 4. Consensus Parameters (consensus/params.h)

### Add New Parameters
```cpp
class Params {
public:
    // Add new parameters
    int32_t nStrictBlockTimeActivationHeight;
    int32_t nFirstHalvingHeight;
    int32_t nSecondHalvingHeight;
    int32_t nThirdHalvingHeight;
    int32_t nFourthHalvingHeight;
    int32_t nSubsidyHalvingInterval;
};
```

## 5. Validation Changes (validation.cpp)

### Block Time Validation
```cpp
bool ContextualCheckBlockHeader(const CBlockHeader& block, CValidationState& state, 
                              const CBlockIndex* pindexPrev, int64_t nAdjustedTime)
{
    // ... existing checks ...

    // Check block timing
    if (nHeight >= consensusParams.nStrictBlockTimeActivationHeight) {
        // After activation height: Enforce exact 1-minute block spacing
        int64_t expectedTime = pindexPrev->GetBlockTime() + consensusParams.nPowTargetSpacing;
        if (block.GetBlockTime() != expectedTime) {
            return state.Invalid(false, REJECT_INVALID, "invalid-time", 
                strprintf("block timestamp %d not at expected time %d", 
                block.GetBlockTime(), expectedTime));
        }
    } else {
        // Before activation height: Use original timing rules
        if (block.GetBlockTime() <= pindexPrev->GetMedianTimePast())
            return state.DoS(100, false, REJECT_INVALID, "time-too-old", false, 
                "block's timestamp is too early");

        if (block.GetBlockTime() > nAdjustedTime + 2 * 60 * 60)
            return state.DoS(100, false, REJECT_INVALID, "time-too-new", false, 
                "block timestamp too far in the future");
    }

    // ... rest of validation ...
    return true;
}
```

These changes implement:
1. Strict 1-minute block timing after activation height
2. Complete halving schedule for both mainnet and testnet
3. Random bonus rewards that persist through halvings
4. End of mining rewards at specified blocks
