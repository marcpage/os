#ifndef __Address_h__
#define __Address_h__

#include <sys/socket.h>

/** Networking related classes.
	@todo Test!
*/
namespace net {

	/** Abstraction of a Network Address */
	class Address {
		public:
			/** noop.*/
			virtual ~Address() {}
			/** Allow an <code>Address</code> to be passed as a sockaddr*.
				@return same as <code>get()</code>
			*/
			operator struct sockaddr *() { return get();}
			/** Get the address for when we are a const instance.
				@return the const sockaddr address
			*/
			const struct sockaddr *get() const { return const_cast<Address*>(this)->get();}
			/** @return the address of the appropriate address structure. */
			virtual struct sockaddr *get()=0;
			/** @return the size of the socket address structure returned by <code>get()</code>. */
			virtual socklen_t size() const=0;
			/** @return the domain or address family used. */
			virtual sa_family_t family() const=0;
	};

}

#endif // __Address_h__
