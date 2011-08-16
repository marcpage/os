#include "../tracer.h"
#include "../TypeSafeEnum.h"

// g++ -o /tmp/test TypeSafeEnum_test.cpp -I.. -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -Wno-non-template-friend
// /tmp/test | grep TypeSafeEnum.h | sort | uniq | wc -l
// max = 133

enum Bits {
	Bit0,
	Bit1,
	Bit2,
	Bit3,
	Bit4,
	Bit5,
	Bit6,
	Bit7,
	Bit8
};

typedef TypeSafeEnum<Bits, Bit8> SafeBits;

int main(int,const char*const[]) {
	SafeBits	bits1, bits2(Bit2);
	int			returnValue= 0;
	
	bits1= bits2;
	bits1.set(Bit0).set(Bit1).set(Bit3).set(Bit6);
	const int	expected= 1 * 1 | 1 * 2 | 1 * 4 | 1 * 8 | 0 * 16 | 0 * 32 | 1 * 64 | 0 * 128 | 0 * 256;
	if(bits1.get<int>() != expected) {
		printf("FAIL: %s:%d =%x vs %x\n", __FILE__, __LINE__, bits1.get<int>(), expected);
	}
	if(bits1.empty()) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits2.empty()) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(SafeBits().set(Bit5)^=Bit5).empty()) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (bits1 | bits2) != bits1 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (bits1 ^ bits2) == bits1 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( ((bits1 ^ bits2) & bits2) != SafeBits() ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (SafeBits().set(Bit2).set(Bit3)&=Bit2) != bits2 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (SafeBits()|=Bit2) != bits2 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( ((SafeBits().set(Bit3)^=Bit2)^=Bit3) != bits2 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( ~bits1 != SafeBits().set(Bit4).set(Bit5).set(Bit7).set(Bit8) ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits1 != SafeBits().set(expected)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits2 != Bit2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits1 < bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(bits1 > bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits1 <= bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(bits1 >= bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits1 == bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(bits1 != bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(Bit2 != bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(bits2 == Bit2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(Bit2 == bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits2 < Bit2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits2 < Bit1) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(bits2 < Bit3)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(Bit2 > bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(Bit1 > bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(Bit3 > bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(Bit2 < bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(Bit2 <= bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(Bit1 < bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(Bit3 < bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(Bit3 <= bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(bits2 >= Bit2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(bits2 >= Bit1)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(bits2 >= Bit3) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(Bit2 >= bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(Bit1 >= bits2) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(!(Bit3 >= bits2)) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (bits2&bits1) != bits2 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (bits1&Bit2) != bits2 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (Bit2&bits1) != bits2 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (SafeBits().set(Bit3)|Bit5) != SafeBits().set(Bit5).set(Bit3) ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (Bit5 | SafeBits().set(Bit3)) != SafeBits().set(Bit5).set(Bit3) ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (Bit5 ^ SafeBits().set(Bit3)) != SafeBits().set(Bit5).set(Bit3) ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if( (Bit5 ^ SafeBits().set(Bit3).set(Bit5)) != Bit3 ) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	if(256 + 255 > bits1.max<int>()) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	try	{
		bits1.set(256 + 255);
	} catch(const std::exception &exception) {
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	}
	try	{
		bits1.set(256 + 255 + 1);
		printf("FAIL: %s:%d\n", __FILE__, __LINE__);
	} catch(const std::exception &exception) {
	}
	return returnValue;
}
