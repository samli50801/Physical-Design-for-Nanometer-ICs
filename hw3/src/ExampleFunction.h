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
    unsigned dimension();

private:
    Placement   _placement;
    int         _hBinNum;
    int         _vBinNum;
    double      _binWidth;
    double      _binHeight;
    double      _binDensLimit;     // upper bound of the bin density
    double      _lambda;
    double      _gamma;
};
#endif // EXAMPLEFUNCTION_H
