// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPC_MINING_H
#define BITCOIN_RPC_MINING_H

class UniValue;
class CScript;

/** Mine blocks */
UniValue generateBlocks(const CScript& coinbase_script, int num_generate, uint64_t max_tries);

#endif
