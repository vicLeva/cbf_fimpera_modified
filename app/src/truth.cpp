#include <math.h>  // TODO remove ?
#include <robin_hood.h>

#include <chrono>
#include <fimpera-lib/CBF.hpp>
#include <fimpera-lib/ResultGetter.hpp>
#include <fimpera-lib/evaluation/UnlimitedTruthInTheShapeOfAnAMQ.hpp>
#include <fimpera-lib/fimpera.hpp>

#include "args.hpp"

inline void add(robin_hood::unordered_map<int, int>& map, const int& key, const int& valueToAdd) {
    robin_hood::unordered_map<int, int>::const_iterator got = map.find(key);
    if (got != map.end()) {                   // found
        int val = got->second;                // sauvegarde de la valeur
        map.erase(key);                       // suppression de la valeur dans la map
        map.insert({key, val + valueToAdd});  // insertion de la nouvelle valeur
    } else {                                  // not found
        map.insert({key, valueToAdd});
    }
}

inline int absDiff(const int& a, const int& b) {
    int abs_diff = (a > b) ? (a - b) : (b - a);
    return abs_diff;
}

inline robin_hood::unordered_map<int, int> getHistogram(std::vector<int> a, std::vector<int> b) {
    assert(a.size() == b.size());
    int n = a.size();
    robin_hood::unordered_map<int, int> histogram;
    for (int i = 0; i < n; ++i) {
        add(histogram, absDiff(a[i], b[i]), 1);
    }
    return histogram;
}

void inline toFileTXT(std::string outfilename, robin_hood::unordered_map<int, int> map) {
    remove(outfilename.c_str());

    // get sorted keys
    std::vector<int> keys;
    for (auto const& [key, b] : map) {
        keys.push_back(key);
    }
    sort(keys.begin(), keys.end());

    // print keys and values to file
    std::ofstream outFile(outfilename);
    for (const auto& key : keys) {
        outFile << key << " " << map[key] << "\n";
    }
}

std::tuple<std::size_t, std::size_t, std::size_t, std::size_t> compareVectors(const std::vector<int>& result, const std::vector<int>& truth) {
    std::size_t tp = 0;
    std::size_t tn = 0;
    std::size_t fp = 0;
    std::size_t fn = 0;

    assert(result.size() == truth.size());

    for (std::size_t i = 0; i < result.size(); i++) {
        if (truth[i]) {
            if (result[i]) {
                tp++;
            } else {
                fn++;
            }
        } else {
            if (result[i]) {
                fp++;
            } else {
                tn++;
            }
        }
    }
    return {tp, tn, fp, fn};
}

void writeConstructionFPToFile(const fimpera<countingBF::CBF>& index, const std::string& queryFile, const std::vector<int>& construction_truth_response, const std::vector<int>& truth_response, const std::string& outfilename) {
    remove(outfilename.c_str());

    // print keys and values to file
    std::ofstream outFile(outfilename);

    FileManager reader = FileManager();
    reader.addFile(queryFile);
    std::string current_read;

    int pos = 0;

    while (!(current_read = reader.get_next_read()).empty()) {
        std::size_t size = current_read.length();
        std::size_t j = 0;  // start of the kmer in the Kmer
        while (j < size - (index.getK() - index.getz()) + 1) {
            if (construction_truth_response[pos] && !truth_response[pos]) {
                outFile << current_read.substr(j, index.getK() - index.getz()) << "\n";
            }
            pos++;
            j++;
        }
    }
}

template <
    class result_t = std::chrono::milliseconds,
    class clock_t = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds>
