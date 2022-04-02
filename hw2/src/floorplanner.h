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
    _alpha(alpha), _beta(1.0-alpha), _gamma(1.0-alpha), _wlNorm(0), _areaNorm(0), _aspRatioNorm(0) 
    {
        parseInput(input_blk, input_net);
        _bt = new B_Tree(_blkArray, _outlineWidth, _outlineHeight);
        _dsrAspRatio = (double)_outlineHeight/(double)_outlineWidth;
    };
    ~Floorplanner() {}

    void parseInput(fstream& input_blk, fstream& input_net);
    void calcCost(int&, int&);
    void recordBest();
    void floorplan();

    void plot(bool);

private:
    const double            _alpha, _beta, _gamma;  // parameter of area, wirelength and aspect ratio
    int                     _outlineWidth;          // fixed-outline width
    int                     _outlineHeight;         // fixed-outline height
    int                     _blkNum;                // total number of block
    int                     _netNum;                // total number of net
    int                     _termNum;               // total number of terminal
    double                  _dsrAspRatio;           // aspect ratio of fixed-outline
    double                  _bestArea, _bestWL;     // the best area and wirelength of floorplan
    double                  _wlCost;                // current wirelength cost
    double                  _areaCost;              // current area cost
    double                  _aspRatioCost;          // current aspect ratio cost
    double                  _wlNorm;                // normalized wirelength value
    double                  _areaNorm;              // normalized area value
    double                  _aspRatioNorm;          // normalized aspect ratio value
    vector<Block*>          _blkArray;              // array storing all blocks
    vector<Net*>            _netArray;
    map<string, Terminal*>  _name2Term;
    B_Tree*                 _bt;
};
#endif  // FLOORPLANNER_H