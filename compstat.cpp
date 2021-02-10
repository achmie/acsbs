#include "compstat.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <cstdlib>
#include <cmath>
#include <zlib.h>


BinSeqStat::BinSeqStat(int n, int k):
	mN(n),
	mK(k),
	mSeq(n, 0),
	mDist(k+1, 0),
	mGenSeq(0),
	mSumAcsbsCompressionBits(0),
	mSumAcsbsCompressionWords(0),
	mSumAcsbsCompressionCodeWordBits(0),
	mSumRiceGolombCodeCompressionBits(0),
	mSumRiceGolombCodeCompressionWords(0),
	mSumRiceGolombCodeCompressionCodeWordBits(0),
	mSumZLibDeflateCompressionBits(0)
{
	random();

	// Compute entropy.
	double entropy = 0;
	for (int i = 0; i < k; i++) {
		entropy += log2(n - i);
		entropy -= log2(i + 1);
	}
	mEntropy = entropy;
}

int
BinSeqStat::entropy() const
{
	return mEntropy;
}

void
BinSeqStat::random()
{
	int k = mK;

	// Clear sequence and distance vector.
	std::fill(mSeq.begin(), mSeq.end(), 0);
	std::fill(mDist.begin(), mDist.end(), 0);
	
	// Set random bits in string.
	while (0 < k) {
		int i = rand() % mN;
		if (mSeq[i]) continue;
		mSeq[i] = 1;
		k--;
	}

	// Pack random sequence.
	packSeq();

	// Find distatnces.
	int d = 0;
	k = 0;
	for (int i = 0; i < mN; i++) {
		if (mSeq[i]) {
			mDist[k++] = d;
			d = 0;
		} else {
			d++;
		}
	}
	mDist[k] = d; // Last 'virtual' distance (distance to the end of string).
}

void
BinSeqStat::findAcsbsStat()
{
	// AC-SBS compression size.
	for (int w = 1; w < WORD_BITS_MAX; w++) {
		int m = (1 << w) - 1;
		int bits = 0;
		int words = 0;
		for (int k = 0; k < mK+1; k++) {
			bits += w*((mDist[k] / m) + 1);
			words += (mDist[k] / m) + 1;
		}
		mAcsbsCompressionBitsByWordSize[w] = bits;
		mAcsbsCompressionWordsByWordSize[w] = words;
	}
	
	mAcsbsOptimalWordBits = 1;
	mAcsbsCompressionBits = mAcsbsCompressionBitsByWordSize[1];
	for (int w = 2; w < WORD_BITS_MAX; w++) {
		int bits = mAcsbsCompressionBitsByWordSize[w];
		int words = mAcsbsCompressionWordsByWordSize[w];
		if (bits < mAcsbsCompressionBits) {
			mAcsbsOptimalWordBits = w;
			mAcsbsCompressionBits = bits;
			mAcsbsCompressionWords = words;
		}
	}
}

void
BinSeqStat::findGolombStat()
{
	// Rice-Golomb compression size.
	for (int w = 0; w < WORD_BITS_MAX; w++) {
		int m = 1 << w;
		int bits = 0;
		int words = 0;
		for (int k = 0; k < mK+1; k++) {
			bits += (mDist[k] / m) + 1 + w;
			words += (mDist[k] / m) + 2;
		}
		mRiceGolombCodeCompressionBitsByWordSize[w] = bits;
		mRiceGolombCodeCompressionWordsByWordSize[w] = words;
	}
	
	mRiceGolombCodeOptimalWordBits = 0;
	mRiceGolombCodeCompressionBits = mRiceGolombCodeCompressionBitsByWordSize[0];
	for (int w = 1; w < WORD_BITS_MAX; w++) {
		int bits = mRiceGolombCodeCompressionBitsByWordSize[w];
		int words = mRiceGolombCodeCompressionWordsByWordSize[w];
		if (bits < mRiceGolombCodeCompressionBits) {
			mRiceGolombCodeOptimalWordBits = w;
			mRiceGolombCodeCompressionBits = bits;
			mRiceGolombCodeCompressionWords = words;
		}
	}
}

void
BinSeqStat::findZlibStat()
{
	// Find zlib DEFLATE compression size.
	unsigned long size = compressBound(mPackSeq.size());
	mPackSeqZlib.resize(size);
	compress(mPackSeqZlib.data(), &size, mPackSeq.data(), mPackSeq.size());
	mPackSeqZlib.resize(size);
	mZLibDeflateCompressionBits = 8*size;
}

