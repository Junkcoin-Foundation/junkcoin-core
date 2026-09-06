<p align="center">
  <img src="https://github.com/Junkcoin-Foundation/junkcoin-docs/blob/main/assets/logos/logo-junkcoin-horizontal-2500px.png" alt="Junkcoin Logo" width="350"/>
</p>

# Junkcoin Core

The **Junkcoin Core** repository hosts the source code for the core of Junkcoin, a historic blockchain launched on **May 3, 2013**, and revived in **November 2024**. Designed as a fork of Litecoin, Junkcoin combines the reliability of Litecoin with exciting features like **random bonus blocks**, making mining a unique and engaging experience.

This repository hosts the **next main Junkcoin Core codebase**, succeeding previous development efforts. It combines the reliability of a clean Litecoin lineage with Junkcoin's unique consensus rules, serving as the foundation for the development, maintenance, and continuous improvement of the Junkcoin Core, ensuring stability, security, and performance.

## Branches
This repository follows a clear branching strategy:
- **`master`**: Stable releases. This is the default branch, hosting thoroughly tested and production-ready code.
- **`dev`**: Active development and feature work. All new contributions and updates are first merged here before reaching `master`.

## Mining Rules
Junkcoin operates on a **Proof-of-Work (PoW)** consensus mechanism. Its current mining parameters, based on the network's consensus code, include:

- **Total supply**: 54 million JKC.
- **Block time**: 1 minute (60 seconds).
- **Difficulty adjustment**: Every 4 hours (every 240 blocks).
- **Reward schedule**:
  - **Day 1**: 500 coins per block.
  - **Day 2**: 200 coins per block.
  - **Day 3 & 4**: 100 coins per block.
  - **Months 1 to 6 (approx)**: 50 coins per block.
  - **Months 6 to 9 (approx)**: 25 coins per block.
  - **Months 9 to 15 (approx)**: 12.5 coins per block.
  - **Afterwards**: Subsidy continues to halve gradually according to a custom schedule defined in the codebase.

### **Lottery Blocks (Random Bonus Blocks)**
In addition to the regular mining rewards, Junkcoin features **random bonus blocks** to incentivize miners and add excitement to the mining process. These include:
- **1% chance**: A block will yield **triple (3x)** the normal reward.
- **0.01% chance**: A block will yield **1,000 coins**, regardless of the mining phase.

This innovative feature brings an element of unpredictability and fun to the mining process, setting Junkcoin apart from other PoW cryptocurrencies.

## Ports

### **Default Ports**
- **Connection**: 9771
- **JSON-RPC**: 9771

### **Testnet Ports**
- **Connection**: 19771
- **JSON-RPC**: 19771

## Official Junkcoin Website
For more information, please visit the official Junkcoin website:
[https://junk-coin.com](https://junk-coin.com)

## Technical Support
For technical support, join our dedicated developer channel on Telegram. Simply contact the admins in our main Telegram group and request access:
[https://t.me/junkcoin_JKC](https://t.me/junkcoin_JKC)

## Official Junkcoin Twitter/X
Join us on X (formerly Twitter) here:
[https://x.com/junkcoinJKC](https://x.com/junkcoinJKC)

## License
This project is licensed under the **MIT License**, ensuring it remains open-source and community-driven.
