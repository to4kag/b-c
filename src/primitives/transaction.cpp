// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/transaction.h>

#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>

template class Transaction<TxType::PURE>;
template class Transaction<TxType::BASIC>;
template class Transaction<TxType::FULL>;

std::string COutPoint::ToString() const
{
    return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10), n);
}

CTxIn::CTxIn(COutPoint prevoutIn, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

CTxIn::CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

std::string CTxIn::ToString() const
{
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
        str += strprintf(", coinbase %s", HexStr(scriptSig));
    else
        str += strprintf(", scriptSig=%s", HexStr(scriptSig).substr(0, 24));
    if (nSequence != SEQUENCE_FINAL)
        str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
}

CTxOut::CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn)
{
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
}

std::string CTxOut::ToString() const
{
    return strprintf("CTxOut(nValue=%d.%08d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, HexStr(scriptPubKey).substr(0, 30));
}

CMutableTransaction::CMutableTransaction() : nVersion(CTransaction::CURRENT_VERSION), nLockTime(0) {}
CMutableTransaction::CMutableTransaction(const CPureTransaction& tx) : vin(tx.vin), vout(tx.vout), nVersion(tx.nVersion), nLockTime(tx.nLockTime) {}
CMutableTransaction::CMutableTransaction(const CBasicTransaction& tx) : vin(tx.vin), vout(tx.vout), nVersion(tx.nVersion), nLockTime(tx.nLockTime) {}
CMutableTransaction::CMutableTransaction(const CTransaction& tx) : vin(tx.vin), vout(tx.vout), nVersion(tx.nVersion), nLockTime(tx.nLockTime) {}

uint256 CMutableTransaction::GetHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

template <TxType t>
uint256 Transaction<t>::ComputeHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

template <TxType t>
const uint256&Transaction<t>::GetHash()const{return hash;}
// GetHash not defined for PURE
template const uint256 &Transaction<TxType::BASIC>::GetHash()const;
template const uint256 &Transaction<TxType::FULL>::GetHash()const;

template <TxType t>
uint256 Transaction< t>::GetWitnessHash() const 
{
    if (!HasWitness()) {
        return GetHash();
    }
    return SerializeHash(*this, SER_GETHASH, 0);
}
// GetWitnessHash not defined for PURE and BASIC
template uint256 Transaction<TxType::FULL>::GetWitnessHash()const;

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
template<TxType t> Transaction<t>::Transaction() : vin(), vout(), nVersion(Transaction<t>::CURRENT_VERSION), nLockTime(0), hash() {}
template<TxType t> Transaction<t>::Transaction(const CMutableTransaction &tx) : vin(tx.vin), vout(tx.vout), nVersion(tx.nVersion), nLockTime(tx.nLockTime), hash(ComputeHash()) {}
template<TxType t> Transaction<t>::Transaction(CMutableTransaction &&tx) : vin(std::move(tx.vin)), vout(std::move(tx.vout)), nVersion(tx.nVersion), nLockTime(tx.nLockTime), hash(ComputeHash()) {}

template Transaction<TxType::PURE>::Transaction();
template Transaction<TxType::BASIC>::Transaction();
template Transaction<TxType::FULL>::Transaction();

template Transaction<TxType::PURE>::Transaction(const CMutableTransaction&);
template Transaction<TxType::BASIC>::Transaction(const CMutableTransaction&);
template Transaction<TxType::FULL>::Transaction(const CMutableTransaction&);

template Transaction<TxType::PURE>::Transaction(CMutableTransaction&&);
template Transaction<TxType::BASIC>::Transaction(CMutableTransaction&&);
template Transaction<TxType::FULL>::Transaction(CMutableTransaction&&);

template <TxType t>
CAmount Transaction<t>::GetValueOut() const
{
    CAmount nValueOut = 0;
    for (const auto& tx_out : vout) {
        nValueOut += tx_out.nValue;
        if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nValueOut;
}

template CAmount Transaction<TxType::PURE>::GetValueOut() const;
template CAmount Transaction<TxType::BASIC>::GetValueOut() const;
template CAmount Transaction<TxType::FULL>::GetValueOut() const;

template <TxType t>
unsigned int Transaction<t>::GetTotalSize() const
{
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

template unsigned Transaction<TxType::PURE>::GetTotalSize() const;
template unsigned Transaction<TxType::BASIC>::GetTotalSize() const;
template unsigned Transaction<TxType::FULL>::GetTotalSize() const;

template <TxType t>
std::string Transaction<t>::ToString() const
{
    std::string str;
    str += strprintf("CTransaction(hash=%s, ver=%d, vin.size=%u, vout.size=%u, nLockTime=%u)\n",
        GetHash().ToString().substr(0,10),
        nVersion,
        vin.size(),
        vout.size(),
        nLockTime);
    for (const auto& tx_in : vin)
        str += "    " + tx_in.ToString() + "\n";
    for (const auto& tx_in : vin)
        str += "    " + tx_in.scriptWitness.ToString() + "\n";
    for (const auto& tx_out : vout)
        str += "    " + tx_out.ToString() + "\n";
    return str;
}

//template std::string Transaction<TxType::PURE>::ToString() const;
template std::string Transaction<TxType::BASIC>::ToString() const;
template std::string Transaction<TxType::FULL>::ToString() const;
