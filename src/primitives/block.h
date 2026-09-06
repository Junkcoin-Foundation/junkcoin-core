// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>
#include <mweb/mweb_models.h>
#include <memory>
#include <auxpow.h>

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    // AuxPoW
    std::shared_ptr<CAuxPow> auxpow;

    CBlockHeader()
    {
        SetNull();
    }

    // AuxPoW helper methods
    static const int32_t VERSION_AUXPOW = (1 << 8);
    static const int32_t VERSION_CHAIN_START = (1 << 16);

    bool IsAuxpow() const
    {
        return nVersion & VERSION_AUXPOW;
    }

    bool IsLegacy() const
    {
        return nVersion == 1
            // junkcoin: We have a random v2 block with no AuxPoW, treat as legacy
            || (nVersion == 2 && GetChainId() == 0);
    }

    int GetChainId() const
    {
        return nVersion >> 16;
    }

    void SetAuxpowVersion(bool auxpow)
    {
        if (auxpow)
            nVersion |= VERSION_AUXPOW;
        else
            nVersion &= ~VERSION_AUXPOW;
    }

    void SetChainId(int chainId)
    {
        nVersion = (nVersion & 0xFFFF) | (chainId << 16);
    }

    int32_t GetBaseVersion() const
    {
        return nVersion % VERSION_AUXPOW;
    }

    void SetBaseVersion(int32_t nBaseVersion, int32_t nChainId)
    {
        assert(nBaseVersion >= 1 && nBaseVersion < VERSION_AUXPOW);
        nVersion = nBaseVersion | (nChainId * VERSION_CHAIN_START);
    }

    void SetAuxpowFlag(bool auxpow)
    {
        SetAuxpowVersion(auxpow);
    }

    void SetAuxpow(CAuxPow* apow)
    {
        if (apow) {
            auxpow.reset(apow);
            SetAuxpowVersion(true);
        } else {
            auxpow.reset();
            SetAuxpowVersion(false);
        }
    }

    template<typename Stream>
    void Serialize(Stream& s) const {
        // Serialize the 80-byte header
        s << nVersion << hashPrevBlock << hashMerkleRoot << nTime << nBits << nNonce;

        // For AuxPoW blocks, also serialize the AuxPoW data
        // This matches the original Junkcoin Core behavior
        if (IsAuxpow()) {
            assert(auxpow);
            s << *auxpow;
        }
    }

    template<typename Stream>
    void Unserialize(Stream& s) {
        // Deserialize the 80-byte header
        s >> nVersion >> hashPrevBlock >> hashMerkleRoot >> nTime >> nBits >> nNonce;

        // For AuxPoW blocks, also deserialize the AuxPoW data
        if (IsAuxpow()) {
            auxpow = std::make_shared<CAuxPow>();
            s >> *auxpow;
        } else {
            auxpow.reset();
        }
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        auxpow.reset();
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    uint256 GetPoWHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // memory only
    mutable bool fChecked;

    MWEB::Block mweb_block;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *(static_cast<CBlockHeader*>(this)) = header;
    }

    SERIALIZE_METHODS(CBlock, obj)
    {
        READWRITEAS(CBlockHeader, obj);
        READWRITE(obj.vtx);
        if (!(s.GetVersion() & SERIALIZE_NO_MWEB)) {
            if (obj.vtx.size() >= 2 && obj.vtx.back()->IsHogEx()) {
                READWRITE(obj.mweb_block);
            }
        }
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
        mweb_block.SetNull();
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        block.auxpow         = auxpow;
        return block;
    }

    std::string ToString() const;

    // Returns the hogex (integrating) transaction, if it exists.
    CTransactionRef GetHogEx() const noexcept;
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    explicit CBlockLocator(const std::vector<uint256>& vHaveIn) : vHave(vHaveIn) {}

    SERIALIZE_METHODS(CBlockLocator, obj)
    {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(obj.vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

// Include auxpow.h at the end to provide complete type for template instantiation
#include <auxpow.h>

#endif // BITCOIN_PRIMITIVES_BLOCK_H
