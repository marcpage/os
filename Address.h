#ifndef __Address_h__
#define __Address_h__

namespace net {

	class Address {
		public:
			Address() {}
			virtual ~Address() {}
			operator const struct sockaddr*() const {return get();}
			const struct sockaddr *get() const {return const_cast<Address*>(this)->get();}
			virtual struct sockaddr *get()=0;
			virtual socklen_t size() const=0;
			virtual sa_family_t family() const=0;
	};

}

#endif // __Address_h__
