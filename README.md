# HANBIT (HBT) — Korean Peer-to-Peer Electronic Cash

<p align="center">
  <b>한빛 — 한국의 빛, 세계의 자유</b><br/>
  <i>Korean Light — Freedom for the World</i>
</p>

---

## What is HANBIT?

HANBIT is a peer-to-peer electronic cash system based on Bitcoin Core 25.0.
It carries the spirit of Satoshi Nakamoto — decentralization, peer-to-peer freedom, and no central authority — with the identity of Korea.

No pre-mine. No ICO. No company behind it.
Just code, nodes, and mathematics.

## Specifications

| Property | Value |
|----------|-------|
| Name | HANBIT (한빛) |
| Ticker | HBT |
| Max Supply | 21,000,000 HBT |
| Block Time | 10 minutes (5 minutes before block 60,480) |
| Difficulty | Per-block ASERT (aserti3-2d, half-life 2 h) from block 64,512 |
| Initial Reward | 50 HBT |
| Halving | Every 210,000 blocks |
| Algorithm | SHA-256d Proof of Work (mine it with any Bitcoin miner) |
| P2P Port | 9777 |
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

**Connect to network** — add to `~/.hanbit/hanbit.conf`:
```
addnode=168.144.33.159:9777
```

## Building from Source

```bash
# Dependencies (Ubuntu 22.04)
sudo apt-get install build-essential libtool autotools-dev automake pkg-config \
  libssl-dev libevent-dev bsdmainutils libboost-all-dev libsqlite3-dev

# Build
git clone https://github.com/cmg9722-beep/hanbit.git
cd hanbit
./autogen.sh
./configure --without-gui --disable-bench
make -j$(nproc)
```

## Key Changes from Bitcoin Core 25.0

- `src/kernel/chainparams.cpp` — HANBIT genesis block and network parameters
- Block interval 10 minutes from height 60,480 (`src/pow.cpp`, `src/consensus/params.h`)
- From height 64,512: per-block ASERT difficulty, the algorithm Bitcoin Cash uses,
  so one large miner joining or leaving cannot flood or stall a small chain.
  If no block is found for 6 hours, the target doubles for every further hour.
  Tests: `src/test/pow_tests.cpp` (`hanbit_*`)
- From height 64,512: BIP34, BIP65 (CLTV), BIP66, CSV and SegWit are enforced
  (they had Bitcoin's activation heights before). Taproot is always active.
- HANBIT defaults: data directory `~/.hanbit` (`%APPDATA%\Hanbit` on Windows),
  config file `hanbit.conf`, RPC port 9776, built-in seed node 168.144.33.159:9777.

**Hard fork notice:** every node must run v1.3.1 or later before block 64,512.

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
