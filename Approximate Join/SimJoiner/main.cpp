#include "SimJoiner.h"

using namespace std;

int main(int argc, char **argv) {
    SimJoiner joiner;

    vector<EDJoinResult> resultED;
    vector<JaccardJoinResult> resultJaccard;

    unsigned edThreshold = 2;
    double jaccardThreshold = 0.85;

    printf("Jaccard:\n");
    joiner.joinJaccard(argv[1], argv[2], jaccardThreshold, resultJaccard);
    printf("ED:\n");
    joiner.joinED(argv[1], argv[2], edThreshold, resultED);

    return 0;
}
