// Copyright (c) 2015-2021 The Junkcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <amount.h>
#include <chain.h>
#include <chainparams.h>
#include <util/system.h>
#include <consensus/params.h>
#include <primitives/block.h>

bool AllowDigishieldMinDifficultyForBlock(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params);

CAmount GetJunkcoinBlockSubsidy(int nHeight, CAmount nFees, const Consensus::Params& consensusParams, uint256 prevHash);

/** Validate block subsidy at halving heights */
bool ValidateBlockSubsidy(const CAmount nSubsidy, int nHeight, const Consensus::Params& consensusParams);

unsigned int CalculateJunkcoinNextWorkRequired(bool fNewDifficultyProtocol, const int64_t nTargetTimespanCurrent, const CBlockIndex* pindexLast, int64_t nLastRetargetTime, const Consensus::Params& params);

/**
 * Check proof-of-work of a block header, taking auxpow into account.
 * @param block The block header.
 * @param params Consensus parameters.
 * @return True iff the PoW is correct.
 */
bool CheckAuxPowProofOfWork(const CBlockHeader& block, const Consensus::Params& params);
