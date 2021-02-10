#include "compress.h"

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <bitset>

#include <zlib.h>

BitString::BitString(int bits): mString(0), mEncString(0)
{
	setBits(bits);
}

BitString::BitString(const BitString &bs): mString(0), mEncString(0)
{
	setBits(bs.mBits);
	mOnes = bs.mOnes;
	mDist = bs.mDist;
	mAcsbsBits = bs.mAcsbsBits;
	mAcsbsEncBits = bs.mAcsbsEncBits;
	mRiceBits = bs.mRiceBits;
	mRiceEncBits = bs.mRiceEncBits;
	mEncBits = bs.mEncBits;
	memcpy((unsigned char*)mString, (const unsigned char*)bs.mString, 8*mWords);
	memcpy((unsigned char*)mEncString, (const unsigned char*)bs.mEncString, 16*mWords);
}

BitString::~BitString()
{
	if (mString) delete [] mString;
	if (mEncString) delete [] mEncString;
}

void
BitString::setBits(int bits)
{
	if (mString) delete [] mString;
	if (mEncString) delete [] mEncString;

	mBits = bits;
	mOnes = 0;
	mWords = (bits + 63) / 64;
	mString = new Word64[mWords];
	mEncBits = 0;
	mEncString = new Word64[2*mWords];

	memset(mString, 0, mWords*8);
}

void
BitString::random(int k, bool increase)
{
	if (increase) {
		if (mBits < mOnes + k) k = mBits - mOnes;
	} else {
		if (mBits < k) k = mBits;
	}
	
	if (increase) {
		mOnes += k;
	} else {
		mOnes = k;
	}
	
	mDist.reserve(mOnes + 1);
	
	// Set random bits in string.
	while (0 < k) {
		int i = rand() % mBits;
		if (getBit(i)) continue;
		setBit(i);
		k--;
	}
}

void
BitString::findDist()
{
	mDist.clear();
	
	int d = 0;
	// Search ones and count distance.
	for (int i = 0; i < mBits; i++) {
		if (getBit(i)) {
			mDist.push_back(d);
			d = 0;
		} else {
			d++;
		}
	}
	// The last 'virtual' one.
	mDist.push_back(d);
	
	// Find optimal word bits for AC-SBS and Rice encodings.
	findAcsbsWordBits();
	findRiceWordBits();
}

void
BitString::setZlibDistEnc()
{
	unsigned long size = compressBound((mBits + 7) / 8);
	compress((unsigned char *)mEncString, &size, (const unsigned char *)mString, (mBits + 7) / 8);
	mEncBits = 8*size;
}

void
BitString::setAcsbsDistEnc()
{
	memset(mEncString, 0, 8*((mAcsbsEncBits + 63) / 64));
	mEncBits = 0;

	Word64 m = 0xFFFFFFFFFFFFFFFF >> (64 - mAcsbsBits);

	for (int i = 0; i < mOnes + 1; i++) {
		int d = mDist[i];
		while (-1 < d) {
			Word64 cw = ((Word64)d < m) ? d : m;
			mEncString[mEncBits / 64] |= cw << (mEncBits % 64);
			if (64 - (mEncBits % 64) < mAcsbsBits) {
				mEncBits += mAcsbsBits;
				mEncString[mEncBits / 64] |= cw >> (mAcsbsBits - mEncBits % 64);
			} else {
				mEncBits += mAcsbsBits;
			}
			d -= m;
		}
	}
}

static std::pair<Word64, int>
intToRiceEncoding(int dist, int riceBits)
{
	std::pair<Word64, int> pr;
	Word64 word = 0x7FFFFFFFFFFFFFFF;
	
	pr.second = riceBits + 1 + (dist >> riceBits);
	word >>= (63 - (dist >> riceBits));
	word |= ((Word64)dist & ((1 << riceBits) - 1)) << ((dist >> riceBits) + 1);
	pr.first = word;
	
	return pr;
}

void
BitString::setRiceDistEnc()
{
	memset(mEncString, 0, 8*((mRiceEncBits + 63) / 64));
	mEncBits = 0;
	
	for (int i = 0; i < mOnes + 1; i++) {
		std::pair<Word64, int> pr = intToRiceEncoding(mDist[i], mRiceBits);
		mEncString[mEncBits / 64] |= pr.first << (mEncBits % 64);
		if (64 - (mEncBits % 64) < pr.second) {
			mEncBits += pr.second;
			mEncString[mEncBits / 64] |= pr.first >> (pr.second - mEncBits % 64);
		} else {
			mEncBits += pr.second;
		}
	}
}

