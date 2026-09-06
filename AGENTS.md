# AGENTS.md — Rules for AI coding agents

This repository is a cryptocurrency node (Junkcoin Core, a Litecoin 0.21 fork).
Every change here can affect people's money. A consensus bug can split the chain
and cause real financial loss. Follow these rules strictly.

## Repo context

- Base: Litecoin 0.21 / MWEB; Junkcoin-specific: MWEB service-flag gating,
  legacy opcode reactivation (OP_CAT, OP_MUL, etc.), tapscript OP_SUCCESS
  carve-out, legacy HD wallet paths, aux-pow mining support.
- Testnet activation heights are shifted (+20k vs upstream) and consensus
  ordering is enforced by `ValidateOpcodeActivationOrdering` in chainparams.

## CRITICAL areas — treat as high-risk, touch only with approval

Any edit to these files must be treated as a consensus/security review, not a
code edit:

- `src/consensus/` — all consensus rules
- `src/script/` — opcode semantics, script validation (ANY change can create
  an "anyone can spend" bug or reorg/mempool split)
- `src/chainparams.cpp` / `src/consensus/params.h` — activation heights,
  powLimit, seeds, assumed chain stats
- `src/validation.cpp` — block/tx acceptance rules
- `src/crypto/` — signature, hashing, ZK code; never "simplify" or refactor
- `src/mweb/` and MWEB wallet code — confidential transaction consensus
- wallet key derivation / HD paths — losing funds is the failure mode

## Hard rules

1. **Never commit, push, merge, tag, or force-push without an explicit
   instruction from the user.** Present the diff and wait for approval.
2. **Never delete or rename branches**, and never rebase shared/pushed branches
   without explicit instruction.
3. **No silent behavior changes.** If a change alters validation, serialization,
   fee/reward math, or any default, it must be called out explicitly.
4. **Consensus rule changes** (new opcode, changed opcode, new activation
   height, fee/subsidy change) require: (a) a new activation height or flag,
   (b) test vectors, (c) functional/unit tests, (d) explanation of the effect on
   old nodes. Never change mainnet consensus rules "just to fix" something.
5. **Always verify before reporting done:**
   - C++: build and run relevant unit tests
     (`make -j$(nproc) && make check` picks up everything)
   - Script/RPC/consensus changes: also run the affected functional test
     (`test/functional/test_runner.py <test_name>`), e.g.
     `reenabled_opcodes_tests`, `mweb_*`, `jkc_replacebyfee`.
   - Go through `git diff` and read every line before presenting the change.
6. **Don't vendor or rewrite cryptographic code.** Only fix real bugs; propose
   changes, don't silently swap implementations (libsecp256k1, bls, zk libs).
7. **Do not modify release-critical files** (`configure.ac` version fields,
   gitian descriptors, seed lists, `chainTxData`/`nMinimumChainWork`/
   `defaultAssumeValid`) unless the user explicitly asks.
8. **Research before editing.** Understand how the code is used elsewhere
   (grep callers, check activation ordering, BIP docs) before writing.

## Recommended workflow for a risky change

1. State the problem and the plan (one paragraph) before touching code.
2. Make the smallest possible diff; one logical change per commit.
3. Add or update tests in the same change.
4. Build, run tests, then show `git diff` and ask for approval.

## Commit style

- Conventional Commits: `feat:`, `fix:`, `refactor:`, `test:`, `chore:`, `build:`, `ci:`, `docs:`.
- One commit = one logical unit. No `wip` commits, no mixed changes.
- Never commit secrets, keys, or wallet data.
- Match the tone of existing history (repo already follows this style).

## Branch policy (current)

- `master` — production line, protected. Only via reviewed merge.
- `dev` — integration.
- feature branches — work happens here, then PR to `dev`/`master`.