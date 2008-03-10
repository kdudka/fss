#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

class RandGenerator {
  public:
    RandGenerator() {
      // Get system clock
      struct timeval tv;
      if (0 != gettimeofday(&tv, NULL))
        // Failed to read system time
        throw std::bad_alloc();

      // Convert to microseconds
      int clk = tv.tv_usec;

      // Initialize seed
      this->seed = clk * getpid();
#ifndef NDEBUG
      std::cerr << "RandGenerator::RandGenerator(): seed=" << seed << std::endl;
#endif // NDEBUG
    }
    float getRand() {
      float rnd = rand_r(&seed);
      return rnd/RAND_MAX;
    }
    float getRand(float min, float max) {
      float rnd = this->getRand();
      rnd *= max-min;
      rnd += min;
      return rnd;
    }
    int getRand(int min, int max) {
      float rnd = this->getRand(
          static_cast<float>(min),
          static_cast<float>(max+1)
          );
      return static_cast<int>(rnd);
    }
  private:
    unsigned int seed;
};

int main(int argc, char *argv[]) {
  if (argc<3)
    return -1;

  const int VARCOUNT = atoi(argv[1]);
  const int FORMSCOUNT = atoi(argv[2]);
  if (0==VARCOUNT || 0==FORMSCOUNT)
    return -1;

  RandGenerator rnd;
  vector<string> varNames(VARCOUNT);
  for(int i=0; i<VARCOUNT; i++) {
    ostringstream stream;
    stream << "a" << i;
    varNames[i] = stream.str();
  }
  vector<string> binopSet;
  binopSet.push_back("AND");
  binopSet.push_back("OR");
  binopSet.push_back("XOR");

  enum EState {
    S_EXPR,
    S_BINOP
  } state = S_EXPR;
  int lParCount = 0;
  for (int fCount=0; fCount<FORMSCOUNT;) {
    switch (state) {
      case S_EXPR:
        // Unary NOT
        if (rnd.getRand() < 0.3) {
          std::cout << "~" << std::flush;
          break;
        }
        // Open a pair of brackets
        if (rnd.getRand(0.1f*lParCount, 1.0f) < 0.5) {
          std::cout << "(" << std::flush;
          lParCount++;
          break;
        }
        // Random variable
        std::cout << varNames[rnd.getRand(0, VARCOUNT-1)] << std::flush;
        state = S_BINOP;
        break;

      case S_BINOP:
        if (rnd.getRand() < 0.1) {
          // Close formula
          for(; lParCount; lParCount--)
            std::cout << ")";
          std::cout << ";" << std::endl;
          fCount++;
          state = S_EXPR;
          break;
        }
        if (rnd.getRand() < 0.05*lParCount) {
          // Close a pair of brackets
          std::cout << ")" << std::flush;
          lParCount--;
          break;
        }
        // Random binary operator
        std::cout << " " << binopSet[rnd.getRand(0, binopSet.size()-1)] << " " << std::flush;
        state = S_EXPR;
        break;
    }
  }
  std::cout << std::endl;
}
