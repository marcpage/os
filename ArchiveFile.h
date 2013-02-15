#ifndef _ArchiveFile_h_
#define _ArchiveFile_h_

#include <os/File.h>
#include <os/POSIXErrno.h>

/// Default signature detects text conversions and has high-bit value to prevent text detection
#define io_ArchiveFile_DefaultSignature "\x89""00\x0D\x0A\x1A\x0A"

namespace io {
	/** A container file that can return allocated sections.
	*/
	class ArchiveFile : public File {
		public:
			/** A block in the file.
				NOTE: Last block is a near-infinite free block
			*/
			class Block {
				public:
					/// Create an invalid block
					Block();
					/// Copy a block (modifications to either may not be reflected in the other)
					Block(const Block &other);
					/// Create a block by reading the block header from the location
					Block(int64_t location, ArchiveFile &storage);
					~Block();
					/// Get the next block (or an invalid block if no more blocks)
					Block next();
					/// Copy a block (modifications to either may not be reflected in the other)
					Block &operator=(const Block &other);
					/// Is the block valid
					operator bool() const;
					/// Allows the block to be passed directly to io::File operations as the offset
					operator off_t() const;
					/// Allows the block to be passed to the Relative parameter to File::write
					operator File::Relative() const;
					/// blocks in same file and same location
					bool operator==(const Block &other);
					/// blocks not in same file or not at same location
					bool operator!=(const Block &other);
					/// blocks in same file and this is earlier in file than other
					bool operator<(const Block &other);
					/// blocks in same file and this is later in file than other
					bool operator>(const Block &other);
					/// blocks in same file and this is earlier or same location in file as other
					bool operator<=(const Block &other);
					/// blocks in same file and this is later or same location in file as other
					bool operator>=(const Block &other);
					/// prefix increment
					Block &operator++();
					/// postfix increment
					Block operator++(int);
					/// the unique identifier of this block (see ArchiveFile::lookup)
					int64_t identifier();
					/// The size of the data in the block (or the size of the entire block)
					int64_t size(bool justData= true);
					/// The offset of the data in the block (or the offset of the block header)
					int64_t offset(bool ofData= true);
					/// Is this a free block?
					bool free();
					/// Attempt to allocate a block of the given data size, false returned if not possible
					bool allocate(int64_t payloadSize, uint8_t userFlags);
					/// Mark this block as free
					Block &dispose();
					/// Is this a valid block (in a file at a possibly valid position)
					bool valid() const;
					/// If this is a free block, looks for free blocks directly after it and merges
					Block &merge();
					/// The ArchiveFile this block is in (or NULL if not a valid Block)
					ArchiveFile &file();
					/// The user flags for the block (lower 7-bits only)
					uint8_t flags();
					/// Sets the user flags for the block (lower 7-bits only) and returns the previous flags
					uint8_t flags(uint8_t newFlags);
					/// Attempts to resize the block, returns false if not possible
					bool resize(int64_t newPayloadSize);
					/// Read the contents of the block
					std::string read();
					/// Read the contents of the block into a buffer
					std::string &read(std::string &buffer);
				protected:
					int64_t		_location;	///< The offset in _storage of the block
					uint8_t		_flags;		///< User flags (lower 7 bits) for allocated block
					int64_t		_size;		///< The size of the entire block, including header
					ArchiveFile	*_storage;	///< The ArchiveFile this block is in (or NULL for invalid)
					/// Reads the block header from _location in _storage
					bool _readHeader();
					/// Writes a free block header at _location in _storage
					void _writeFreeHeader();
					/// Writes _flags to _location in _storage
					void _writeHeaderFlags();
					/// Writes _flags and _size - (header size) to _location in _storage
					void _writeHeader();
					/// Writes allocated header and adds free block after if it is bigger than needed
					void _allocate(int64_t payloadSize, uint8_t userFlags);
			};
			/// Open or Create ArchiveFile at given path
			ArchiveFile(const char *path, Protection protection= WriteIfPossible, uint16_t version= 1, const std::string &signature= io_ArchiveFile_DefaultSignature);
			/// Open or Create ArchiveFile at given path
			ArchiveFile(const std::string &path, Protection protection= WriteIfPossible, uint16_t version= 1, const std::string &signature= io_ArchiveFile_DefaultSignature);
			/// Destructor
			virtual ~ArchiveFile();
			/// Allocate a block from the file for user writing
			Block allocate(int64_t dataSize, uint8_t flags= 0);
			/// Allocate a block from the file and write data to it
			Block allocate(const std::string &data, uint8_t flags= 0);
			/// Load the block from the file for the given identifier
			Block lookup(int64_t identifier);
			/// The first block in the file
			Block begin();
			/// Invalid block representing the block after the last block
			Block end();
		private:
			int64_t	_headerSize;	///< The size of the header (signature and version)
			/// Create file if necessary or validate the header
			void _init(uint16_t version, const std::string &signature);
	};

