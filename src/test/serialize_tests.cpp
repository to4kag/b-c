// Copyright (c) 2012-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <hash.h>
#include <serialize.h>
#include <streams.h>
#include <test/util/setup_common.h>
#include <util/strencodings.h>

#include <stdint.h>
#include <string>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(serialize_tests, BasicTestingSetup)

class CSerializeMethodsTestSingle
{
protected:
    int intval;
    bool boolval;
    std::string stringval;
    char charstrval[16];
    CTransactionRef txval;
public:
    CSerializeMethodsTestSingle() = default;
    CSerializeMethodsTestSingle(int intvalin, bool boolvalin, std::string stringvalin, const uint8_t* charstrvalin, const CTransactionRef& txvalin) : intval(intvalin), boolval(boolvalin), stringval(std::move(stringvalin)), txval(txvalin)
    {
        memcpy(charstrval, charstrvalin, sizeof(charstrval));
    }

    SERIALIZE_METHODS(CSerializeMethodsTestSingle, obj)
    {
        READWRITE(obj.intval);
        READWRITE(obj.boolval);
        READWRITE(obj.stringval);
        READWRITE(obj.charstrval);
        READWRITE(obj.txval);
    }

    bool operator==(const CSerializeMethodsTestSingle& rhs) const
    {
        return intval == rhs.intval &&
               boolval == rhs.boolval &&
               stringval == rhs.stringval &&
               strcmp(charstrval, rhs.charstrval) == 0 &&
               *txval == *rhs.txval;
    }
};

class CSerializeMethodsTestMany : public CSerializeMethodsTestSingle
{
public:
    using CSerializeMethodsTestSingle::CSerializeMethodsTestSingle;

    SERIALIZE_METHODS(CSerializeMethodsTestMany, obj)
    {
        READWRITE(obj.intval, obj.boolval, obj.stringval, obj.charstrval, obj.txval);
    }
};

