#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"
#include <float.h>

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

    double boundW = _placement.boundryRight() - _placement.boundryLeft();
    double boundH = _placement.boundryTop() - _placement.boundryBottom();

    for (size_t iter = 0; iter < 4; ++iter) {
        ef.setLambda(iter*5000);
        if (iter == 0) {
            no.setX(x);             // set initial solution
            no.setStepSizeBound(400000);
            //no.setStepSizeBound((_placement.boundryRight() - _placement.boundryLeft())*7); // user-specified parameter
            no.setNumIteration(80); // user-specified parameter
        } else {
            no.setStepSizeBound(400000);
            no.setNumIteration(70); // user-specified parameter
        }
        no.solve();             // Conjugate Gradient solver
        /*for (size_t i = 0; i < moduleNum; ++i) {
            double centX = no.x(2*i);
            double centY = no.x(2*i+1);
            if (centX > boundW) {
                centX = boundW - 0.5*_placement.module(i).width();
            } 
            else if (centX < 0.0) {
                centX = 0.0 + 0.5*_placement.module(i).width();
            }
            if (centY > boundH) {
                centY = boundH - 0.5*_placement.module(i).height();
            } 
            else if (centY < 0.0) {
                centY = 0.0 + 0.5*_placement.module(i).height();
            } 
            x[2*i] = centX;
            x[2*i+1] = centY;
        }*/
    }

    for (size_t i = 0; i < moduleNum; ++i) {
        double centX = no.x(2*i);
        double centY = no.x(2*i+1);
        if (centX > boundW) {
            centX = boundW - 0.5*_placement.module(i).width();
        } 
        else if (centX < 0.0) {
            centX = 0.0 + 0.5*_placement.module(i).width();
        }
        if (centY > boundH) {
            centY = boundH - 0.5*_placement.module(i).height();
        } 
        else if (centY < 0.0) {
            centY = 0.0 + 0.5*_placement.module(i).height();
        } 
        _placement.module(i).setCenterPosition(centX + _placement.boundryLeft(), centY + _placement.boundryBottom());
    }

    /*cout << "Current solution:" << endl;
    for (unsigned i = 0; i < no.dimension(); i++) {
        cout << "x[" << i << "] = " << no.x(i) << endl;
    }*/
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
