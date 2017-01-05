#include<iostream>
#include <string>
#include<fstream>
#include <sys/time.h>
#include<stdlib.h>
#include "instance.h"
#include "ordering.h"
#include "localsearch.h"
#include "debug.h"
#include "resultregister.h"
#include <unistd.h>
#include "util.h"
#include "types.h"
#include "math.h"

int main(int argc, char* argv[]) {
  std::string method = argv[1];
  std::string instanceFile = argv[2];
  float cutoffTime = atof(argv[4]);
  int cutoffLength = atoi(argv[5]);
  int seed = atoi(argv[6]);
  seed = seed == -1 ? time(NULL) : seed;
  ResultRegister rr;
  srand(seed);
  std::ifstream expIn(instanceFile);
  std::string fileName;
  Types::Score opt;
  expIn >> fileName >> opt;
  Instance instance(fileName);
  LocalSearch localSearch(instance);
  rr.setOrigin();
  int n = instance.getN();
  if( method == "ILS") {
    int greediness = 10;
    Types::Score bestKnown = std::stoll(argv[3], NULL, 0);
    DBG("Best Known: " << bestKnown);
    int perturb = 4;
    int soft = 10;
    int hard = 150;
    float updateTolerance = 0;
    float perturbFactor = 0.2;
    for (int i = 7; i < 19 && i < argc; i += 2) {
      std::string param(argv[i]);
      DBG(argv[i]);
      if (param == "-greediness") {
        greediness = atoi(argv[i+1]);
      } else if (param == "-perturb") {
        perturb = atoi(argv[i+1]);
      } else if (param == "-soft") {
        soft = atoi(argv[i+1]);
      } else if (param == "-hard") {
        hard = atoi(argv[i+1]);
      } else if (param == "-update") {
        updateTolerance = atof(argv[i+1]);
      } else if (param == "-perturbfactor") {
        perturbFactor = atof(argv[i+1]);
        perturb = ceil(n*perturbFactor);
      }
    }
    DBG("Instance: " << instanceFile << "Cutoff time: " << cutoffTime << " seed " << seed << " soft " << soft);
    rr.set();
    SearchResult sr = localSearch.ILSWithNRestarts(cutoffTime, greediness, hard, soft, perturb, updateTolerance, rr, opt);
    localSearch.checkSolution(sr.getOrdering());
    if (Util::isOpt(sr, opt)) {
      std::cout << "Result for ParamILS: SAT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    } else if (bestKnown == 0) {
      std::cout << "Result for ParamILS: TIMEOUT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    } else {
      double approx = (double)1 - (double) bestKnown / (double) sr.getScore();
      std::cout << "Result for ParamILS: TIMEOUT, " << rr.check() << ", " << 0 << ", " << approx << ", " << seed << std::endl;
    }
    rr.dump("ILS_SEED_" + std::to_string(seed) + "_"  + std::to_string(time(NULL)), instanceFile, argc, argv);
  } else if (method == "SA") {
    double initTemp = 1000000000;
    int numSteps = 10000;
    float decayFactor = 0.98;
    Neighbours neighbour = Neighbours::SWAP;
    for (int i = 7; i < 15 && i < argc; i += 2) {
      std::string param(argv[i]);
      DBG(argv[i]);
      if (param == "-inittemp") {
        initTemp = atof(argv[i+1]);
      } else if (param == "-steps") {
        numSteps = atoi(argv[i+1]);
      } else if (param == "-decay") {
        decayFactor = atof(argv[i+1]);
      } else if (param == "-neighbour") {
        std::string neighbourInput = argv[i+1];
        neighbour = neighbourInput == "INSERT" ? Neighbours::INSERT : Neighbours::SWAP;
      }
    }
    SearchResult sr = localSearch.simulatedAnnealing(initTemp, numSteps, decayFactor, cutoffTime, opt, neighbour, rr);
    if (Util::isOpt(sr, opt)) {
      std::cout << "Result for ParamILS: SAT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    } else {
      std::cout << "Result for ParamILS: TIMEOUT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    }
  } else if (method == "TABU") {
    int listSize = 20;
    int softRestart = 20;
    for (int i = 7; i < 11 && i < argc; i += 2) {
      std::string param(argv[i]);
      if (param == "-listsize") {
        listSize = atoi(argv[i+1]);
      } else if (param == "-soft") {
        softRestart = atoi(argv[i+1]);
      }
    }
    SearchResult sr = localSearch.tabuSearchWithNRestarts(cutoffTime, listSize, softRestart, rr, opt);
    if (Util::isOpt(sr, opt)) {
      std::cout << "Result for ParamILS: SAT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    } else {
      std::cout << "Result for ParamILS: TIMEOUT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    }
  } else if (method == "KOLLER") {
    int listSize = instance.getN()*2;
    for (int i = 7; i < 9 && i < argc; i += 2) {
      std::string param(argv[i]);
      if (param == "-listsize") {
        listSize = atoi(argv[i+1]);
      }
    }
    SearchResult sr = localSearch.kollerSearchRestarts(listSize, cutoffTime, opt, rr);
    localSearch.checkSolution(sr.getOrdering());
    if (Util::isOpt(sr, opt)) {
      std::cout << "Result for ParamILS: SAT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    } else {
      std::cout << "Result for ParamILS: TIMEOUT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    }
    rr.dump("KOLLER_SEED_" + std::to_string(seed) + "_"  + std::to_string(time(NULL)), instanceFile, argc, argv);
  } else if (method == "KOLLERV2") {
    int listSize = 10;
    for (int i = 7; i < 9 && i < argc; i += 2) {
      std::string param(argv[i]);
      if (param == "-listsize") {
        listSize = atoi(argv[i+1]);
      }
    }
    SearchResult sr = localSearch.kollerSearchRestarts(listSize, cutoffTime, opt, rr);
    localSearch.checkSolution(sr.getOrdering());
    if (Util::isOpt(sr, opt)) {
      std::cout << "Result for ParamILS: SAT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    } else {
      std::cout << "Result for ParamILS: TIMEOUT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    }
  } else if (method == "MEMETIC") {
    Types::Score bestKnown = std::stoll(argv[3], NULL, 0);
    DBG("Best Known: " << bestKnown);
    int initPopulationSize = 20;
    int numCrossovers = 10;
    int numMutations = 3;
    int mutationPower = 3;
    int divLookahead = 10;
    int numKeep = 2;
    float divTolerance = 0.001;
    int greediness = 32;
    CrossoverType crossoverType = CrossoverType::OB;
    for (int i = 7; i < 27 && i < argc; i++) {
      std::string param(argv[i]);
      DBG(argv[i]);
      if (param == "-populationsize") {
        initPopulationSize = atoi(argv[i+1]);
      } else if (param == "-crossover") {
        numCrossovers = atoi(argv[i+1]);
      } else if (param == "-nummutation") {
        numMutations = atoi(argv[i+1]);
      } else if (param == "-mutationpower") {
        mutationPower = atoi(argv[i+1]);
      } else if (param == "-divlookahead") {
        divLookahead = atoi(argv[i+1]);
      } else if (param == "-numkeep") {
        numKeep = atoi(argv[i+1]);
      } else if (param == "-divtolerance") {
        divTolerance = atof(argv[i+1]);
      } else if (param == "-greediness") {
        greediness = atoi(argv[i+1]);
      } else if (param == "-crossovertype") {
        std::string crossoverTypeString = argv[i+1];
        if (crossoverTypeString == "OB") {
          crossoverType = CrossoverType::OB;
        } else if (crossoverTypeString == "RK") {
          crossoverType = CrossoverType::RK;
        } else {
          crossoverType = CrossoverType::CX;
        }
      } else if (param == "-powerfactor") {
        float powerfactor = atof(argv[i+1]);
        mutationPower = ceil(n*powerfactor);
      }
    }
    rr.set();
    SearchResult sr = localSearch.genetic(cutoffTime, initPopulationSize, numCrossovers, numMutations, mutationPower, divLookahead, numKeep, divTolerance, crossoverType, greediness, opt, rr);
    localSearch.checkSolution(sr.getOrdering());

    if (Util::isOpt(sr, opt)) {
      std::cout << "Result for ParamILS: SAT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    } else if (bestKnown == 0) {
      std::cout << "Result for ParamILS: TIMEOUT, " << rr.check() << ", " << 0 << ", " << sr.getScore() << ", " << seed << std::endl;
    } else {
      double approx = (double)1 - (double) bestKnown / (double) sr.getScore();
      std::cout << "Result for ParamILS: TIMEOUT, " << rr.check() << ", " << 0 << ", " << approx << ", " << seed << std::endl;
    }
    rr.dump("MEMETIC_SEED_" + std::to_string(seed) + "_"  + std::to_string(time(NULL)), instanceFile, argc, argv);
  } else if (method == "PROBESELECT") {
    rr.set();
    SelectType type = SelectType::BEST;
    std::string typeString = "BEST";
    std::string prefix = "";
    int numRuns = 1000000000;
    for (int i = 7; i < 13 && i < argc; i += 2) {
      std::string param(argv[i]);
      if (param == "-type") {
        typeString = argv[i+1];
        if (typeString == "BEST") {
          type = SelectType::BEST;
        } else if (typeString == "FIRSTV1") {
          type = SelectType::FIRSTV1;
        } else if (typeString == "FIRSTV2") {
          type = SelectType::FIRSTV2;
        } else if (typeString == "HYBRID") {
          type = SelectType::HYBRID;
        } else if (typeString == "OLDHYBRID") {
          type = SelectType::OLDHYBRID;
        }
      }
      if (param == "-prefix") {
        prefix = argv[i+1];
      }
      if (param == "-runs") {
        numRuns = atoi(argv[i+1]);
      }
    }
    SearchResult sr = localSearch.hillClimbWithRestartsProbe(type, numRuns, cutoffTime, rr);
    DBG(sr);
    localSearch.checkSolution(sr.getOrdering());
    rr.dump(prefix + "_PROBSELECT_" + typeString + "_SEED_" + std::to_string(seed) + "_"  + std::to_string(time(NULL)), instanceFile, argc, argv);
  } else if (method == "PROBEGREEDY") {
    rr.set();
    int greediness = -1;
    std::string prefix = "";
    int numRuns = 1000000000;
    for (int i = 7; i < 15 && i < argc; i += 2) {
      std::string param(argv[i]);
      if (param == "-prefix") {
        prefix = argv[i+1];
      }
      if (param == "-runs") {
        numRuns = atoi(argv[i+1]);
      }
      if (param == "-greediness") {
        greediness = atoi(argv[i+1]);
      }
    }
    SearchResult sr = localSearch.hillClimbWithRestartsProbe(SelectType::HYBRID, numRuns, cutoffTime, rr, greediness);
    DBG(sr);
    rr.dump(prefix + "_PROBEGREEDY_SEED_" + std::to_string(seed) + "_GREED_"  + std::to_string(greediness) + "_" + std::to_string(time(NULL)), instanceFile, argc, argv);
  }
  return 0;
}
