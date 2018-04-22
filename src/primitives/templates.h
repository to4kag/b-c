// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_TEMPLATES_H
#define BITCOIN_PRIMITIVES_TEMPLATES_H

#include <memory>

// Forward declarations and typedefs to be used as replacement for the full
// transaction.h or block.h header

template <bool BASIC>
class Transaction;
using CTransaction = Transaction<false>;     //!< Transaction that also caches its witness hash
using CBasicTransaction = Transaction<true>; //!< A bare Transaction that only caches its non-witness hash
using CTransactionRef = std::shared_ptr<const CTransaction>;
using CBasicTransactionRef = std::shared_ptr<const CBasicTransaction>;

template <typename TxRef>
class Block;
using CBlock = Block<CTransactionRef>;
using CBasicBlock = Block<CBasicTransactionRef>;

#endif // BITCOIN_PRIMITIVES_TEMPLATES_H
