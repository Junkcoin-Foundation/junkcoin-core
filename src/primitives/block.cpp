// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <crypto/common.h>
#include <crypto/scrypt.h>
#include <auxpow.h>
#include <primitives/pureheader.h>

uint256 CBlockHeader::GetHash() const
{
    // CRITICAL: Only hash the 80-byte header, NOT the auxpow data
    // The block hash is the hash of the 80-byte header only.
    // AuxPoW data is additional merge-mining proof, not part of the block's identity.
    // Including auxpow in the hash would break the blockchain's hash chain:
    // - Block N+1's hashPrevBlock must equal Hash(Block N's 80-byte header)
    // - If we include auxpow, the hashes won't match and sync will fail
    CPureBlockHeader pureHeader;
    pureHeader.nVersion = nVersion;
    pureHeader.hashPrevBlock = hashPrevBlock;
    pureHeader.hashMerkleRoot = hashMerkleRoot;
    pureHeader.nTime = nTime;
    pureHeader.nBits = nBits;
    pureHeader.nNonce = nNonce;

    return SerializeHash(pureHeader);
}

uint256 CBlockHeader::GetPoWHash() const
{
    // Create a CPureBlockHeader for PoW hashing
    // CBlockHeader contains shared_ptr which breaks POD layout
    CPureBlockHeader pureHeader;
    pureHeader.nVersion = nVersion;
    pureHeader.hashPrevBlock = hashPrevBlock;
    pureHeader.hashMerkleRoot = hashMerkleRoot;
    pureHeader.nTime = nTime;
    pureHeader.nBits = nBits;
    pureHeader.nNonce = nNonce;
    
    return pureHeader.GetPoWHash();
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}

CTransactionRef CBlock::GetHogEx() const noexcept
{
    if (vtx.size() >= 2 && vtx.back()->IsHogEx()) {
        assert(!vtx.back()->vout.empty());
        return vtx.back();
    }

    return nullptr;
}