#include "SimJoiner.h"
#include <iostream>
using namespace std;

inline int min(int a, int b, int c){
	return (a > b) ? ((b > c) ? c : b) : ((a > c) ? c : a);
}

SimJoiner::SimJoiner() { 
    use_str_search = false;

    DEBUG = false;

    buf = new char[1024];
    inter_cnt = new int[200001];
	scan_count = new int [200001];

	for(int j = 0; j < MAXLEN; j++) f[0][j] = j;
	for(int j = 0; j < MAXLEN; j++) f[j][0] = j;
}

SimJoiner::~SimJoiner() {
    delete [] buf, inter_cnt, scan_count;
}

void SimJoiner::printResult(const vector<JaccardJoinResult> &result) {
    for(auto iter = result.begin(); iter != result.end(); iter ++ ) {
        printf("%d. %s\t|\t%d. %s\t|\t%.4f\n", iter->id1, dataset_query[iter->id1].c_str(), iter->id2, dataset_ref[iter->id2].c_str(), iter->s);
    }
}

void SimJoiner::printResult(const vector<EDJoinResult> &result) {
    for(auto iter = result.begin(); iter != result.end(); iter ++ ) {
        printf("%d. %s\t|\t%d. %s\t|\t%d\n", iter->id1, dataset_query[iter->id1].c_str(), iter->id2, dataset_ref[iter->id2].c_str(), iter->s);
    }
}

int SimJoiner::ED(int k) {
	int i, j, res = 0xfffffff;
	short t = 0;

	for(i = 1; i <= qlen; i++) {
		res = 0xfffffff;
		for(j = 1; j <= ch_len[k]; j++) {
			if(query[i-1] == dataset_ref[k][j-1]) t = 0; else t = 1;
			f[i][j] = min(f[i-1][j] + 1, f[i][j-1] + 1, f[i-1][j-1] + t);
			if(res > f[i][j]) res = f[i][j];
		}

		if(res > TE)
			return -1;
	}

	res = f[qlen][ch_len[k]];

	return res;
}

int SimJoiner::createRefDatasetED(const char *filename) {
    int size = 0, i = 0, j = 0, last = 0, cnt = 0;
	vector<int> *iter = NULL;
	string key, qgram;
	size_t n = 260;
	FILE* fin = fopen(filename, "r");

    dataset_ref.clear();
    ch_len.clear();

	for(i = 0;; i++) {
		size = getline(&buf, &n, fin) - 1;
		if(size <= 0) break;

		if(buf[size] == '\n') buf[size] = '\0';
		else size++;
		
		key = buf;
		// record dataset
		dataset_ref.push_back(key);
		// record length
		ch_len.push_back(key.length());

		// split the sentence
		last = cnt = 0;
		for(j = 0; j <= size; j ++) {
			// Split q-gram
			if(j <= size - (int)QGRAM_LEN) {
				qgram = string(buf + j, QGRAM_LEN);

				iter = inv_qram->findp(qgram);
				if(iter == &inv_qram->ValueNULL) {
					// new gram
					inv_qram->insert(qgram, vector<int>{i});
				} else if((*iter)[iter->size()-1] < i) {
					// ignore same gram
					inv_qram->find(qgram).push_back(i);
				}
			}
		}

		// record word number
		jac_len.push_back(cnt);
	}
	//printf("DONE INDEX\n");
	record_num = i;
	return SUCCESS;
}

int SimJoiner::createRefDatasetJaccard(const char *filename) {
	int size = 0, i = 0, j = 0, last = 0, cnt = 0;
	string key, qgram;
	size_t n = 260;
	FILE* fin = fopen(filename, "r");

	vector<int> *iter = NULL;
    dataset_ref.clear();
    ch_len.clear();
    jac_len.clear();

	for(i = 0;; i++) {
		size = getline(&buf, &n, fin) - 1;
		if(size <= 0) break;

		if(buf[size] == '\n') buf[size] = '\0';
		else size++;
		
		key = buf;

		// record dataset
		dataset_ref.push_back(key);
		// record length
		ch_len.push_back(size);

		// split the sentence
		last = cnt = 0;
		for(j = 0; j <= size; j ++) {
			if(buf[j] == ' ' || j == size) {				
				if (j == size) buf[j+1] = '\0';
				else buf[j] = '\0';
				key = buf + last;
				iter = inv_list_ref->findp(key);
				if(iter == NULL) {printf("NULL HASH.\n"); exit(0);}
				if(iter == &inv_list_ref->ValueNULL) {
					// new word
					cnt ++;
					inv_list_ref->insert(key, vector<int>{i});
				} else if((*iter)[iter->size()-1] < i) {
					iter->push_back(i);
					cnt ++;
				}
				last = j+1;
			}
		}

		// record word number
		jac_len.push_back(cnt);
	}

    record_num = i;
	return SUCCESS;
}

int SimJoiner::searchJaccard(const char *query, vector<JaccardJoinResult> &result) {
	int last, i, j, size = buflen - 1, cnt = 0;
	//inv_list_ref
	vector<int> *iter = NULL;
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
			
			if (visited.find(key) != visited.end())
				continue;

			visited[key] = true;
			cnt ++;
			
			iter = inv_list_ref->findp(key);
			if (iter == &inv_list_ref->ValueNULL)
				continue;
			for(auto jter = iter->begin(); jter != iter->end(); jter++)
				inter_cnt[*jter] ++;
		}
	}

	for(i = 0; i < record_num; i++) {
		res = (double)inter_cnt[i] / (double)(jac_len[i] + cnt - inter_cnt[i]);
		if(res >= TJ)
			result.push_back(JaccardJoinResult(id_query, i, res));
	}
	
	sort(result.begin() + last_result, result.end(), compareJoinResultJaccard);
	return SUCCESS;
}

