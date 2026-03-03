#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <limits>

using namespace std;
using namespace std::chrono;

class Compressor {
public:
    static void compressChunk(const string & input, size_t start, size_t end, vector<uint16_t> & output) {
        if (start >= end) return;
        
        unordered_map<string, uint16_t> dict;
        for (uint32_t i = 0; i < 256; i++) {
            dict[string(1, i)] = i;
        }
        
        uint16_t dictSize = 256;
        string w = "";
        
        for (size_t i = start; i < end; i++) {
            char c = input[i];
            string wc = w + c;
            if (dict.count(wc)) {
                w = wc;
            } else {
                output.push_back(dict[w]);
                if (dictSize < 65535) {
                    dict[wc] = dictSize++;
                }
                w = string(1, c);
            }
        }
        if (!w.empty()) {
            output.push_back(dict[w]);
        }
    }

    static void decompressChunk(const vector<uint16_t> & input, string & output) {
        if (input.empty()) return;

        unordered_map<uint16_t, string> dict;
        for (uint32_t i = 0; i < 256; i++) {
            dict[i] = string(1, i);
        }
        
        uint16_t dictSize = 256;
        uint16_t old = input[0];
        string s = dict[old];
        output += s;
        string c = "";
        c += s[0];
        
        for (size_t i = 1; i < input.size(); i++) {
            uint16_t n = input[i];
            if (dict.count(n)) {
                s = dict[n];
            } else if (n == dictSize) {
                s = dict[old];
                s += c;
            }
            output += s;
            c = "";
            c += s[0];
            if (dictSize < 65535) {
                dict[dictSize++] = dict[old] + c;
            }
            old = n;
        }
    }
};

int main(int argc, char * argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    string filename = argv[1];
    
    ofstream createTemp(filename, ios::app | ios::binary);
    createTemp.close();

    fstream mainFile(filename, ios::in | ios::out | ios::binary);
    if (!mainFile.is_open()) {
        cout << "Critical error: Could not open file.\n";
        return 1;
    }

    int option;
    do {
        cout << "\n Multithreaded LZW Compressor \n";
        cout << "1 : Compress File (Performance Test)\n";
        cout << "2 : Decompress File\n";
        cout << "3 : Exit\n";
        cout << "Choice: ";
        cin >> option;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "  Invalid input. Please enter a valid number.\n";
            option = 0; 
            continue;
        }

        if (option == 1) {
            mainFile.clear();
            mainFile.seekg(0, ios::end);
            size_t fileSize = mainFile.tellg();
            mainFile.seekg(0, ios::beg);

            if (fileSize == 0) {
                cout << "  File is empty.\n";
                continue;
            }

            string content(fileSize, '\0');
            mainFile.read(&content[0], fileSize);

            vector<uint16_t> singleThreadOutput;
            auto startSingle = high_resolution_clock::now();
            Compressor::compressChunk(content, 0, content.length(), singleThreadOutput);
            auto endSingle = high_resolution_clock::now();
            auto durationSingle = duration_cast<microseconds>(endSingle - startSingle).count();

            int numThreads = 4;
            vector<vector<uint16_t>> results(numThreads);
            vector<thread> threads;
            size_t chunkSize = content.length() / numThreads;

            auto startMulti = high_resolution_clock::now();

            for (int i = 0; i < numThreads; ++i) {
                size_t start = i * chunkSize;
                size_t end = (i == numThreads - 1) ? content.length() : start + chunkSize;
                threads.emplace_back(Compressor::compressChunk, cref(content), start, end, ref(results[i]));
            }

            for (auto & t : threads) {
                t.join();
            }

            auto endMulti = high_resolution_clock::now();
            auto durationMulti = duration_cast<microseconds>(endMulti - startMulti).count();

            mainFile.close();
            mainFile.open(filename, ios::out | ios::trunc | ios::binary);
            
            uint32_t magicNumber = 0x5A4C5721;
            mainFile.write(reinterpret_cast<char *>(&magicNumber), sizeof(magicNumber));

            uint32_t threadCount = numThreads;
            mainFile.write(reinterpret_cast<char *>(&threadCount), sizeof(threadCount));
            
            for (const auto & chunk : results) {
                uint32_t sz = chunk.size();
                mainFile.write(reinterpret_cast<char *>(&sz), sizeof(sz));
            }
            for (const auto & chunk : results) {
                mainFile.write(reinterpret_cast<const char *>(chunk.data()), chunk.size() * sizeof(uint16_t));
            }

            mainFile.close();
            mainFile.open(filename, ios::in | ios::out | ios::binary);

            cout << "\n  --- Performance Results ---\n";
            cout << "  Single-threaded time : " << durationSingle << " microseconds\n";
            cout << "  Multi-threaded time  : " << durationMulti << " microseconds\n";
            cout << "  ---------------------------\n";

        } else if (option == 2) {
            mainFile.clear();
            mainFile.seekg(0, ios::beg);

            uint32_t magicCheck;
            if (!mainFile.read(reinterpret_cast<char *>(&magicCheck), sizeof(magicCheck)) || magicCheck != 0x5A4C5721) {
                cout << "  Error: File is not compressed or is an incompatible format.\n";
                continue;
            }

            uint32_t numThreads;
            if (!mainFile.read(reinterpret_cast<char *>(&numThreads), sizeof(numThreads))) {
                cout << "  Error: File is corrupted.\n";
                continue;
            }

            vector<uint32_t> sizes(numThreads);
            for (uint32_t i = 0; i < numThreads; i++) {
                mainFile.read(reinterpret_cast<char *>(&sizes[i]), sizeof(sizes[i]));
            }

            vector<vector<uint16_t>> compressedData(numThreads);
            for (uint32_t i = 0; i < numThreads; i++) {
                compressedData[i].resize(sizes[i]);
                mainFile.read(reinterpret_cast<char *>(compressedData[i].data()), sizes[i] * sizeof(uint16_t));
            }

            vector<string> decompressedResults(numThreads);
            vector<thread> threads;

            for (uint32_t i = 0; i < numThreads; ++i) {
                threads.emplace_back(Compressor::decompressChunk, cref(compressedData[i]), ref(decompressedResults[i]));
            }

            for (auto & t : threads) {
                t.join();
            }

            mainFile.close();
            mainFile.open(filename, ios::out | ios::trunc | ios::binary);
            
            for (const auto & res : decompressedResults) {
                mainFile.write(res.c_str(), res.length());
            }

            mainFile.close();
            mainFile.open(filename, ios::in | ios::out | ios::binary);

            cout << "  File decompressed and restored successfully.\n";
        }
    } while (option != 3);

    mainFile.close();
    return 0;
}