BOOST_AUTO_TEST_CASE(sizes)
{
    BOOST_CHECK_EQUAL(sizeof(unsigned char), GetSerializeSize((unsigned char)0, 0));
    BOOST_CHECK_EQUAL(sizeof(int8_t), GetSerializeSize(int8_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(uint8_t), GetSerializeSize(uint8_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(int16_t), GetSerializeSize(int16_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(uint16_t), GetSerializeSize(uint16_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(int32_t), GetSerializeSize(int32_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(uint32_t), GetSerializeSize(uint32_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(int64_t), GetSerializeSize(int64_t(0), 0));
    BOOST_CHECK_EQUAL(sizeof(uint64_t), GetSerializeSize(uint64_t(0), 0));
    // Bool is serialized as uint8_t
    BOOST_CHECK_EQUAL(sizeof(uint8_t), GetSerializeSize(bool(0), 0));

    // Sanity-check GetSerializeSize and c++ type matching
    BOOST_CHECK_EQUAL(GetSerializeSize((unsigned char)0, 0), 1U);
    BOOST_CHECK_EQUAL(GetSerializeSize(int8_t(0), 0), 1U);
    BOOST_CHECK_EQUAL(GetSerializeSize(uint8_t(0), 0), 1U);
    BOOST_CHECK_EQUAL(GetSerializeSize(int16_t(0), 0), 2U);
    BOOST_CHECK_EQUAL(GetSerializeSize(uint16_t(0), 0), 2U);
    BOOST_CHECK_EQUAL(GetSerializeSize(int32_t(0), 0), 4U);
    BOOST_CHECK_EQUAL(GetSerializeSize(uint32_t(0), 0), 4U);
    BOOST_CHECK_EQUAL(GetSerializeSize(int64_t(0), 0), 8U);
    BOOST_CHECK_EQUAL(GetSerializeSize(uint64_t(0), 0), 8U);
    BOOST_CHECK_EQUAL(GetSerializeSize(bool(0), 0), 1U);
}

BOOST_AUTO_TEST_CASE(varints)
{
    // encode

    DataStream ss{};
    DataStream::size_type size = 0;
    for (int i = 0; i < 100000; i++) {
        ss << VARINT_MODE(i, VarIntMode::NONNEGATIVE_SIGNED);
        size += ::GetSerializeSize(VARINT_MODE(i, VarIntMode::NONNEGATIVE_SIGNED), 0);
        BOOST_CHECK(size == ss.size());
    }

    for (uint64_t i = 0;  i < 100000000000ULL; i += 999999937) {
        ss << VARINT(i);
        size += ::GetSerializeSize(VARINT(i), 0);
        BOOST_CHECK(size == ss.size());
    }

    // decode
    for (int i = 0; i < 100000; i++) {
        int j = -1;
        ss >> VARINT_MODE(j, VarIntMode::NONNEGATIVE_SIGNED);
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }

    for (uint64_t i = 0;  i < 100000000000ULL; i += 999999937) {
        uint64_t j = std::numeric_limits<uint64_t>::max();
        ss >> VARINT(j);
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }
}

BOOST_AUTO_TEST_CASE(varints_bitpatterns)
{
    DataStream ss{};
    ss << VARINT_MODE(0, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "00"); ss.clear();
    ss << VARINT_MODE(0x7f, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "7f"); ss.clear();
    ss << VARINT_MODE(int8_t{0x7f}, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "7f"); ss.clear();
    ss << VARINT_MODE(0x80, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "8000"); ss.clear();
    ss << VARINT(uint8_t{0x80}); BOOST_CHECK_EQUAL(HexStr(ss), "8000"); ss.clear();
    ss << VARINT_MODE(0x1234, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "a334"); ss.clear();
    ss << VARINT_MODE(int16_t{0x1234}, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "a334"); ss.clear();
    ss << VARINT_MODE(0xffff, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "82fe7f"); ss.clear();
    ss << VARINT(uint16_t{0xffff}); BOOST_CHECK_EQUAL(HexStr(ss), "82fe7f"); ss.clear();
    ss << VARINT_MODE(0x123456, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "c7e756"); ss.clear();
    ss << VARINT_MODE(int32_t{0x123456}, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "c7e756"); ss.clear();
    ss << VARINT(0x80123456U); BOOST_CHECK_EQUAL(HexStr(ss), "86ffc7e756"); ss.clear();
    ss << VARINT(uint32_t{0x80123456U}); BOOST_CHECK_EQUAL(HexStr(ss), "86ffc7e756"); ss.clear();
    ss << VARINT(0xffffffff); BOOST_CHECK_EQUAL(HexStr(ss), "8efefefe7f"); ss.clear();
    ss << VARINT_MODE(0x7fffffffffffffffLL, VarIntMode::NONNEGATIVE_SIGNED); BOOST_CHECK_EQUAL(HexStr(ss), "fefefefefefefefe7f"); ss.clear();
    ss << VARINT(0xffffffffffffffffULL); BOOST_CHECK_EQUAL(HexStr(ss), "80fefefefefefefefe7f"); ss.clear();
}

BOOST_AUTO_TEST_CASE(compactsize)
{
    DataStream ss{};
    std::vector<char>::size_type i, j;

    for (i = 1; i <= MAX_SIZE; i *= 2)
    {
        WriteCompactSize(ss, i-1);
        WriteCompactSize(ss, i);
    }
    for (i = 1; i <= MAX_SIZE; i *= 2)
    {
        j = ReadCompactSize(ss);
        BOOST_CHECK_MESSAGE((i-1) == j, "decoded:" << j << " expected:" << (i-1));
        j = ReadCompactSize(ss);
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
    }
}

static bool isCanonicalException(const std::ios_base::failure& ex)
{
    std::ios_base::failure expectedException("non-canonical ReadCompactSize()");

    // The string returned by what() can be different for different platforms.
    // Instead of directly comparing the ex.what() with an expected string,
    // create an instance of exception to see if ex.what() matches
    // the expected explanatory string returned by the exception instance.
    return strcmp(expectedException.what(), ex.what()) == 0;
}

BOOST_AUTO_TEST_CASE(vector_bool)
{
    std::vector<uint8_t> vec1{1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1};
    std::vector<bool> vec2{1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1};

    BOOST_CHECK(vec1 == std::vector<uint8_t>(vec2.begin(), vec2.end()));
    BOOST_CHECK(SerializeHash(vec1) == SerializeHash(vec2));
}

BOOST_AUTO_TEST_CASE(noncanonical)
{
    // Write some non-canonical CompactSize encodings, and
    // make sure an exception is thrown when read back.
    DataStream ss{};
    std::vector<char>::size_type n;

    // zero encoded with three bytes:
    ss.write(MakeByteSpan("\xfd\x00\x00").first(3));
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure, isCanonicalException);

    // 0xfc encoded with three bytes:
    ss.write(MakeByteSpan("\xfd\xfc\x00").first(3));
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure, isCanonicalException);

    // 0xfd encoded with three bytes is OK:
    ss.write(MakeByteSpan("\xfd\xfd\x00").first(3));
    n = ReadCompactSize(ss);
    BOOST_CHECK(n == 0xfd);

    // zero encoded with five bytes:
    ss.write(MakeByteSpan("\xfe\x00\x00\x00\x00").first(5));
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure, isCanonicalException);

    // 0xffff encoded with five bytes:
    ss.write(MakeByteSpan("\xfe\xff\xff\x00\x00").first(5));
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure, isCanonicalException);

    // zero encoded with nine bytes:
    ss.write(MakeByteSpan("\xff\x00\x00\x00\x00\x00\x00\x00\x00").first(9));
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure, isCanonicalException);

    // 0x01ffffff encoded with nine bytes:
    ss.write(MakeByteSpan("\xff\xff\xff\xff\x01\x00\x00\x00\x00").first(9));
    BOOST_CHECK_EXCEPTION(ReadCompactSize(ss), std::ios_base::failure, isCanonicalException);
}

BOOST_AUTO_TEST_CASE(class_methods)
{
    int intval(100);
    bool boolval(true);
    std::string stringval("testing");
    const uint8_t charstrval[16]{"testing charstr"};
    CMutableTransaction txval;
    CTransactionRef tx_ref{MakeTransactionRef(txval)};
    CSerializeMethodsTestSingle methodtest1(intval, boolval, stringval, charstrval, tx_ref);
    CSerializeMethodsTestMany methodtest2(intval, boolval, stringval, charstrval, tx_ref);
    CSerializeMethodsTestSingle methodtest3;
    CSerializeMethodsTestMany methodtest4;
    CDataStream ss(SER_DISK, PROTOCOL_VERSION);
    BOOST_CHECK(methodtest1 == methodtest2);
    ss << methodtest1;
    ss >> methodtest4;
    ss << methodtest2;
    ss >> methodtest3;
    BOOST_CHECK(methodtest1 == methodtest2);
    BOOST_CHECK(methodtest2 == methodtest3);
    BOOST_CHECK(methodtest3 == methodtest4);

    CDataStream ss2(SER_DISK, PROTOCOL_VERSION, intval, boolval, stringval, charstrval, txval);
    ss2 >> methodtest3;
    BOOST_CHECK(methodtest3 == methodtest4);
}

enum class BaseFormat {
    DEC,
    HEX,
};

/// (Un)serialize a number as a string either from/to decimal or hexadecimal.
class Base
{
public:
    uint8_t m_base_data;

    Base() : m_base_data(17) {}
    explicit Base(uint8_t data) : m_base_data(data) {}

#if 0 // the following two are equivalent
    SERIALIZE_METHODS_PARAMS(Base, obj, BaseFormat, fmt)
    {
        if (ser_action.ForRead()) {
            std::string str;
            s >> str;
            assert(str.size() == 2 || (fmt == BaseFormat::DEC && str.size() == 3));
            uint32_t data;
            bool ok;
            if (fmt == BaseFormat::DEC) {
                ok = ParseUint32(str, &data);
            } else {
                ok = IsHex(str);
                data = ParseHex(str)[0];
            }
            assert(ok);
            SER_READ(obj, obj.m_base_data = data);
        } else if (fmt == BaseFormat::DEC) {
            s << strprintf("%02u", obj.m_base_data);
        } else {
            s << strprintf("%02X", obj.m_base_data);
        }
    }
#else
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        if (s.GetParams() == BaseFormat::DEC) {
            s << strprintf("%02u", m_base_data);
        } else {
            s << strprintf("%02X", m_base_data);
        }
    }

    template <typename Stream>
    void Unserialize(Stream& s)
    {
        std::string str;
        s >> str;
        assert(str.size() == 2 || (s.GetParams() == BaseFormat::DEC && str.size() == 3));
        uint32_t data;
        bool ok;
        if (s.GetParams() == BaseFormat::DEC) {
            ok = ParseUInt32(str, &data);
        } else {
            ok = IsHex(str);
            data = ParseHex(str)[0];
        }
        assert(ok);
        m_base_data = data;
    }
#endif
};

