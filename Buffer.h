#ifndef __Buffer_h__
#define __Buffer_h__

#ifndef trace_scope
	#define trace_scope ///< @brief in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< @brief in case Tracer.h is not included
#endif

/** Abstract away buffers.
	@todo Test!
*/
class Buffer {
	public:
		/** Get the start of the buffer for when we are const.
			@return the address of the start of the buffer
		*/
		const void *start() const {trace_scope return const_cast<Buffer*>(this)->start();}
		/** noop. */
		virtual ~Buffer() {trace_scope}
		/** @return the address of the beginning of the buffer. */
		virtual void *start()=0;
		/** @return the maximum offset off <code>start()</code> you can access. */
		virtual size_t size() const=0;
};

#endif // __Buffer_h__
