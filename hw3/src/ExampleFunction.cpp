#include "ExampleFunction.h"
#include <cmath>
#include <tuple>

typedef tuple<size_t, size_t, double> Info; // module_id, offset, parameter

ExampleFunction::ExampleFunction(Placement& placement) : _placement(placement)
{
    double boundryW = _placement.boundryWidth();
    double boundryH = _placement.boundryHeight();
    /* bin info. */
    _hBinNum = 14; // 16
    _vBinNum = 14;
    _binWidth = boundryW/_hBinNum;
    _binHeight = boundryH/_vBinNum;
    _binDensLimit = 0.0; // 0.6
    for (Module& it : _placement.getModules())
        _binDensLimit += it.area() / (boundryW*boundryH);

    _modExp.resize(_placement.numModules()*4);
    _ovlpDiff.resize(_placement.numModules()*2);

    _lambda = 0.0;    // bigger is better
    _gamma = boundryH/600.0; //500.0;
}

void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g)
{
    unsigned netNum = _placement.numNets();
    unsigned modNum = _placement.numModules();
    f = 0.0;
    std::fill(g.begin(), g.end(), 0.0);
    std::fill(_ovlpDiff.begin(), _ovlpDiff.end(), 0.0);

    for (size_t i = 0; i < modNum; ++i) {
        _modExp[4*i+0] = exp(x[2*i]/_gamma);
        _modExp[4*i+1] = exp(-x[2*i]/_gamma);
        _modExp[4*i+2] = exp(x[2*i+1]/_gamma);
        _modExp[4*i+3] = exp(-x[2*i+1]/_gamma);
    }

    /* LSE wirelength model */
    for (size_t n = 0; n < netNum; ++n) { // foreach net
        
        Net& net = _placement.net(n);
        unsigned pinNum = net.numPins();
        //std::vector<double> _modExp(pinNum*4);
        double sumExp0 = 0.0, sumExp1 = 0.0, sumExp2 = 0.0, sumExp3 = 0.0;

        for (size_t p = 0; p < pinNum; ++p) {  // foreach pin in net
            unsigned modId = net.pin(p).moduleId();
            /*_modExp[4*p+0] = exp(x[2*modId]/_gamma);
            _modExp[4*p+1] = exp(-x[2*modId]/_gamma);
            _modExp[4*p+2] = exp(x[2*modId+1]/_gamma);
            _modExp[4*p+3] = exp(-x[2*modId+1]/_gamma);*/

            /*_modExp[4*p+0] = exp(_placement.module(modId).centerX()/_gamma);
            _modExp[4*p+1] = exp(-_placement.module(modId).centerX()/_gamma);
            _modExp[4*p+2] = exp(_placement.module(modId).centerY()/_gamma);
            _modExp[4*p+3] = exp(-_placement.module(modId).centerY()/_gamma);*/

            sumExp0 += _modExp[4*modId+0];
            sumExp1 += _modExp[4*modId+1];
            sumExp2 += _modExp[4*modId+2];
            sumExp3 += _modExp[4*modId+3];
        }
        f += (log(sumExp0) + log(sumExp1) + log(sumExp2) + log(sumExp3));

        for (size_t p = 0, modId; p < pinNum; ++p) {
            modId = net.pin(p).moduleId();
            g[2*modId+0] += _modExp[4*modId+0] / (sumExp0 * _gamma);
            g[2*modId+0] -= _modExp[4*modId+1] / (sumExp1 * _gamma);
            g[2*modId+1] += _modExp[4*modId+2] / (sumExp2 * _gamma);
            g[2*modId+1] -= _modExp[4*modId+3] / (sumExp3 * _gamma);
        }
    }
    _HPWLF = f;
    cout << "// LSE f: " << _HPWLF << endl;

    if (_lambda == 0) return;

    /* Bin density: Bell-Shaped function */
    double alpha[2], beta[2];
    double dx, dy, absDx, absDy;
    double binCentX;
    double binCentY;
    double diff_xOverlap, diff_yOverlap;
    double xOverlap, yOverlap;
    double overlapAreaSum;
    for (size_t i = 0; i < _hBinNum; ++i) {
        for (size_t j = 0; j < _vBinNum; ++j) {
            overlapAreaSum = 0.0;
            //vector<Info> info;
            for (size_t m = 0; m < modNum; ++m) {
                Module& module = _placement.module(m);
                double c = module.area() / (_binWidth*_binHeight);
                binCentX = (i+0.5)*_binWidth;
                binCentY = (j+0.5)*_binHeight;
                dx = x[2*m+0] - binCentX;
                dy = x[2*m+1] - binCentY;
                absDx = std::abs(dx);
                absDy = std::abs(dy);

                alpha[0] = 4.0/((module.width() + 2.0*_binWidth)*(module.width() + 4.0*_binWidth));
                beta[0] = 2.0/(_binWidth*(module.width() + 4.0*_binWidth));
                alpha[1] = 4.0/((module.height() + 2.0*_binHeight)*(module.height() + 4.0*_binHeight));
                beta[1] = 2.0/(_binHeight*(module.height() + 4.0*_binHeight));

                //   x-overlap   
                if (absDx <= module.width()/2.0 + _binWidth) {
                    xOverlap = 1-(alpha[0]*pow(absDx,2));
                    _ovlpDiff[2*m] = -2*alpha[0]*dx;
                    //diff_xOverlap = -2*alpha[0]*dx;
                }
                else if (absDx <= module.width()/2.0 + 2.0*_binWidth) {
                    xOverlap = beta[0]*pow(absDx-module.width()/2.0-2.0*_binWidth, 2);
                    _ovlpDiff[2*m] = 2*beta[0]*(dx<0 ? -1.0 : 1.0)*(absDx-module.width()/2.0-2.0*_binWidth);
                    //diff_xOverlap = 2*beta[0]*(dx<0 ? -1.0 : 1.0)*(absDx-module.width()/2.0-2.0*_binWidth);
                } 
                else {
                    xOverlap = 0;
                    _ovlpDiff[2*m] = 0;
                    //diff_xOverlap = 0;
                }
               
                //   y-overlap   
                if (absDy <= module.height()/2.0 + _binHeight) {
                    yOverlap = 1-(alpha[1]*pow(absDy,2));
                    _ovlpDiff[2*m+1] = -2*alpha[1]*dy;
                    //diff_yOverlap = -2*alpha[1]*dy;
                }
                else if (absDy <= module.height()/2.0 + 2.0*_binHeight) {
                    yOverlap = beta[1]*pow(absDy-module.height()/2.0-2.0*_binHeight, 2);
                    _ovlpDiff[2*m+1] = 2*beta[1]*(dx<0 ? -1.0 : 1.0)*(absDy-module.height()/2.0-2.0*_binHeight);
                    //diff_yOverlap = 2*beta[1]*(dx<0 ? -1.0 : 1.0)*(absDy-module.height()/2.0-2.0*_binHeight);
                }
                else  {
                    yOverlap = 0;
                    _ovlpDiff[2*m+1] = 0;
                    //diff_yOverlap = 0;
                }
               

                overlapAreaSum += c * xOverlap * yOverlap;
                _ovlpDiff[2*m] *= (c * yOverlap);
                _ovlpDiff[2*m+1] *= (c * xOverlap);
                //info.push_back(make_tuple(m, 0, c * diff_xOverlap * yOverlap));
                //info.push_back(make_tuple(m, 1, c * diff_yOverlap * xOverlap));
            }

            f += _lambda * pow(overlapAreaSum-_binDensLimit, 2);

            /*for (auto& it : info) 
                g[2*get<0>(it)+get<1>(it)] += 2.0 * _lambda * (overlapAreaSum-_binDensLimit) * get<2>(it);*/
            for (size_t m = 0; m < modNum; ++m) {
                g[2*m+0] += 2.0 * _lambda * (overlapAreaSum-_binDensLimit) * _ovlpDiff[2*m + 0];
                g[2*m+1] += 3.0 * _lambda * (overlapAreaSum-_binDensLimit) * _ovlpDiff[2*m + 1];
            }
            
        }
    }

    _DensF = f-_HPWLF;
    cout << "// Density: " << _DensF << endl;
}

