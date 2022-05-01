#define _GLIBCXX_USE_CXX11_ABI 0
#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include "NumericalOptimizerInterface.h"
#include "Placement.h"

class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(Placement&);

    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    void increaseLambda() {_lambda *= 2.0;}
    void setLambda(double lambda)   {_lambda = lambda;}
    double calLambda()  { return _HPWLF / _DensF; }
    unsigned dimension();

public:
    Placement&  _placement;
    int         _hBinNum;
    int         _vBinNum;
    double      _binWidth;
    double      _binHeight;
    double      _binDensLimit;     // upper bound of the bin density
    double      _lambda;
    double      _gamma;
    double      _HPWLF;
    double      _DensF;
    vector<double>  _modExp;
    vector<double>  _ovlpDiff;
};
#endif // EXAMPLEFUNCTION_H
