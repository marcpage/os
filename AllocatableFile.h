#ifndef __AllocatableFile_f__
#define __AllocatableFile_f__

#include "Exception.h"
#include "File.h"

namespace io {

	/**
		@todo When allocating at end, we should allocate actual disk space, maybe a write?
	*/
	class AllocatableFile : public File {
		public:
			AllocatableFile(const std::string &path, Protection protection= WriteIfPossible);
			~AllocatableFile();
			off_t allocate(uint64_t bytes, uint8_t flags= 0);
			uint64_t size(off_t offset) const;
			void free(off_t block);
			int8_t flags(off_t block) const;
			AllocatableFile &flags(off_t block, int8_t flags);
		private:
			class _Block {
				public:
					_Block();
					_Block(File *file, off_t offset= 0, bool headerOffset= false);
					_Block(const File *file, off_t offset= 0, bool headerOffset= false);
					_Block(const _Block &other);
					~_Block();
					operator off_t() const;
					_Block &operator=(const _Block &other);
					off_t offset() const;
					bool isFree() const;
					/// Either free an allocated block, or join consecutive free blocks.
					void free();
					/// Size of the allocatable data, not including header
					uint64_t size() const;
					_Block &operator++();
					_Block operator++(int);
					_Block &operator+=(uint32_t count);
					_Block operator+(uint32_t count);
					/// @returns The free block after this block, or end() for perfect fit
					_Block use(uint64_t size, uint8_t flags);
					/// Only returns user accessible bits
					uint8_t flags() const;
					/// Ignores non-user accessible bits
					_Block &flags(uint8_t f);
					bool end() const;
				private:
					File				*_file;
					bool				_free;
					bool				_const;
					off_t				_header;
					uint64_t			_size;	//< Size of block including header
					uint64_t			_flags;
					void _readHeader();
					uint8_t _writeFreeBlock(off_t offset, uint64_t size);
			};
	};

