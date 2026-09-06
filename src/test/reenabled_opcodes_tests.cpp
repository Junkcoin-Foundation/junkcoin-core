// Copyright (c) 2024 The Junkcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <script/script_error.h>
#include <streams.h>
#include <test/util/setup_common.h>
#include <util/strencodings.h>

#if defined(HAVE_CONSENSUS_LIB)
#include <script/bitcoinconsensus.h>
#endif

#include <boost/test/unit_test.hpp>

#include <vector>

BOOST_FIXTURE_TEST_SUITE(reenabled_opcodes_tests, BasicTestingSetup)

static const unsigned int REENABLED = SCRIPT_VERIFY_DISABLED_OPCODES_REENABLED;

/** Evaluate a script and report success plus the resulting error code. */
static bool Eval(const CScript& script, unsigned int flags, ScriptError& err,
                 std::vector<std::vector<unsigned char>>& stack,
                 SigVersion sigversion = SigVersion::BASE)
{
    return EvalScript(stack, script, flags, BaseSignatureChecker(), sigversion, &err);
}

static bool EvalOk(const CScript& script, unsigned int flags, ScriptError& err)
{
    std::vector<std::vector<unsigned char>> stack;
    return Eval(script, flags, err, stack);
}

// ---------------------------------------------------------------------------
// Activation gating
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(disabled_without_flag)
{
    ScriptError err;
    const opcodetype disabled[] = {
        OP_CAT, OP_SUBSTR, OP_LEFT, OP_RIGHT, OP_INVERT, OP_AND, OP_OR, OP_XOR,
        OP_2MUL, OP_2DIV, OP_MUL, OP_DIV, OP_MOD, OP_LSHIFT, OP_RSHIFT,
    };
    for (const opcodetype op : disabled) {
        BOOST_CHECK(IsReenabledOpcode(op));
        const CScript script = CScript() << OP_1 << OP_1 << op;
        BOOST_CHECK(!EvalOk(script, SCRIPT_VERIFY_NONE, err));
        BOOST_CHECK_EQUAL(err, SCRIPT_ERR_DISABLED_OPCODE);
    }
}

