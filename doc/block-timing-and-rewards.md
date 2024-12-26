# JunkCoin Block Timing and Reward Schedule

## Block Timing Changes

### Current Implementation
- Before activation height: Random timing within 1-minute target
- After activation height: Strict 1-minute block spacing

### Technical Details
- Controlled by `nStrictBlockTimeActivationHeight` parameter
- Mainnet activation: Block 262,800 (January 2024)
- Testnet activation: Block 2,880

### Implementation
Located in `miner.cpp`:
```cpp
if (pindexPrev->nHeight + 1 >= consensusParams.nStrictBlockTimeActivationHeight) {
    nNewTime = pindexPrev->GetBlockTime() + consensusParams.nPowTargetSpacing;
} else {
    nNewTime = GetAdjustedTime();
    // Flexible timing logic
}
```

## Reward Schedule

### Initial Distribution (First 4 Days)
1. First 100 blocks: 1000 JKC each
2. Blocks 101-1540 (1st day): 500 JKC each
3. Blocks 1541-2980 (2nd day): 200 JKC each
4. Blocks 2981-5860 (3rd/4th days): 100 JKC each

### Base Reward Period (Block 5861+)
- Base reward: 50 JKC
- Random bonus chances:
  * 0.01% chance for 20x reward (1000 JKC)
  * 1% chance for 3x reward (150 JKC)
  * 98.99% chance for normal reward (50 JKC)

### Mainnet Halving Schedule
1. First halving at block 262,800 (January 2024)
   - Reward reduces to 25 JKC
2. Second halving at block 394,200 (After 3 months)
   - Reward reduces to 12.5 JKC
3. Third halving at block 919,800 (After 10 months)
   - Reward reduces to 6.25 JKC
4. Fourth halving at block 1,708,200 (After 22 months)
   - Reward reduces to 3.125 JKC
5. Further halvings every 788,400 blocks (1.5 years)
   - Each halving reduces reward by 50%
6. Mining rewards end at block 1,707,600 (October 2025)

### Testnet Halving Schedule
1. First halving at block 2,880
   - Reward reduces to 25 JKC
2. Subsequent halvings every 28,800 blocks
   - Each halving reduces reward by 50%
3. Mining rewards end at block 89,280

### Random Bonus Rules
- Random bonuses continue to apply after halvings
- Bonus is applied to the base reward for that period
- Example after first halving:
  * Normal reward: 25 JKC
  * 3x bonus: 75 JKC
  * 20x bonus: 500 JKC

## Technical Implementation

### Block Time Enforcement
- Implemented in `UpdateTime()` function in `miner.cpp`
- Uses consensus parameter `nStrictBlockTimeActivationHeight`
- After activation, blocks must be exactly 60 seconds apart
- Before activation, flexible timing with median time checks

### Reward Calculation
- Implemented in `GetJunkcoinBlockSubsidy()` function in `junkcoin.cpp`
- Uses consensus parameters for halving heights and intervals
- Random bonus generation using Mersenne Twister
- Separate logic paths for mainnet and testnet

### Consensus Parameters
Located in `chainparams.cpp`:
```cpp
// Mainnet
consensus.nFirstHalvingHeight = 262800;
consensus.nSecondHalvingHeight = 394200;
consensus.nThirdHalvingHeight = 919800;
consensus.nFourthHalvingHeight = 1708200;
consensus.nSubsidyHalvingInterval = 788400;

// Testnet
consensus.nFirstHalvingHeight = 2880;
consensus.nSubsidyHalvingInterval = 28800;
```

### Random Number Generation
- Uses Mersenne Twister algorithm for random bonus generation
- Seed based on block height for deterministic results
- Implemented in `generateMTRandom()` function in `junkcoin.cpp`:
```cpp
int static generateMTRandom(unsigned int s, int range)
{
    boost::mt19937 gen(s);
    boost::uniform_int<> dist(1, range);
    return dist(gen);
}
```