	inline AllocatableFile::AllocatableFile(const std::string &path, Protection protection)
		:File(path, Binary, protection) {
	}
	inline AllocatableFile::~AllocatableFile() {}
	inline off_t AllocatableFile::allocate(uint64_t bytes, uint8_t flags) {
		_Block	block(this);

		while( !block.end() && (block.size() < bytes) ) {
			++block;
		}
		block.use(bytes, flags);
		return block.offset();
	}
	inline uint64_t AllocatableFile::size(off_t offset) const {
		return _Block(this, offset).size();
	}
	inline void AllocatableFile::free(off_t block) {
		_Block(this, block).free();
	}
	inline int8_t AllocatableFile::flags(off_t block) const {
		return _Block(this, block).flags();
	}
	inline AllocatableFile &AllocatableFile::flags(off_t block, int8_t flags) {
		_Block(this, block).flags(flags);
		return *this;
	}
	inline AllocatableFile::_Block::_Block()
			:_file(NULL), _free(true), _const(true), _header(0), _size(0), _flags(0) {}
	/**
		@param file			The file we are referencing
		@param offset		The offset on the file of the block, see headerOffset
		@param headerOffset	Is offset the start of the block?
								This is *always* true for free blocks.
	*/
	inline AllocatableFile::_Block::_Block(File *file, off_t offset, bool headerOffset)
			:_file(file), _free(headerOffset), _const(false),
			_header( ((0 == offset) || headerOffset)
					? offset
					: static_cast<off_t>(offset - sizeof(uint8_t) - sizeof(uint64_t))),
			_size(0), _flags(0) {
		_readHeader();
	}
	inline AllocatableFile::_Block::_Block(const File *file, off_t offset, bool headerOffset)
			:_file(const_cast<File*>(file)), _free(headerOffset), _const(true),
			_header( ((0 == offset) || headerOffset)
					? offset
					: static_cast<off_t>(offset - sizeof(uint8_t) - sizeof(uint64_t))),
			_size(0), _flags(0) {
		_readHeader();
	}
	inline AllocatableFile::_Block::_Block(const _Block &other)
			:_file(other._file), _free(other._free), _const(other._const),
			_header(other._header), _size(other._size), _flags(other._flags) {}
	inline AllocatableFile::_Block::~_Block() {}
	inline AllocatableFile::_Block::operator off_t() const {
		return offset();
	}
	inline AllocatableFile::_Block &AllocatableFile::_Block::operator=(const _Block &other) {
		_file= other._file;
		_free= other._free;
		_const= other._const;
		_header= other._header;
		_size= other._size;
		_flags= other._flags;
		return *this;
	}
	inline off_t AllocatableFile::_Block::offset() const {
		const size_t	kHeaderSize= sizeof(uint8_t) + sizeof(uint64_t);

		return _free ? _header : static_cast<off_t>(_header + kHeaderSize);
	}
	inline bool AllocatableFile::_Block::isFree() const {
		return _free;
	}
	inline void AllocatableFile::_Block::free() {
		_Block			future= (*this) + 1U;
		uint64_t		futureFreeBlockSize= 0;

		AssertMessageException(!_const);
		AssertMessageException(NULL != _file);
		// join any future free blocks into this one
		while(future.isFree() && !future.end()) {
			futureFreeBlockSize+= future._size;
		}
		if(!_free || (futureFreeBlockSize > 0) ) {
			_free= true;
			_size+= futureFreeBlockSize;
			_flags= _writeFreeBlock(_header, _size);
		}
	}
	inline uint64_t AllocatableFile::_Block::size() const {
		const size_t	kHeaderSize= sizeof(uint8_t) + sizeof(uint64_t);

		return (_size <= kHeaderSize) ? 0 : (_size - kHeaderSize);
	}
	inline AllocatableFile::_Block &AllocatableFile::_Block::operator++() {
		_header+= _size;
		_size= 0;
		if(!end()) {
			_readHeader();
		}
		// compact future blocks
		if(isFree()) {
			free();
		}
		return *this;
	}
	inline AllocatableFile::_Block AllocatableFile::_Block::operator++(int) {
		_Block	now(*this);

		++(*this);
		return now;
	}
	inline AllocatableFile::_Block &AllocatableFile::_Block::operator+=(uint32_t count) {
		for(uint32_t iteration= 0; !end() && (iteration < count); ++iteration) {
			++(*this);
		}
		return *this;
	}
	inline AllocatableFile::_Block AllocatableFile::_Block::operator+(uint32_t count) {
		return _Block(*this)+=count;
	}
	inline AllocatableFile::_Block AllocatableFile::_Block::use(uint64_t size, uint8_t flags) {
		const size_t	kHeaderSize= sizeof(uint8_t) + sizeof(uint64_t);
		const uint8_t	kFreeBit= 0x80;
		const bool		isEnd= end();
		_Block			tail;

		AssertMessageException(!_const);
		AssertMessageException( (flags & kFreeBit) != kFreeBit );
		AssertMessageException(end() || (kHeaderSize + size <= _size));
		AssertMessageException(NULL != _file);
		_file->write<uint8_t>(flags, BigEndian, _header, FromStart);
		_file->write<uint64_t>(size, BigEndian);
		if(kHeaderSize + size < _size) {
			_writeFreeBlock(_header + _size, _size - size);
			tail= (*this) + 1U;
			tail.free(); // merge with any future free blocks
		}
		_size= size;
		_flags= flags;
		_free= false;
		if(isEnd) {
			_writeFreeBlock(_header + _size, 1);
		}
		return tail;
	}
	inline uint8_t AllocatableFile::_Block::flags() const {
		const uint8_t	kFreeBit= 0x80;

		AssertMessageException(_free || ((kFreeBit & _flags) != kFreeBit));
		return _free ? 0 : _flags;
	}
	inline AllocatableFile::_Block &AllocatableFile::_Block::flags(uint8_t f) {
		const uint8_t	kFreeBit= 0x80;

		AssertMessageException(!_const);
		AssertMessageException(!_free);
		AssertMessageException( (f & kFreeBit) != kFreeBit );
		return *this;
	}
	inline bool AllocatableFile::_Block::end() const {
		if(NULL == _file) {
			return true;
		}
		const off_t	fileSize= _file->size();

		return (static_cast<off_t>(_header + _size) >= fileSize);
	}
	inline void AllocatableFile::_Block::_readHeader() {
		const size_t	kHeaderSize= sizeof(uint8_t) + sizeof(uint64_t);
		const uint8_t	kFreeBit= 0x80;

		AssertMessageException(NULL != _file);
		if(end()) {
			_flags= kFreeBit;
			_free= true;
			_size= 0;
		} else {
			_flags= _file->read<uint8_t>(BigEndian, _header, FromStart);
			_free= (_flags & kFreeBit) == kFreeBit;
			_size= (_free && (_flags != kFreeBit)) // _flags == kFreeBit is free & read size
						? (_flags & ~kFreeBit) // clear free bit will give you free size
						: _file->read<uint64_t>(BigEndian) + kHeaderSize;
		}
	}
	/**
		@param offset	The offset at which to write the free block header
		@param size		The size of the entire free block
	*/
	inline uint8_t AllocatableFile::_Block::_writeFreeBlock(off_t offset, uint64_t size) {
		const size_t	kHeaderSize= sizeof(uint8_t) + sizeof(uint64_t);
		const uint8_t	kMaxCompactFreeBlockSize= 0x7F;
		const uint8_t	kFreeBit= 0x80;
		const bool 		fitsInCompactFreeBlock= (size <= kMaxCompactFreeBlockSize);
		const uint8_t	flags= kFreeBit | static_cast<uint8_t>(fitsInCompactFreeBlock ? size : 0);

		AssertMessageException(size > 0);
		AssertMessageException(!_const);
		_file->write<uint8_t>(flags, BigEndian, offset, FromStart);
		if(!fitsInCompactFreeBlock) {
			_file->write<uint64_t>(size - kHeaderSize, BigEndian);
		}
		return flags;
	}

}

#endif // __AllocatableFile_f__
