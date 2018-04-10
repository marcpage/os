#ifndef __Address_h__
#define __Address_h__

#include "os/Exception.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

/** Networking related classes.
	@todo Test!
*/
namespace net {

	/** Abstraction of a Network Address */
	class Address {
		public:
			enum Qualified {
				FullyQualified,
				UnqualifiedLocal
			};
			enum Format {
				Numeric,
				Name
			};
			enum Required {
				NameRequired,
				NameIfAvailable
			};
			/** noop.*/
			virtual ~Address() {}
			/** Allow an <code>Address</code> to be passed as a sockaddr*.
				@return same as <code>get()</code>
			*/
			operator struct sockaddr *() { return get();}
			/** @todo Document */
			std::string name(Qualified qualified=FullyQualified, Format format=Name, Required required=NameIfAvailable) const;
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

	/**
		@todo create exceptions
     const char *
     gai_strerror(int ecode);

DESCRIPTION
     The gai_strerror() function returns an error message string corresponding to the error code returned by
     getaddrinfo(3) or getnameinfo(3).

     The following error codes and their meaning are defined in <netdb.h>:

           EAI_AGAIN     temporary failure in name resolution
           EAI_BADFLAGS  invalid value for ai_flags
           EAI_BADHINTS  invalid value for hints
           EAI_FAIL      non-recoverable failure in name resolution
           EAI_FAMILY    ai_family not supported
           EAI_MEMORY    memory allocation failure
           EAI_NONAME    hostname or servname not provided, or not known
           EAI_OVERFLOW  argument buffer overflow
           EAI_PROTOCOL  resolved protocol is unknown
           EAI_SERVICE   servname not supported for ai_socktype
           EAI_SOCKTYPE  ai_socktype not supported
           EAI_SYSTEM    system error returned in errno
	*/
	inline std::string Address::name(Qualified qualified, Format format, Required required) const {
		std::string dnsName(NI_MAXHOST, '\0');
		const int	flags= (UnqualifiedLocal == qualified ? NI_NOFQDN : 0) | (Numeric == format ? NI_NUMERICHOST : 0) | (NameRequired == required ? NI_NAMEREQD : 0);

		AssertMessageException( 0 != ::getnameinfo(get(), size(), const_cast<char*>(dnsName.data()), dnsName.size(), NULL, 0, flags) );
		return dnsName;
	}
}

#endif // __Address_h__
