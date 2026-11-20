#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <cctype>
#include <cmath>
#include <cstdint>

using namespace std;
namespace fs = std::filesystem;

// Matching threshold: sentences with Jaccard score >= 0.50 are considered plagiarized.
const double THRESHOLD = 0.50;

// ============================================================================
// DATA STRUCTURES
// ============================================================================

// Represents a single sentence
struct Sentence {
    string text;                // Original sentence text
    vector<string> words;       // Sorted unique lowercase words
};

// Represents a sentence from one of the reference documents
struct SourceSentence {
    int docId;                  // Document index
    int sentenceId;             // Sentence index within the document
    Sentence sentence;          // The sentence content
};

// Represents a match between a main_doc sentence and a document sentence
struct Match {
    int sourceIndex = -1;       // Index in allSources vector (-1 means no match)
    double score = 0.0;         // Jaccard similarity score [0.0 - 1.0]
};

// Matrix of results: results[docId][mainSentenceIndex] = best Match
using Results = vector<vector<Match>>;

// ============================================================================
// HASH FUNCTION (Lecture 7: Hashing - written manually as required)
// 64-bit Polynomial Rolling Hash: h = sum(c * base^i)
// ============================================================================
uint64_t polyHash(const string& str) {
    uint64_t h = 0;
    const uint64_t base = 1315423911ULL;
    for (unsigned char c : str) {
        h = (h * base) + c;
    }
    return h;
}

// Custom hash functor so std::unordered_map uses our polynomial hash
struct CustomHash {
    size_t operator()(const string& s) const {
        return (size_t)polyHash(s);
    }
};

// ============================================================================
// TEXT PROCESSING & SENTENCE SPLITTING
// ============================================================================