auto since(std::chrono::time_point<clock_t, duration_t> const& start) {
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

void queryLowMemory(fimpera<countingBF::CBF>& index, fimpera<UnlimitedTruthInTheShapeOfAnAMQ>& truth, fimpera<UnlimitedTruthInTheShapeOfAnAMQ>& ctruth, const std::string& filename) {
    FileManager reader = FileManager();
    reader.addFile(filename);
    std::string current_read;

    uint64_t tp = 0;
    uint64_t tn = 0;
    uint64_t fp = 0;
    uint64_t fn = 0;

    uint64_t tpc = 0;
    uint64_t tnc = 0;
    uint64_t fpc = 0;
    uint64_t fnc = 0;

    uint64_t nbIterMax = 100000;
    uint64_t i = 0;

    while ((!(current_read = reader.get_next_read()).empty()) && (i < nbIterMax)) {
        std::string current_data = reader.get_data();
        std::string current_header = current_data.substr(0, current_data.find('\n'));

        std::vector<int> res = index.queryRead(current_read);
        std::vector<int> res_truth = truth.queryRead(current_read);
        std::vector<int> res_ctruth = ctruth.queryRead(current_read);

        const auto& [tpp, tnp, fpp, fnp] = compareVectors(res, res_truth);
        tp += tpp;
        tn += tnp;
        fp += fpp;
        fn += fnp;

        const auto& [tpcp, tncp, fpcp, fncp] = compareVectors(res_ctruth, res_truth);
        tpc += tpcp;
        tnc += tncp;
        fpc += fpcp;
        fnc += fncp;

        // response.processResult(res, _k + _z, current_header, current_read);
        i++;
    }
    std::cout << tp << " " << tn << " " << fp << " " << fn << std::endl;
    std::cout << "fpr = " << ((double)fp) / ((double)(fp + tn)) << std::endl;

    std::cout << tpc << " " << tnc << " " << fpc << " " << fnc << std::endl;
    std::cout << "fprc = " << ((double)fpc) / ((double)(fpc + tnc)) << std::endl;
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void compareWithTruth(const std::string& indexFilenameTemplate, const std::string& KMCFilename, const std::string& queryFile, uint64_t size, uint64_t nbBuckets, std::size_t k) {
    const std::vector<int> zs = {0, 1, 2, 3, 4, 5, 6, 10, 12, 15, 18, 21, 24, 27, 30};
    auto start = std::chrono::steady_clock::now();

    fimpera<UnlimitedTruthInTheShapeOfAnAMQ> truth = fimpera<UnlimitedTruthInTheShapeOfAnAMQ>(KMCFilename, k, 0, false, size, nbBuckets);
    std::cout << "index truth in (ms)=" << since(start).count() << std::endl;

    for (int z : zs) {
        auto start_of_this_z = std::chrono::steady_clock::now();
        std::cout << "starting analyzing z = " << z << " after " << std::chrono::duration<double>(start_of_this_z - start).count() << " s." << std::endl;

        std::string indexFilename = ReplaceAll(indexFilenameTemplate, "_z_", "_z" + std::to_string(z) + "_");
        indexFilename = ReplaceAll(indexFilename, "_k_", "_k" + std::to_string(k) + "_");
        std::cout << "index filename " << indexFilename << std::endl;

        fimpera<countingBF::CBF> index = fimpera<countingBF::CBF>(indexFilename);
        std::cout << "index BF in (ms)=" << since(start_of_this_z).count() << std::endl;

        fimpera<UnlimitedTruthInTheShapeOfAnAMQ> ctruth = fimpera<UnlimitedTruthInTheShapeOfAnAMQ>(KMCFilename, k, index.getz(), false, size, nbBuckets);
        std::cout << "index ctruth in (ms)=" << since(start_of_this_z).count() << std::endl;

        queryLowMemory(index, truth, ctruth, queryFile);
    }

    // TODO move to an evaluation module
    // read / create indexes

    // const std::string name = std::to_string(index.getK()) + "_" + std::to_string(index.getz()) + "_" + std::to_string(nbBuckets);

    // toFileTXT("histo_" + name + ".txt", getHistogram(index_response, truth_response));
    // toFileTXT("histo_construction_" + name + ".txt", getHistogram(construction_truth_response, truth_response));
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("fimpera_index", "0.0.1");
    // mandatory arguments
    program.add_argument("input_filename").help("index you want to query");
    program.add_argument("query_filename").help("file you want to query against the index");
    program.add_argument("kmc_filename").help("kmc file that contains the truth for the Kmers");

    parse(program, argc, argv);

    const std::string index_filename = program.get("input_filename");
    const std::string query_filename = program.get("query_filename");
    const std::string kmc_filename = program.get("kmc_filename");

    compareWithTruth(index_filename, kmc_filename, query_filename, 1, 1, 31);
}
