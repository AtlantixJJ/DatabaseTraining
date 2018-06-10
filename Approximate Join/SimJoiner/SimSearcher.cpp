#include "SimSearcher.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include<iostream>
#include<algorithm>
using namespace std;

bool compareJoinResultJaccard(const JaccardJoinResult &a, const JaccardJoinResult &b) {
	/*
	if (a.id1 < b.id1) return true;
	else if (a.id1 == b.id1) {
		return a.id2 < b.id2;
	}
	return false;
	*/
	return a.id2 < b.id2;
}

bool compareJoinResultED(const EDJoinResult &a, const EDJoinResult &b) {
	/*
	if (a.id1 < b.id1) return true;
	else if (a.id1 == b.id1) {
		return a.id2 < b.id2;
	}
	return false;
	*/
	return a.id2 < b.id2;
}

inline bool compare_simplepair(const SimplePair &a, const SimplePair &b) {
  return a.count > b.count; //return a>b，则为降序
}

void printInvList(map<string, vector<int>> inv_list) {
	printf("Invert list:\n");
	for(auto iter = inv_list.begin(); iter != inv_list.end(); iter++) {
		printf("%s: ", iter->first.c_str());
		for(auto jter = iter->second.begin(); jter != iter->second.end(); jter++) {
			printf("%d, ", *jter);
		}
		printf("\n");
	}
}

void SimSearcher::printResult(std::vector<JaccardJoinResult> &result) {
	for(int i = 0; i < result.size(); i++)
		printf("%d %d %.4f\n", result[i].id1, result[i].id2, result[i].s);
	printf("\n");
}

void SimSearcher::printResult(std::vector<EDJoinResult> &result) {
	for(int i = 0; i < result.size(); i++)
		printf("%d %d %d\n", result[i].id1, result[i].id2, result[i].s);
	printf("\n");
}

inline int min(int a, int b, int c){
	return (a > b) ? ((b > c) ? c : b) : ((a > c) ? c : a);
}

SimSearcher::SimSearcher()
{
	ISDEBUG = ISDISPLAY = false;
	buf = new char[1024];
	
}

SimSearcher::~SimSearcher()
{
	delete [] buf;
	delete [] inter_cnt;
	delete [] scan_count;
}

void SimSearcher::init(int i) {
	record_num = i;
	inter_cnt = new int [i];
	scan_count = new int [i];

	for(int j = 0; j < MAXLEN; j++) f[0][j] = j;
	for(int j = 0; j < MAXLEN; j++) f[j][0] = j;
}

int SimSearcher::createIndex(const char *filename, unsigned q)
{
	int size = 0, i = 0, j = 0, last = 0, cnt = 0;
	auto iter = inv_list.begin();
	string key, qgram;
	size_t n = 260;
	FILE* fin = fopen(filename, "r");

	for(i = 0;; i++) {
		size = getline(&buf, &n, fin) - 1;
		if(size <= 0) break;

		if(buf[size] == '\n') buf[size] = '\0';
		else size++;
		
		key = buf;
		// record dataset
		dataset.push_back(key);
		// record length
		ch_len.push_back(key.length());

		// split the sentence
		last = cnt = 0;
		for(j = 0; j <= size; j ++) {
			// Split q-gram
			if(q < 0xfff0 && j <= size - (int)q) {
				qgram = string(buf + j, q);

				iter = inv_qram.find(qgram);
				if(iter == inv_qram.end()) {
					// new gram
					inv_qram[qgram] = vector<int>{i};
				} else if(iter->second[iter->second.size()-1] < i) {
					// ignore same gram
					inv_qram[qgram].push_back(i);
				}
			}

			if(buf[j] == ' ' || j == size) {				
				if (j == size) buf[j+1] = '\0';
				else buf[j] = '\0';
				key = buf + last;
				iter = inv_list.find(key);
				if(iter == inv_list.end()) {
					// new word
					cnt ++;
					inv_list[key] = vector<int>{i};
				} else if(iter->second[iter->second.size()-1] < i) {
						inv_list[key].push_back(i);
					cnt ++;
				}
				last = j+1;
			}
		}

		// record word number
		jac_len.push_back(cnt);
	}
	
	init(i); // init with i records
	QLEN = q;
	return SUCCESS;
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<JaccardJoinResult> &result)
{
	int last, i, j, size = strlen(query), cnt = 0;
	auto iter = inv_list.begin();
	double res = 0;
	string key;
	visited.clear();
	int last_result = result.size();
	memset(inter_cnt, 0, sizeof(int) * record_num);

	last = cnt = 0;
	for(i = 0; i <= size; i ++) {
		if(query[i] == ' ' || i == size) {
			// a key encountered
			key = string(query + last, i - last);
			last = i + 1;
			
			if (visited.find(key) != visited.end()) {
				// duplicated keys
				continue;
			}
			visited[key] = true;
			cnt ++;
			
			iter = inv_list.find(key);
			if (iter == inv_list.end()) {
				// not found
				continue;
			}
			for(auto jter = iter->second.begin(); jter != iter->second.end(); jter++) {
				inter_cnt[*jter] ++;
			}
		}
	}

	for(i = 0; i < record_num; i++) {
		res = (double)inter_cnt[i] / (double)(jac_len[i] + cnt - inter_cnt[i]);
		if(res >= threshold)
			result.push_back(JaccardJoinResult(id_query, i, res));
	}
	
	sort(result.begin() + last_result, result.end(), compareJoinResultJaccard);
	return SUCCESS;
}