void ExampleFunction::evaluateF(const vector<double> &x, double &f)
{
    unsigned netNum = _placement.numNets();
    unsigned modNum = _placement.numModules();
    f = 0.0;

    for (size_t i = 0; i < modNum; ++i) {
        _modExp[4*i+0] = exp(x[2*i]/_gamma);
        _modExp[4*i+1] = exp(-x[2*i]/_gamma);
        _modExp[4*i+2] = exp(x[2*i+1]/_gamma);
        _modExp[4*i+3] = exp(-x[2*i+1]/_gamma);
    }

    /* LSE wirelength model */
    for (size_t n = 0; n < netNum; ++n) { // foreach net
        
        Net& net = _placement.net(n);
        unsigned pinNum = net.numPins();
        //std::vector<double> _modExp(pinNum*4);
        double sumExp0 = 0.0, sumExp1 = 0.0, sumExp2 = 0.0, sumExp3 = 0.0;

        for (size_t p = 0; p < pinNum; ++p) {  // foreach pin in net
            unsigned modId = net.pin(p).moduleId();
            /*_modExp[4*p+0] = exp(x[2*modId]/_gamma);
            _modExp[4*p+1] = exp(-x[2*modId]/_gamma);
            _modExp[4*p+2] = exp(x[2*modId+1]/_gamma);
            _modExp[4*p+3] = exp(-x[2*modId+1]/_gamma);*/

            /*_modExp[4*p+0] = exp(_placement.module(modId).centerX()/_gamma);
            _modExp[4*p+1] = exp(-_placement.module(modId).centerX()/_gamma);
            _modExp[4*p+2] = exp(_placement.module(modId).centerY()/_gamma);
            _modExp[4*p+3] = exp(-_placement.module(modId).centerY()/_gamma);*/

            sumExp0 += _modExp[4*modId+0];
            sumExp1 += _modExp[4*modId+1];
            sumExp2 += _modExp[4*modId+2];
            sumExp3 += _modExp[4*modId+3];
        }
        f += (log(sumExp0) + log(sumExp1) + log(sumExp2) + log(sumExp3));
    }

    if (_lambda == 0) return;
    
    /* Bin density: Bell-Shaped function */
    double alpha[2], beta[2];
    double dx, dy, absDx, absDy;
    double binCentX;
    double binCentY;
    double xOverlap, yOverlap;
    double overlapAreaSum;
    for (size_t i = 0; i < _hBinNum; ++i) {
        for (size_t j = 0; j < _vBinNum; ++j) {
            overlapAreaSum = 0.0;
            vector<Info> info;
            for (size_t m = 0; m < modNum; ++m) {
                Module& module = _placement.module(m);
                double c = module.area() / (_binWidth*_binHeight);
                binCentX = (i+0.5)*_binWidth;
                binCentY = (j+0.5)*_binHeight;
                dx = x[2*m+0] - binCentX;
                dy = x[2*m+1] - binCentY;
                absDx = std::abs(dx);
                absDy = std::abs(dy);

                alpha[0] = 4.0/((module.width() + 2.0*_binWidth)*(module.width() + 4.0*_binWidth));
                beta[0] = 2.0/(_binWidth*(module.width() + 4.0*_binWidth));
                alpha[1] = 4.0/((module.height() + 2.0*_binHeight)*(module.height() + 4.0*_binHeight));
                beta[1] = 2.0/(_binHeight*(module.height() + 4.0*_binHeight));

                //   x-overlap   
                if (absDx <= module.width()/2.0 + _binWidth) {
                    xOverlap = 1-(alpha[0]*pow(absDx,2));
                }
                else if (absDx <= module.width()/2.0 + 2.0*_binWidth) {
                    xOverlap = beta[0]*pow(absDx-module.width()/2.0-2.0*_binWidth, 2);
                } 
                else {
                    xOverlap = 0;
                }

                //   y-overlap   
                if (absDy <= module.height()/2.0 + _binHeight) {
                    yOverlap = 1-(alpha[1]*pow(absDy,2));
                }
                else if (absDy <= module.height()/2.0 + 2.0*_binHeight) {
                    yOverlap = beta[1]*pow(absDy-module.height()/2.0-2.0*_binHeight, 2);
                }
                else {
                    yOverlap = 0;
                }

                overlapAreaSum += c * xOverlap * yOverlap;
            }

            f += _lambda * pow(overlapAreaSum-_binDensLimit, 2);   
        }
    }
}

unsigned ExampleFunction::dimension()
{
    return 2*_placement.numModules(); // num_blocks*2 
    // each two dimension represent the X and Y dimensions of each block
}