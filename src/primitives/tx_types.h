// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_TX_TYPES_H
#define BITCOIN_PRIMITIVES_TX_TYPES_H

#include <memory>

// Forward declarations and typedefs to be used as replacement for the full
// transaction.h or block.h header

enum class TxType {
    PURE,  //!< Provides just the data
    BASIC, //!< data + cache of the non-witness hash
    FULL,  //!< all cached data (used for validation)
};

template <TxType t>
class Transaction;

using CPureTransaction = Transaction<TxType::PURE>;
using CBasicTransaction = Transaction<TxType::BASIC>;
using CTransaction = Transaction<TxType::FULL>;

using CPureTransactionRef = std::shared_ptr<const CPureTransaction>;
using CBasicTransactionRef = std::shared_ptr<const CBasicTransaction>;
using CTransactionRef = std::shared_ptr<const CTransaction>;

template <typename TxRef>
class Block;

using CPureBlock = Block<CPureTransactionRef>;
using CBasicBlock = Block<CBasicTransactionRef>;
using CBlock = Block<CTransactionRef>;

#endif // BITCOIN_PRIMITIVES_TX_TYPES_H
