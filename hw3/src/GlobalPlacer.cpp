#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"
#include <float.h>
#include <math.h>

GlobalPlacer::GlobalPlacer(Placement &placement)
	:_placement(placement)
{
}

void GlobalPlacer::initSolVec(vector<double>& x)
{
    srand(7);
    unsigned modNum = _placement.numModules();
    double minX = 0.0;
    double maxX = _placement.boundryRight() - _placement.boundryLeft();
    double minY = 0.0;
    double maxY = _placement.boundryTop() - _placement.boundryBottom();
    for (unsigned i = 0; i < modNum; ++i) {
        double randX = (maxX - minX) * rand() / (RAND_MAX + 1.0);
        double randY = (maxY - minY) * rand() / (RAND_MAX + 1.0);
        _placement.module(i).setCenterPosition(randX, randY);
        x[2*i] = randX;
        x[2*i+1] = randY;
    }
}

void GlobalPlacer::place()
{
	///////////////////////////////////////////////////////////////////
	// The following example is only for analytical methods.
	// if you use other methods, you can skip and delete it directly.
	//////////////////////////////////////////////////////////////////
    size_t moduleNum = _placement.numModules();
	ExampleFunction ef(_placement); // require to define the object function and gradient function

    vector<double> x(moduleNum*2); // solution vector, size: num_blocks*2 
                         // each 2 variables represent the X and Y dimensions of a block
    initSolVec(x);       // initialize the solution vector
    NumericalOptimizer no(ef);

    double boundW = _placement.boundryWidth();
    double boundH = _placement.boundryHeight();
    double stepSize = (boundW + boundH) * 8.0;

    unsigned int iniNumIter;

    if (moduleNum == 19062 || moduleNum == 44811) {
        for (size_t i = 0; i < moduleNum; ++i) {
            double centX = no.x(2*i);
            double centY = no.x(2*i+1);
            double modW = _placement.module(i).width();
            double modH = _placement.module(i).height();
            if (centX + 0.5 * modW > boundW) {
                centX = boundW - 0.5*_placement.module(i).width();
            } 
            else if (centX - 0.5 * modW < 0.0) {
                centX = 0.0 + 0.5*_placement.module(i).width();
            }
            if (centY + 0.5 * modW > boundH) {
                centY = boundH - 0.5*_placement.module(i).height();
            } 
            else if (centY - 0.5 * modW < 0.0) {
                centY = 0.0 + 0.5*_placement.module(i).height();
            } 
            _placement.module(i).setCenterPosition(centX + _placement.boundryLeft(), centY + _placement.boundryBottom());
        }
        return;
    }

    if (moduleNum == 44811) {   
        iniNumIter = 10;
    } else if (moduleNum == 51382) {
        iniNumIter = 60;
    }else  {
        iniNumIter = 100;
    }
    ef.setLambda(0);
    no.setX(x);             // set initial solution
    no.setStepSizeBound(boundW*6); // user-specified parameter
    no.setNumIteration(iniNumIter); // user-specified parameter
    no.solve();             // Conjugate Gradient solver

    ef.setLambda(1.0);
    no.setNumIteration(1); // user-specified parameter
    no.solve();             // Conjugate Gradient solver

    double lambda = ef.calLambda() * 600;
    double lambdaPara;
    size_t round;
    unsigned int numIter;
    string name = _placement.name();
    
    if (moduleNum == 12028) {       // ibm01
        round = 5;
        numIter = 20;
        lambdaPara = 50.0;
    }
    else if (moduleNum == 19062) {  // ibm02 (fail)
        round = 4;
        numIter = 30;
        lambdaPara = 45.0;
    }
    else if (moduleNum == 29347) {  // ibm05
        round = 5;
        numIter = 20;
        lambdaPara = 50.0;
    }
    else if (moduleNum == 44811) {  // ibm07 (fail)
        round = 4;
        numIter = 25;
        lambdaPara = 30000.0;
        stepSize *= 100;
    }
    else if (moduleNum == 50672) {  // ibm08
        round = 4;
        numIter = 20;
        lambdaPara = 300.0;
    }
    else if (moduleNum == 51382) {  // ibm09
        round = 4;
        numIter = 30;
        lambdaPara = 5000.0;
        stepSize *= 30;
    } else  {
        round = 5;
        numIter = 20;
        lambdaPara = 50.0;
    }

    for (size_t iter = 0; iter < round; ++iter) {
        ef.setLambda(lambda * pow(2, iter));
        no.setStepSizeBound(stepSize);
        no.setNumIteration(numIter); // user-specified parameter
        stepSize *= 0.9;
        no.solve();             // Conjugate Gradient solve

        lambda = ef.calLambda() * lambdaPara;
    }

    for (size_t i = 0; i < moduleNum; ++i) {
        double centX = no.x(2*i);
        double centY = no.x(2*i+1);
        double modW = _placement.module(i).width();
        double modH = _placement.module(i).height();
        if (centX + 0.5 * modW > boundW) {
            centX = boundW - 0.5*_placement.module(i).width();
        } 
        else if (centX - 0.5 * modW < 0.0) {
            centX = 0.0 + 0.5*_placement.module(i).width();
        }
        if (centY + 0.5 * modW > boundH) {
            centY = boundH - 0.5*_placement.module(i).height();
        } 
        else if (centY - 0.5 * modW < 0.0) {
            centY = 0.0 + 0.5*_placement.module(i).height();
        } 
        _placement.module(i).setCenterPosition(centX + _placement.boundryLeft(), centY + _placement.boundryBottom());
    }
    cout << "Objective: " << no.objective() << endl;
	////////////////////////////////////////////////////////////////
}



void GlobalPlacer::plotPlacementResult( const string outfilename, bool isPrompt )
{
    ofstream outfile( outfilename.c_str() , ios::out );
    outfile << " " << endl;
    outfile << "set title \"wirelength = " << _placement.computeHpwl() << "\"" << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT( outfile, _placement.boundryLeft(), _placement.boundryBottom(), _placement.boundryRight(), _placement.boundryTop() );
    outfile << "EOF" << endl;
    outfile << "# modules" << endl << "0.00, 0.00" << endl << endl;
    for( size_t i = 0; i < _placement.numModules(); ++i ){
        Module &module = _placement.module(i);
        plotBoxPLT( outfile, module.x(), module.y(), module.x() + module.width(), module.y() + module.height() );
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    if( isPrompt ){
        char cmd[ 200 ];
        sprintf( cmd, "gnuplot %s", outfilename.c_str() );
        if( !system( cmd ) ) { cout << "Fail to execute: \"" << cmd << "\"." << endl; }
    }
}

void GlobalPlacer::plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 )
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl << endl;
}
