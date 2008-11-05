/*
 * Copyright (C) 2008 Kamil Dudka <xdudka00@stud.fit.vutbr.cz>
 *
 * This file is part of fss (Fast SAT Solver).
 *
 * fss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * fss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fss.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    // Initialize random generator seed
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
#if 0//ndef NDEBUG
      std::cerr << "RandGenerator::RandGenerator(): seed=" << seed << std::endl;
#endif // NDEBUG
    }
    // Return random number in interval <0,1)
    float getRand() {
      float rnd = rand_r(&seed);
      return rnd/RAND_MAX;
    }
    // Return random float in interval <min,max)
    float getRand(float min, float max) {
      float rnd = this->getRand();
      rnd *= max-min;
      rnd += min;
      return rnd;
    }
    // Return random int in interval <min,max)
    int getRand(int min, int max) {
      float rnd = this->getRand(
          static_cast<float>(min),
          static_cast<float>(max)
          );
      return static_cast<int>(rnd);
    }
  private:
    unsigned int seed;
};

/**
 * USAGE:
 * ./fss-satgen VARCOUNT FORMSCOUNT
 */
int main(int argc, char *argv[]) {
  if (argc<3) {
    std::cerr << "Usage: fss-satgen VARCOUNT FORMSCOUNT" << std::endl;
    return -1;
  }

  const int VARCOUNT = atoi(argv[1]);
  const int FORMSCOUNT = atoi(argv[2]);
  if (0==VARCOUNT || 0==FORMSCOUNT) {
    std::cerr << "Usage: fss-satgen VARCOUNT FORMSCOUNT" << std::endl;
    return -1;
  }

  // Random numbers generator
  RandGenerator rnd;

  // Variable names
  vector<string> varNames(VARCOUNT);
  for(int i=0; i<VARCOUNT; i++) {
    ostringstream stream;
    stream << "a" << i;
    varNames[i] = stream.str();
  }

  // Binary operators
  vector<string> binopSet;
  binopSet.push_back("AND");
  binopSet.push_back("OR");
  binopSet.push_back("XOR");

  // FA control state
  enum EState {
    S_EXPR,
    S_BINOP
  } state = S_EXPR;

  // count of opened parenthesis
  int lParCount = 0;

  // Generator main loop
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
        std::cout << varNames[rnd.getRand(0, VARCOUNT)] << std::flush;
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
        std::cout << " " << binopSet[rnd.getRand(0, binopSet.size())] << " " << std::flush;
        state = S_EXPR;
        break;
    }
  }
  // new line at end of file generated
  std::cout << std::endl;
}
