# mmx-node

MMX is a blockchain written from scratch, using proven logic from Chia's Proof of Space and improved SHA256 VDF similar to Solana.

Design
- High transaction capacity, with low and consistent block times
- Energy efficient Proof of Space, future transition to virtual storage
- Rich compatible feature set, built into base blockchain
- Balanced hardware requirements, with nodes in home networks
- Native MMX is designed with the properties and usage of an actual currency
- Variable supply stabilizes the price, a key property of any currency
- Efficient implementation provides low transaction fees, at high throughput

Features
- High performance code (1000 transactions per second or more)
- Custom high-level VM for smart contracts (similar to JavaScript)
- Variable supply (block reward scales with netspace, but also capped by TX fees)
- Consistent block times (created every 10 seconds, governed by VDF)
- Native token and NFT support (atomic swaps with standard transactions)
- Smart contract offers for on-chain trading (bids locked into contract)
- Energy efficient Proof of Space (same as Chia)
- Standard ECDSA signatures for seamless integration (same as Bitcoin)

Roadmap

| Release | Planned | Description |
| :--- | :--- | :--- |
| testnet7 | Sep 2022 | Finished. <sup>[1]</sup> |
| testnet8 | Oct 2022 | Finished. Incentivized testnet, until height 425k. <sup>[2]</sup> |
| testnet9 | Dec 2022 | Finished. Incentivized testnet, until height 1220k.  <sup>[2]</sup> |
| testnet10 | Apr 2023 | Active. Incentivized testnet. <sup>[2]</sup> |
| testnet11 | Q3 2023 | Planned. <sup>[2]</sup> |
| testnetX | _tbd_ | As needed. <sup>[2]</sup> |
| mainnet | _tbd_ | Expected Q1 2023. |

_<sup>[1]</sup> Coins farmed on testnets are not worth anything, now or later._\
_<sup>[2]</sup> A fixed reward of 0.5 MMX per block win, in incentivized testnet, will be given on mainnet genesis._

See `#mmx-news` and `#mmx-general` on Discord: https://discord.gg/BswFhNkMzY

## Installation / Setup / Usage

Please take a look at the Wiki:

- [Installation](https://github.com/madMAx43v3r/mmx-node/wiki/Installation)
- [Getting Started](https://github.com/madMAx43v3r/mmx-node/wiki/Getting-Started)
- [CLI Commands](https://github.com/madMAx43v3r/mmx-node/wiki/CLI-Commands)
- [Frequently Asked Questions](https://github.com/madMAx43v3r/mmx-node/wiki/Frequently-Asked-Questions)

## WebGUI

To access WebGUI, go to: http://localhost:11380/gui/

Only available on localhost, because it has _full access_ to your wallet.

Login password is auto-generated at first launch, located in `$MMX_HOME/PASSWD` file.

## Release Notes

### Testnet9

- New `Swap` contract, to trade via liquidity pools (similar to _UniSwap_, anyone can provide liquidity and earn fees)
- Improved `Offer` contract: partial trades and extending offers are now possible
- Re-designed `Virtual Plots`: withdrawal is now possible with a 10% fee (no lock duration, fee is burned), also there is no more expiration
- Network traffic is now compressed via `deflate` level 1
- Second best farmer will now create an empty block (just to get the reward), in case the first farmer fails or is too slow

### Testnet10

- Decentralized Timelord rewards (fixed 0.0025 MMX per block, paid by TX fees when possible (but never more than 1/8))
- New AMM Swap with multi-fee tier system (you can chose between 10 fee levels for your liquidity)
- Account activation TX fee (0.01 MMX when transferring to a new address with zero balance)
- Increased min block reward to 0.5 MMX
- Full transition to smart contracts with compiled JS like code
- Harvester improvements for compressed plots (graceful degradation)
- New dev fee system, based only on TX fees, with a fixed amount plus 1% (but never more than 1/4 of TX fees)
- Plot filter has been decreased to 256 now, as it is planned for mainnet
- Many improvements to the code base
