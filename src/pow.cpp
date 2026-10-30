// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>

// HANBIT: aserti3-2d, same integer math as Bitcoin Cash (BCHN CalculateASERT).
arith_uint256 CalculateASERT(const arith_uint256& refTarget, int64_t nPowTargetSpacing, int64_t nTimeDiff,
                             int64_t nHeightDiff, const arith_uint256& powLimit, int64_t nHalfLife)
{
    assert(refTarget > 0 && refTarget <= powLimit);
    assert(nHeightDiff >= 0);
    // refTarget * factor (< 2^17) must not overflow 256 bits.
    assert((powLimit >> 236) == 0);

    const int64_t exponent = ((nTimeDiff - nPowTargetSpacing * (nHeightDiff + 1)) * 65536) / nHalfLife;
    static_assert(int64_t(-1) >> 1 == int64_t(-1), "ASERT needs arithmetic shift");
    int64_t shifts = exponent >> 16;
    const auto frac = uint16_t(exponent);
    assert(exponent == (shifts * 65536) + frac);

    // 2^(frac/65536) * 65536, cubic approximation
    const uint32_t factor = 65536 + ((
        + 195766423245049ull * frac
        + 971821376ull * frac * frac
        + 5127ull * frac * frac * frac
        + (1ull << 47)) >> 48);
    arith_uint256 nextTarget = refTarget * factor;

    shifts -= 16;
    if (shifts <= 0) {
        nextTarget >>= -shifts;
    } else {
        const arith_uint256 shifted = nextTarget << shifts;
        nextTarget = ((shifted >> shifts) != nextTarget) ? powLimit : shifted;
    }
    if (nextTarget == 0) {
        nextTarget = arith_uint256(1);
    } else if (nextTarget > powLimit) {
        nextTarget = powLimit;
    }
    return nextTarget;
}

static unsigned int GetNextWorkRequiredASERT(const CBlockIndex* pindexLast, const CBlockHeader* pblock, const Consensus::Params& params)
{
    assert(params.nAsertHalfLife > 0 && params.nStallReliefStep > 0);
    const arith_uint256 powLimit = UintToArith256(params.powLimit);
    const CBlockIndex* pindexAnchor = pindexLast->GetAncestor(params.nAsertActivationHeight - 1);
    assert(pindexAnchor && pindexAnchor->pprev);

    arith_uint256 refTarget;
    refTarget.SetCompact(pindexAnchor->nBits);
    if (refTarget > powLimit) refTarget = powLimit;

    const int64_t nTimeDiff = pindexLast->GetBlockTime() - pindexAnchor->pprev->GetBlockTime();
    const int64_t nHeightDiff = pindexLast->nHeight - pindexAnchor->nHeight;
    arith_uint256 nextTarget = CalculateASERT(refTarget, params.nPowTargetSpacing, nTimeDiff, nHeightDiff,
                                              powLimit, params.nAsertHalfLife);

    // Stall relief: lets small miners restart the chain after a large miner leaves.
    if (params.nStallReliefAfter > 0 && pblock) {
        const int64_t gap = pblock->GetBlockTime() - pindexLast->GetBlockTime();
        if (gap > params.nStallReliefAfter) {
            int64_t steps = (gap - params.nStallReliefAfter) / params.nStallReliefStep;
            while (steps-- > 0 && nextTarget < powLimit) nextTarget <<= 1;
            if (nextTarget > powLimit) nextTarget = powLimit;
        }
    }
    return nextTarget.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    if (params.AsertActiveAt(pindexLast->nHeight + 1)) {
        return GetNextWorkRequiredASERT(pindexLast, pblock, params);
    }

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    const int64_t nTargetTimespan = params.PowTargetTimespanAt(pindexLast->nHeight + 1);

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < nTargetTimespan/4)
        nActualTimespan = nTargetTimespan/4;
    if (nActualTimespan > nTargetTimespan*4)
        nActualTimespan = nTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

// Check that on difficulty adjustments, the new difficulty does not increase
// or decrease beyond the permitted limits.
bool PermittedDifficultyTransition(const Consensus::Params& params, int64_t height, uint32_t old_nbits, uint32_t new_nbits)
{
    if (params.fPowAllowMinDifficultyBlocks) return true;
    // HANBIT: ASERT changes the target every block.
    if (params.AsertActiveAt(height)) return true;

    if (height % params.DifficultyAdjustmentInterval() == 0) {
        const int64_t nTargetTimespan = params.PowTargetTimespanAt(height);
        int64_t smallest_timespan = nTargetTimespan/4;
        int64_t largest_timespan = nTargetTimespan*4;

        const arith_uint256 pow_limit = UintToArith256(params.powLimit);
        arith_uint256 observed_new_target;
        observed_new_target.SetCompact(new_nbits);

        // Calculate the largest difficulty value possible:
        arith_uint256 largest_difficulty_target;
        largest_difficulty_target.SetCompact(old_nbits);
        largest_difficulty_target *= largest_timespan;
        largest_difficulty_target /= nTargetTimespan;

        if (largest_difficulty_target > pow_limit) {
            largest_difficulty_target = pow_limit;
        }

        // Round and then compare this new calculated value to what is
        // observed.
        arith_uint256 maximum_new_target;
        maximum_new_target.SetCompact(largest_difficulty_target.GetCompact());
        if (maximum_new_target < observed_new_target) return false;

        // Calculate the smallest difficulty value possible:
        arith_uint256 smallest_difficulty_target;
        smallest_difficulty_target.SetCompact(old_nbits);
        smallest_difficulty_target *= smallest_timespan;
        smallest_difficulty_target /= nTargetTimespan;

        if (smallest_difficulty_target > pow_limit) {
            smallest_difficulty_target = pow_limit;
        }

        // Round and then compare this new calculated value to what is
        // observed.
        arith_uint256 minimum_new_target;
        minimum_new_target.SetCompact(smallest_difficulty_target.GetCompact());
        if (minimum_new_target > observed_new_target) return false;
    } else if (old_nbits != new_nbits) {
        return false;
    }
    return true;
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
