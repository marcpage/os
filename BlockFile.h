#include "os/File.h"
#include "os/Hash.h"
#include "os/Exception.h"
#include "os/ZCompression.h"
#include <string>
#include <stdint.h>
#include <map>
#include <utility>

/**
File Header
-----------
 sig	10 bytes	"\x89""fsBF\x0D\x0A\x1A\x0A\x00"
size	 8 bytes	Total size of data on disk (including headers)
count	 8 bytes	Total number of allocated blocks on disk
first	 2 bytes	size of header extra (add 12)
extra	 [first]	reserved for future use
	Block			first block

Block (19 + n bytes)
-----
type	 1 byte
size	 2 bytes	not valid for block type 2, for block type 0 size includes md5
 md5	16 bytes	only valid for block type 1 and 3
data	  [size]	only valid for block type 1 and 3

Block Types
-----------
0 = Free Byte, skip
1 = Allocated not compressed
2 = Free Block
3 = Allocated zlib compressed
all others are reserved for future use

	@todo Test
	@todo add instrumentation
	@todo add to README
*/
class BlockFile {
	public:
		typedef uint64_t	DiskSize;	///< The size of something on disk
		typedef uint16_t	Size;		///< The size of a block
		typedef uint64_t	Tag;		///< The identifier given when data is stored
		/// Open or create a BlockFile at the given path.
		BlockFile(const std::string &path, bool readonly= false);
		/// Close the BlockFile.
		~BlockFile();
		/// Store data in the BlockFile and get a reference to retrieve it later.
		Tag allocate(const std::string &data, bool canCompress= true);
		/// Retrieve the data associated with a Tag.
		std::string &read(Tag location, std::string &data);
		/// Recycle the space used by this Tag
		void deallocate(Tag location);
		/// Determine if the data for the Tag was compressed when written to disk
		bool compressed(Tag location);
		/// Get the size of the data on disk for this tag (includes headers)
		Size sizeOnDisk(Tag location);
		/// Size of the file on disk
		off_t size();
	private:
		/// Types of blocks in the file
		enum BlockType {
			FreeByte,				///< A single free byte (0)
			Allocated,				///< An allocated block
			FreeBlock,				///< A free block
			AllocatedCompressed,	///< An allocated block with compressed data
			BlockTypeCount			///< The number of block types
		};
		typedef std::map<Tag,Size>					FreeList;	///< offset -> free size map
		typedef hash::SpecificHash<hash::MD5Hasher>	Hash;		///< Hasher used to validate blocks
		/// The size of a FreeByte type block
		static const Size	kHeaderMinSize= sizeof(uint8_t);
		/// The size of a FreeBlock type header
		static const Size	kFreeHeaderSize= kHeaderMinSize + sizeof(Size);
		/// The size of an allocated block header
		static const Size	kBlockHeaderSize= kFreeHeaderSize + Hash::Size;
		/// The largest size of the payload for a block
		static const Size	kMaxDataSize= static_cast<Size>((1L << sizeof(Size)) - kBlockHeaderSize - 1);
		io::File	_file;		///< The disk file we are working with
		FreeList	_free;		///< The cache of free blocks
		Tag			_first;		///< The identifier (offset) of the first block
		DiskSize	_usedSize;	///< The total size of all allocated blocks (including headers) on disk
		DiskSize	_count;		///< The number of allocated blocks on disk
		Tag			_freeScan;	///< The place to start scanning for free blocks
		/// Gets a free block at least as big as requested and gets the size of the free block
		Tag _findFreeBlock(Size &size);
		/// Gets free blocks from the cache
		Tag _findCachedFreeBlock(Size &size);
		/// Reads the entire block header for a Tag
		Tag _readBlockHeader(Tag location, bool &free, bool &compressed, Size &size, Hash hash, off_t &data);
		/// Walks the blocks on disk looking for a free block
		Tag _scanFreeBlock(Size &size);
		/// Marks block(s) on disk as free
		void _writeFree(Tag location, Size size);
		void _updateFileHeaderAndFlush();
		static std::string _signature() {
			static const char	kSignature[]= {0x89, 'f', 's', 'B', 'F', 0x0D, 0x0A, 0x1A, 0x0A, 0x00};
			return std::string(kSignature, sizeof(kSignature));
		}
};