int SimJoiner::bruteForce(vector<EDJoinResult> &result) {
	int res, k;
	int last_result = result.size();
	for(k = 0; k < record_num; k++) {
		if(ch_len[k] > TE + qlen || ch_len[k] < qlen - TE)
			continue;
		res = ED(k);
		if(res >= 0 and res <= TE) result.push_back(EDJoinResult(id_query, k, res));
	}
	sort(result.begin() + last_result, result.end(), compareJoinResultED);
	return SUCCESS;
}

int SimJoiner::searchED(const char *query, std::vector<EDJoinResult> &result) {
	int temp;
	int k, i, j, l;
	int res;
	string key;
	int last_result = result.size();
	this->query = query;
	qlen = buflen - 1;
	l = qlen - QGRAM_LEN + 1;
	T = qlen - QGRAM_LEN + 1 - TE * QGRAM_LEN;
	L = 0.95 * T;
	//cout << qlen << " " << T << endl;
	if(T <= 0) return bruteForce(result);

	candidate.clear();

	// sort the q-gram of query
	query_gram = new SimplePair[l];
	for(i = 0; i < l; i ++) {
		key = string(query + i, QGRAM_LEN);
		query_gram[i].name = key;
		query_gram[i].count = inv_qram->findp(key)->size(); 
	}

	sort(query_gram, query_gram + l, compare_simplepair);
//printf("INV QRAM DONE\n");
	// scan in the l - L short lists for T - L occurrance
	for(i = L; i < l; i ++) {
		auto iter = inv_qram->findp(query_gram[i].name);
		if(iter == &inv_qram->ValueNULL) continue;
		for(auto jter = iter->begin(); jter != iter->end(); jter++) {
			temp = *jter;
			// 0 for succeeded; length filter
			if(ch_len[temp] > (int)TE + qlen || ch_len[temp] < qlen - (int)TE) continue;
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
			auto jter = inv_qram->findp(query_gram[i].name);
			auto kter = lower_bound(jter->begin(), jter->end(), temp);
			
			// in case that nothing is lower than temp
			if(kter != jter->end()) res = *kter;
			else res = -1;

			if(res == temp) {
				k++;
				// successful
				if(k >= T) break;
			} else if(k + L - i - 1 < T) {
				// failed
				candidate.erase(iter++);
				break;
			}
		}

	}

	// verify
	for(auto iter = candidate.begin(); iter != candidate.end(); iter ++) {
		res = ED(*iter);

		if(res >= 0) {
			if(res <= TE) result.push_back(EDJoinResult(id_query, *iter, res));
		}
	}

	// reset scan_count
	for(i = L; i < l; i ++) {
		auto iter = inv_qram->findp(query_gram[i].name);
		if(iter == &inv_qram->ValueNULL) continue;
		for(auto jter = iter->begin(); jter != iter->end(); jter++)
			scan_count[*jter] = 0;
	}
	
	delete [] query_gram;

	sort(result.begin() + last_result, result.end(), compareJoinResultED);

	return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    int i, j, last;
    size_t n;
    FILE* f = fopen(filename1, "r");

    result.clear();
    dataset_query.clear();

    if(false) {
        str_searcher = new SimSearcher();

        str_searcher->createIndex(filename2, QGRAM_LEN);
        for (i = 0; (buflen=getline(&buf, &n, f)) > 0; i++)
        {
            buf[buflen-1]='\0';
            dataset_query.push_back(buf);
            last = result.size();
            str_searcher->id_query = i;
            str_searcher->searchED(buf, threshold, result);
        }

        if (DEBUG) {
            printResult(result);
        }

        delete str_searcher;
		return SUCCESS;
    }

	inv_qram = new HashMap<string, vector<int>, HashFunc, EqualKey>(HASH_SIZE);
    createRefDatasetED(filename2);
	TE = threshold;
    for (i = 0; (buflen=getline(&buf, &n, f)) > 0; i++)
    {
        buf[buflen-1]='\0';
        dataset_query.push_back(buf);
        last = result.size();
        id_query = i;
        searchED(buf, result);
    }

	if (DEBUG) {
		printResult(result);
	}
	delete inv_qram;
    return SUCCESS;
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    int i, j, last;
    size_t n;
    FILE* f = fopen(filename1, "r");

    result.clear();
    dataset_query.clear();

    if(false) {
        str_searcher = new SimSearcher();

        str_searcher->createIndex(filename2, 0xfffff);
        for (i = 0; (buflen=getline(&buf, &n, f)) > 0; i++)
        {
            buf[buflen-1]='\0';
            dataset_query.push_back(buf);
            last = result.size();
            str_searcher->id_query = i;
            str_searcher->searchJaccard(buf, threshold, result);
        }
        
        if (DEBUG) {
            printResult(result);
        }

        delete str_searcher;
        return SUCCESS;
    }

	inv_list_ref = new HashMap<string, vector<int>, HashFunc, EqualKey>(HASH_SIZE);
    createRefDatasetJaccard(filename2);
    TJ = threshold;
    for (i = 0; (buflen=getline(&buf, &n, f)) > 0; i++)
    {
        buf[buflen-1]='\0';
        dataset_query.push_back(buf);
        last = result.size();
        id_query = i;
        searchJaccard(buf, result);
    }

    if (DEBUG) {
        printResult(result);
    }

	delete inv_list_ref;
    return SUCCESS;
}