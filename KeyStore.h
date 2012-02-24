#ifndef __KeyStore_h__
#define __KeyStore_h__

#include <string>

/** KeyStore interface.
	@todo More documentation
	@todo Test
*/
class KeyStore {
	public:
		KeyStore();
		virtual ~KeyStore();
		/**
			@return	true if the key has a value (or is linked to a key that has a value).
		*/
		virtual bool has(const std::string &key)= 0;
		/**
			@return true if key already existed and was replaced
		*/
		virtual bool set(const std::string &key, const std::string &value)= 0;
		/**
			@param key		The key to store data for.
			@param value	Receives the value of key, or remains unchanged if the key
								does not exist.
			@return			Reference to value parameter
		*/
		virtual std::string &get(const std::string &key, std::string &value)= 0;
		/** Removes the key.
				If this is the last reference to the data (last link), then the data
				is also removed.
			@return true if the key existed and was removed.
		*/
		virtual bool remove(const std::string &key)= 0;
		/** Creates a link from one key-value to another.
				After linking, linkKey and existingKey will return the same value
				and they are peers, treated equally.
				The link can be broken by doing a set on either key, or removing a key.
		*/
		virtual void link(const std::string &linkKey, const std::string &existingKey)= 0;
};

inline KeyStore::KeyStore() {}
inline KeyStore::~KeyStore() {}

#endif // __KeyStore_h__