class DerivedAndBaseFormat
{
public:
    BaseFormat m_base_format;

    enum class DerivedFormat {
        LOWER,
        UPPER,
    } m_derived_format;
};

class Derived : public Base
{
public:
    std::string m_derived_data;

    SERIALIZE_METHODS_PARAMS(Derived, obj, DerivedAndBaseFormat, fmt)
    {
        READWRITE(WithParams(fmt.m_base_format, AsBase<Base>(obj)));

        if (ser_action.ForRead()) {
            std::string str;
            s >> str;
            SER_READ(obj, obj.m_derived_data = str);
        } else {
            s << (fmt.m_derived_format == DerivedAndBaseFormat::DerivedFormat::LOWER ?
                      ToLower(obj.m_derived_data) :
                      ToUpper(obj.m_derived_data));
        }
    }
};

BOOST_AUTO_TEST_CASE(with_params_base)
{
    Base b{15};

    CDataStream stream(SER_DISK, PROTOCOL_VERSION);

    stream << WithParams(BaseFormat::DEC, b);
    BOOST_CHECK_EQUAL(stream.str(), "\2" "15");

    b.m_base_data = 0;
    stream >> WithParams(BaseFormat::DEC, b);
    BOOST_CHECK_EQUAL(b.m_base_data, 15);

    stream.clear();

    stream << WithParams(BaseFormat::HEX, b);
    BOOST_CHECK_EQUAL(stream.str(), "\2" "0F");

    b.m_base_data = 0;
    stream >> WithParams(BaseFormat::HEX, b);
    BOOST_CHECK_EQUAL(b.m_base_data, 0x0F);
}

