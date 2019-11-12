#ifndef __Buffer_h__
#define __Buffer_h__

/** Abstract away buffers.
	@todo Test!
*/
class Buffer {
	public:
		/** Get the start of the buffer for when we are const.
			@return the address of the start of the buffer
		*/
		const void *start() const { return const_cast<Buffer*>(this)->start();}
		/** noop. */
		virtual ~Buffer() {}
		/** @return the address of the beginning of the buffer. */
		virtual void *start()=0;
		/** @return the maximum offset off <code>start()</code> you can access. */
		virtual size_t size() const=0;
};

#endif // __Buffer_h__
