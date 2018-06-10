#ifndef __EXP2_SIMJOINER_H__
#define __EXP2_SIMJOINER_H__

#include <vector>
#include "SimSearcher.h"
#include "hashmap.h"
using namespace std;

const int QGRAM_LEN = 4;
const int HASH_SIZE = 4711313;

class SimJoiner {
public:
    // DEBUG flags
    bool DEBUG;
    bool use_str_search;

public:
    SimJoiner();
    ~SimJoiner();

    int joinJaccard(const char *filename1, const char *filename2, double threshold, std::vector<JaccardJoinResult> &result);
    int joinED(const char *filename1, const char *filename2, unsigned threshold, std::vector<EDJoinResult> &result);
    void printResult(const vector<JaccardJoinResult> &result);
    void printResult(const vector<EDJoinResult> &result);

private:
    void init();
    int ED(int k);
    int bruteForce(vector<EDJoinResult> &result);
    int createRefDatasetED(const char *filename);
    int createRefDatasetJaccard(const char *filename);
    int searchJaccard(const char *query, vector<JaccardJoinResult> &result);
    int searchED(const char *query, std::vector<EDJoinResult> &result);

private:
    /// datasets
    vector<string> dataset_query, dataset_ref;
    vector<int> ch_len, jac_len;
    int record_num;
    map<string, bool> visited;

    /// Jaccard Threshold
    double TJ;
    /// ED Threshold
    int TE;
    int T, L;

    /// inverse lists
    HashMap<string, vector<int>, HashFunc, EqualKey> *inv_list_ref;
    HashMap<string, vector<int>, HashFunc, EqualKey> *inv_qram;
	SimplePair *query_gram;
    list<int> candidate;

    /// temporals
    char *buf;
    const char *query; int buflen, qlen;
    int id_query;
    int *inter_cnt, *scan_count;
    short f[MAXLEN][MAXLEN];

    SimSearcher *str_searcher;
};

#endif