BOOST_AUTO_TEST_CASE(with_params_vector_of_base)
{
    std::vector<Base> v{Base{15}, Base{255}};

    CDataStream stream(SER_DISK, PROTOCOL_VERSION);

    stream << WithParams(BaseFormat::DEC, v);
    BOOST_CHECK_EQUAL(stream.str(), "\2\2" "15" "\3" "255");

    v[0].m_base_data = 0;
    v[1].m_base_data = 0;
    stream >> WithParams(BaseFormat::DEC, v);
    BOOST_CHECK_EQUAL(v[0].m_base_data, 15);
    BOOST_CHECK_EQUAL(v[1].m_base_data, 255);

    stream.clear();

    stream << WithParams(BaseFormat::HEX, v);
    BOOST_CHECK_EQUAL(stream.str(), "\2\2" "0F" "\2" "FF");

    v[0].m_base_data = 0;
    v[1].m_base_data = 0;
    stream >> WithParams(BaseFormat::HEX, v);
    BOOST_CHECK_EQUAL(v[0].m_base_data, 0x0F);
    BOOST_CHECK_EQUAL(v[1].m_base_data, 0xFF);
}

BOOST_AUTO_TEST_CASE(with_params_derived)
{
    Derived d;
    d.m_base_data = 15;
    d.m_derived_data = "xY";

    DerivedAndBaseFormat fmt;

    CDataStream stream(SER_DISK, PROTOCOL_VERSION);

    fmt.m_base_format = BaseFormat::DEC;
    fmt.m_derived_format = DerivedAndBaseFormat::DerivedFormat::LOWER;
    stream << WithParams(fmt, d);

    fmt.m_base_format = BaseFormat::HEX;
    fmt.m_derived_format = DerivedAndBaseFormat::DerivedFormat::UPPER;
    stream << WithParams(fmt, d);

    BOOST_CHECK_EQUAL(stream.str(), "\2" "15" "\2" "xy" "\2" "0F" "\2" "XY");
}

BOOST_AUTO_TEST_SUITE_END()
