#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include <cstdint>
#include <vector>

//! 64-bit word.
typedef uint64_t Word64;
//! 32-bit word.
typedef uint32_t Word32;

#ifndef WORD_BITS_MAX
//! Maximum size of compression Word.
#define WORD_BITS_MAX 30
#endif

//! Class for binary string.
class BitString
{
public:
	//! Zero bitsring with given length.
	BitString(int bits = 10000);
	//! Bitstring copy constructor.
	BitString(const BitString &bs);
	//! Bitstring destructor.
	~BitString();

	//! Change sice of bitstring length.
	void setBits(int bits);

	//! Set k ones in string at random.
	void random(int k, bool increase = false);
	//! Determine distances between ones.
	void findDist();
	//! Compress using Lempel-Ziv (ZLIB DEFLATE).
	void setZlibDistEnc();
	//! Compress using AC-SBS.
	void setAcsbsDistEnc();
	//! Compress using Rice-Golomb.
	void setRiceDistEnc();

	//! Decompress using Lempel-Ziv (ZLIB DEFLATE).
	void getZlibDistEnc(std::vector<int> &dist) const;
	//! Decompress using AC-SBS.
	void getAcsbsDistEnc(std::vector<int> &dist) const;
	//! Decompress using Rice-Golomb.
	void getRiceDistEnc(std::vector<int> &dist) const;

	//! Set all bits to zero.
	void clear();
	//! Set given bit to one.
	void setBit(int bit);
	//! Get valu of given bit.
	int getBit(int bit) const;

	//! Print binary string.
	void print(const char *begin = "", const char *end = "\n") const;
	//! Print binary string current encoding (determined by last use of setXxxxxEnc()).
	void printEnc(const char *begin = "", const char *end = "\n") const;
	//! Print distances between ones.
	void printDist(const char *begin = "", const char *end = "\n") const;
	//! Print String info.
	void printInfo(const char *begin = "", const char *end = "\n") const;
	
	//! Print distances vector.
	static void printDist(const std::vector<int> &dist, const char *begin = "", 
		const char *end = "\n");

protected:
	//! Determine optimal AC-SBS word bits for the string.
	void findAcsbsWordBits();
	//! Determine optimal Rice-Golomb word bits for the string.
	void findRiceWordBits();
	//! Get encoding bits.
	int getEncBit(int bit) const;

private:
	//! Number of bits in string.
	int mBits;
	//! Number of ones in string.
	int mOnes;
	//! Number of 64-bit words in string.
	int mWords;
	//! String in packed form.
	Word64 *mString;
	
	//! Vector containing distances between ones.
	std::vector<int> mDist;
	//! Code word bit size for AC-SBS.
	int mAcsbsBits;
	//! Bit length of the AC-SBS encoding.
	int mAcsbsEncBits;
	//! Code word bit size for Rice-Golomb.
	int mRiceBits;
	//! Bit length of the Rice-Golomb encoding.
	int mRiceEncBits;
	//! Bit length of the last used encoding algorithm.
	int mEncBits;
	//! Encoding of the last used algorithm.
	Word64 *mEncString;
	
};

#endif // __COMPRESS_H__
