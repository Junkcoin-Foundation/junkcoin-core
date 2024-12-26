# Junkcoin Core 3.0.0-alpha1 Release Notes

This release includes several major changes and improvements to the Junkcoin Core software.

## Major Changes

### Block Timing
- Mainnet: Strict 1-minute block spacing activated at block 262,800
- Testnet: Strict 1-minute block spacing activated at block 2,880

### New Halving Schedule
Mainnet:
- First halving at block 262,800 (25 JKC)
- Second halving at block 394,200 (12.5 JKC)
- Third halving at block 657,000 (6.25 JKC)
- Fourth halving at block 1,182,600 (3.125 JKC)
- Further halvings every 788,400 blocks (1.5 years)
- Mining rewards end at block 1,707,600 (October 2025)

Testnet:
- First halving at block 2,880 (25 tJKC)
- Subsequent halvings every 28,800 blocks
- Mining rewards end at block 89,280

### Network Infrastructure
- Added new DNS seed servers:
  - Mainnet: mainnet.junk-coin.com
  - Testnet: testnet.junk-coin.com

### Other Changes
- Fixed testnet address generation to use proper prefixes
  - Addresses now start with 'm' or 'n' on testnet
  - Script addresses start with '2' on testnet
  - Private keys start with '9' or 'c' on testnet
- Added network-specific currency units
  - Mainnet uses JKC
  - Testnet uses tJKC
  - Updated all displays and RPC outputs to use correct currency unit

## How to Upgrade
1. Shut down your existing Junkcoin Core node:
   ```bash
   ./src/junkcoin-cli stop
   ```

2. Clone the dev branch:
   ```bash
   git clone -b dev https://github.com/Junkcoin-Foundation/junkcoin-core-wallet.git
   cd junkcoin-core-wallet
   ```

3. Build and install:
   ```bash
   ./autogen.sh
   ./configure
   make
   ```

4. Run the new version:
   ```bash
   ./src/junkcoind -daemon
   ```

5. For testnet:
   ```bash
   ./src/junkcoind -testnet -daemon
   ```

## Note
This is an alpha release and may contain bugs. Use at your own risk and please report any issues you encounter.
