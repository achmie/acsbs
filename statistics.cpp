#include "compstat.h"

#include <iostream>
#include <string>

const char *help =
	"-n\tNumber of bits in sequence.\n"
	"-min\tMinimum number of ones in sequence (minimum k).\n"
	"-max\tMaximum number of ones in sequence (maximum k).\n"
	"-s\tStep to the next k.\n";

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
	// Default option to test ZLIB.
	int z = 1;
	
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
		// Turn off ZLIB.
		} else if (*argv == std::string("-z")) {
			z = 0;
		} else {
			printf("%s", help);
			return 0;
		}
	}
	
	std::cout << "n=" << n << std::endl;
	BinSeqStat::printStatHeader();
	
	for (int k = kMin; k < kMax; k += s) {
		BinSeqStat bs(n, k);
	
		for (int i = 0; i < 100; i++) {
			bs.random();
			bs.findCompStat(z == 0);
		}
	
		bs.printStat();
	}
	
	return 0;
}
