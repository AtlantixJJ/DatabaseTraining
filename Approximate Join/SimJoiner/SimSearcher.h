#ifndef SIM_SEARCHER_H_
#define SIM_SEARCHER_H_

#pragma once
#include <vector>
#include <utility>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <list>
#include <map>
#include <iostream>
using namespace std;

const int SUCCESS = 0;
const int FAILURE = 1;
const int MAXLEN = 512;

template <typename IDType, typename SimType>
struct JoinResult {
    IDType id1;
    IDType id2;
    SimType s;

	JoinResult(IDType _id1, IDType _id2, SimType _s) {id1 = _id1; id2 = _id2; s = _s;}
	JoinResult(const JoinResult &obj) {id1 = obj.id1; id2 = obj.id2; s = obj.s;}
};

typedef JoinResult<unsigned, double> JaccardJoinResult;
typedef JoinResult<unsigned, unsigned> EDJoinResult;

bool compareJoinResultJaccard(const JaccardJoinResult &a, const JaccardJoinResult &b);
bool compareJoinResultED(const EDJoinResult &a, const EDJoinResult &b);

class SimplePair{
public:
	string name;
	int count;
	SimplePair():count(0) {}
	SimplePair(const string &n, int c) : name(n), count(c) {}
	const SimplePair& operator = (const SimplePair &obj) {name = obj.name; count = obj.count;return *this;}
};

inline bool compare_simplepair(const SimplePair &a, const SimplePair &b);
inline int min(int a, int b, int c);
void printInvList(map<string, list<pair<int, int>>> inv_list);

class SimJoiner;

class SimSearcher
{
friend class SimJoiner;
public:
	bool ISDEBUG, ISDISPLAY;
	SimSearcher();
	~SimSearcher();

	int createIndex(const char *filename, unsigned q);
	int searchJaccard(const char *query, double threshold, std::vector<JaccardJoinResult> &result);
	int searchED(const char *query, unsigned threshold, std::vector<EDJoinResult> &result);
	int bruteForce( std::vector<EDJoinResult> &result);
	// edit distance with no.k record
	int ED(int k);


	void init(int i);
	void printResult(std::vector<JaccardJoinResult> &result);
	void printResult(std::vector<EDJoinResult> &result);
	int record_num;

	char *buf;
	
	const char *query;
	int id_query;
	int qlen, threshold, QLEN, T, L;

	// Dataset
	vector<string> dataset;
	// Jaccard intersection count
	int *inter_cnt;
	// Word length
	vector<int> jac_len;
	// Line length
	vector<int> ch_len;

	// DP array for edit distance
	short f[MAXLEN][MAXLEN];

	map<string, bool> visited;

	// Word inverse list for jaccard
	map<string, vector<int>> inv_list;
	// Q-gram inverse list for ED filter
	map<string, vector<int>> inv_qram;
	SimplePair *query_gram;
	// As the element must be smaller than T = |Q| - q + 1 - tao * q and |Q| < 256
	int *scan_count;
	list<int> candidate;
};


#endif