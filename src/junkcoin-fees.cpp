// Copyright (c) 2021 The Junkcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/random/uniform_int.hpp>

#include <policy/policy.h>
#include <util/translation.h>
#include <arith_uint256.h>
#include <junkcoin.h>
#include <txmempool.h>
#include <util/system.h>
#include <validation.h>
#include <junkcoin-fees.h>
#include <amount.h>
#include <policy/fees.h>
#include <script/standard.h>
#ifdef ENABLE_WALLET
#include <wallet/wallet.h>
#endif

#ifdef ENABLE_WALLET

CFeeRate GetJunkcoinFeeRate(int priority)
{
    // v0.21: CWallet::minTxFee is deprecated, use DEFAULT_FALLBACK_FEE
    // Base fee rate: 200,000 satoshis per KB (0.002 JKC/KB)
    const CAmount baseFeePerK = DEFAULT_FALLBACK_FEE;
    
    switch(priority)
    {
    case SUCH_EXPENSIVE:
        return CFeeRate(COIN / 100 * 258); // 2.58 JKC, but very carefully avoiding floating point maths
    case MANY_GENEROUS:
        return CFeeRate(baseFeePerK * 50);
    case AMAZE:
        return CFeeRate(baseFeePerK * 8);
    case WOW:
        return CFeeRate(baseFeePerK * 4);
    case MORE:
        return CFeeRate(baseFeePerK * 2);
    case MINIMUM:
    default:
        break;
    }
    return CFeeRate(baseFeePerK);
}

const std::string GetJunkcoinPriorityLabel(int priority)
{
    // v0.21: _() returns bilingual_str, need .translated to get std::string
    switch(priority)
    {
    case SUCH_EXPENSIVE:
        return _("Such expensive").translated;
    case MANY_GENEROUS:
        return _("Many generous").translated;
    case AMAZE:
        return _("Amaze").translated;
    case WOW:
        return _("Wow").translated;
    case MORE:
        return _("More").translated;
    case MINIMUM:
        return _("Minimum").translated;
    default:
        break;
    }
    return _("Default").translated;
}

#endif

CAmount GetJunkcoinMinRelayFee(const CTransaction& tx, unsigned int nBytes, bool fAllowFree)
{
    // v0.21: Simplified version without mempool dependency
    // Get the minimum relay fee
    CAmount nMinFee = ::minRelayTxFee.GetFee(nBytes);
    
    // Add dust fee for small outputs
    const CAmount dustLimit = GetDustThreshold(CTxOut(0, CScript()), ::minRelayTxFee);
    nMinFee += GetjunkcoindustFee(tx.vout, dustLimit);

    // v0.21: Priority-based free transactions removed
    // if (fAllowFree) { ... }

    if (!MoneyRange(nMinFee))
        nMinFee = MAX_MONEY;
    return nMinFee;
}

CAmount GetjunkcoindustFee(const std::vector<CTxOut> &vout, const CAmount dustLimit) {
    CAmount nFee = 0;

    // To limit dust spam, add the dust limit for each output
    // less than the (soft) dustlimit
    CFeeRate dustRelayFeeRate(dustLimit);
    for (const CTxOut& txout : vout) {
        if (IsDust(txout, dustRelayFeeRate))
            nFee += dustLimit;
    }

    return nFee;
}
