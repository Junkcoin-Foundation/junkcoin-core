// Copyright (c) 2026 The Junkcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <amount.h>
#include <key.h>
#include <key_io.h>
#include <primitives/transaction.h>
#include <outputtype.h>
#include <policy/policy.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <script/sign.h>
#include <script/signingprovider.h>
#include <script/standard.h>
#include <streams.h>
#include <test/util/setup_common.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(taproot_tests, BasicTestingSetup)

// Official BIP340 test vector 1 (seckey = 3, msg = 00..00, aux = 00..00).
BOOST_AUTO_TEST_CASE(bip340_sign_vector)
{
    const std::vector<unsigned char> seckey_bytes = ParseHex("0000000000000000000000000000000000000000000000000000000000000003");
    const std::vector<unsigned char> want_sig = ParseHex(
        "E907831F80848D1069A5371B402410364BDF1C5F8307B0084C55F1CE2DCA8215"
        "25F66A4A85EA8B71E482A74F382D2CE5EBEEE8FDB2172F477DF4900D310536C0");
    const std::vector<unsigned char> want_xonly = ParseHex(
        "F9308A019258C31049344F85F89D5229B531C845836F99B08601F113BCE036F9");

    CKey key;
    key.Set(seckey_bytes.begin(), seckey_bytes.end(), true);
    BOOST_CHECK(key.IsValid());

    XOnlyPubKey xonly(key.GetPubKey());
    BOOST_CHECK(xonly.IsFullyValid());
    BOOST_CHECK(std::equal(xonly.begin(), xonly.end(), want_xonly.begin()));

    uint256 msg, aux;
    std::vector<unsigned char> sig(64);
    BOOST_CHECK(key.SignSchnorr(msg, sig, nullptr, aux));
    BOOST_CHECK(std::equal(sig.begin(), sig.end(), want_sig.begin()));
    BOOST_CHECK(xonly.VerifySchnorr(msg, sig));

    // Odd-parity internal key: sign-with-tweak path (BIP341 key path).
    CKey odd_key;
    std::vector<unsigned char> odd_seckey = ParseHex("0000000000000000000000000000000000000000000000000000000000000002");
    odd_key.Set(odd_seckey.begin(), odd_seckey.end(), true);
    XOnlyPubKey odd_xonly(odd_key.GetPubKey());
    boost::optional<std::pair<XOnlyPubKey, bool>> tweaked = odd_xonly.CreateTapTweak(nullptr);
    BOOST_REQUIRE(tweaked != boost::none);

    uint256 merkle_root_null; // empty merkle root => key path with no scripts
    std::vector<unsigned char> sig2(64);
    BOOST_CHECK(odd_key.SignSchnorr(msg, sig2, &merkle_root_null, aux));
    BOOST_CHECK(tweaked->first.VerifySchnorr(msg, sig2));
    BOOST_CHECK(tweaked->first.CheckTapTweak(odd_xonly, merkle_root_null, tweaked->second));
    BOOST_CHECK(!tweaked->first.VerifySchnorr(msg, sig)); // other key must not verify
}

// bech32m address + destination + script pipeline.
BOOST_AUTO_TEST_CASE(taproot_destination_pipeline)
{
    CKey key;
    key.MakeNewKey(true);
    const CTxDestination dest = GetDestinationForKey(key.GetPubKey(), OutputType::BECH32M, SecretKey{});

    const auto* tr = boost::get<WitnessV1Taproot>(&dest);
    BOOST_REQUIRE(tr != nullptr);
    BOOST_CHECK(tr->IsFullyValid());

    const std::string addr = EncodeDestination(dest);
    BOOST_CHECK(addr.size() > 4);
    BOOST_CHECK_EQUAL(addr.substr(0, Params().Bech32HRP().size()), Params().Bech32HRP());
    BOOST_CHECK_EQUAL(addr.substr(Params().Bech32HRP().size(), 2), "1p"); // bech32m witness v1 marker "1p"

    const CTxDestination decoded = DecodeDestination(addr, Params());
    const auto* tr2 = boost::get<WitnessV1Taproot>(&decoded);
    BOOST_REQUIRE(tr2 != nullptr);
    BOOST_CHECK(*tr2 == *tr);

    const CScript spk = GetScriptForDestination(dest);
    std::vector<std::vector<unsigned char>> solutions;
    BOOST_CHECK_EQUAL(Solver(spk, solutions), TxoutType::WITNESS_V1_TAPROOT);

    CTxDestination extracted;
    BOOST_CHECK(ExtractDestination(spk, extracted));
    BOOST_CHECK(boost::get<WitnessV1Taproot>(&extracted) != nullptr);
}

// Full key-path spend: sign a taproot input and have the consensus
// interpreter validate the produced witness.
BOOST_AUTO_TEST_CASE(taproot_keypath_spend)
{
    CKey key;
    key.MakeNewKey(true);
    XOnlyPubKey xonly(key.GetPubKey());

    TaprootBuilder builder;
    builder.Finalize(xonly);
    const WitnessV1Taproot output = builder.GetOutput();

    // Build the provider with the key and its taproot spend data.
    FlatSigningProvider provider;
    provider.keys[key.GetPubKey().GetID()] = key;
    provider.tr_spenddata[output] = builder.GetSpendData();

    CMutableTransaction mtx;
    mtx.vin.resize(1);
    mtx.vin[0].prevout = COutPoint(uint256::ONE, 0);
    mtx.vin[0].scriptWitness.SetNull();
    mtx.vout.resize(1);
    mtx.vout[0].nValue = 1000 * COIN;
    mtx.vout[0].scriptPubKey = GetScriptForDestination(PKHash(key.GetPubKey()));

    const CAmount amount = 2000 * COIN;
    const CScript spk = GetScriptForDestination(output);
    const CTransaction tx_const(mtx);
    PrecomputedTransactionData txdata;
    txdata.Init(tx_const, std::vector<CTxOut>{CTxOut(amount, spk)});
    BOOST_CHECK(txdata.m_bip341_taproot_ready);
    BOOST_CHECK(txdata.m_spent_outputs_ready);

    SignatureData sigdata;
    const MutableTransactionSignatureCreator creator(&mtx, 0, amount, &txdata, SIGHASH_DEFAULT);
    BOOST_CHECK(ProduceSignature(provider, creator, spk, sigdata));
    BOOST_CHECK(sigdata.complete);
    BOOST_CHECK(sigdata.scriptWitness.stack.size() == 1);
    BOOST_CHECK_EQUAL(sigdata.scriptWitness.stack[0].size(), 64);

    mtx.vin[0].scriptWitness = sigdata.scriptWitness;

    ScriptError serror = SCRIPT_ERR_OK;
    const bool ok = VerifyScript(
        mtx.vin[0].scriptSig, spk, &mtx.vin[0].scriptWitness,
        STANDARD_SCRIPT_VERIFY_FLAGS,
        TransactionSignatureChecker(&tx_const, 0, amount, txdata), &serror);
    BOOST_CHECK(ok);
    BOOST_CHECK_EQUAL(serror, SCRIPT_ERR_OK);

    // Corrupting the signature must make consensus reject it.
    mtx.vin[0].scriptWitness.stack[0][0] ^= 0x01;
    const bool bad = VerifyScript(
        mtx.vin[0].scriptSig, spk, &mtx.vin[0].scriptWitness,
        STANDARD_SCRIPT_VERIFY_FLAGS,
        TransactionSignatureChecker(&tx_const, 0, amount, txdata), &serror);
    BOOST_CHECK(!bad);
}

BOOST_AUTO_TEST_SUITE_END()