// Extract unique, sorted lowercase words from a string
vector<string> extractWords(string text) {
    for (char& c : text) {
        c = isalnum((unsigned char)c) ? (char)tolower((unsigned char)c) : ' ';
    }

    stringstream ss(text);
    string word;
    vector<string> words;

    while (ss >> word) {
        words.push_back(word);
    }

    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

// Check if a period '.' is part of a common abbreviation or decimal
bool isAbbreviation(const string& text, int i) {
    // Check single letter initial (e.g. "U.S." or "A.")
    if (i > 0 && isalpha((unsigned char)text[i - 1])) {
        if (i == 1 || !isalpha((unsigned char)text[i - 2])) {
            return true;
        }
    }

    // Common abbreviations
    static const vector<string> titles = {
        "Mr", "Mrs", "Ms", "Dr", "St", "Mt", "No", "Gen", "Col", "Capt", "Sen", "Rep"
    };

    for (const auto& title : titles) {
        int len = (int)title.size();
        if (i >= len) {
            int start = i - len;
            if (text.compare(start, len, title) == 0) {
                if (start == 0 || !isalpha((unsigned char)text[start - 1])) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Read a file and split into clean sentences
vector<Sentence> readFile(const string& path) {
    ifstream f(path);
    if (!f) {
        cerr << "Error: Cannot open " << path << '\n';
        return {};
    }

    string text((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    vector<Sentence> sentences;
    string current;

    auto save = [&]() {
        size_t start = current.find_first_not_of(" \t\r\n");
        if (start != string::npos) {
            size_t end = current.find_last_not_of(" \t\r\n");
            string s = current.substr(start, end - start + 1);
            if (!s.empty()) {
                sentences.push_back({s, extractWords(s)});
            }
        }
        current.clear();
    };

    int n = (int)text.size();
    for (int i = 0; i < n; i++) {
        char c = text[i];
        current += c;

        // Skip decimal points in numbers (e.g. 465.91)
        bool decimal = (c == '.' && i > 0 && i + 1 < n &&
                        isdigit((unsigned char)text[i - 1]) &&
                        isdigit((unsigned char)text[i + 1]));

        // Skip abbreviations
        bool abbr = (c == '.' && isAbbreviation(text, i));

        if ((c == '.' || c == '!' || c == '?') && !decimal && !abbr) {
            save();
        }
    }

    if (!current.empty()) {
        save();
    }

    return sentences;
}

// ============================================================================
// SIMILARITY CALCULATION (Jaccard Index)
// |A ∩ B| / |A ∪ B| using two pointers on sorted vectors in O(|A| + |B|) time.
// ============================================================================
double jaccard(const vector<string>& a, const vector<string>& b) {
    int i = 0, j = 0, common = 0;
    int sizeA = (int)a.size();
    int sizeB = (int)b.size();

    while (i < sizeA && j < sizeB) {
        if (a[i] == b[j]) {
            common++;
            i++;
            j++;
        } else if (a[i] < b[j]) {
            i++;
        } else {
            j++;
        }
    }

    int totalUnion = sizeA + sizeB - common;
    return totalUnion > 0 ? (double)common / totalUnion : 0.0;
}

// Helper to keep the best matching sentence for a given document
void updateMatch(Results& r, int docId, int mainIdx, int sourceIdx, double score) {
    if (score < THRESHOLD) return;
    Match& m = r[docId][mainIdx];
    if (score > m.score || (score == m.score && (m.sourceIndex == -1 || sourceIdx < m.sourceIndex))) {
        m.sourceIndex = sourceIdx;
        m.score = score;
    }
}

// ============================================================================
// ALGORITHM 1: BRUTE FORCE (Lecture 4: Baseline Exhaustive Search)
// Compares every sentence of mainDoc with every sentence of all documents.
// Time Complexity: O(S_m * S_d * L)
// ============================================================================
Results bruteForce(const vector<Sentence>& mainDoc, const vector<SourceSentence>& sources, int numDocs) {
    Results r(numDocs, vector<Match>(mainDoc.size()));

    for (int i = 0; i < (int)mainDoc.size(); i++) {
        for (int j = 0; j < (int)sources.size(); j++) {
            double score = jaccard(mainDoc[i].words, sources[j].sentence.words);
            updateMatch(r, sources[j].docId, i, j, score);
        }
    }
    return r;
}

// ============================================================================
// ALGORITHM 2: EFFICIENT DETECTOR (Lecture 7: Hashing & Inverted Index)
// 1. Inverted index maps words to sentence IDs using our custom polynomial hash.
// 2. Length filter: skips pairs where max possible Jaccard is < THRESHOLD.
// 3. Prefix filter: searches rare words first to prune candidate comparisons.
// Time Complexity: O(S_d * L) index construction + O((S_m + candidates) * L) search
// ============================================================================
Results efficientSearch(const vector<Sentence>& mainDoc, const vector<SourceSentence>& sources, int numDocs) {
    // 1. Build Inverted Index using custom polynomial hash
    unordered_map<string, vector<int>, CustomHash> index;
    for (int j = 0; j < (int)sources.size(); j++) {
        for (const string& w : sources[j].sentence.words) {
            index[w].push_back(j);
        }
    }

    Results r(numDocs, vector<Match>(mainDoc.size()));
    vector<int> seen(sources.size(), -1);

    // 2. Query each mainDoc sentence
    for (int i = 0; i < (int)mainDoc.size(); i++) {
        vector<string> query = mainDoc[i].words;
        int lenA = (int)query.size();
        if (lenA == 0) continue;

        // Sort query words by frequency (rarest words first)
        auto freq = [&](const string& w) {
            auto it = index.find(w);
            return (it == index.end()) ? 0 : (int)it->second.size();
        };

        sort(query.begin(), query.end(), [&](const string& a, const string& b) {
            return freq(a) < freq(b);
        });

        // Prefix Filtering: At least one of the first p words must match
        int p = (int)floor((1.0 - THRESHOLD) * lenA + 1e-9) + 1;
        if (p > lenA) p = lenA;

        for (int k = 0; k < p; k++) {
            auto it = index.find(query[k]);
            if (it == index.end()) continue;

            for (int j : it->second) {
                if (seen[j] == i) continue; // Already tested for this query sentence
                seen[j] = i;

                int lenB = (int)sources[j].sentence.words.size();

                // Length filter: Jaccard <= min(a,b) / max(a,b)
                if (lenB < THRESHOLD * lenA || lenA < THRESHOLD * lenB) continue;

                double score = jaccard(mainDoc[i].words, sources[j].sentence.words);
                updateMatch(r, sources[j].docId, i, j, score);
            }
        }
    }
    return r;
}

// ============================================================================
// MAIN FUNCTION & OUTPUT FORMATTER
// ============================================================================
int main(int argc, char* argv[]) {
    bool modeBrute = false;
    bool modeBenchmark = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--brute") modeBrute = true;
        if (arg == "--benchmark") modeBenchmark = true;
    }

    // 1. Locate and read main_doc.txt
    string mainPath = "main_doc.txt";
    if (!fs::exists(mainPath) && fs::exists("data/sample_set/main_doc.txt")) {
        mainPath = "data/sample_set/main_doc.txt";
    }

    vector<Sentence> mainDoc = readFile(mainPath);
    if (mainDoc.empty()) {
        cerr << "Error: main_doc.txt is missing or empty.\n";
        return 1;
    }

    // 2. Locate and read documents/
    string docDir = "documents";
    if (!fs::exists(docDir) && fs::exists("data/sample_set/documents")) {
        docDir = "data/sample_set/documents";
    }

    vector<fs::path> paths;
    if (fs::exists(docDir)) {
        for (const auto& entry : fs::directory_iterator(docDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                paths.push_back(entry.path());
            }
        }
    }
    sort(paths.begin(), paths.end());

    if (paths.empty()) {
        cerr << "Error: No .txt documents found.\n";
        return 1;
    }

    vector<string> docNames;
    vector<SourceSentence> allSources;

    for (int d = 0; d < (int)paths.size(); d++) {
        docNames.push_back(paths[d].filename().string());
        vector<Sentence> docSentences = readFile(paths[d].string());
        for (int sIdx = 0; sIdx < (int)docSentences.size(); sIdx++) {
            allSources.push_back({d, sIdx, docSentences[sIdx]});
        }
    }

    int numDocs = (int)docNames.size();
    int totalMain = (int)mainDoc.size();

    // 3. Run selected algorithm(s)
    Results fastRes, slowRes;
    double tBrute = 0.0, tFast = 0.0;

    if (modeBenchmark) {
        auto t0 = chrono::steady_clock::now();
        slowRes = bruteForce(mainDoc, allSources, numDocs);
        auto t1 = chrono::steady_clock::now();
        tBrute = chrono::duration<double, milli>(t1 - t0).count();

        auto t2 = chrono::steady_clock::now();
        fastRes = efficientSearch(mainDoc, allSources, numDocs);
        auto t3 = chrono::steady_clock::now();
        tFast = chrono::duration<double, milli>(t3 - t2).count();
    } else if (modeBrute) {
        fastRes = bruteForce(mainDoc, allSources, numDocs);
    } else {
        fastRes = efficientSearch(mainDoc, allSources, numDocs);
    }

    // 4. Calculate similarities and rank documents
    struct DocSummary {
        int docId;
        string name;
        int count = 0;
        double rate = 0.0;
        vector<int> matches;
    };

    vector<DocSummary> summaries(numDocs);
    for (int d = 0; d < numDocs; d++) {
        summaries[d].docId = d;
        summaries[d].name = docNames[d];
        for (int i = 0; i < totalMain; i++) {
            if (fastRes[d][i].sourceIndex != -1) {
                summaries[d].count++;
                summaries[d].matches.push_back(i);
            }
        }
        summaries[d].rate = (100.0 * summaries[d].count) / totalMain;
    }

    sort(summaries.begin(), summaries.end(), [](const DocSummary& a, const DocSummary& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.name < b.name;
    });

    // 5. Print output matching expected format
    cout << fixed << setprecision(1);
    int rank = 1;
    int nonZero = 0;

    for (const auto& doc : summaries) {
        if (doc.count == 0) continue;
        nonZero++;

        cout << rank << ") " << doc.name
             << "   similarity " << doc.rate << "%"
             << "   (" << doc.count << " of " << totalMain << " sentences)\n";

        // Sort matched pairs by score descending
        vector<int> sortedMatches = doc.matches;
        sort(sortedMatches.begin(), sortedMatches.end(), [&](int a, int b) {
            double sa = fastRes[doc.docId][a].score;
            double sb = fastRes[doc.docId][b].score;
            if (fabs(sa - sb) > 1e-9) return sa > sb;
            return a < b;
        });

        // Print up to 5 pairs
        int limit = min(5, (int)sortedMatches.size());
        for (int p = 0; p < limit; p++) {
            int mIdx = sortedMatches[p];
            int sIdx = fastRes[doc.docId][mIdx].sourceIndex;
            cout << "   \"" << mainDoc[mIdx].text << "\"\n";
            cout << "   \"" << allSources[sIdx].sentence.text << "\"\n";
        }

        if ((int)sortedMatches.size() > 5) {
            cout << "   ... and " << (sortedMatches.size() - 5) << " more pairs\n";
        }
        cout << '\n';
        rank++;
    }

    if (numDocs > nonZero) {
        cout << "All other documents: 0.0%.\n";
    }

    // 6. Benchmark summary (if requested)
    if (modeBenchmark) {
        cerr << "\n=================== BENCHMARK REPORT (PURE C++) ===================\n";
        cerr << "| Documents (D) | Brute Force (ms) | Efficient (ms) | Speedup Factor |\n";
        cerr << "|:-------------:|:----------------:|:--------------:|:--------------:|\n";

        // Test 5 evenly spaced input sizes
        vector<int> testSizes;
        for (int step = 1; step <= 5; step++) {
            int count = (numDocs * step) / 5;
            if (count > 0 && (testSizes.empty() || count > testSizes.back())) {
                testSizes.push_back(count);
            }
        }

        bool allIdentical = true;

        for (int dCount : testSizes) {
            // Filter sources belonging to the first dCount documents
            vector<SourceSentence> subsetSources;
            for (const auto& s : allSources) {
                if (s.docId < dCount) {
                    subsetSources.push_back(s);
                }
            }

            // Time Brute Force
            auto t0 = chrono::steady_clock::now();
            Results slow = bruteForce(mainDoc, subsetSources, dCount);
            auto t1 = chrono::steady_clock::now();
            double bMs = chrono::duration<double, milli>(t1 - t0).count();

            // Time Efficient Hashing
            auto t2 = chrono::steady_clock::now();
            Results fast = efficientSearch(mainDoc, subsetSources, dCount);
            auto t3 = chrono::steady_clock::now();
            double fMs = chrono::duration<double, milli>(t3 - t2).count();

            // Verify correctness
            for (int d = 0; d < dCount; d++) {
                for (int i = 0; i < totalMain; i++) {
                    if (fast[d][i].sourceIndex != slow[d][i].sourceIndex) {
                        allIdentical = false;
                    }
                }
            }

            double sp = (fMs > 0.0) ? (bMs / fMs) : 0.0;
            cerr << "| " << setw(13) << dCount << " | "
                 << setw(13) << fixed << setprecision(2) << bMs << " ms | "
                 << setw(11) << fixed << setprecision(2) << fMs << " ms | "
                 << setw(12) << fixed << setprecision(2) << sp << "x |\n";
        }

        cerr << "===================================================================\n";
        cerr << "Match Verification : " << (allIdentical ? "ALL MATCHES 100% IDENTICAL" : "MISMATCH DETECTED") << "\n";
    }

    return 0;
}
