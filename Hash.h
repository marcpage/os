#ifndef __Hash_h__
#define __Hash_h__

/** @file Hash.h
        @todo evaluate the ability to do partial hashes so not all the data
   needs to be in memory.
*/

#include "Exception.h"
#include "Text.h"
#include <ctype.h>
#include <string.h>
#include <string>

#if __APPLE_CC__ || __APPLE__
#include <CommonCrypto/CommonDigest.h>
#endif

#if OpenSSLAvailable
#include <openssl/md5.h>
#include <openssl/sha.h>
#endif

/** Hashing support.
 */
namespace hash {
/** Pure virtual parent of all specific hashers.
 */
class Hash {
public:
  /// Default constructor.
  Hash() {}
  /// Nothing to do in destructor.
  virtual ~Hash() {}
  /// Returns the hex value of the hash.
  virtual std::string hex() const {
    std::string tempBuffer;
    return hex(tempBuffer);
  }
  /// Fills <code>value</code> with the hex value of the hash.
  virtual std::string &hex(std::string &value) const = 0;
  /// Returns the base64 value of the hash.
  virtual std::string
  base64(text::Base64Style style = text::Base64URLNoPadding) const {
    std::string tempBuffer;
    return base64(tempBuffer, style);
  }
  /// Fills <code>value</code> with the base64 value of the hash.
  virtual std::string &
  base64(std::string &value,
         text::Base64Style style = text::Base64URLNoPadding) const = 0;
  /// Returns the number of bytes in the hash.
  virtual uint32_t size() const = 0;
  /// Returns the pointer to the internal hash buffer.
  virtual const uint8_t *buffer() const = 0;
  /// Returns the contents of the binary hash @todo test
  std::string data() {
    return std::string(reinterpret_cast<const char *>(buffer()), size());
  }
  /// Returns the contents of the binary hash in raw @todo test
  std::string &data(std::string &raw) {
    raw.assign(reinterpret_cast<const char *>(buffer()), size());
    return raw;
  }
  /// Resets the hash to a new hex value.
  virtual void assignFromHex(const std::string &hex) = 0;
  /// Resets the hash to a new base64 value.
  virtual void assignFromBase64(const std::string &base64) = 0;
  /// Resets the hash and starts hashing new data.
  virtual void reset(const void *data, size_t count) = 0;
  /// Resets the hash and starts hashing a new string.
  virtual void reset(const std::string &data) = 0;
  /// The name of the hashing function.
  virtual const char *name() const = 0;
};
/** The specific instantiation of a Hash.
        @tparam Hasher	See CommonCryptoSHA256Hasher below as an example.
                                        Must implement name(), hash() and have a
   member Size.
*/
template <class Hasher> class SpecificHash : public Hash {
public:
  enum {
    Size = Hasher::Size ///< The number of bytes in the hash.
  };
  /// Create a hash object from raw hash data
  static SpecificHash<Hasher> fromData(const void *buffer, uint32_t size);
  /// Create a hash object from hex hash
  static SpecificHash<Hasher> fromHex(const std::string &hex);
  /// Create a hash object from base64 hash
  static SpecificHash<Hasher> fromBase64(const std::string &base64);
  /// The default constructor.
  SpecificHash();
  /// Construct and initialize with hash of data.
  SpecificHash(const void *data, size_t count);
  /// Construct and initialize with hash of a string.
  explicit SpecificHash(const std::string &data);
  /// Copy another hash's data.
  SpecificHash(const SpecificHash &other);
  /// Nothing to do in destructor.
  virtual ~SpecificHash() {}
  /// Assign another hasher's data to us.
  SpecificHash &operator=(const SpecificHash &other);
  /// Determine if our current hash is the same as another.
  bool operator==(const SpecificHash &other);
  /// Determine if we are different than another hash.
  bool operator!=(const SpecificHash &other);
  /// Determine if the hash is valid
  bool valid() const;
  /// Determine if the hashes represent the same value
  bool same(const SpecificHash &other);
  /// Determine if the hash is valid
  operator bool() const;
  /// Get a pointer to the hash data
  uint8_t *buffer();
  /// Get a const pointer to the hash data
  const uint8_t *buffer() const override;
  /// Get the number of bytes of hash data
  uint32_t size() const override;
  /// Get the hex value of the hash
  std::string hex() const override { return Hash::hex(); }
  /** Get the hex value of the hash.
        @param value The buffer to fill with the hex value
        @return reference to value
  */
  std::string &hex(std::string &value) const override;
  /// Get the base64 value of the hash
  std::string
  base64(text::Base64Style style = text::Base64URLNoPadding) const override {
    return Hash::base64(style);
  }
  /**
  @todo document
  */
  std::string &
  base64(std::string &value,
         text::Base64Style style = text::Base64URLNoPadding) const override;
  /** Parse a hex hash value.
        @param hex string that points to a null-terminated hex value
  */
  void assignFromHex(const std::string &hex) override;
  /**
        @todo test
  */
  void assignFromBase64(const std::string &base64) override;
  /** Calculate the hash of the given data.
        @param data The data to calculate the hash of
        @param count The number of bytes in data to use to calculate the hash
  */
  void reset(const void *data, size_t count) override;
  /** Calculate the hash of the given data.
        @param data The data to calculate the hash of
  */
  void reset(const std::string &data) override;
  /// Get the name of the hasher, ie sha256 or md5
  const char *name() const override;

private:
  uint8_t _hash[Size]; ///< The hash value binary data
};

#if OpenSSLAvailable

/// Open SSL version of MD5 Hasher
struct OpenSLLMD5Hasher {
  enum { Size = MD5_DIGEST_LENGTH };
  static const char *name() { return "md5"; }
  static void hash(const void *data, size_t dataSize,
                   void *hash) { /// @too test
    MD5(reinterpret_cast<const unsigned char *>(data), dataSize,
        reinterpret_cast<unsigned char *>(hash));
  }
};

/// Open SLL version of SHA256 Hasher
struct OpenSSLSHA256Hasher {
  enum { Size = SHA256_DIGEST_LENGTH };
  static const char *name() { return "sha256"; }
  static void hash(const void *data, size_t dataSize, void *hash) {
    SHA256(reinterpret_cast<const unsigned char *>(data), dataSize,
           reinterpret_cast<unsigned char *>(hash));
  }
};

typedef SpecificHash<OpenSSLSHA256Hasher> openssl_sha256;
typedef SpecificHash<OpenSLLMD5Hasher> openssl_md5;
typedef SpecificHash<OpenSLLMD5Hasher>
    md5; ///< Apple doesn't support md5 anymore

#if !(__APPLE_CC__ || __APPLE__)
typedef SpecificHash<OpenSSLSHA256Hasher> sha256;
#endif

#endif

#if __APPLE_CC__ || __APPLE__

/// A SHA256 example of a hasher. See SpecificHash.
struct CommonCryptoSHA256Hasher {
  enum {
    Size = CC_SHA256_DIGEST_LENGTH ///< Every SpecificHash hasher must have a
                                   ///< Size enum. This is the number of bytes
                                   ///< in the hash.
  };
  /** Every SpecificHash hasher must have a name() method. This is the name of
     this hash.
          @return	The name of this hash method.
  */
  static const char *name() { return "sha256"; }
  /** Every SpecificHash hasher must have a hash() method. This computes the
     hash of the given data.
          @param data		The data to hash.
          @param dataSize	The number of bytes in <code>data</code> to
     hash.
          @param hash		An area of memory, at least <code>Size</code>
     bytes in length that will hold the hash.
  */
  static void hash(const void *data, size_t dataSize, void *hash) {
    CC_SHA256(data, dataSize, reinterpret_cast<unsigned char *>(hash));
  }
};

typedef SpecificHash<CommonCryptoSHA256Hasher> sha256;
#endif

/** Copy the raw hash data into the hasher.
        @param buffer The hash to copy
        @param size The size of the hash
 */
template <class Hasher>
inline SpecificHash<Hasher> SpecificHash<Hasher>::fromData(const void *buffer,
                                                           uint32_t size) {
  SpecificHash<Hasher> result;

  AssertMessageException(size == Size);
  memcpy(result._hash, buffer, Size);
  return result;
}
/// Create a hash object from hex string
template <class Hasher>
inline SpecificHash<Hasher>
SpecificHash<Hasher>::fromHex(const std::string &hex) {
  SpecificHash<Hasher> result;

  result.assignFromHex(hex);
  return result;
}
template <class Hasher>
inline SpecificHash<Hasher>
SpecificHash<Hasher>::fromBase64(const std::string &base64) {
  SpecificHash<Hasher> result;

  result.assignFromBase64(base64);
  return result;
}
/** Empty hash value.
 */
template <class Hasher> inline SpecificHash<Hasher>::SpecificHash() : _hash() {
  memset(_hash, 0, sizeof(_hash));
}
/** Calculates the hash of some data.
        @param data the data to calculate the hash of.
        @param count the number of bytes of data to calculate the hash of
*/
template <class Hasher>
inline SpecificHash<Hasher>::SpecificHash(const void *data, size_t count)
    : _hash() {
  Hasher::hash(data, count, _hash);
}
/** Calculates the hash of some data.
        @param data the data to calculate the hash of.
*/
template <class Hasher>
inline SpecificHash<Hasher>::SpecificHash(const std::string &data) : _hash() {
  Hasher::hash(data.data(), data.size(), _hash);
}
/** Copy constructor.
        @param other other hash to copy the hash value of.
*/
template <class Hasher>
inline SpecificHash<Hasher>::SpecificHash(const SpecificHash &other)
    : Hash(other), _hash() {
  memcpy(_hash, other._hash, sizeof(_hash));
}
/** Assignment operator.
        @param other hash to copy.
        @return reference to this
*/
template <class Hasher>
inline SpecificHash<Hasher> &
SpecificHash<Hasher>::operator=(const SpecificHash<Hasher> &other) {
  if (this != &other) {
    memcpy(_hash, other._hash, sizeof(_hash));
  }
  return *this;
}
/** Comparison operator, compares the binary of the hashes.
        @param other the hash to compare.
        @return true if the binary representation of each hash is identical.
*/
template <class Hasher>
inline bool
SpecificHash<Hasher>::operator==(const SpecificHash<Hasher> &other) {
  return same(other);
}
/** Comparison operator, compares the binary of the hashes.
        @param other the hash to compare.
        @return false if the binary representation of each hash is identical.
*/
template <class Hasher>
inline bool
SpecificHash<Hasher>::operator!=(const SpecificHash<Hasher> &other) {
  return !same(other);
}
/** Hash a hash been calculated or assigned, or is this still an invalid,
   uninitialized hash. Since an uninitialized hash is zeroed out, we check to
   see if all bytes are zero. If they are, then this is not a hash value.
        @return true if any of the bytes are non-zero
*/
template <class Hasher> inline bool SpecificHash<Hasher>::valid() const {
  for (int i = 0; (i < static_cast<int>(sizeof(_hash))); ++i) {
    if (_hash[i] != 0) {
      return true;
    }
  }
  return false;
}
/** Compares the binary of the hashes.
        @param other the hash to compare.
        @return false if the binary representation of each hash is identical.
*/
template <class Hasher>
inline bool SpecificHash<Hasher>::same(const SpecificHash<Hasher> &other) {
  return (memcmp(_hash, other._hash, sizeof(_hash)) == 0);
}
/** Operator typecast to bool, returns true if this hash has been initialized.
        @return true if a hash value has been calculated or assigned.
*/
template <class Hasher> inline SpecificHash<Hasher>::operator bool() const {
  return valid();
}
/** The pointer to the actual data buffer of this hash.
        @return the address of the buffer used to store the binary hash.
*/
template <class Hasher> inline uint8_t *SpecificHash<Hasher>::buffer() {
  return _hash;
}
/** The pointer to the actual data buffer of this hash.
        @return the address of the buffer used to store the binary hash.
*/
template <class Hasher>
inline const uint8_t *SpecificHash<Hasher>::buffer() const {
  return _hash;
}
/** The size of the hash, in bytes.
        @return the number of bytes in the buffer().
*/
template <class Hasher> inline uint32_t SpecificHash<Hasher>::size() const {
  return Hasher::Size;
}
/** Get the hex value of the hash.
        @return a reference to value
*/
template <class Hasher>
inline std::string &SpecificHash<Hasher>::hex(std::string &value) const {
  value.clear();
  return text::toHex(
      std::string(reinterpret_cast<const char *>(_hash), sizeof(_hash)), value);
}
template <class Hasher>
inline std::string &
SpecificHash<Hasher>::base64(std::string &value,
                             text::Base64Style style) const {
  value.clear();
  return text::base64Encode(
      std::string(reinterpret_cast<const char *>(_hash), sizeof(_hash)), value,
      style);
}
/** Make the value of this hash be the given lowercase hex value.
 */
template <class Hasher>
inline void SpecificHash<Hasher>::assignFromHex(const std::string &hex) {
  std::string buffer;
  text::fromHex(hex, buffer)
      .copy(reinterpret_cast<char *>(_hash), sizeof(_hash));
  AssertMessageException(buffer.size() == sizeof(_hash));
}
template <class Hasher>
inline void SpecificHash<Hasher>::assignFromBase64(const std::string &base64) {
  std::string buffer;
  text::base64Decode(base64, buffer)
      .copy(reinterpret_cast<char *>(_hash), sizeof(_hash));
  AssertMessageException(buffer.size() == sizeof(_hash));
}
/** Recalculate the hash to be the hash of the given data.
 */
template <class Hasher>
inline void SpecificHash<Hasher>::reset(const void *data, size_t count) {
  Hasher::hash(data, count, _hash);
}
/** Recalculate the hash to be the hash of the given data.
 */
template <class Hasher>
inline void SpecificHash<Hasher>::reset(const std::string &data) {
  Hasher::hash(data.data(), data.size(), _hash);
}
/** The name of the hashing function used.
        @return the name from the Hasher, ie sha256, md5, etc.
*/
template <class Hasher> inline const char *SpecificHash<Hasher>::name() const {
  return Hasher::name();
}
} // namespace hash

#endif // __Hash_h__
