#include <iostream>
#include <string>

#include <fimpera-lib/CBF.hpp>
#include <fimpera-lib/abundanceToIdentifierStrategy.hpp>
#include <fimpera-lib/fimpera.hpp>
#include <iostream>

#include "args.hpp"


int main(int argc, char* argv[]) {
    std::string seq;
    std::vector<int> answer;

    std::cout << "start cbf 6_1" << std::endl;

    auto ttot = std::chrono::high_resolution_clock::now();
    fimpera<countingBF::CBF>
        f2("~/data/AHX_ACXIOSF_6_1_19_2andmore.txt", 32, 13, true, 9126805504, 5);
    std::cout << std::to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (build+inserts)\n";


    std::ifstream infile1("~/data/6_1_reads.fasta");
    ttot = std::chrono::high_resolution_clock::now();
    while (!infile1.eof()) {
        std::getline(infile1, seq);  // skip header line
        std::getline(infile1, seq);

        answer = f2.queryRead(seq);
    }
    std::cout << std::to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (pos queries)\n";


    std::ifstream infile2("~/data/neg_queries.fasta");
    ttot = std::chrono::high_resolution_clock::now();
    while (!infile2.eof()) {
        std::getline(infile2, seq);  // skip header line
        std::getline(infile2, seq);

        answer = f2.queryRead(seq);
    }
    std::cout << std::to_string( std::chrono::duration<double, std::milli>( std::chrono::high_resolution_clock::now() - ttot ).count()) << " ms (neg queries)\n";

}
