#ifndef __Buffer_h__
#define __Buffer_h__

/** Abstract away buffers.
*/
class Buffer {
	public:
		Buffer() {}
		const void *start() const {return const_cast<Buffer*>(this)->start();}
		virtual ~Buffer() {}
		virtual void *start()=0;
		virtual size_t size() const=0;
};

#endif // __Buffer_h__
