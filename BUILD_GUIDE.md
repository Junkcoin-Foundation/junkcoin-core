# Junkcoin Core v2 Build Guide

This guide explains how to build Junkcoin Core v2 from source.

## Quick Start (Recommended)

### Using Devcontainer (Easiest)

The devcontainer provides a pre-configured build environment with all dependencies:

```bash
# 1. Open in VS Code with Dev Containers extension
# 2. Press F1 → "Dev Containers: Reopen in Container"
# 3. Wait for container to build
# 4. Run the build script:
./build.sh
```

See [.devcontainer/README.md](.devcontainer/README.md) for details.

---

## Build Methods

### Method 1: Devcontainer (Recommended)

**Pros:**
- ✅ All dependencies pre-installed
- ✅ Consistent build environment
- ✅ Fast builds (uses system packages)
- ✅ Works on any OS (Windows/Mac/Linux)

**Cons:**
- ❌ Requires Docker

**Steps:**
1. Install Docker and VS Code with Dev Containers extension
2. Open `junkcoin-core-v2` folder in VS Code
3. Reopen in container
4. Run `./build.sh`

---

### Method 2: System Libraries (Linux)

**Pros:**
- ✅ Fast builds
- ✅ No Docker required

**Cons:**
- ❌ Must install dependencies manually
- ❌ Different on each distro

**Ubuntu/Debian:**
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
  build-essential libtool autotools-dev automake pkg-config \
  libboost-system-dev libboost-filesystem-dev libboost-test-dev \
  libboost-thread-dev libssl-dev libevent-dev libdb-dev libdb++-dev \
  libzmq3-dev libminiupnpc-dev libqrencode-dev

# Build
./autogen.sh
./configure --with-incompatible-bdb
make -j$(nproc)
```

**Arch Linux:**
```bash
# Install dependencies
sudo pacman -S base-devel boost libevent db zeromq miniupnpc qrencode

# Build
./autogen.sh
./configure --with-incompatible-bdb
make -j$(nproc)
```

---

### Method 3: Depends System (Cross-platform)

**Pros:**
- ✅ Reproducible builds
- ✅ Cross-compilation support
- ✅ No system dependencies

**Cons:**
- ❌ Very slow (builds all dependencies from source)
- ❌ Requires ~10GB disk space
- ❌ Takes 1-2 hours on first build

**Steps:**
```bash
# Build dependencies (takes 1-2 hours)
cd depends
make -j$(nproc)
cd ..

# Configure with depends
./autogen.sh
./configure --prefix=$(pwd)/depends/x86_64-pc-linux-gnu
make -j$(nproc)
```

---

## Build Configuration Options

### Minimal Build (No GUI, No Tests)
```bash
./configure \
  --without-gui \
  --disable-tests \
  --disable-bench \
  --with-incompatible-bdb
```

### Full Build (With GUI and Tests)
```bash
./configure \
  --with-gui=qt5 \
  --enable-tests \
  --enable-bench \
  --with-incompatible-bdb
```

### Debug Build
```bash
./configure \
  --enable-debug \
  --with-incompatible-bdb \
  CXXFLAGS="-O0 -g3" \
  CFLAGS="-O0 -g3"
```

### Release Build (Optimized)
```bash
./configure \
  --enable-reduce-exports \
  --with-incompatible-bdb \
  CXXFLAGS="-O3 -march=native" \
  CFLAGS="-O3 -march=native"
```

---

## Build Output

After successful build, binaries are in `src/`:

| Binary | Description |
|--------|-------------|
| `junkcoind` | Junkcoin daemon (node + wallet) |
| `junkcoin-cli` | Command-line RPC client |
| `junkcoin-tx` | Transaction creation/manipulation tool |
| `junkcoin-wallet` | Wallet management tool |
| `junkcoin-qt` | GUI wallet (if built with `--with-gui`) |

---

## Installation

### Install to system (optional)
```bash
sudo make install
```

### Or run from build directory
```bash
# Start daemon
./src/junkcoind -daemon

# Use CLI
./src/junkcoin-cli getblockchaininfo

# Stop daemon
./src/junkcoin-cli stop
```

---

## Troubleshooting

### "configure: error: libdb_cxx headers missing"

**Solution:** Install BerkeleyDB development package:
```bash
# Ubuntu/Debian
sudo apt-get install libdb-dev libdb++-dev

# Or use incompatible BDB flag
./configure --with-incompatible-bdb
```

### "cannot find -lboost_system"

**Solution:** Install Boost development packages:
```bash
# Ubuntu/Debian
sudo apt-get install libboost-all-dev

# Arch
sudo pacman -S boost
```

### "autogen.sh: command not found"

**Solution:** Make it executable:
```bash
chmod +x autogen.sh
./autogen.sh
```

### Build fails with "virtual memory exhausted"

**Solution:** Reduce parallel jobs:
```bash
# Instead of make -j$(nproc), use fewer cores:
make -j2
```

### Linking errors with depends build

**Solution:** Make sure depends finished building:
```bash
cd depends
make -j$(nproc)
# Wait for "Done building ..." message
cd ..
```

---

## Development Fund Compatibility

Junkcoin Core v2 includes full development fund support:

- **Active Period:** Blocks 365,001 to 3,547,800
- **Amount:** 20% of base block reward (excluding fees)
- **Addresses:** 3 rotating development fund addresses
- **Validation:** Blocks without dev fund outputs are rejected during active period

---

## Feature Activation Heights

| Feature | Activation Height | Status |
|---------|------------------|--------|
| AuxPoW | 173,000 | ✅ Active |
| BIP34/65/66 | 8,460 | ✅ Active |
| Development Fund | 365,001 - 3,547,800 | ✅ Active |
| CSV | 700,000 | ⏳ Scheduled |
| SegWit | 700,000 | ⏳ Scheduled |
| Taproot | 800,000 | ⏳ Scheduled |
| MWEB | 900,000 | ⏳ Scheduled |

---

## Testing Your Build

### Quick test
```bash
./src/junkcoind -version
```

### Connect to testnet
```bash
./src/junkcoind -testnet -daemon
./src/junkcoin-cli -testnet getblockchaininfo
```

### Connect to mainnet
```bash
./src/junkcoind -daemon
./src/junkcoin-cli getblockchaininfo
```

### Verify development fund
```bash
# Check a block in dev fund range (e.g., block 400000)
./src/junkcoin-cli getblock $(./src/junkcoin-cli getblockhash 400000) 2

# Should show development fund output in coinbase transaction
```

---

## Clean Build

```bash
# Clean build artifacts
make distclean

# Or if Makefile doesn't exist
rm -rf autom4te.cache/
rm -f configure config.log config.status
rm -f Makefile.in aclocal.m4
rm -f src/Makefile.in
```

---

## Getting Help

- **Build issues:** Check [.devcontainer/README.md](.devcontainer/README.md)
- **Runtime issues:** Check `debug.log` in data directory
- **RPC commands:** Run `./src/junkcoin-cli help`

---

## Next Steps

After building:

1. **Test locally:** Run on testnet first
2. **Verify compatibility:** Ensure it syncs with old junkcoin-core nodes
3. **Check development fund:** Verify blocks 365001-3547800 have dev fund outputs
4. **Monitor activation:** Watch for CSV/SegWit activation at block 700000

