# Junkcoin Core — Release Checklist

One release = one tagged, tested, reproducible set of binaries that people run
with real money. A consensus bug shipped in a release splits the chain. Work
through this checklist top to bottom and tick every box before publishing.

## 0. Pre-release hygiene (2–4 weeks before tag)

- [ ] `git log` review: every commit on `dev` merged to `master` has a PR,
      an author, and a reason. No `wip`, no force-pushed history.
- [ ] Branches: only `master` (production) + short-lived feature branches.
      Delete merged feature branches.
- [ ] `git diff dev master` shows exactly what you intend to ship, nothing else.

## 1. Consensus changes (if any)

- [ ] Every consensus change has its **own activation height** (or BIP9 flag)
      in `src/consensus/params.h` + `src/chainparams.cpp`.
- [ ] Ordering enforced by `ValidateOpcodeActivationOrdering` still passes for
      mainnet, testnet, and regtest (run `src/test/test_bitcoin
      --run_test=chainparams_tests`).
- [ ] Activation heights are **shifted +20k on testnet** relative to mainnet,
      so the testnet activates *first*.
- [ ] Testnet has been running with the new rules for at least one full
      activation window with no reorgs/splits.
- [ ] Test vectors exist (`src/test/*`, `test/functional/*`) and pass.
- [ ] Effect on old nodes is documented: hardfork (old nodes fork off) vs
      softfork (old nodes keep validating) — written in the release notes.

### v4.0.2 — consensus change to document in release notes

- [ ] **Kernel feature-payload rule** (commit `9b1222669`, backport of
      litecoin `1db3fd05f`): blocks containing kernels with the
      `PEGOUT_FEATURE_BIT` or `EXTRA_DATA_FEATURE_BIT` set but an **empty
      payload** are rejected. Activates together with MWEB itself
      (`mweb_pegout_feature_activation_height` = `mweb_extradata_feature_activation_height`
      = `MWEBHeight`): testnet **180,000**, mainnet **disabled**
      (`INT_MAX`), regtest 0.
- [ ] Effect on old nodes: before the activation height behavior is
      identical (no fork). At testnet height 180,000 an old node would
      accept such kernels while a new node rejects them — **all nodes must
      run v4.0.2+ before testnet 180,000**. Mainnet is unaffected.
- [ ] Release notes must state: "opcode re-activation rules (OP_CAT, OP_MUL,
      etc. at testnet 160,000) and MWEB kernel rules (180,000) are final."

## 2. Code freeze & testing

- [ ] `make -j$(nproc) && make check` green on the release branch.
- [ ] `test/functional/test_runner.py` green (or the curated subset; note
      which tests were skipped and why).
- [ ] At least one clean **reindex + assumevalid=0** sync on mainnet and
      testnet (catches block-rejection defects in history).
- [ ] Upgrade path tested: wallet from previous version upgrades without data
      loss (functional `wallet_upgrade`/`mweb_wallet_upgrade`).
- [ ] RPC + mining: `getblocktemplate`/`submitblock` round-trip tested with a
      real miner (aux-pow path included if mining is active).

## 3. Version & release-critical files

- [ ] `configure.ac` version bumped, `CLIENT_VERSION_IS_RELEASE` true.
- [ ] `contrib/gitian-descriptors/*.yml` versions updated.
- [ ] `chainTxData`, `nMinimumChainWork`, `defaultAssumeValid` updated from
      `getchaintxstats` / `getblockheader` (values a few blocks behind tip).
- [ ] Seeds refreshed (`contrib/seeds/makeseeds.py`).
- [ ] Manpages regenerated (`contrib/devtools/gen-manpages.sh`).

## 4. Release notes

- [ ] Written as `doc/release-notes.md` in the repo.
- [ ] Sections: consensus changes (activation heights!), features, bug fixes,
      compatibility notes for old nodes, upgrading instructions.
- [ ] Reviewed by someone who was not the author.

## 5. Tag & builds

- [ ] Tag signed: `git tag -s vX.Y.Z` (GPG key used for releases).
- [ ] Reproducible builds (gitian or CI cross-compile) for linux/win/osx.
- [ ] Two or more independent builds produce identical hashes.

## 6. GitHub release

- [ ] Upload binaries + debug files *and* `SHA256SUMS.asc`
      (`sha256sum * > SHA256SUMS`, then `gpg --clearsign`).
- [ ] Release title: `Junkcoin Core vX.Y.Z`; body = release notes +
      activation heights + link to source tag.
- [ ] Tag set as "latest" only after binaries verified from two sources.

## 7. Announce

- [ ] Announce on the project's public channel (GitHub discussions / site)
      with: version, consensus changes, upgrade urgency ("upgrade before
      height N"), and checksum verification instructions.

## 8. Post-release

- [ ] `master` tagged; `dev` rebased/merged to continue development.
- [ ] If a consensus bug is found: **do not silently patch.** Follow the
      softfork/hardfork decision process; the failure mode is a chain split.
