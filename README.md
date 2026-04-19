# HANBIT (HBT) — Korean Peer-to-Peer Electronic Cash

<p align="center">
  <img src="https://raw.githubusercontent.com/cmg9722-beep/hanbit/main/logo.png" width="120" alt="HANBIT Logo"/>
</p>

<p align="center">
  <b>한빛 — 한국의 빛, 세계의 자유</b><br/>
  <i>Korean Light — Freedom for the World</i>
</p>

---

## What is HANBIT?

HANBIT is a peer-to-peer electronic cash system based on Bitcoin Core.  
It carries the spirit of Satoshi Nakamoto — decentralization, peer-to-peer freedom, and no central authority — with the identity of Korea.

No pre-mine. No ICO. No company behind it.  
Just code, nodes, and mathematics.

## Specifications

| Property | Value |
|----------|-------|
| Name | HANBIT (한빛) |
| Ticker | HBT |
| Symbol | ₭ |
| Max Supply | 21,000,000 HBT |
| Block Time | 5 minutes |
| Initial Reward | 50 HBT |
| Halving | Every 210,000 blocks |
| Algorithm | SHA-256d Proof of Work |
| Port | 9777 |
| Address Prefix | hb1... (bech32) |
| Genesis Date | 2026-04-19 |

## Genesis Block

```
Hash    : 00000aef5bca6af5b63f8e0a56b605d5cc07c1291a1a4b3db5800cb8e9d51196
Message : "HANBIT 19/Apr/2026 Korean light rising in world blockchain"
Time    : 1776556800
Nonce   : 808525
```

## Network

| | |
|-|-|
| Block Explorer | http://168.144.33.159:3002 |
| Node (mainnet) | 168.144.33.159:9777 |

**Connect to network** — add to `hanbit.conf`:
```
addnode=168.144.33.159:9777
```

## Building from Source

```bash
# Dependencies (Ubuntu 22.04)
apt-get install build-essential libtool autotools-dev automake pkg-config \
  libssl-dev libevent-dev bsdmainutils libboost-all-dev \
  libsqlite3-dev libzmq3-dev

# Build
git clone https://github.com/cmg9722-beep/hanbit.git
cd hanbit
./autogen.sh
./configure --without-gui
make -j$(nproc)
```

## Key Changes from Bitcoin Core

- `src/kernel/chainparams.cpp` — HANBIT genesis block & network parameters
- `src/kernel/chainparams.h` — Empty checkpoint fix

## Philosophy

> "What is needed is an electronic payment system based on cryptographic proof  
> instead of trust, allowing any two willing parties to transact directly  
> with each other without the need for a trusted third party."
> — Satoshi Nakamoto, 2008

HANBIT extends this vision with the spirit of Korea (태극기):  
**Balance. Harmony. Freedom.**

## License

MIT License — same as Bitcoin Core.

---

*Satoshi Nakamoto's vision. Korean light.*
