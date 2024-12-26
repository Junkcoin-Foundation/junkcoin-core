# Running Junkcoin Testnet

This guide explains how to set up and run a Junkcoin testnet node.

## Overview

The testnet is a separate network used for testing without risking real coins. Testnet coins are denoted as tJKC to distinguish them from mainnet JKC. The testnet has different parameters from mainnet:

- Block rewards halve more frequently (every 28,800 blocks)
- First halving at block 2,880
- Strict 1-minute block timing activates at block 2,880
- Different address formats (starting with 'm' or 'n')
- Different network ports (19771 instead of 9771)

## Requirements

- At least 2GB of RAM
- 10GB of free disk space
- Stable internet connection
- Junkcoin Core v3.0.0-alpha1 or later

## Installation

1. Download Junkcoin Core:
```bash
git clone -b dev https://github.com/Junkcoin-Foundation/junkcoin-core-wallet.git
cd junkcoin-core-wallet
```

2. Build from source:
```bash
./autogen.sh
./configure
make
```

## Running a Testnet Node

1. Start the daemon in testnet mode:
```bash
./src/junkcoind -testnet -daemon
```

2. Check the status:
```bash
./src/junkcoin-cli -testnet getinfo
```

3. Generate a new address:
```bash
./src/junkcoin-cli -testnet getnewaddress
```
The address should start with 'm' or 'n'.

## Configuration

Create or edit your junkcoin.conf file:
```bash
mkdir -p ~/.junkcoin/testnet3
nano ~/.junkcoin/testnet3/junkcoin.conf
```

Add these settings:
```
testnet=1
server=1
listen=1
rpcuser=your_username
rpcpassword=your_secure_password
rpcport=19772
port=19771
```

## Mining on Testnet

1. Generate a mining address:
```bash
./src/junkcoin-cli -testnet getnewaddress
```

2. Start mining:
```bash
./src/junkcoin-cli -testnet generatetoaddress 1 "your_testnet_address"
```

## Connecting to Other Nodes

The testnet uses DNS seed server: testnet.junk-coin.com

You can also add specific nodes:
```bash
./src/junkcoind -testnet -addnode=<ip_address>:19771
```

## Monitoring

1. Check sync status:
```bash
./src/junkcoin-cli -testnet getblockchaininfo
```

2. View network info:
```bash
./src/junkcoin-cli -testnet getnetworkinfo
```

3. List connected peers:
```bash
./src/junkcoin-cli -testnet getpeerinfo
```

## Testnet Faucets

Currently, there are no official testnet faucets. You can mine tJKC or request them from other testnet users.

## Data Directory

Testnet data is stored in:
- Linux: ~/.junkcoin/testnet3/
- macOS: ~/Library/Application Support/Junkcoin/testnet3/
- Windows: %APPDATA%\Junkcoin\testnet3\

## Common Issues

1. **Can't connect to network**
   - Check if port 19771 is open
   - Verify your internet connection
   - Try adding nodes manually

2. **Wallet not syncing**
   - Check disk space
   - Verify network connectivity
   - Try restarting the daemon

3. **Invalid addresses**
   - Ensure you're using testnet addresses (starting with 'm' or 'n')
   - Check if you're running in testnet mode

## Testnet Reset

If needed, you can reset your testnet data:
```bash
./src/junkcoind -testnet stop
rm -rf ~/.junkcoin/testnet3
./src/junkcoind -testnet -daemon
```

## Development Tips

1. Use testnet for:
   - Testing new features
   - Experimenting with transactions
   - Testing mining algorithms
   - Integration testing

2. Remember:
   - Testnet coins (tJKC) have no value
   - Network parameters are different from mainnet
   - Block times are enforced after block 2,880

## Support

For help or to report issues:
- GitHub: [Junkcoin Core repository](https://github.com/Junkcoin-Foundation/junkcoin-core-wallet/tree/dev)
- Email: [contact@junk-coin.com](mailto:contact@junk-coin.com)