void
BitString::getZlibDistEnc(std::vector<int> &dist) const
{
	unsigned long size = 8*mWords;
	Word64 *buf = new Word64[mWords];
	uncompress((unsigned char *)buf, &size, (const unsigned char *)mEncString, mEncBits / 8);
	
	dist.reserve(mOnes + 1);
	dist.clear();
	dist.push_back(0);
	
	int i = 0;
	int d = 0;
	for (int bit = 0; bit < 8*(int)size; bit++) {
		if ((buf[bit / 64] >> (bit % 64)) & 1) {
			dist[i++] = d;
			dist.push_back(0);
			d = 0;
		} else {
			d += 1;
		}
	}
	
	dist.pop_back();
	delete [] buf;
}

void
BitString::getAcsbsDistEnc(std::vector<int> &dist) const
{
	int m = 0xFFFFFFFF >> (32 - mAcsbsBits);
	const Word32 *enc = (const Word32*)mEncString;

	dist.reserve(mOnes + 1);
	dist.clear();
	dist.push_back(0);

	int i = 0;
	for (int bit = 0; bit < mEncBits; bit += mAcsbsBits) {
		int d = (*(const Word64*)(enc + (bit / 32)) >> (bit % 32)) & m;
		dist[i] += d;
		if (d != m) {
			dist.push_back(0);
			i += 1 ;
		}
	}
	
	dist.pop_back();
}

void
BitString::getRiceDistEnc(std::vector<int> &dist) const
{
	int m = 1 << mRiceBits;
	const Word32 *enc = (const Word32*)mEncString;

	dist.reserve(mOnes + 1);
	dist.clear();
	dist.push_back(0);

	int i = 0;
	int bit = 0;
	while (bit < mEncBits) {
		Word64 word = (*(const Word64*)(enc + (bit / 32)) >> (bit % 32));
		// Count ones.
		while (word & 1) {
			dist[i] += m;
			word >>= 1;
			bit += 1;
		}
		// Skip zero.
		word >>= 1;
		bit += 1;
		// Decode remainder.
		dist[i] += word & (m - 1);
		bit += mRiceBits;
		// Set next distance.
		dist.push_back(0);
		i += 1;
	}
	
	dist.pop_back();
}

void
BitString::clear()
{
	memset(mString, 0, mWords*8);
	mDist.clear();
}

void
BitString::setBit(int bit)
{
	mString[bit / 64] |= (Word64)1 << (bit % 64);
}

int
BitString::getBit(int bit) const
{
	return (mString[bit / 64] >> (bit % 64)) & 1;
}

void
BitString::print(const char *begin, const char *end) const
{
	std::cout << begin;
	
	for (int i = 0; i < mBits; i++) {
		std::cout << getBit(i);
	}
	
	std::cout << end;
}

void
BitString::printEnc(const char *begin, const char *end) const
{
	std::cout << begin;
	
	for (int i = 0; i < mEncBits; i++) {
		std::cout << getEncBit(i);
	}
	
	std::cout << end;
}

void
BitString::printDist(const char *begin, const char *end) const
{
	BitString::printDist(mDist, begin, end);
}

void
BitString::printInfo(const char *begin, const char *end) const
{
	std::cout << begin;
	std::cout << "[bits: " << mBits;
	std::cout << ", ones: " << mOnes;
	std::cout << ", acsbs-bits: " << mAcsbsBits;
	std::cout << ", rice-bits: " << mRiceBits;
	std::cout << "]" << end;
}

void
BitString::printDist(const std::vector<int> &dist, const char *begin, const char *end)
{
	std::cout << begin;

	for (int i = 0; i < (int)dist.size() - 1; i++) {
		std::cout << dist[i] << ":";
	}
	
	std::cout << dist[dist.size() - 1] << end;
}

void
BitString::findAcsbsWordBits()
{
	mAcsbsBits = 1;
	mAcsbsEncBits = mBits + 1;
	
	// Fixed coode word compression size.
	for (int w = 1; w < WORD_BITS_MAX; w++) {
		int m = (1 << w) - 1;
		int bits = 0;
		for (int k = 0; k < mOnes + 1; k++) {
			bits += w*((mDist[k] / m) + 1);
		}
		// Check for optimal code word bits.
		if (bits < mAcsbsEncBits) {
			mAcsbsBits = w;
			mAcsbsEncBits = bits;
		}
	}
}

void
BitString::findRiceWordBits()
{
	mRiceBits = 1;
	mRiceEncBits = mBits + 1;

	// Rice-Golomb compression size.
	for (int w = 0; w < WORD_BITS_MAX; w++) {
		int m = 1 << w;
		int bits = 0;
		for (int k = 0; k < mOnes + 1; k++) {
			bits += (mDist[k] / m) + 1 + w;
		}
		// Check for optimal code word bits.
		if (bits < mRiceEncBits) {
			mRiceBits = w;
			mRiceEncBits = bits;
		}
	}
}

int
BitString::getEncBit(int bit) const
{
	return (mEncString[bit / 64] >> (bit % 64)) & 1;
}