	inline ArchiveFile::Block::Block()
			:_location(0), _flags(0), _size(0), _storage(NULL) {trace_scope}
	inline ArchiveFile::Block::Block(const Block &other)
			:_location(other._location), _flags(other._flags), _size(other._size), _storage(other._storage) {trace_scope}
	inline ArchiveFile::Block::Block(int64_t location, ArchiveFile &storage)
			:_location(location), _flags(0), _size(0), _storage(&storage) {trace_scope
		_readHeader();
	}
	inline ArchiveFile::Block::~Block() {trace_scope}
	/**
		@return	next Block or an invalid (!block.valid()) block if no more blocks or this is invalid.
	*/
	inline ArchiveFile::Block ArchiveFile::Block::next() {trace_scope
		const int64_t	kFileSizeMax= INT64_MAX;
		Block			n;

		if( (_location + _size) < kFileSizeMax) {
			n._storage= _storage;
			n._location= _location + _size;
			n._readHeader();
		}
		return n;
	}
	inline ArchiveFile::Block &ArchiveFile::Block::operator=(const Block &other) {trace_scope
		_location= other._location;
		_flags= other._flags;
		_size= other._size;
		_storage= other._storage;
		return *this;
	}
	/** Just calls valid()
	*/
	inline ArchiveFile::Block::operator bool() const {trace_scope
		return valid();
	}
	inline ArchiveFile::Block::operator off_t() const {trace_scope
		const int64_t	kHeaderSize= sizeof(uint8_t) + sizeof(int64_t);

		return _location + kHeaderSize;
	}
	inline ArchiveFile::Block::operator File::Relative() const {trace_scope
		return FromStart;
	}
	inline bool ArchiveFile::Block::operator==(const Block &other) {trace_scope
		return trace_bool((_storage == other._storage) && (_location == other._location));
	}
	inline bool ArchiveFile::Block::operator!=(const Block &other) {trace_scope
		return trace_bool(!(*this == other));
	}
	inline bool ArchiveFile::Block::operator<(const Block &other) {trace_scope
		return trace_bool((_storage == other._storage) && (_location < other._location));
	}
	inline bool ArchiveFile::Block::operator>(const Block &other) {trace_scope
		return trace_bool((_storage == other._storage) && (_location > other._location));
	}
	inline bool ArchiveFile::Block::operator<=(const Block &other) {trace_scope
		return trace_bool((_storage == other._storage) && (_location <= other._location));
	}
	inline bool ArchiveFile::Block::operator>=(const Block &other) {trace_scope
		return trace_bool((_storage == other._storage) && (_location >= other._location));
	}
	inline ArchiveFile::Block &ArchiveFile::Block::operator++() {trace_scope
		*this= next();
		return *this;
	}
	inline ArchiveFile::Block ArchiveFile::Block::operator++(int) {trace_scope
		Block	old= *this;

		*this= next();
		return old;
	}
	inline int64_t ArchiveFile::Block::identifier() {trace_scope
		return _location;
	}
	/** Gets the size.
		@param justData if true, then gets the size of the data in the block, otherwise the size of the whole block
		@return			The size of the data or block depending on justData
	*/
	inline int64_t ArchiveFile::Block::size(bool justData) {trace_scope
		const int64_t	kHeaderSize= sizeof(uint8_t) + sizeof(int64_t);

		if(justData) {
			if(_size < kHeaderSize) { // @todo Test
				return 0;
			}
			return _size - kHeaderSize;
		}
		return _size;
	}
	/** Gets the offset.
		@param ofData	If true, gets the offset of the data that can be read or written, if false, the offset of the block header
		@return			Offset of data or header of block, depending on ofData
	*/
	inline int64_t ArchiveFile::Block::offset(bool ofData) {trace_scope
		const int64_t	kHeaderSize= sizeof(uint8_t) + sizeof(int64_t);

		if(ofData) {
			return _location + kHeaderSize;
		}
		return _location;
	}
	/**
		@return true if this block has not been allocated for use
	*/
	inline bool ArchiveFile::Block::free() {trace_scope
		const uint8_t	kAllocatedBit= 0x80;

		return trace_bool((_flags & kAllocatedBit) != kAllocatedBit);
	}
	/** Creates a block in the file with the given data size and user flags.
		@param payloadSize	The size of data that can be written to the block
		@param userFlags	The flags the user can associate with the block (lower 7 bits only)
		@return				true if a block can be allocated
	*/
	inline bool ArchiveFile::Block::allocate(int64_t payloadSize, uint8_t userFlags) {trace_scope
		if( !free() || (NULL == _storage) || (payloadSize > size()) ) { // @todo Test
			return false;
		}
		_allocate(payloadSize, userFlags);
		return true;
	}
	/** Marks the block as available for user by others
	*/
	inline ArchiveFile::Block &ArchiveFile::Block::dispose() {trace_scope
		if(!free() && (NULL != _storage) ) {
			_writeFreeHeader();
			_storage->flush();
			merge();
		}
		return *this;
	}
	/**
		@return	true if we have a valid ArchiveFile (not NULL), the identifier is not 0 and the offset is within the file
	*/
	inline bool ArchiveFile::Block::valid() const {trace_scope
		return trace_bool((NULL != _storage) && (_location > 0) && (_location < _storage->size()));
	}
	/** If this is a free block, looks for a series of free blocks after this block
			and merges them with this block.
	*/
	inline ArchiveFile::Block &ArchiveFile::Block::merge() {trace_scope
		if( (NULL == _storage) || (_location == 0) ) { // @todo Test
			return *this;
		}
		if(free()) { // @todo Test
			Block	current= *this;
			Block	after= next();

			while(after.valid() && after.free()) {
				current= after;
				after= after.next();
			}
			if(current != *this) { // We found some free blocks after this
				_size= current._location + current._size - _location;
				_writeFreeHeader();
				_storage->flush();
			}
		}
		return *this;
	}
	inline ArchiveFile &ArchiveFile::Block::file() {trace_scope
		return *_storage;
	}
	/**
		@return The lower 7 bits of the flags (highest 8th bit is reserved and not modifiable)
	*/
	inline uint8_t ArchiveFile::Block::flags() {trace_scope
		const uint8_t	kUserFlagsMask= 0x7F;

		return (_flags & kUserFlagsMask);
	}
	/**
		@param newFlags If the highest bit is set, it will be cleared and ignored
		@todo Test
	*/
	inline uint8_t ArchiveFile::Block::flags(uint8_t newFlags) {trace_scope
		const uint8_t	kUserFlagsMask= 0x7F;
		const uint8_t	oldFlags= _flags & kUserFlagsMask;

		_flags= (newFlags & kUserFlagsMask);
		return oldFlags;
	}
	/** Shrinking this block should always work. Growing this block depends on
			free blocks after this one.
		@param newPayloadSize	The requested new size of the block
		@return					true if the block could be resized.
	*/
	inline bool ArchiveFile::Block::resize(int64_t newPayloadSize) {trace_scope
		Block	block, nextBlock;

		if( free() || (NULL == _storage) || (0 == _location) ) {
			return false;
		}
		if(newPayloadSize > size()) {
			block= *this;
			nextBlock= block.next();
			while(nextBlock.free() && nextBlock.valid()) {
				block= nextBlock;
				nextBlock= nextBlock.next();
			}
			if(block == *this) {
				return false;
			}
			const int64_t	extraFree= block._location + block._size - (_location + _size);
			if(newPayloadSize > size() + extraFree) {
				return false;
			}
			_size+= extraFree;
		}
		if(newPayloadSize == size()) {
			_writeHeader();
		} else {
			_allocate(newPayloadSize, _flags);
		}
		return true;
	}
	inline std::string ArchiveFile::Block::read() {trace_scope
		std::string	buffer;

		return read(buffer);
	}
	inline std::string &ArchiveFile::Block::read(std::string &buffer) {trace_scope
		_storage->read(buffer, size(), *this, *this);
		return buffer;
	}
	/** Assuming the _storage and _location are set, the _flags and _size are read from the header.
		@throw posix::err::EILSEQ_ErrNo if _flags on disk are no in the ranges of 00-07 and 7F-FF
		@return	true if we were able to read the header
	*/
	inline bool ArchiveFile::Block::_readHeader() {trace_scope
		const uint8_t	kFlagsFreeBlockFullHeader= 0x7F;
		const uint8_t	kAllocatedBit= 0x80;
		const int64_t	kFlagsSize= sizeof(uint8_t);
		const int64_t	kMaxMiniFreeSize= sizeof(int64_t) - 1;
		const int64_t	kHeaderSize= kFlagsSize + sizeof(int64_t);

		if( (NULL == _storage) || (0 == _location) ) { // @todo Test
			return false;
		}
		_flags= _storage->read<uint8_t>(BigEndian, _location, FromStart);
		if( ( (_flags & kAllocatedBit) == kAllocatedBit )
				|| (_flags == kFlagsFreeBlockFullHeader) ) { // @todo Test
			_size= kHeaderSize + _storage->read<int64_t>(BigEndian);
		} else if(_flags > kMaxMiniFreeSize) {
			ErrnoCodeThrow(EILSEQ, "File Block is corrupt");
		} else {
			_size= kFlagsSize + _flags;
		}
		return true;
	}
	/** Writes the _flags and _size for a free block.
		If _size is not big enough to hold flags and size, then _size is stored in the _flags
	*/
	inline void ArchiveFile::Block::_writeFreeHeader() {trace_scope
		const int64_t	kFlagsSize= sizeof(uint8_t);
		const int64_t	kHeaderSize= kFlagsSize + sizeof(int64_t);
		const uint8_t	kFlagsFreeBlockFullHeader= 0x7F;

		if(_size < kHeaderSize) {
			_flags= _size - kFlagsSize;
			_writeHeaderFlags();
		} else {
			_flags= kFlagsFreeBlockFullHeader;
			_writeHeader();
		}
	}
	inline void ArchiveFile::Block::_writeHeaderFlags() {trace_scope
		_storage->write<uint8_t>(_flags, BigEndian, _location, FromStart);
	}
	inline void ArchiveFile::Block::_writeHeader() {trace_scope
		const int64_t	kHeaderSize= sizeof(uint8_t) + sizeof(int64_t);

		_writeHeaderFlags();
		_storage->write<int64_t>(_size - kHeaderSize, BigEndian);
	}
	/** Assumes the block is free, and writes to disk the header for the given size and flags.
			Also marks any trailing data free.
		@param payloadSize	The size of data to carve out of this block
		@param userFlags	The 7-bit value the user wants associated with this block
	*/
	inline void ArchiveFile::Block::_allocate(int64_t payloadSize, uint8_t userFlags) {trace_scope
		const uint8_t	kAllocatedBit= 0x80;
		const int64_t	kFlagsSize= sizeof(uint8_t);
		const int64_t	kHeaderSize= kFlagsSize + sizeof(int64_t);
		const int64_t	oldSize= _size;

		_size= kHeaderSize + payloadSize;
		if(oldSize != _size) { // @todo Test
			Block	freeTailBlock;

			freeTailBlock._storage= _storage;
			freeTailBlock._location= _location + _size;
			freeTailBlock._size= oldSize - _size;
			freeTailBlock._writeFreeHeader();
		}
		_flags= kAllocatedBit | userFlags;
		_writeHeader();
		_storage->flush();
	}
	inline ArchiveFile::ArchiveFile(const char *path, Protection protection, uint16_t version, const std::string &signature)
			:File(path, File::Binary, protection), _headerSize(0) {trace_scope
		_init(version, signature);
	}
	inline ArchiveFile::~ArchiveFile() {trace_scope}
	/// @todo Test
	inline ArchiveFile::ArchiveFile(const std::string &path, Protection protection, uint16_t version, const std::string &signature)
			:File(path, File::Binary, protection), _headerSize(0) {trace_scope
		_init(version, signature);
	}
	/** Walks the blocks, starting the the first block, looking for a free block big enough to hold
			the requested data size.
		NOTE: All free blocks are Block::merge()d
	*/
	inline ArchiveFile::Block ArchiveFile::allocate(int64_t dataSize, uint8_t flags) {trace_scope
		for(Block b= begin(); b != end(); ++b) {
			if(b.free()) {
				b.merge();
				if( (dataSize <= b.size()) && b.allocate(dataSize, flags) ) { // @todo Test
					return b;
				}
			}
		}
		return Block();
	}
	inline ArchiveFile::Block ArchiveFile::allocate(const std::string &data, uint8_t flags) {trace_scope
		Block	block= allocate(data.size(), flags);

		write(data, block, block);
		return block;
	}
	/// @todo Test
	inline ArchiveFile::Block ArchiveFile::lookup(int64_t identifier) {trace_scope
		return Block(identifier, *this);
	}
	inline ArchiveFile::Block ArchiveFile::begin() {trace_scope
		return Block(_headerSize, *this);
	}
	inline ArchiveFile::Block ArchiveFile::end() {trace_scope
		return Block();
	}
	/** If the file doesn't exist or it is zero length, it is created and the header is written.
			If the file exists, the header is read and verified.
		@throw posix::err::ERANGE_ErrNo if file is not empty but too small for the header
		@throw posix::err::EILSEQ_ErrNo Signature or version do not match
	*/
	inline void ArchiveFile::_init(uint16_t version, const std::string &signature) {trace_scope
		const uint8_t	kFlagsFreeBlockFullHeader= 0x7F;
		const int64_t	kSignatureSize= signature.size();
		const int64_t	kFileSizeMax= INT64_MAX;
		const int64_t	kBlockHeaderSize= sizeof(uint8_t) + sizeof(int64_t);
		const int64_t	kHeaderSize= kSignatureSize + sizeof(uint16_t);
		Block	block;

		if(size() == 0) {
			write(signature, 0, FromStart);
			write<uint16_t>(version, BigEndian);
			write<uint8_t>(kFlagsFreeBlockFullHeader, BigEndian);
			write<int64_t>(kFileSizeMax - location() - sizeof(int64_t), BigEndian);
			flush();
		} else if(size() < kHeaderSize + kBlockHeaderSize) {
			ErrnoCodeThrow(ERANGE, "File is too small for header");
		} else {
			std::string	readHeader;
			uint16_t	readVersion;

			read(readHeader, kSignatureSize, 0, FromStart);
			if(readHeader != signature) {
				ErrnoCodeThrow(EILSEQ, "File Header is corrupt");
			}
			readVersion= read<uint16_t>(BigEndian);
			if(readVersion != version) {
				ErrnoCodeThrow(EILSEQ, "File Unknown Version");
			}
			block= Block(location(), *this); // verify we can read the first block
		}
		_headerSize= kHeaderSize;
	}
}

#endif // _ArchiveFile_h_
