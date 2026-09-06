// Copyright (c) 2021 The Junkcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_JUNKCOIN_FEES_H
#define BITCOIN_JUNKCOIN_FEES_H

#include <amount.h>
#include <chain.h>
#include <chainparams.h>
#include <primitives/transaction.h>
#include <policy/feerate.h>

#ifdef ENABLE_WALLET

enum FeeRatePreset
{
    MINIMUM,
    MORE,
    WOW,
    AMAZE,
    MANY_GENEROUS,
    SUCH_EXPENSIVE
};

/** Estimate fee rate needed to get into the next nBlocks */
CFeeRate GetJunkcoinFeeRate(int priority);
const std::string GetJunkcoinPriorityLabel(int priority);
#endif // ENABLE_WALLET
CAmount GetJunkcoinMinRelayFee(const CTransaction& tx, unsigned int nBytes, bool fAllowFree);
CAmount GetjunkcoindustFee(const std::vector<CTxOut> &vout, const CAmount dustLimit);

#endif // BITCOIN_JUNKCOIN_FEES_H
