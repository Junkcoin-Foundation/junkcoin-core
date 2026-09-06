# Quick Reference Card

## Build Commands

```bash
# One-command build (recommended)
./build.sh

# Manual build
./autogen.sh
./configure --with-incompatible-bdb
make -j$(nproc)

# Clean build
make distclean
./build.sh

# Install system-wide
sudo make install
```

## Docker Commands

```bash
# Build container image
docker build -t junkcoin-builder .devcontainer/

# Run container
docker run -it --rm \
  -v $(pwd):/workspace \
  -w /workspace/junkcoin-core-v2 \
  junkcoin-builder

# List running containers
docker ps

# Stop container
docker stop <container-id>

# Remove all stopped containers
docker container prune
```

## VS Code Devcontainer

```
F1 → "Dev Containers: Reopen in Container"
F1 → "Dev Containers: Rebuild Container"
F1 → "Dev Containers: Reopen Folder Locally"
```

## Running Junkcoin

```bash
# Start daemon
./src/junkcoind -daemon

# Start with custom datadir
./src/junkcoind -datadir=/path/to/data -daemon

# Start on testnet
./src/junkcoind -testnet -daemon

# Stop daemon
./src/junkcoin-cli stop
```

## Common RPC Commands

```bash
# Get blockchain info
./src/junkcoin-cli getblockchaininfo

# Get wallet info
./src/junkcoin-cli getwalletinfo

# Get block by height
./src/junkcoin-cli getblockhash 400000
./src/junkcoin-cli getblock <hash> 2

# Get development fund info
./src/junkcoin-cli getblocksubsidy 400000

# Get mining template
./src/junkcoin-cli getblocktemplate

# Get peer info
./src/junkcoin-cli getpeerinfo

# Get network info
./src/junkcoin-cli getnetworkinfo
```

## Debugging

```bash
# View debug log
tail -f ~/.junkcoin/debug.log

# View with custom datadir
tail -f /path/to/data/debug.log

# Check if daemon is running
./src/junkcoin-cli getblockcount

# Get help for RPC command
./src/junkcoin-cli help <command>

# List all RPC commands
./src/junkcoin-cli help
```

## ccache Stats

```bash
# View ccache statistics
ccache -s

# Clear ccache
ccache -C

# Set ccache size
ccache -M 5G
```

## File Locations

```bash
# Default datadir (Linux)
~/.junkcoin/

# Config file
~/.junkcoin/junkcoin.conf

# Debug log
~/.junkcoin/debug.log

# Wallet
~/.junkcoin/wallet.dat

# Testnet datadir
~/.junkcoin/testnet3/
```

## Configuration File Example

```ini
# ~/.junkcoin/junkcoin.conf

# Network
listen=1
maxconnections=125

# RPC
server=1
rpcuser=yourusername
rpcpassword=yourpassword
rpcallowip=127.0.0.1

# Mining (if applicable)
mineraddress=<your-junkcoin-address>

# Logging
debug=net
debug=rpc
```

## Useful Flags

```bash
# Reindex blockchain
-reindex

# Rescan wallet
-rescan

# Prune blockchain (keep only 550MB)
-prune=550

# Connect to specific peer
-connect=<ip:port>

# Add node
-addnode=<ip:port>

# Testnet
-testnet

# Regtest (local testing)
-regtest
```

## Development Fund Verification

```bash
# Check if block has dev fund (blocks 365001-3547800)
./src/junkcoin-cli getblock $(./src/junkcoin-cli getblockhash 400000) 2 | grep -A5 "coinbase"

# Get dev fund amount for specific height
./src/junkcoin-cli getblocksubsidy 400000

# Verify dev fund address
# Should be one of:
# - 3P3UvT6vdDJVrbB2mn6WrP8gywpu2Knx8C
# - 34cGTrxRD4VvfbDri6RhQDKPBokfLTNJse
# - 37NpTG2p6gjVeZDmAiPLKNs6Nhj5EfTR55
```

## Troubleshooting Quick Fixes

```bash
# "Database corrupted"
./src/junkcoind -reindex

# "Wallet locked"
./src/junkcoin-cli walletpassphrase <passphrase> <timeout>

# "Cannot connect to peers"
# Check firewall, add nodes manually:
./src/junkcoin-cli addnode <ip:port> add

# "Out of sync"
# Wait for sync, or use -reindex if stuck

# "RPC connection refused"
# Check junkcoin.conf has:
# server=1
# rpcuser=...
# rpcpassword=...
```

## Performance Tips

```bash
# Use ccache for faster rebuilds
export CCACHE_DIR=/workspace/.ccache

# Reduce parallel jobs if low memory
make -j2

# Use tmpfs for faster builds (Linux)
mount -t tmpfs -o size=4G tmpfs /tmp

# Enable debug symbols for debugging
./configure --enable-debug CXXFLAGS="-O0 -g3"
```

## Testing Checklist

- [ ] Build completes without errors
- [ ] `junkcoind -version` shows correct version
- [ ] Daemon starts successfully
- [ ] Syncs with network
- [ ] RPC commands work
- [ ] Development fund validated (blocks 365001-3547800)
- [ ] Compatible with old junkcoin-core nodes
- [ ] Wallet operations work
- [ ] Can create and broadcast transactions

