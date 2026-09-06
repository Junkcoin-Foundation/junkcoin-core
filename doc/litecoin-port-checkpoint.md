# Litecoin upstream port checkpoint

Checkpoint terakhir: 2026-08-17 (dev head `5a5919b99`)

## Tujuan file ini

Daftar kemajuan port fix dari Litecoin `24.x` ke Junkcoin Core, supaya
dev/AI agent berikutnya bisa melanjutkan dari sini tanpa menganalisa ulang
history. Base Junkcoin = Litecoin 0.21.4 (`84d4b4ea6`), jadi patch tidak
pernah apply bersih otomatis — selalu perlu adaptasi.

## Cara cek update Litecoin baru

```sh
git fetch litecoin
git merge-base master litecoin/24.x   # base fork (0.21.4 era)
# Commit terakhir yang sudah ditriage di 24.x: f538ed5aa (lihat tabel di bawah).
# Lihat commit litecoin baru di atas checkpoint itu:
git log --oneline f538ed5aa..litecoin/24.x
```
Patokan yang benar: commit `f538ed5aa` ke bawah s/d `7fa962b33` sudah
ditriage (55 commit). Commit baru Litecoin = semua yang muncul `--not f538ed5aa`
di `litecoin/24.x`. Triage:

1. `git merge-base master litecoin/24.x` — base aman (fork 0.21.4).
2. Filter commit yang header-nya `fix:`, `net:`, `validation:`, `build:`,
   `policy:`, `mining:`, `mweb:` dan tidak menyentuh wallet/Qt GUI (feature MWEB-wallet 24.x tidak relevan untuk base 0.21).
3. Test apply: `git show <sha> | git apply --check -` — hampir pasti gagal
   (context drift 24.x). Jika gagal, port manual dengan diff.
4. Untuk consensus (Kernel, params, validation): ikuti pola commit JKC yang
   sudah ada — set height aktivasi mengikuti `MWEBHeight`/`DisabledScriptReactivationHeight`
   milik JKC, jangan pakai height Litecoin mainnet.

## Sudah diport (9 fix) — upstream -> JKC

| Upstream (litecoin/24.x) | Fix | Commit JKC (dev) |
|---|---|---|
| `f538ed5aa` | validation: fix script check data lifetime | `6ae2b48c7` |
| `1db3fd05f` | mweb: reject empty kernel feature payloads | `5919e6feb` |
| `1723dc636` | mining: enforce aggregate MWEB input limits | `99db445f9` |
| `3069b4a89` | net: rate-limit MWEB state serving node-wide | `9780afc9a` |
| `6776c4d04` | policy: limit MWEB mempool verification cost | `a14bf88df` |
| `127ee52de` | validation: recover from mutated MWEB block bodies | `c2a72cfdc` |
| `98be688a5` | net: discourage mutated MWEB compact blocks | `578826014` |
| `72278f9d9` | mempool: serialize pegout status in transaction undo | `19e8c4f5a` |
| `1066167b8` | build: remove macOS bind_at_load linker flag | `5a5919b99` |

Semua commit bertanda "Backport litecoin/24.x <sha>" — verify dengan
`git show <jkc-sha> | head -30`.

## Adaptasi khusus JKC yang perlu diingat

- `MWEBHeight` (params.h) menjadi patokan height aktivasi MWEB; commit kernel
  feature pakai `mweb_pegout_feature_activation_height` =
  `mweb_extradata_feature_activation_height` = `MWEBHeight` tiap network
  (mainnet INT_MAX, testnet 180000, regtest 0). JANGAN pakai angka Litecoin.
- JKC 0.21: `Kernel` di global scope (bukan `mw::Kernel`), `namespace node`
  TIDAK ada di miner.cpp, `PF_NOBAN` (bukan `NetPermissionFlags::NoBan`),
  `fRequireStandard` global (bukan `m_pool.m_require_standard`),
  `CChainState`/`PeerManager` (bukan `Chainstate`/`PeerManagerImpl`),
  free-function `ProcessGetMWEBLeafset/UTXOs`.
- `mw::MAX_NUM_INPUTS` (50.000) dan `mw::MAX_BLOCK_WEIGHT` (200.000) ada di
  libmw 0.21 — constant sama dengan 24.x.
- Ulang polish `src/crypto/scrypt.{h,cpp}` tetap upstream 0.21 — jangan
  tarik versi 24.x.

## Ditriage dan TIDAK diport (55 commit litecoin 24.x)

### Feature / non-fix (skip, tidak akan diport)
- 44 commit MWEB-wallet/QT/docs/psbt/descriptor/addrs — implementasi wallet
  penuh MWEB 24.x, beda arsitektur dari 0.21 (wallet tidak aktif di JKC mainnet).
- `40db64c78` fix network test race — test-only untuk framework 24.x, skip.
- `5c2091ec3`, `3a0e153f9`, `99cc83f1b`, `9133f169f`, `a5a5cc1af`,
  `757e1d003`, `36db25b81`, `25b0b0e55`, `37b37038d`, `b16270243`,
  `fbe9bddeb`, `bbbc7e95c`, `4af3bbdd7`, `194cb94c4`, `7fa962b33`,
  `d8aa1b7b3`, `da1dcb4c7`, `e7d5c115e`, `f90bdcdaf`, `4d826f78e` —
  setup project Litecoin (chainparams/version/pow/key_io/terminologi),
  sudah tidak relevan untuk JKC yang sudah branded. JANGAN apply.

### Fix yang belum diport (kandidat untuk agent berikutnya)
- `37fee9c35` net: resume low-work headers presync across peers — butuh
  headerssync (bitcoin 24), tidak ada di 0.21; port besar. DELIBERASI DULU.
- `ec5bc4f61` validation: parallelize header proof-of-work checks — port
  ParallelBlockHeaderChecks dari 24.x; uji dengan header presync di atas.
- `b5472bdc8` wallet: migrate v0.21 pure MWEB tx keys — hanya relevan jika
  JKC mulai support MWEB wallet; delayed.
- `3db9ea50f` test: add Litecoin scrypt Python module — util test, optional.
- `1066167b8/...` sudah di atas.

### Fix MWEB 24.x yang TIDAK diport karena beda arsitektur (re-evaluasi jika libmw naik)
- `b338ab731`, `a900883fa` (libmw konsensus library refactor) — beda layout,
  skip permanen (base 0.21).
- `2ecc468d4`, `dc6fe354f`, `af156e64b` — test infrastructure 24.x p2p,
  skip (framework berbeda).

## Catatan testing (2026-08-17)

- Build: `make -j$(nproc)` OK (junkcoind produk jadi).
- Unit pass: `coins_tests` (termasuk txinundo_serialization),
  `validation_tests`, `reenabled_opcodes_tests`, `script_tests`,
  `script_standard_tests`, `interpreter_tests`, `TestKernel`,
  `TestBlockBuilder`.
- `miner_tests` + `blockfilter_index_tests` GAGAL di environment ini
  SEBELUM dan SESUDAH port (111 failure, error `bad-txns-inputs-missingorspent`
  / `extra-hogex-input`) — pre-existing, bukan regresi port. Verifikasi
  baseline di `/tmp/opencode/jkc-baseline` (worktree `8e28ed983`).
- Functional test `mweb_*` belum dijalankan (butuh regtest penuh + waktu).
- Run berikutnya: setelah verification baseline, jalankan functional
  `test/functional/test_runner.py mweb_p2p mweb_mining mweb_weight mweb_undo`
  (mweb_undo.py baru update dari commit 72278f9d9).