#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <map>
#include "btree.h"
using namespace std;

class Floorplanner 
{
public:
    Floorplanner(double alpha, double beta, fstream& input_blk, fstream& input_net) : 
    _alpha(alpha), _beta(beta), _gamma(1.0-alpha-beta), _wlNorm(0), _areaNorm(0), _aspRatioNorm(0) 
    {
        parseInput(input_blk, input_net);
        _bt = new B_Tree(_blkArray, _outlineWidth, _outlineHeight);
        _dsrAspRatio = (double)_outlineHeight/(double)_outlineWidth;
    };
    ~Floorplanner() {}

    void parseInput(fstream& input_blk, fstream& input_net);
    void calcCost();
    void recordBest();
    void floorplan();

    void plot(bool);

private:
    const double            _alpha;
    const double            _beta;
    const double            _gamma;
    double                  _dsrAspRatio;
    int                     _outlineWidth;
    int                     _outlineHeight;
    int                     _blkNum;
    int                     _netNum;
    int                     _termNum;
    double                  _wlCost;
    double                  _areaCost;
    double                  _aspRatioCost;
    double                  _wlNorm;
    double                  _areaNorm;
    double                  _aspRatioNorm;
    vector<Block*>          _blkArray;
    // vector<Terminal*>       _termArray;     // seems duplicate
    vector<Net*>            _netArray;
    map<string, Terminal*>  _name2Term;
    B_Tree*                 _bt;
};
#endif  // FLOORPLANNER_H