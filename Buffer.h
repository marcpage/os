#ifndef __Buffer_h__
#define __Buffer_h__

/** Abstract away buffers.
*/
class Buffer {
	public:
		Buffer() {}
		virtual ~Buffer() {}
		virtual void *start()=0;
		virtual size_t size()=0;
};

#endif // __Buffer_h__
