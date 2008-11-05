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

#include <iostream>
#include <iomanip>
#include <string>
#include <ga/GAParameter.h>
#include <ga/GAStatistics.h>
#include "fssIO.h"
#include "SatProblem.h"
#include "BlindSatSolver.h"
#include "GaSatSolver.h"
#include "SatSolverObserver.h"

using std::string;
using namespace FastSatSolver;
using namespace FastSatSolver::StreamDecorator;

// Create observer OBSERVER and attach it to subject
template <
      class OBSERVER,
      class SUBJECT,
      class ARG>
inline OBSERVER* createAttached(SUBJECT *subject, ARG &arg) {
      OBSERVER *observer= new OBSERVER(subject, arg);
      subject->addObserver(observer);
      return observer;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr <<
      "Usage\n"
      "=====\n"
      "fss NAME VALUE [NAME VALUE [NAME VALUE [...]]]\n"
      "where NAME is name of parameter and VALUE its value\n"
      "\n"
      "Possible parameters are (abbreviation in parenthesis)\n"
      "=====================================================\n"
      "input_file(input)............... File containing SAT problem specification.\n"
      "                                 '-' means standard input.\n"
      "color_output(color)............. 1/0 turns on/off console colored output.\n"
      "verbose_mode(verbose)........... 1/0 turns on/off GAlib verbose mode.\n"
      "blind_solver(blind)............. Switch between blind and GA solver.\n"
      "                                 1 means blind solver,\n"
      "                                 0 means GA solver(default).\n"
      "step_width(stepw)............... (only for blind solver) granularity of solver's\n"
      "                                 notifications and control. Default is 16.\n"
      "min_count_of_solutions(minslns). Minimal count of solutions requested.\n"
      "max_count_of_solutions(maxslns). Maximal count of solutions to look for.\n"
      "max_count_of_runs(maxruns)...... GA is restarted for max. maxruns times if\n"
      "                                 not all minslns solutions are found.\n"
      "max_time_per_run(maxtime)....... Run is be stopped unconditionally if maxtime\n"
      "                                 time is exceed (in miliseconds).\n"
      "term_upon_convergence(convterm). 0 -> Run is be stopped after ngen generations.\n"
      "                                 1 -> Run is be stopped upon convergence.\n"
      "\n"
      "NOTE\n"
      "====\n"
      "There are more parameters for GA solver. Consider GAlib documentation,\n"
      "especially its class GASimpleGA.\n";
    return 1;
  }
  int exitCode = 0;

  // Used for final clean-up on exit
  SatProblem          *satProblem = 0;
  AbstractSatSolver   *satSolver = 0;
  ProgressWatch       *progressWatch = 0;
  SolutionsCountStop  *slnsCountStop = 0;
  TimedStop           *timedStop = 0;
  FitnessWatch        *fitnessWatch = 0;
  ResultsWatch        *resultsWatch = 0;
  SatItemVector       *results = 0;
  try {
    // Parse cmd-line parameters
    GAParameterList params;
    // GaSatSolver-specific parameters
    GaSatSolver::registerDefaultParameters(params);

    // Default values of parameters
    const char DEF_INPUT_FILE[] = "";
    const GABoolean DEF_VERBOSE_MODE = gaFalse;
    const GABoolean DEF_COLOR_OUTPUT = gaFalse;
    const GABoolean DEF_BLIND_SOLVER = gaFalse;
    const int DEF_MIN_COUNT_OF_SOLUTIONS =  1;
    const int DEF_MAX_COUNT_OF_SOLUTIONS =  8;
    const int DEF_MAX_COUNT_OF_RUNS =       8;
    const int DEF_MAX_TIME_PER_RUN =        0;
    const int DEF_STEP_WIDTH =              16;

    // Register extra parameters
    params.add("verbose_mode",            "verbose",  GAParameter::BOOLEAN,     &DEF_VERBOSE_MODE);
    params.add("color_output",            "color",    GAParameter::BOOLEAN,     &DEF_COLOR_OUTPUT);
    params.add("blind_solver",            "blind",    GAParameter::BOOLEAN,     &DEF_BLIND_SOLVER);
    params.add("input_file",              "input",    GAParameter::STRING,      &DEF_INPUT_FILE);
    params.add("min_count_of_solutions",  "minslns",  GAParameter::INT,         &DEF_MIN_COUNT_OF_SOLUTIONS);
    params.add("max_count_of_solutions",  "maxslns",  GAParameter::INT,         &DEF_MAX_COUNT_OF_SOLUTIONS);
    params.add("max_count_of_runs",       "maxruns",  GAParameter::INT,         &DEF_MAX_COUNT_OF_RUNS);
    params.add("max_time_per_run",        "maxtime",  GAParameter::INT,         &DEF_MAX_TIME_PER_RUN);
    params.add("step_width",              "stepw",    GAParameter::INT,         &DEF_STEP_WIDTH);

    // parse using GAParameterList class
    params.parse(argc, argv, gaTrue);

    // true for blind solver, false for GA solver
    GABoolean useBlindSolver= DEF_BLIND_SOLVER;
    params.get("blind_solver", &useBlindSolver);

    // turn on/off color output (using escape squences)
    GABoolean useColorOutput= DEF_COLOR_OUTPUT;
    params.get("color_output", &useColorOutput);
    Color::enable(useColorOutput);

    // turn on/off GAlib verbose mode
    GABoolean verboseMode= DEF_VERBOSE_MODE;
    params.get("verbose_mode", &verboseMode);

    // Read input file name
    const char *szFileName=
      static_cast<const char *>
      (params("input_file")->value());
    if (0==szFileName)
      szFileName = DEF_INPUT_FILE;

    // Minimum of solutions (to declare as solution)
    int minSlns= DEF_MIN_COUNT_OF_SOLUTIONS;
    params.get("min_count_of_solutions", &minSlns);
    if (minSlns <= 0) {
      printError("min_count_of_solutions out of range, using default");
      minSlns = DEF_MIN_COUNT_OF_SOLUTIONS;
    }

    // Maximum number of solutions (solver stop when reached)
    int maxSlns= DEF_MAX_COUNT_OF_SOLUTIONS;
    params.get("max_count_of_solutions", &maxSlns);
    if (maxSlns <= 0) {
      printError("max_count_of_solutions out of range, using default");
      maxSlns = DEF_MAX_COUNT_OF_SOLUTIONS;
    }
    if (maxSlns < minSlns)
      maxSlns = minSlns;


    // Maximum nuber of runs (if satisfaction is not reachable)
    int maxRuns= DEF_MAX_COUNT_OF_RUNS;
    params.get("max_count_of_runs", &maxRuns);
    if (maxRuns <= 0) {
      printError("max_count_of_runs out of range, using default");
      maxRuns = DEF_MAX_COUNT_OF_RUNS;
    }

    // Maximum time elapsed for one run, 0 means infinity
    int maxTime= DEF_MAX_TIME_PER_RUN;
    params.get("max_time_per_run", &maxTime);
    if (maxTime < 0) {
      printError("max_time_per_run out of range, using default");
      maxTime = DEF_MAX_TIME_PER_RUN;
    }

    // Step width (only for blind solver)
    int stepWidth= DEF_STEP_WIDTH;
    params.get("step_width", &stepWidth);
    if (stepWidth <= 0) {
      printError("step_width out of range, using default");
      stepWidth = DEF_STEP_WIDTH;
    }

    if (useBlindSolver) {
      // exclude parameters for blind solver
      if (maxRuns != DEF_MAX_COUNT_OF_RUNS) {
        printError("Parameter 'max_count_of_runs' is irrelevant for blind solver");
      }
      maxRuns = 1;
    } else {
      // exclude parameters for GA solver
      if (stepWidth != DEF_STEP_WIDTH) {
        printError("Parameter 'step_width' is irrelevant for GA solver");
        stepWidth = DEF_STEP_WIDTH;
      }
    }

    if (verboseMode)
      std::cout << Color(C_CYAN) << params << Color() << std::endl;

    // Create SAT problem instace
    satProblem = new SatProblem;

    // Read input data
    static const char INPUT_STDIN[] = "-";
    if (0==strcmp(szFileName, INPUT_STDIN))
      satProblem->loadFromInput();
    else
      satProblem->loadFromFile(szFileName);
    if (satProblem->hasError())
      throw GenericException("Invalid input data");

    // Write out compilation statistics
    const int varsCount = satProblem->getVarsCount();
    std::cout << Color(C_YELLOW) << "--- Formulas count: " << Color(C_RED) << satProblem->getFormulasCount() << std::endl;
    std::cout << Color(C_YELLOW) << "--- Variables count: " << Color(C_RED) << varsCount << std::endl;
    std::cout << Color(C_YELLOW) << "--- Variables: " << Color();
    for(int i=0; i< varsCount; i++) {
      std::cout  << Color(C_RED) << satProblem->getVarName(i) << Color();
      if (i==varsCount-1)
        std::cout << Color() << std::endl;
      else
        std::cout << ", ";
    }

    // create desired solver
    if (useBlindSolver) {

      // create blind solver
      satSolver = new BlindSatSolver(satProblem, stepWidth);
      std::cout << Color(C_LIGHT_BLUE) << ">>> Using blind solver" << Color() << std::endl;

      // attach progress indicator
      const int progressBits = satProblem->getVarsCount()-stepWidth;
      if (progressBits > 0) {
        progressWatch = new ProgressWatch(satSolver, 1<<progressBits, std::cout);
        satSolver->addObserver(progressWatch);
      }
    } else {

      // create GA solver
      satSolver = GaSatSolver::create(satProblem, params);
      std::cout << Color(C_LIGHT_BLUE) << ">>> Using GAlib solver" << Color() << std::endl;
    }

    // Display message if maxFitness is increased
    fitnessWatch = createAttached<FitnessWatch>(satSolver, std::cout);

    // Display message if solution is discovered
    resultsWatch = createAttached<ResultsWatch>(satSolver, std::cout);

    // Stop progress after maxSlns solutions are found
    slnsCountStop = createAttached<SolutionsCountStop>(satSolver, maxSlns);

    if (maxTime)
      // Run will be stopped if its time exceeds
      timedStop = createAttached<TimedStop>(satSolver, maxTime);

    int totalSolutions = 0;
    float timeTotal = 0.0;
    for(int i=0; i<maxRuns; i++) {
      if (1<maxRuns)
        std::cout << Color(C_GREEN) << ">>> Run" << std::setw(4) << i+1 << " of" << std::setw(4) << maxRuns << Color() << std::endl;

      // Initialization
      satSolver->reset();
      fitnessWatch->reset();

      // Start progress
      satSolver->start();

      // Fetch progresse's results
      delete results;
      results= satSolver->getSolutionVector();
      const int runSolutions= results->getLength() - totalSolutions;
      totalSolutions+= runSolutions;
      const float timeElapsed= satSolver->getTimeElapsed()/1000.0;
      timeTotal+= timeElapsed;
      std::cout
        << Color(C_GREEN) << "<<< Found" << std::setw(5) << runSolutions << " solutions"
        << " in " << FixedFloat(3,2) << timeElapsed << " s" << Color() << std::endl;
      if (1<maxRuns) std::cout
        << Color(C_RED) << "<<< Total" << std::setw(5) << totalSolutions << " solutions"
        << " in " << FixedFloat(3,2) << timeTotal << " s" << Color() << std::endl;
      std::cout << std::endl;

      if (totalSolutions >= minSlns)
        // minSlns reached, cancel rest of runs
        break;
    }
    std::cout << Color(C_LIGHT_BLUE);
    results->writeOut(satSolver->getProblem(), std::cout);
    std::cout << Color() << std::endl;

    if (verboseMode && !useBlindSolver) {
      GaSatSolver *gaSolver= dynamic_cast<GaSatSolver *>(satSolver);
      GAStatistics stats= gaSolver->getStatistics();
      std::cout << std::endl << Color(C_CYAN) << stats << Color() << std::endl;
    }
  }
  catch (GenericException e) {
    printError(e.getText());
    exitCode = 1;
  }
  // Final clean-up
  delete results;
  delete resultsWatch;
  delete fitnessWatch;
  delete timedStop;
  delete slnsCountStop;
  delete progressWatch;
  delete satSolver;
  delete satProblem;

  return exitCode;
}

