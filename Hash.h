#ifndef __Hash_h__
#define __Hash_h__

#include <string>

class Hash {
	public:
		Hash() {}
		virtual ~Hash() {}
		virtual std::string &hex(std::string &value) const= 0;
		virtual uint32_t size() const= 0;
		virtual const uint8_t *buffer() const= 0;
		virtual void reset(const char *hash)= 0;
		virtual void reset(const void *data, size_t count)= 0;
		virtual void reset(const std::string &data)= 0;
		virtual const char *name() const= 0;
};
template<class Hasher>
class SpecificHash : public Hash {
	public:
		enum {
			Size = Hasher::Size
		};
		SpecificHash();
		SpecificHash(const char *hash);
		SpecificHash(const void *data, size_t count);
		SpecificHash(const std::string &data);
		SpecificHash(const SpecificHash &other);
		virtual ~SpecificHash();
		SpecificHash &operator=(const SpecificHash &other);
		bool operator==(const SpecificHash &other);
		bool operator!=(const SpecificHash &other);
		bool valid() const;
		bool same(const SpecificHash &other);
		operator bool() const;
		uint8_t *buffer();
		virtual const uint8_t *buffer() const;
		virtual uint32_t size() const;
		virtual std::string &hex(std::string &value) const;
		virtual void reset(const char *hash);
		virtual void reset(const void *data, size_t count);
		virtual void reset(const std::string &data);
		virtual const char *name() const;
	private:
		uint8_t	_hash[Size];
};

#if __APPLE_CC__ || __APPLE__

#include <CommonCrypto/CommonDigest.h>

struct SHA256Hasher {
	enum {Size= CC_SHA256_DIGEST_LENGTH};
	static const char *name() {return "sha256";}
	static void hash(const void *data, size_t dataSize, void *hash) {CC_SHA256(data, dataSize, reinterpret_cast<unsigned char*>(hash));}
};

#endif

#include "MessageException.h"

#include <string.h>
#include <ctype.h>

template<class Hasher> inline SpecificHash<Hasher>::SpecificHash()
	:_hash() {
	memset(_hash, 0, sizeof(_hash));
}
template<class Hasher> inline SpecificHash<Hasher>::SpecificHash(const char *hash)
	:_hash() {
	reset(hash);
}
template<class Hasher> inline SpecificHash<Hasher>::SpecificHash(const void *data, size_t count)
	:_hash() {
	Hasher::hash(data, count, _hash);
}
template<class Hasher> inline SpecificHash<Hasher>::SpecificHash(const std::string &data)
	:_hash() {
	Hasher::hash(data.data(), data.size(), _hash);
}
template<class Hasher> inline SpecificHash<Hasher>::SpecificHash(const SpecificHash &other)
	:_hash() {
	memcpy(_hash, other._hash, sizeof(_hash));
}
template<class Hasher> inline SpecificHash<Hasher>::~SpecificHash() {
}
template<class Hasher> inline SpecificHash<Hasher> &SpecificHash<Hasher>::operator=(const SpecificHash<Hasher> &other) {
	if(this != &other) {
		memcpy(_hash, other._hash, sizeof(_hash));
	}
	return *this;
}
template<class Hasher> inline bool SpecificHash<Hasher>::operator==(const SpecificHash<Hasher> &other) {
	return memcmp(_hash, other._hash, sizeof(_hash)) == 0;
}
template<class Hasher> inline bool SpecificHash<Hasher>::operator!=(const SpecificHash<Hasher> &other) {
	return memcmp(_hash, other._hash, sizeof(_hash)) != 0;
}
template<class Hasher> inline bool SpecificHash<Hasher>::valid() const {
	for(int i= 0; i < static_cast<int>(sizeof(_hash)); ++i) {
		if(_hash[i] != 0) {
			return true;
		}
	}
	return false;
}
template<class Hasher> inline bool SpecificHash<Hasher>::same(const SpecificHash<Hasher> &other) {
	return memcmp(_hash, other._hash, sizeof(_hash)) == 0;
}
template<class Hasher> inline SpecificHash<Hasher>::operator bool() const {
	return valid();
}
template<class Hasher> inline uint8_t *SpecificHash<Hasher>::buffer() {
	return _hash;
}
template<class Hasher> inline const uint8_t *SpecificHash<Hasher>::buffer() const {
	return _hash;
}
template<class Hasher> inline uint32_t SpecificHash<Hasher>::size() const {
	return Hasher::Size;
}
template<class Hasher> inline std::string &SpecificHash<Hasher>::hex(std::string &value) const {
	const char * const	hexDigits= "0123456789abcdef";

	value.clear();
	for(int i= 0; i < static_cast<int>(sizeof(_hash)); ++i) {
		value.append(1, hexDigits[_hash[i] & 0x0F]);
		value.append(1, hexDigits[_hash[i] >> 4]);
	}
	return value;
}
template<class Hasher> inline void SpecificHash<Hasher>::reset(const char *hash) {
	std::string	hexDigits("0123456789abcdef");
	bool		eosFound= false;
	AssertMessageException(strlen(hash) <= Size * 2);
	for(int byte= 0; byte < static_cast<int>(sizeof(_hash)); ++byte) {
		const int	nibble1= byte * 2;
		eosFound= eosFound || hash[nibble1] == '\0';
		std::string::size_type	found1= eosFound ? 0 : hexDigits.find(hash[nibble1]);

		const int	nibble2= nibble1 + 1;
		eosFound= eosFound || hash[nibble2] == '\0';
		std::string::size_type	found2= eosFound ? 0 : hexDigits.find(hash[nibble2]);

		AssertMessageException(found1 != std::string::npos);
		AssertMessageException(found2 != std::string::npos);
		_hash[byte]= (found2 << 4) | found1;
	}
	AssertMessageException(hash[sizeof(_hash)*2] == '\0');
}
template<class Hasher> inline void SpecificHash<Hasher>::reset(const void *data, size_t count) {
	Hasher::hash(data, count, _hash);
}
template<class Hasher> inline void SpecificHash<Hasher>::reset(const std::string &data) {
	Hasher::hash(data.data(), data.size(), _hash);
}
template<class Hasher> inline const char *SpecificHash<Hasher>::name() const {
	return Hasher::name();
}

#endif // __Hash_h__