void
BinSeqStat::findCompStat(bool excludeZlib)
{
	findAcsbsStat();
	findGolombStat();
	if (!excludeZlib) {
		findZlibStat();
	} else {
		mZLibDeflateCompressionBits = 0;
	}
	// Aggregation of all statistics.
	aggregateStat();
}

void
BinSeqStat::printSeq(bool withDistances)
{
	int k = 0;
	
	// Print zeros and ones.
	for (int i = 0; i < mN; i++) {
		std::cout << (int)mSeq[i];
		// Print distance for ones.
		if (withDistances && mSeq[i] == 1) {
			std::cout << "(" << mDist[k++] << ") ";
		}
	}
	
	// Print 'virtual' one.
	std::cout << ".";
	if (withDistances) std::cout << "(" << mDist[k++] << ")";
	std::cout << std::endl;
}

void
BinSeqStat::printAcsbsStat()
{
	std::map<int,int> freqMap;
	
	int m = (1 << mAcsbsOptimalWordBits) - 1;
	for (int k = 0; k < mK+1; k++) {
		freqMap[mDist[k] / m] += 1;
	}
	
	std::cout << "w = " << mAcsbsOptimalWordBits << std::endl;
	for (std::map<int,int>::iterator it = freqMap.begin(); it != freqMap.end(); it++) {
		std::cout << it->first << " => " << it->second << std::endl;
	}
}

void
BinSeqStat::printGolombStat()
{
	std::map<int,int> freqMap;
	
	int m = 1 << mRiceGolombCodeOptimalWordBits;
	for (int k = 0; k < mK+1; k++) {
		freqMap[mDist[k] / m] += 1;
	}
	
	std::cout << "w = " << mRiceGolombCodeOptimalWordBits << std::endl;
	for (std::map<int,int>::iterator it = freqMap.begin(); it != freqMap.end(); it++) {
		std::cout << it->first << " => " << it->second << std::endl;
	}
}

void
BinSeqStat::printStat()
{
	std::cout << std::setprecision(6) << (double)mK / mN << "\t";
	std::cout << (mSumZLibDeflateCompressionBits / mGenSeq) << "\t";
	std::cout << (mSumAcsbsCompressionBits / mGenSeq) << "\t";
	std::cout << (mSumRiceGolombCodeCompressionBits / mGenSeq) << "\t";
	std::cout << (mEntropy) << "\t";
	std::cout << (mSumAcsbsCompressionWords / mGenSeq) << "\t";
	std::cout << (mSumRiceGolombCodeCompressionWords / mGenSeq) << "\t";
	std::cout << ((double)mSumAcsbsCompressionCodeWordBits / mGenSeq) << "\t";
	std::cout << ((double)mSumRiceGolombCodeCompressionCodeWordBits / mGenSeq) << "\t";
	std::cout << std::endl << std::flush;
}

void
BinSeqStat::printStatHeader()
{
	std::cout << "k/n" << "\t";
	std::cout << "ZLIB" << "\t";
	std::cout << "AC-SBS" << "\t";
	std::cout << "Rice-Golomb" << "\t";
	std::cout << "Entropy" << "\t";
	std::cout << "AC-SBS words" << "\t";
	std::cout << "Rice-Golomb words" << "\t";
	std::cout << "AC-SBS code word bits" << "\t";
	std::cout << "Rice-Golomb code word bits" << "\t";
	std::cout << std::endl << std::flush;
}

void
BinSeqStat::packSeq()
{
	mPackSeq = std::vector<unsigned char>((mN+7)/8, 0);
	
	unsigned char *data = mPackSeq.data();

	for (int i = 0; i < mN; i++) {
		*data |= mSeq[i] << (i % 8);

		if ((i % 8) == 7) {
			data++;
		}
	}
}

void
BinSeqStat::aggregateStat()
{
	// Increase number of aggregated statistics.
	mGenSeq++;

	mSumAcsbsCompressionBits += mAcsbsCompressionBits;
	mSumAcsbsCompressionWords += mAcsbsCompressionWords;
	mSumAcsbsCompressionCodeWordBits += mAcsbsOptimalWordBits;
	mSumRiceGolombCodeCompressionBits += mRiceGolombCodeCompressionBits;
	mSumRiceGolombCodeCompressionWords += mRiceGolombCodeCompressionWords;
	mSumRiceGolombCodeCompressionCodeWordBits += mRiceGolombCodeOptimalWordBits;
	mSumZLibDeflateCompressionBits += mZLibDeflateCompressionBits;	
}
