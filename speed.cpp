#include "compress.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <string>

const char *help = 
	"-n\tNumber of bits in sequence.\n"
	"-min\tMinimum number of ones in sequence (minimum k).\n"
	"-max\tMaximum number of ones in sequence (maximum k).\n"
	"-s\tStep to the next k.\n"
	"-l\tNumber of tested sequences.\n"
	"-z\tTurn off ZLIB.\n";

int
main(int argc, char *argv[])
{
	// Default number of bits in sequence.
	int n = 10000;
	// Default minimum number of ones in sequence.
	int kMin = 1;
	// Default maximum number of ones in sequence.
	int kMax = n/2;
	// Default step to the next k.
	int s = 1;
	// Default number of tested random sequences.
	int l = 1;
	// Default option to test ZLIB.
	int z = 1;
	// Average time.
	double avgT;
	// Current time.
	clock_t t;
	
	while (*(++argv)) {
		// Number of bits in sequence.
		if (*argv == std::string("-n")) {
			n = std::stoi(*(++argv));
		// Minimum number of ones in sequence (minimum k).
		} else if (*argv == std::string("-min")) {
			kMin = std::stoi(*(++argv));
		// Maximum number of ones in sequence (maximum k).
		} else if (*argv == std::string("-max")) {
			kMax = std::stoi(*(++argv));
		// Step to the next k.
		} else if (*argv == std::string("-s")) {
			s = std::stoi(*(++argv));
		// Number of tested sequences.
		} else if (*argv == std::string("-l")) {
			l = std::stoi(*(++argv));
		// Turn off ZLIB.
		} else if (*argv == std::string("-z")) {
			z = 0;
		} else {
			printf("%s", help);
			return 0;
		}
	}

	// Iteration bounds for speed test.
	int B1, B2, B3;
	B1 = B2 = B3 = 1;
	
	srand(clock());

	// Generation of sequences to test.
	std::vector<BitString> bsVec;
	std::vector<int> v;
	
	for (int i = 0; i < l; i++) {
		bsVec.push_back(BitString(n));
		bsVec[i].random(kMin);
		bsVec[i].findDist();
	}
	
	// Print headers.
	std::cout << "n=" << n << std::endl;
	std::cout << "l=" << l << std::endl;
	std::cout << "k/n\tk\tAC-SBS (comp.) [us]\tAC-SBS (decomp.) [us]";
	std::cout << "\tRice-Golomb (comp.) [us]\tRice-Golomb (decomp.) [us]";
	if (z) {
		std::cout << "\tZ-LIB (comp.) [us]\tZ-LIB (decomp.) [us]";
	}
	std::cout << std::endl;

	// Measure compression/decompression time.
	for (int k = kMin; k < kMax; k += s) {
		std::cout << std::setprecision(4) << (double)k / n << "\t" << k;

		// =============================================================
		// Speed of AC-SBS
		// =============================================================
		
		// Compression time.
		avgT = 0;
		t = clock();
		for (int i = 0; i < l; i++) {
			for (int j = 0; j < B1; j++) {
				bsVec[i].setAcsbsDistEnc();
			}
		}
		avgT += clock() - t;
		
		avgT /= l*B1; // Average time in clocks.
		if (CLOCKS_PER_SEC < l*B1*avgT) { // Adjust test time to about 1s.
			B1 = CLOCKS_PER_SEC / (l*avgT) + 1;
		}
		avgT /= CLOCKS_PER_SEC; // Average time in seconds.
		avgT *= 1E6; // Average time in micro-seconds.
		std::cout << "\t" << avgT;

		// Decompression time.
		avgT = 0;
		t = clock();
		for (int i = 0; i < l; i++) {
			for (int j = 0; j < B1; j++) {
				bsVec[i].getAcsbsDistEnc(v);
			}
		}
		avgT += clock() - t;

		avgT /= l*B1; // Average time in clocks.
		if (CLOCKS_PER_SEC < l*B1*avgT) { // Adjust test time to about 1s.
			B1 = CLOCKS_PER_SEC / (l*avgT) + 1;
		}
		avgT /= CLOCKS_PER_SEC; // Average time in seconds.
		avgT *= 1E6; // Average time in micro-seconds.
		std::cout << "\t" << avgT;

		// =============================================================
		// Speed of Rice-Golomb
		// =============================================================

		// Compression time.
		avgT = 0;
		t = clock();
		for (int i = 0; i < l; i++) {
			for (int j = 0; j < B2; j++) {
				bsVec[i].setRiceDistEnc();
			}
		}
		avgT += clock() - t;
		
		avgT /= l*B2; // Average time in clocks.
		if (CLOCKS_PER_SEC < l*B2*avgT) { // Adjust test time to about 1s.
			B2 = CLOCKS_PER_SEC / (l*avgT) + 1;
		}
		avgT /= CLOCKS_PER_SEC; // Average time in seconds.
		avgT *= 1E6; // Average time in micro-seconds.
		std::cout << "\t" << avgT;

		// Decompression time.
		avgT = 0;
		t = clock();
		for (int i = 0; i < l; i++) {
			for (int j = 0; j < B2; j++) {
				bsVec[i].getRiceDistEnc(v);
			}
		}
		avgT += clock() - t;
		

		avgT /= l*B2; // Average time in clocks.
		if (CLOCKS_PER_SEC < l*B2*avgT) { // Adjust test time to about 1s.
			B2 = CLOCKS_PER_SEC / (l*avgT) + 1;
		}
		avgT /= CLOCKS_PER_SEC; // Average time in seconds.
		avgT *= 1E6; // Average time in micro-seconds.
		std::cout << "\t" << avgT;

		// =============================================================
		// Speed of ZLIB Deflate
		// =============================================================
		
		if (!z) {
			std::cout << std::endl << std::flush;
			continue;
		}
		
		// Compression time.
		avgT = 0;
		t = clock();
		for (int i = 0; i < l; i++) {
			for (int j = 0; j < B3; j++) {
				bsVec[i].setZlibDistEnc();
			}
		}
		avgT += clock() - t;
		
		avgT /= l*B3; // Average time in clocks.
		if (CLOCKS_PER_SEC < l*B3*avgT) { // Adjust test time to about 1s.
			B3 = CLOCKS_PER_SEC / (l*avgT) + 1;
		}
		avgT /= CLOCKS_PER_SEC; // Average time in seconds.
		avgT *= 1E6; // Average time in micro-seconds.
		std::cout << "\t" << avgT;

		// Decompression time.
		avgT = 0;
		t = clock();
		for (int i = 0; i < l; i++) {
			for (int j = 0; j < B3; j++) {
				bsVec[i].getZlibDistEnc(v);
			}
		}
		avgT += clock() - t;
		
		avgT /= l*B3; // Average time in clocks.
		if (CLOCKS_PER_SEC < l*B3*avgT) { // Adjust test time to about 1s.
			B3 = CLOCKS_PER_SEC / (l*avgT) + 1;
		}
		avgT /= CLOCKS_PER_SEC; // Average time in seconds.
		avgT *= 1E6; // Average time in micro-seconds.
		std::cout << "\t" << avgT;

		std::cout << std::endl << std::flush;
		
		// =============================================================
		// Find new vectors.
		// =============================================================
		for (int i = 0; i < l; i++) {
			// Increase number of ones by s.
			bsVec[i].random(s, true);
			bsVec[i].findDist();
		}
	}
	
	return 0;
}
