#ifndef __Address_h__
#define __Address_h__

namespace net {

	class Address {
		public:
			Address() {}
			~Address() {}
			operator struct sockaddr*() {return get();}
			operator socklen_t() {return size();}
			struct sockaddr *get()=0;
			socklen_t size()=0;
	};

}

#endif // __Address_h__
