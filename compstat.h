#ifndef __COMPSTAT_H__
#define __COMPSTAT_H__

#include <vector>

#ifndef WORD_BITS_MAX
//! Maximum size of compression Word.
#define WORD_BITS_MAX 30
#endif

//! Class of binary sequence statistics.
class BinSeqStat
{
public:
	//! Random binery sequence of n elaments with exactly k ones.
	BinSeqStat(int n = 0, int k = 0);

	//! Entropy of sequence.
	int entropy() const;

	//! Random binery sequence of n elaments with exactly k ones.
	void random();
	//! Determine AC-SBS statistics.
	void findAcsbsStat();
	//! Determine Rice-Golomb statistics.
	void findGolombStat();
	//! Determine Lempel-Ziv (zlib) statistics.
	void findZlibStat();
	//! Determine all avaliable statistics.
	void findCompStat(bool excludeZlib = false);
	
	//! Print sequence.
	void printSeq(bool withDistances = true);
	//! Print AC-SBS statistics.
	void printAcsbsStat();
	//! Print Rice-Golomb statistics.
	void printGolombStat();
	//! Print all statistics.
	void printStat();
	//! Print header for all statistics.
	static void printStatHeader();

protected:
	//! Pack sequence vector (every bit in separate byte) to binary string.
	void packSeq();
	//! Add statistics of this sequence to aggregation.
	void aggregateStat();

private:
	//! \defgroup CompstatSP Sequence properties.
	//! \{
	
	//! Sequence length.
	int mN;
	//! Number of ones in sequence.
	int mK;
	//! Sequence entropy.
	int mEntropy;
	//! Vector of bits (single bit in single char).
	std::vector<unsigned char> mSeq;
	//! Packed sequence of bits (8 bits in single char).
	std::vector<unsigned char> mPackSeq;
	//! Sequence packed with ZLIB.
	std::vector<unsigned char> mPackSeqZlib;
	//! Sequence of distances between ones.
	std::vector<int> mDist;
	//! \}
	
	//! \defgroup CompstatSCS Sequence compression statistics.
	//! \{
	
	//! Optimal coding word bits for AC-SBS.
	int mAcsbsOptimalWordBits;
	//! Number of bits in AC-SBS compressed stream.
	int mAcsbsCompressionBits;
	//! Number of coding words in AC-SBS compressed stream.
	int mAcsbsCompressionWords;
	//! Number of bits in AC-SBS compressed stream by code word bits.
	int mAcsbsCompressionBitsByWordSize[WORD_BITS_MAX];
	//! Number of coding words in AC-SBS compressed stream by code word bits.
	int mAcsbsCompressionWordsByWordSize[WORD_BITS_MAX];
	//! Optimal coding word bits for Rice-Golomb.
	int mRiceGolombCodeOptimalWordBits;
	//! Number of bits in Rice-Golomb compressed stream.
	int mRiceGolombCodeCompressionBits;
	//! Number of coding words in Rice-Golomb compressed stream.
	int mRiceGolombCodeCompressionWords;
	//! Number of bits in Rice-Golomb compressed stream by code word bits.
	int mRiceGolombCodeCompressionBitsByWordSize[WORD_BITS_MAX];
	//! Number of coding words in Rice-Golomb compressed stream by code word bits.
	int mRiceGolombCodeCompressionWordsByWordSize[WORD_BITS_MAX];
	//! Number of bits in Lempel-Ziv (ZLIB DEFLATE) compressed stream.
	int mZLibDeflateCompressionBits;
	//! \}
	
	//! \defgroup CompstatAvg Averages for a set of random sequences.
	//! \{
	
	//! Number of generated random sequences.
	int mGenSeq;
	//! Sum of number of bits in all generated AC-SBS compressed streams.
	long mSumAcsbsCompressionBits;
	//! Sum of number of coding words in all generated AC-SBS compressed streams.
	long mSumAcsbsCompressionWords;
	//! Sum of number of optimal coding words bits in all generated AC-SBS compressed streams.
	long mSumAcsbsCompressionCodeWordBits;
	//! Sum of number of bits in all generated Rice-Golomb compressed streams.
	long mSumRiceGolombCodeCompressionBits;
	//! Sum of number of coding words in all generated Rice-Golomb compressed streams.
	long mSumRiceGolombCodeCompressionWords;
	//! Sum of number of optimal coding words bits in all generated Rice-Golomb compressed streams.
	long mSumRiceGolombCodeCompressionCodeWordBits;
	//! Sum of number of bits in all generated Lempel-Ziv compressed streams.
	long mSumZLibDeflateCompressionBits;
	//! \}

};

#endif // __COMPSTAT_H__