int SimSearcher::ED(int k) {
	int i, j, res = 0xfffffff;
	short t = 0;

	for(i = 1; i <= qlen; i++) {
		res = 0xfffffff;
		for(j = 1; j <= ch_len[k]; j++) {
			if(query[i-1] == dataset[k][j-1]) t = 0; else t = 1;
			f[i][j] = min(f[i-1][j] + 1, f[i][j-1] + 1, f[i-1][j-1] + t);
			if(res > f[i][j]) res = f[i][j];
		}

		if(res > threshold)
			return -1;
	}

	return f[qlen][ch_len[k]];
}

int SimSearcher::bruteForce(vector<EDJoinResult> &result) {
	int res, k;
	int last_result = result.size();
	for(k = 0; k < record_num; k++) {
		if(ch_len[k] > threshold + qlen || ch_len[k] < qlen - threshold)
			continue;
		res = ED(k);
		if(res >= 0 and res <= threshold) result.push_back(EDJoinResult(id_query, k, res));
	}
	sort(result.begin() + last_result, result.end(), compareJoinResultED);
	return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<EDJoinResult> &result)
{
	int temp;
	int k, i, j, l;
	int res;
	string key;
	int last_result = result.size();
	this->query = query; this->threshold = threshold; qlen = strlen(query);
	l = qlen - QLEN + 1;
	T = qlen - QLEN + 1 - threshold * QLEN;
	L = 0.95 * T;

	if(T <= 0) return bruteForce(result);

	candidate.clear();

	// sort the q-gram of query
	query_gram = new SimplePair[l];
	for(i = 0; i < l; i ++) {
		key = string(query + i, QLEN);
		query_gram[i].name = key;
		query_gram[i].count = inv_qram[key].size(); 
	}

	sort(query_gram, query_gram + l, compare_simplepair);

	// scan in the l - L short lists for T - L occurrance
	for(i = L; i < l; i ++) {
		auto iter = inv_qram.find(query_gram[i].name);
		if(iter == inv_qram.end()) continue;
		for(auto jter = iter->second.begin(); jter != iter->second.end(); jter++) {
			temp = *jter;
			// 0 for succeeded; length filter
			if(ch_len[temp] > (int)threshold + qlen || ch_len[temp] < qlen - (int)threshold) continue;
			scan_count[temp] ++;
			if(scan_count[temp] == T - L) {
				candidate.push_back(temp);
			}
		}
	}

	// search each in long lists
	for(auto iter = candidate.begin(); iter != candidate.end(); iter ++) {
		temp = *iter;
		k = scan_count[temp];

		for(i = 0; i < L; i++) {
			auto jter = inv_qram.find(query_gram[i].name);
			auto kter = lower_bound(jter->second.begin(), jter->second.end(), temp);
			
			// in case that nothing is lower than temp
			if(kter != jter->second.end()) res = *kter;
			else res = -1;

			if(res == temp) {
				k++;
				// successful
				if(k >= T) break;
			} else if(k + L - i - 1 < T) {
				// failed
				if (ISDISPLAY) printf("Erase: %s k:%d L:%d i:%d S:%d T:%d\n", dataset[temp].c_str(), k, L, i, k + L - i - 1, T);
				candidate.erase(iter++);
				break;
			}
		}

	}

	// verify
	for(auto iter = candidate.begin(); iter != candidate.end(); iter ++) {
		res = ED(*iter);

		if(res >= 0) {
			if(res <= threshold) result.push_back(EDJoinResult(id_query, *iter, res));
		}
	}

	// reset scan_count
	for(i = L; i < l; i ++) {
		auto iter = inv_qram.find(query_gram[i].name);
		if(iter == inv_qram.end()) continue;
		for(auto jter = iter->second.begin(); jter != iter->second.end(); jter++)
			scan_count[*jter] = 0;
	}
	
	delete [] query_gram;

	sort(result.begin() + last_result, result.end(), compareJoinResultED);

	return SUCCESS;
}

