#include "compstat.h"

#include <iostream>
#include <string>
#include <cmath>

const char *help =
	"-n\tNumber of bits in sequence.\n"
	"-m\tMaximum number of ones in sequence (minimum k).\n"
	"-s\tStep to the next k.\n"
	"-cl\tLower entropy bound coefficient.\n"
	"-ch\tUpper entropy bound coefficient.\n";

int
main(int argc, char *argv[])
{
	// Default number of bits in sequence.
	int n = 10000;
	// Default upper bound for number of ones insequence.
	int m = 1000;
	// Default step size for k values.
	int s = 1;
	// Default lower entropy bound coefficient.
	double cl = 0.48;
	// Default upper entropy bound coefficient.
	double ch = 2.72;
	
	while (*(++argv)) {
		// Number of bits in sequence.
		if (*argv == std::string("-n")) {
			n = std::stoi(*(++argv));
		// Upper limit of number of ones in sequence.
		} else if (*argv == std::string("-m")) {
			m = std::stoi(*(++argv));
		// Step for k values.
		} else if (*argv == std::string("-s")) {
			s = std::stoi(*(++argv));
		// Lower entropy bound coefficient.
		} else if (*argv == std::string("-cl")) {
			cl = std::stof(*(++argv));
		// Upper entropy bound coefficient.
		} else if (*argv == std::string("-ch")) {
			ch = std::stof(*(++argv));
		} else {
			printf("%s", help);
			return 0;
		}
	}
	
	if (n < m) {
		m = n;
	}
	
	// Print headers.
	std::cout << "n=" << n << std::endl;
	std::cout << "k\tEntropy\tk*log2(" << cl << "*n/k)\tk*log2(" << ch << "*n/k)" << std::endl;
	
	// Print data.
	for (int k = 1; k < m; k += s) {
		BinSeqStat bs(n, k);
		std::cout << k << "\t";
		std::cout << bs.entropy() << "\t";
		std::cout << (k*log2(cl*(double)n/k)) << "\t";
		std::cout << (k*log2(ch*(double)n/k)) << "\t";
		std::cout << std::endl;
	}
	
	return 0;
}