/** Opens or creates a BlockFile.
	If the file cannot be opened for write, it will be opened readonly.
	@param path		The path to the file
	@param readonly	Force the file to be readonly, if false the file may still be opened read only
						if permissions do not allow writing.
	@throw msg::MessageException	If the signature of the file is not correct or the first block
										offset in the header goes off the end of the file.
*/
inline BlockFile::BlockFile(const std::string &path, bool readonly)
	:_file(path, io::File::Binary, readonly ? io::File::ReadOnly : io::File::WriteIfPossible), _free(), _first(0), _usedSize(0), _count(0), _freeScan(0) {
	if(_file.size() == 0) {
		_file.write(_signature());
		_file.write<DiskSize>(0, io::File::BigEndian); // start out with no allocated data
		_file.write<DiskSize>(0, io::File::BigEndian); // start out with no data on disk
		_file.write<DiskSize>(0, io::File::BigEndian); // start out with no free
		_file.write<uint16_t>(0, io::File::BigEndian); // for now we don't put anything else in the header
		_first= static_cast<Tag>(_file.location());
		_usedSize= 0;
		_count= 0;
	} else {
		std::string			data;
		std::string			expectedSignature(_signature());

		_file.read(data, expectedSignature.size());
		AssertMessageException(data == expectedSignature);
		_usedSize= _file.read<DiskSize>(io::File::BigEndian);
		_count= _file.read<DiskSize>(io::File::BigEndian);
		_first= static_cast<Tag>(_file.location() + _file.read<uint16_t>(io::File::BigEndian));
		AssertMessageException(_first <= static_cast<Tag>(_file.size()));
	}
	_freeScan= _first;
}
/** Destructor does nothing explicitly.
*/
inline BlockFile::~BlockFile() {}
/** Writes a block of data to the BlockFile and returns a tag to retrieve it later.
	@param data						The data to store
	@param canCompress				If true, the data may be compressed (if compression does not save anything
										it will not be compressed).
									If false, the data will not be compressed.
	@return							The identifier to retrieve the data later.
	@throw msg::MessageException	If the file is corrupted or data.size() is too big (see kMaxDataSize).
*/
inline BlockFile::Tag BlockFile::allocate(const std::string &data, bool canCompress) {
	std::string			compressedValue;
	const std::string	*towrite= &data;
	Tag					location= 0;
	Size				blockSize;
	uint8_t				type;
	Hash				hash(data);

	AssertMessageException(data.size() <= kMaxDataSize);
	if(canCompress) {
		z::compress(data, compressedValue, 9);
		canCompress= (compressedValue.size() < data.size());
		if(canCompress) {
			towrite= &compressedValue;
		}
	}
	blockSize= towrite->size();
	location= _findFreeBlock(blockSize);
	type= static_cast<uint8_t>(canCompress ? AllocatedCompressed : Allocated);
	_file.write<uint8_t>(type, io::File::BigEndian, static_cast<off_t>(location), io::File::FromStart);
	_file.write<uint16_t>(towrite->size(), io::File::BigEndian);
	_file.write(hash.buffer(), Hash::Size);
	_file.write(*towrite);
	_usedSize+= kBlockHeaderSize + towrite->size();
	++_count;
	_updateFileHeaderAndFlush();
	return location;
}
/** Reads the data associated with a Tag.
	@param location					The Tag received when allocating the data.
	@param data						Receives the data for the associated Tag
	@return							Reference to data
	@throw msg::MessageException	If the Tag is a free block.
									If the file is corrupt (hash doesn't match, etc.)
*/
inline std::string &BlockFile::read(Tag location, std::string &data) {
	bool		free, isCompressed;
	Size		blockSize;
	Hash		hash, dataHash;
	off_t		offset;
	std::string	compressedData;
	std::string	*toRead= &data;

	_readBlockHeader(location, free, isCompressed, blockSize, hash, offset);
	AssertMessageException(!free);
	if(isCompressed) {
		toRead= &compressedData;
	}
	_file.read(data, blockSize, offset, io::File::FromStart);
	if(isCompressed) {
		z::uncompress(compressedData, data, kMaxDataSize);
	}
	dataHash.reset(data);
	AssertMessageException(hash == dataHash);
	return data;
}
/** Marks an allocated block as free.
	@param location					The location of an allocated block to free
	@throw msg::MessageException	If the block is not free
*/
inline void BlockFile::deallocate(BlockFile::Tag location) {
	bool	free, isCompressed;
	Size	blockSize;
	Hash	hash;
	off_t	data;

	_readBlockHeader(location, free, isCompressed, blockSize, hash, data);
	AssertMessageException(!free);
	_writeFree(location, kBlockHeaderSize + blockSize);
	_usedSize-= (kBlockHeaderSize + blockSize);
	--_count;
	_updateFileHeaderAndFlush();
	if( (_freeScan >= location) && (_freeScan < location + kBlockHeaderSize + blockSize) ) {
		_freeScan= _first; // We've deallocated the block we were on, so time to start at the beginning
	}
}
/** Gets the size of the entire block on disk, including headers.
	@param location	The Tag of the data block to query.
	@return			The number of bytes taken up on disk for this block
*/
BlockFile::Size BlockFile::sizeOnDisk(BlockFile::Tag location) {
	bool	free, isCompressed;
	Size	blockSize;
	Hash	hash;
	off_t	data;

	_readBlockHeader(location, free, isCompressed, blockSize, hash, data);
	AssertMessageException(!free);
	return kBlockHeaderSize + blockSize;
}
/** Determines if a block is compressed on disk.
	@param location	The location of the block to check
	@return			true if the block is compressed on disk,
					false if the block is not compressed
					(even if it was requested to be compressed)
	@todo have a headers cache so things like this don't have to read from disk
*/
inline bool BlockFile::compressed(BlockFile::Tag location) {
	bool	free, isCompressed;
	Size	blockSize;
	Hash	hash;
	off_t	data;

	_readBlockHeader(location, free, isCompressed, blockSize, hash, data);
	AssertMessageException(!free);
	return isCompressed;
}
inline off_t BlockFile::size() {
	return _file.size();
}
/** Finds a free block for a given payload data size.
	@param theSize	On call: The size of the payload that needs to be stored.
					On Return: The size of the entire block found
	@return			The location of the block that can be allocated
	@todo Keep track of how far we've scanned the file. Start off
			again where we left off. When we free a block, see
			if there are surrounding blocks in the cache that can be merged.
			Keep track of how many frees have been done and reset scan
			after certain number.
*/
inline BlockFile::Tag BlockFile::_findFreeBlock(BlockFile::Size &theSize) {
	Tag	found= _findCachedFreeBlock(theSize);

	if(!found) { // we don't have a cached block big enough
		found= _scanFreeBlock(theSize); // scan disk for a free block
	}
	return found;
}
/** Scans the BlockFile for a free block for a given payload data size.
	@param theSize	On call: The size of the payload that needs to be stored.
					On Return: The size of the entire block found
	@return		The location of the block that can be allocated
	@todo Keep track of how far we've scanned the file. Start off
			again where we left off.
*/
inline BlockFile::Tag BlockFile::_scanFreeBlock(BlockFile::Size &theSize) {
	Tag		next, found, freeSince= 0, freeSize= 0, current= _freeScan;
	bool	free, isCompressed;
	Size	blockSize;
	Hash	hash;
	off_t	data, max= _file.size();
	std::string	___debug___;

	found= max;
	while(current < found) { // walk through all blocks on disk until found or end reached
		next= _readBlockHeader(current, free, isCompressed, blockSize, hash, data);
		if(free) { // is it a free block
			if(0 == freeSince) { // last block was not a free block, so this begins the run
				freeSince= current;
			} else { // last block was free, let's see if we've cached this block as free
				FreeList::iterator	has= _free.find(freeSince);

				if(_free.end() != has) { // we've cached this free block, remove it
					_free.erase(has);
				}
			}
			freeSize= theSize + (current - freeSince); // size of previous free blocks added in
			if(static_cast<Tag>(kBlockHeaderSize + theSize) <= freeSize) {
				found= freeSince;
			}
		} else { // not a free block
			if(0 != freeSince) { // was the last block free
				if(_free.end() == _free.find(freeSince)) { // do we already have it cached
					_free[freeSince]= freeSize; // cache the free block
				}
				freeSince= 0; // this block is not a free block, end the free block run
				freeSize= 0;
			}
		}
		current= next; // go to the next block
	}
	if(static_cast<off_t>(found) == max) {
		freeSize= kBlockHeaderSize + theSize;
		_freeScan= _first;
	} else {
		_freeScan= found;
	}
	theSize= freeSize;
	return found;
}
/** Finds a free block for a given size.
	@param theSize	On call: The size of the payload that needs to be stored.
					On Return: The size of the entire block found
	@return		The location of the block that can be allocated
*/
inline BlockFile::Tag BlockFile::_findCachedFreeBlock(BlockFile::Size &theSize) {
	for(FreeList::iterator free= _free.begin(); free != _free.end(); ++free) {
		if(kBlockHeaderSize + theSize <= free->second) {
			Tag	found= free->first;

			theSize= free->second;
			_free.erase(free);
			return found;
		}
	}
	return Tag(0);
}
/** Reads the block header at the given location.
	@param location					The location of the block
	@param free	`					Receives if the block is a free block
	@param isCompressed				Receives if the block is compressed
	@param theSize					Receives the size of the data.
										For free blocks it's the entire size of the block including headers.
										For allocated blocks, is the size of the payload on disk
											(after compression for compressed blocks).
	@param hash						Receives the hash value for allocated blocks
	@param data						Receives the offset in the file of the payload data.
	@return							The next block after this one.
	@throw msg::MessageException	If the type at location is not a valid type or if the next block is past the end of the file.
*/
inline BlockFile::Tag BlockFile::_readBlockHeader(BlockFile::Tag location, bool &free, bool &isCompressed, Size &theSize, Hash hash, off_t &data) {
	BlockType	type= static_cast<BlockType>(_file.read<uint8_t>(io::File::BigEndian, static_cast<off_t>(location), io::File::FromStart));
	Tag			next= 0;

	AssertMessageException( (type >= static_cast<BlockType>(0)) && (type < BlockTypeCount) );
	free= true;
	data= 0;
	if(FreeByte == type) {
		isCompressed= true;
		theSize= kHeaderMinSize;
		return location + theSize;
	}
	theSize= _file.read<Size>(io::File::BigEndian);
	if(FreeBlock == type) {
		isCompressed= false;
		theSize= kFreeHeaderSize + theSize; // calculate the size of the entire block
		return location + theSize;
	}
	free= false;
	isCompressed= (AllocatedCompressed == type);
	data= location + kBlockHeaderSize;
	_file.read(hash.buffer(), Hash::Size);
	next= location + kBlockHeaderSize + theSize;
	AssertMessageException( static_cast<off_t>(next) <= _file.size() );
	return next;
}
/** Writes out a free block.
	@param location	The location to commit to being a free block
	@param theSize	The size of the entire block (including all headers)
	@todo Check the cache for adjacent or even overlapping blocks with this
			one and join them.
	@todo Keep track of how many frees have been done and reset scan offset after certain number.
*/
inline void BlockFile::_writeFree(BlockFile::Tag location, BlockFile::Size theSize) {
	_file.moveto(static_cast<off_t>(location));
	if(theSize < kFreeHeaderSize) { // too small to write a full header, write FreeByte types
		for(Size block= 0; block < theSize; ++block) {
			_file.write<uint8_t>(static_cast<uint8_t>(FreeByte), io::File::BigEndian);
		}
	} else {
		_file.write<uint8_t>(static_cast<uint8_t>(FreeBlock), io::File::BigEndian);
		_file.write<Size>(theSize - kFreeHeaderSize, io::File::BigEndian);
	}
	_free[location]= theSize;
}
/** Updates any stats that need to be stored in the file header and flushes the file.
	This is very dependenant on the structure of the header of the file.
*/
inline void BlockFile::_updateFileHeaderAndFlush() {
	static off_t	kSignatureSize= 0;

	if(0 == kSignatureSize) {
		kSignatureSize= static_cast<off_t>(_signature().size());
	}
	_file.write<DiskSize>(_usedSize, io::File::BigEndian, kSignatureSize, io::File::FromStart);
	_file.write<DiskSize>(_count, io::File::BigEndian, kSignatureSize, io::File::FromStart);
	_file.flush();
}

