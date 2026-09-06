# Junkcoin Core v2 Devcontainer

This devcontainer provides a complete build environment for Junkcoin Core v2 with all necessary dependencies pre-installed.

## Features

- **Ubuntu 22.04** base image
- **GCC 11.4** compiler (compatible with the codebase)
- **Boost 1.74** libraries (system packages)
- **BerkeleyDB 4.8** for wallet support
- **All build dependencies** pre-installed
- **ccache** for faster rebuilds
- **Non-root user** for security

## Quick Start

### Option 1: Using VS Code

1. Install **Docker** and **VS Code** with the **Dev Containers** extension
2. Open the `junkcoin-core-v2` folder in VS Code
3. Press `F1` and select **"Dev Containers: Reopen in Container"**
4. Wait for the container to build (first time only, ~5-10 minutes)
5. Once inside the container, run:
   ```bash
   ./build.sh
   ```

### Option 2: Using Docker CLI

```bash
# Build the devcontainer image
cd junkcoin-core-v2/.devcontainer
docker build -t junkcoin-builder .

# Run the container
docker run -it --rm \
  -v $(pwd)/../..:/workspace \
  -w /workspace/junkcoin-core-v2 \
  junkcoin-builder

# Inside the container, build junkcoin
./build.sh
```

## Manual Build Steps

If you prefer to build manually instead of using `build.sh`:

```bash
# 1. Generate configure script
./autogen.sh

# 2. Configure
./configure \
  --disable-tests \
  --disable-bench \
  --with-incompatible-bdb \
  --enable-reduce-exports

# 3. Build
make -j$(nproc)

# 4. Optionally install
sudo make install
```

## Build Output

After a successful build, you'll find these binaries in the `src/` directory:

- `junkcoind` - Junkcoin daemon
- `junkcoin-cli` - Command-line interface
- `junkcoin-tx` - Transaction utility
- `junkcoin-wallet` - Wallet utility

## Troubleshooting

### Build fails with "cannot find -lboost_system"

The devcontainer uses system Boost libraries. If you see this error, make sure you're building inside the devcontainer, not on your host system.

### Permission denied errors

The devcontainer runs as user `builder` (non-root). If you need root access:
```bash
sudo <command>
```

### Out of disk space

The build requires ~2-3 GB of disk space. Check available space:
```bash
df -h /workspace
```

### Slow build

First build takes 10-30 minutes. Subsequent builds are faster thanks to ccache:
```bash
# Check ccache stats
ccache -s
```

## Differences from Depends Build

This devcontainer uses **system libraries** instead of the `depends/` build system:

| Aspect | Depends Build | Devcontainer Build |
|--------|---------------|-------------------|
| Boost | Custom 1.70 | System 1.74 |
| BDB | Custom 4.8 | System 4.8 |
| Build time | Longer (builds deps) | Faster (uses packages) |
| Reproducibility | High | Medium |
| Ease of use | Complex | Simple |

## Development Workflow

1. **Edit code** on your host machine using your favorite editor
2. **Build** inside the devcontainer using `./build.sh`
3. **Test** the binaries inside the container or copy them out
4. **Commit** changes from your host machine

## Compatibility Notes

This build environment is configured to build junkcoin-core-v2 with:

- ✅ Development fund support (blocks 365001-3547800)
- ✅ AuxPoW support (chain ID 0x2020, start height 173000)
- ✅ SegWit activation at height 700000
- ✅ MWEB activation at height 900000
- ✅ Full backward compatibility with old junkcoin-core before block 700000

## Clean Build

To start fresh:

```bash
# Clean build artifacts
make distclean

# Or use the build script (it cleans automatically)
./build.sh
```

## Advanced Configuration

### Enable tests and benchmarks

Edit `build.sh` and remove these flags:
```bash
--disable-tests \
--disable-bench \
```

### Enable GUI (Qt wallet)

The GUI dependencies are already installed. Just remove `--without-gui` if it's present in the configure command.

### Cross-compilation

For cross-compilation (e.g., Windows), you'll need to use the `depends/` system instead of this devcontainer.