BOOST_AUTO_TEST_CASE(disabled_even_in_unexecuted_branch)
{
    // Historical behaviour: a disabled opcode fails the script even when it
    // sits in a branch that is never executed.
    ScriptError err;
    const CScript script = CScript() << OP_0 << OP_IF << OP_CAT << OP_ENDIF << OP_1;
    BOOST_CHECK(!EvalOk(script, SCRIPT_VERIFY_NONE, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_DISABLED_OPCODE);

    // Once re-enabled it is simply skipped along with the rest of the branch.
    BOOST_CHECK(EvalOk(script, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_OK);
}

// ---------------------------------------------------------------------------
// Fix #1: re-enabled opcodes must not be OP_SUCCESSx in tapscript
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(op_success_excludes_reenabled_opcodes)
{
    const opcodetype reenabled[] = {
        OP_CAT, OP_SUBSTR, OP_LEFT, OP_RIGHT, OP_INVERT, OP_AND, OP_OR, OP_XOR,
        OP_2MUL, OP_2DIV, OP_MUL, OP_DIV, OP_MOD, OP_LSHIFT, OP_RSHIFT,
    };
    for (const opcodetype op : reenabled) {
        // BIP342 classifies all of these as OP_SUCCESSx...
        BOOST_CHECK(IsOpSuccess(op, /*disabled_opcodes_reenabled=*/false));
        // ...but after reactivation they must execute normally instead.
        BOOST_CHECK(!IsOpSuccess(op, /*disabled_opcodes_reenabled=*/true));
    }

    // Genuine OP_SUCCESSx opcodes are unaffected by the reactivation flag.
    for (const opcodetype op : {static_cast<opcodetype>(80), static_cast<opcodetype>(98),
                                static_cast<opcodetype>(187), static_cast<opcodetype>(254)}) {
        BOOST_CHECK(IsOpSuccess(op, false));
        BOOST_CHECK(IsOpSuccess(op, true));
    }
}

BOOST_AUTO_TEST_CASE(tapscript_verify_script_activation_transition)
{
    const std::vector<unsigned char> a{0xaa};
    const std::vector<unsigned char> b{0xbb};
    const std::vector<unsigned char> wrong{0xff, 0xff};
    const CScript script = CScript() << a << b << OP_CAT << wrong << OP_EQUAL;
    const std::vector<unsigned char> output_key = ParseHex("acb23454d77794fdeb6e0fa000e016fdbc7bbf798fa03b34ed23082076d7752e");
    const std::vector<unsigned char> control = ParseHex("c079be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798");
    const CScript script_pub_key = CScript() << OP_1 << output_key;

    CScriptWitness witness;
    witness.stack.emplace_back(script.begin(), script.end());
    witness.stack.push_back(control);

    ScriptError err;
    const unsigned int taproot_flags = SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS | SCRIPT_VERIFY_TAPROOT;

    BOOST_CHECK(VerifyScript(CScript(), script_pub_key, &witness, taproot_flags,
                             BaseSignatureChecker(), &err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_OK);

    BOOST_CHECK(!VerifyScript(CScript(), script_pub_key, &witness, taproot_flags | REENABLED,
                              BaseSignatureChecker(), &err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_EVAL_FALSE);
}

#if defined(HAVE_CONSENSUS_LIB)
BOOST_AUTO_TEST_CASE(libconsensus_taproot_requires_spent_outputs)
{
    const std::vector<unsigned char> output_key = ParseHex("acb23454d77794fdeb6e0fa000e016fdbc7bbf798fa03b34ed23082076d7752e");
    const std::vector<unsigned char> control = ParseHex("c079be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798");
    const CScript script = CScript() << std::vector<unsigned char>{0xaa}
                                     << std::vector<unsigned char>{0xbb}
                                     << OP_CAT
                                     << std::vector<unsigned char>{0xff, 0xff}
                                     << OP_EQUAL;
    const CScript script_pub_key = CScript() << OP_1 << output_key;

    CMutableTransaction tx;
    tx.vin.resize(1);
    tx.vout.emplace_back(1, CScript() << OP_1);
    tx.vin[0].scriptWitness.stack.emplace_back(script.begin(), script.end());
    tx.vin[0].scriptWitness.stack.push_back(control);

    CDataStream tx_stream(SER_NETWORK, PROTOCOL_VERSION);
    tx_stream << tx;
    CDataStream spent_stream(SER_NETWORK, PROTOCOL_VERSION);
    spent_stream << std::vector<CTxOut>{CTxOut(1000, script_pub_key)};

    const unsigned int taproot_flags = bitcoinconsensus_SCRIPT_FLAGS_VERIFY_P2SH |
                                       bitcoinconsensus_SCRIPT_FLAGS_VERIFY_WITNESS |
                                       bitcoinconsensus_SCRIPT_FLAGS_VERIFY_TAPROOT;
    bitcoinconsensus_error err;
    const auto* tx_data = reinterpret_cast<const unsigned char*>(tx_stream.data());
    const auto* spent_data = reinterpret_cast<const unsigned char*>(spent_stream.data());

    BOOST_CHECK_EQUAL(bitcoinconsensus_verify_script_with_amount(
        script_pub_key.data(), script_pub_key.size(), 1000, tx_data, tx_stream.size(),
        0, taproot_flags, &err), 0);
    BOOST_CHECK_EQUAL(err, bitcoinconsensus_ERR_SPENT_OUTPUTS_REQUIRED);

    BOOST_CHECK_EQUAL(bitcoinconsensus_verify_script_with_spent_outputs(
        script_pub_key.data(), script_pub_key.size(), tx_data, tx_stream.size(),
        spent_data, spent_stream.size(), 0, taproot_flags, &err), 1);
    BOOST_CHECK_EQUAL(err, bitcoinconsensus_ERR_OK);

    BOOST_CHECK_EQUAL(bitcoinconsensus_verify_script_with_spent_outputs(
        script_pub_key.data(), script_pub_key.size(), tx_data, tx_stream.size(),
        spent_data, spent_stream.size(), 0,
        taproot_flags | bitcoinconsensus_SCRIPT_FLAGS_VERIFY_DISABLED_OPCODES_REENABLED,
        &err), 0);
    BOOST_CHECK_EQUAL(err, bitcoinconsensus_ERR_OK);

    CDataStream wrong_spent_stream(SER_NETWORK, PROTOCOL_VERSION);
    wrong_spent_stream << std::vector<CTxOut>{CTxOut(1000, CScript() << OP_1)};
    BOOST_CHECK_EQUAL(bitcoinconsensus_verify_script_with_spent_outputs(
        script_pub_key.data(), script_pub_key.size(), tx_data, tx_stream.size(),
        reinterpret_cast<const unsigned char*>(wrong_spent_stream.data()),
        wrong_spent_stream.size(), 0, taproot_flags, &err), 0);
    BOOST_CHECK_EQUAL(err, bitcoinconsensus_ERR_SPENT_OUTPUTS_MISMATCH);
}
#endif

BOOST_AUTO_TEST_CASE(tapscript_op_cat_is_not_anyone_can_spend)
{
    // Regression test for the critical bug: with OP_CAT treated as
    // OP_SUCCESSx, a tapleaf covenant would succeed without executing,
    // making it spendable by anyone. Under TAPSCRIPT with the reactivation
    // flag the script must actually run, so a script that concatenates two
    // values and compares against the wrong result has to fail.
    ScriptError err;
    std::vector<std::vector<unsigned char>> stack;

    const std::vector<unsigned char> a{0xaa};
    const std::vector<unsigned char> b{0xbb};
    const std::vector<unsigned char> wrong{0xff, 0xff};

    const CScript script = CScript() << a << b << OP_CAT << wrong << OP_EQUAL;

    BOOST_CHECK(Eval(script, REENABLED, err, stack, SigVersion::TAPSCRIPT));
    // EvalScript succeeds, but the comparison must be false (not a free spend).
    BOOST_CHECK_EQUAL(stack.size(), 1U);
    BOOST_CHECK(stack.back().empty()); // OP_EQUAL pushed false

    // And the correct concatenation must evaluate true.
    stack.clear();
    const std::vector<unsigned char> correct{0xaa, 0xbb};
    const CScript good = CScript() << a << b << OP_CAT << correct << OP_EQUAL;
    BOOST_CHECK(Eval(good, REENABLED, err, stack, SigVersion::TAPSCRIPT));
    BOOST_CHECK_EQUAL(stack.size(), 1U);
    BOOST_CHECK(stack.back() == std::vector<unsigned char>({0x01})); // true
}

// ---------------------------------------------------------------------------
// Fix #2: OP_SUBSTR bounds must not overflow
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(op_substr_overflow_rejected)
{
    ScriptError err;
    const std::vector<unsigned char> data{0x11, 0x22, 0x33, 0x44};

    // nBegin + nSize overflows signed int; must be rejected, not accepted via
    // a wrapped comparison (which previously read out of bounds).
    const CScript overflow = CScript() << data << CScriptNum(0x40000000)
                                       << CScriptNum(0x40000000) << OP_SUBSTR;
    BOOST_CHECK(!EvalOk(overflow, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

    // Out-of-range begin.
    const CScript past_end = CScript() << data << CScriptNum(5) << CScriptNum(0) << OP_SUBSTR;
    BOOST_CHECK(!EvalOk(past_end, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

    // Begin in range but size runs past the end.
    const CScript too_long = CScript() << data << CScriptNum(2) << CScriptNum(3) << OP_SUBSTR;
    BOOST_CHECK(!EvalOk(too_long, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

    // Valid extraction still works, including the exact-boundary case.
    std::vector<std::vector<unsigned char>> stack;
    const CScript valid = CScript() << data << CScriptNum(1) << CScriptNum(2) << OP_SUBSTR;
    BOOST_CHECK(Eval(valid, REENABLED, err, stack));
    BOOST_CHECK_EQUAL(stack.size(), 1U);
    BOOST_CHECK(stack.back() == std::vector<unsigned char>({0x22, 0x33}));

    stack.clear();
    const CScript boundary = CScript() << data << CScriptNum(4) << CScriptNum(0) << OP_SUBSTR;
    BOOST_CHECK(Eval(boundary, REENABLED, err, stack));
    BOOST_CHECK_EQUAL(stack.size(), 1U);
    BOOST_CHECK(stack.back().empty());
}

// ---------------------------------------------------------------------------
// Fix #3: shifts must reject negative operands (UB / consensus divergence)
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(shift_negative_operand_rejected)
{
    ScriptError err;

    for (const opcodetype op : {OP_LSHIFT, OP_RSHIFT}) {
        // Negative value being shifted.
        const CScript neg_value = CScript() << CScriptNum(-8) << CScriptNum(1) << op;
        BOOST_CHECK(!EvalOk(neg_value, REENABLED, err));
        BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

        // Negative shift count.
        const CScript neg_count = CScript() << CScriptNum(8) << CScriptNum(-1) << op;
        BOOST_CHECK(!EvalOk(neg_count, REENABLED, err));
        BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

        // Shift count at or beyond the width limit.
        const CScript wide = CScript() << CScriptNum(1) << CScriptNum(32) << op;
        BOOST_CHECK(!EvalOk(wide, REENABLED, err));
        BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);
    }

    // Well-formed shifts still work.
    std::vector<std::vector<unsigned char>> stack;
    const CScript lsh = CScript() << CScriptNum(3) << CScriptNum(4) << OP_LSHIFT;
    BOOST_CHECK(Eval(lsh, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == CScriptNum(48).getvch());

    stack.clear();
    const CScript rsh = CScript() << CScriptNum(48) << CScriptNum(4) << OP_RSHIFT;
    BOOST_CHECK(Eval(rsh, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == CScriptNum(3).getvch());
}

// ---------------------------------------------------------------------------
// Fix #5: arithmetic results are bounded to 4-byte CScriptNum
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(arithmetic_overflow_rejected)
{
    ScriptError err;

    // OP_MUL overflowing the 4-byte CScriptNum range fails deterministically
    // at the point of multiplication.
    const CScript mul_overflow = CScript() << CScriptNum(0x7fffffff) << CScriptNum(2) << OP_MUL;
    BOOST_CHECK(!EvalOk(mul_overflow, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

    const CScript mul_overflow_neg = CScript() << CScriptNum(-0x7fffffff) << CScriptNum(2) << OP_MUL;
    BOOST_CHECK(!EvalOk(mul_overflow_neg, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

    const CScript two_mul_overflow = CScript() << CScriptNum(0x7fffffff) << OP_2MUL;
    BOOST_CHECK(!EvalOk(two_mul_overflow, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

    const CScript shift_overflow = CScript() << CScriptNum(0x7fffffff) << CScriptNum(1) << OP_LSHIFT;
    BOOST_CHECK(!EvalOk(shift_overflow, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

    // In-range arithmetic is unaffected.
    std::vector<std::vector<unsigned char>> stack;
    const CScript mul_ok = CScript() << CScriptNum(30000) << CScriptNum(7) << OP_MUL;
    BOOST_CHECK(Eval(mul_ok, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == CScriptNum(210000).getvch());

    // Exact boundary: ±nMaxScriptNumValue (0x7fffffff) must be accepted.
    stack.clear();
    const CScript mul_pos_boundary = CScript() << CScriptNum(0x7fffffff) << CScriptNum(1) << OP_MUL;
    BOOST_CHECK(Eval(mul_pos_boundary, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == CScriptNum(0x7fffffff).getvch());

    stack.clear();
    const CScript mul_neg_boundary = CScript() << CScriptNum(-0x7fffffff) << CScriptNum(1) << OP_MUL;
    BOOST_CHECK(Eval(mul_neg_boundary, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == CScriptNum(-0x7fffffff).getvch());

    // One past the boundary must fail.
    const CScript mul_pos_past = CScript() << CScriptNum(0x40000000) << CScriptNum(2) << OP_MUL;
    BOOST_CHECK(!EvalOk(mul_pos_past, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);

    const CScript mul_neg_past = CScript() << CScriptNum(-0x40000000) << CScriptNum(2) << OP_MUL;
    BOOST_CHECK(!EvalOk(mul_neg_past, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);
}

// ---------------------------------------------------------------------------
// Remaining opcode semantics
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(op_cat_respects_element_size_limit)
{
    ScriptError err;
    const std::vector<unsigned char> big(MAX_SCRIPT_ELEMENT_SIZE, 0x00);
    const std::vector<unsigned char> one{0x01};

    const CScript too_big = CScript() << big << one << OP_CAT;
    BOOST_CHECK(!EvalOk(too_big, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_PUSH_SIZE);
}

BOOST_AUTO_TEST_CASE(bitwise_requires_equal_sizes)
{
    ScriptError err;
    const std::vector<unsigned char> a{0x0f, 0x0f};
    const std::vector<unsigned char> b{0xff};

    for (const opcodetype op : {OP_AND, OP_OR, OP_XOR}) {
        const CScript mismatched = CScript() << a << b << op;
        BOOST_CHECK(!EvalOk(mismatched, REENABLED, err));
        BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);
    }

    std::vector<std::vector<unsigned char>> stack;
    const std::vector<unsigned char> mask{0xf0, 0xff};
    const CScript and_ok = CScript() << a << mask << OP_AND;
    BOOST_CHECK(Eval(and_ok, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == std::vector<unsigned char>({0x00, 0x0f}));
}

BOOST_AUTO_TEST_CASE(div_mod_by_zero_rejected)
{
    ScriptError err;
    for (const opcodetype op : {OP_DIV, OP_MOD}) {
        const CScript by_zero = CScript() << CScriptNum(10) << CScriptNum(0) << op;
        BOOST_CHECK(!EvalOk(by_zero, REENABLED, err));
        BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);
    }
}

// ---------------------------------------------------------------------------
// OP_LEFT / OP_RIGHT / OP_INVERT semantics
// ---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(op_left_semantics)
{
    ScriptError err;
    const std::vector<unsigned char> data{0x11, 0x22, 0x33, 0x44};
    std::vector<std::vector<unsigned char>> stack;

    // Take left 2 bytes.
    const CScript left2 = CScript() << data << CScriptNum(2) << OP_LEFT;
    BOOST_CHECK(Eval(left2, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == std::vector<unsigned char>({0x11, 0x22}));

    // Clamping: nSize > data length returns the whole input.
    stack.clear();
    const CScript left_clamp = CScript() << data << CScriptNum(100) << OP_LEFT;
    BOOST_CHECK(Eval(left_clamp, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == data);

    // Zero length returns empty.
    stack.clear();
    const CScript left_zero = CScript() << data << CScriptNum(0) << OP_LEFT;
    BOOST_CHECK(Eval(left_zero, REENABLED, err, stack));
    BOOST_CHECK(stack.back().empty());

    // Negative size is rejected.
    const CScript left_neg = CScript() << data << CScriptNum(-1) << OP_LEFT;
    BOOST_CHECK(!EvalOk(left_neg, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);
}

BOOST_AUTO_TEST_CASE(op_right_semantics)
{
    ScriptError err;
    const std::vector<unsigned char> data{0x11, 0x22, 0x33, 0x44};
    std::vector<std::vector<unsigned char>> stack;

    // Take right 2 bytes.
    const CScript right2 = CScript() << data << CScriptNum(2) << OP_RIGHT;
    BOOST_CHECK(Eval(right2, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == std::vector<unsigned char>({0x33, 0x44}));

    // Clamping: nSize > data length returns the whole input.
    stack.clear();
    const CScript right_clamp = CScript() << data << CScriptNum(100) << OP_RIGHT;
    BOOST_CHECK(Eval(right_clamp, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == data);

    // Zero length returns empty.
    stack.clear();
    const CScript right_zero = CScript() << data << CScriptNum(0) << OP_RIGHT;
    BOOST_CHECK(Eval(right_zero, REENABLED, err, stack));
    BOOST_CHECK(stack.back().empty());

    // Negative size is rejected.
    const CScript right_neg = CScript() << data << CScriptNum(-1) << OP_RIGHT;
    BOOST_CHECK(!EvalOk(right_neg, REENABLED, err));
    BOOST_CHECK_EQUAL(err, SCRIPT_ERR_INVALID_STACK_OPERATION);
}

BOOST_AUTO_TEST_CASE(op_invert_semantics)
{
    ScriptError err;
    std::vector<std::vector<unsigned char>> stack;

    // Invert flips all bits.
    const std::vector<unsigned char> input{0x0f, 0xf0, 0x00, 0xff};
    const CScript invert_script = CScript() << input << OP_INVERT;
    BOOST_CHECK(Eval(invert_script, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == std::vector<unsigned char>({0xf0, 0x0f, 0xff, 0x00}));

    // Double-invert yields the original.
    stack.clear();
    const CScript double_invert = CScript() << input << OP_INVERT << OP_INVERT;
    BOOST_CHECK(Eval(double_invert, REENABLED, err, stack));
    BOOST_CHECK(stack.back() == input);

    // Invert on empty element.
    stack.clear();
    const std::vector<unsigned char> empty{};
    const CScript invert_empty = CScript() << empty << OP_INVERT;
    BOOST_CHECK(Eval(invert_empty, REENABLED, err, stack));
    BOOST_CHECK(stack.back().empty());
}

BOOST_AUTO_TEST_SUITE_END()

