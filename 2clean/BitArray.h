#ifndef __BitArray_h__
#define __BitArray_h__

class BitArray {
	public:
		class ConstReference {
			public:
				ConstReference();
				ConstReference(const ConstReference &other);
				~ConstReference();
				ConstReference &operator=(const ConstReference &other);
				operator bool() const;
				ConstReference &operator++();
				ConstReference operator++(int);
				ConstReference &operator--();
				ConstReference operator--(int);
			protected:
				friend class BitArray;
				ConstReference(BitArray &array, size_t index);
			private:
				BitArray	*_array;
				size_t		_index;
		};
		class Reference : public ConstReference {
			public:
				Reference();
				Reference &operator=(bool value);
				~Reference();
			protected:
				friend class BitArray;
				Reference(BitArray &array, size_t index);
		};
		BitArray(size_t bits);
		BitArray(const BitArray &other);
		~BitArray();
		BitArray &operator=(const BitArray &other);
		Reference operator[](size_t index);
		ConstReference operator[](size_t index) const;
		size_t size() const;
	private:
		std::vector<uint8_t>	_data;
};

BitArray::ConstReference::ConstReference()
	:_array(NULL), _index(static_cast<size_t>(-1)) {}
BitArray::ConstReference::ConstReference(const ConstReference &other)
	:_array(other._array), _index(other._index) {}
BitArray::ConstReference::~ConstReference() {}
BitArray::ConstReference &BitArray::ConstReference::operator=(const ConstReference &other) {
	_array= other._array;
	_index= other._index;
}
BitArray::ConstReference::operator bool() const {
	return _array[_index/8] & (0x01 << (_index%8)) != 0;
}
BitArray::ConstReference &BitArray::ConstReference::operator++() {
	++_index;
	return *this;
}
BitArray::ConstReference BitArray::ConstReference::operator++(int) {
	ConstReference	r(*this);

	++_index;
	return r;
}
BitArray::ConstReference &BitArray::ConstReference::operator--() {
	--_index;
	return *this;
}
BitArray::ConstReference BitArray::ConstReference::operator--(int) {
	ConstReference	r(*this);

	--_index;
	return r;
}
BitArray::ConstReference(BitArray &array, size_t index)
	:_{
}
BitArray::Reference::Reference() {
}
BitArray::Reference &BitArray::Reference::operator=(bool value) {
}
BitArray::Reference::~Reference() {
}
BitArray::Reference(BitArray &array, size_t index) {
}
BitArray::BitArray(size_t bits) {
}
BitArray::BitArray(const BitArray &other) {
}
BitArray::~BitArray() {
}
BitArray &BitArray::operator=(const BitArray &other) {
}
Reference BitArray::operator[](size_t index) {
}
BitArray::ConstReference BitArray::operator[](size_t index) const {
}
size_t BitArray::size() const {
}

#endif // __BitArray_h__
