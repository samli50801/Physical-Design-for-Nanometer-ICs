#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"

GlobalPlacer::GlobalPlacer(Placement &placement)
	:_placement(placement)
{
}

void GlobalPlacer::initSolVec(vector<double>& x)
{
    srand( time(NULL) );
    double boundryXCent = (_placement.boundryLeft()+_placement.boundryRight()) / 2.0;
    double boundryYCent = (_placement.boundryBottom()+_placement.boundryTop()) / 2.0;
    unsigned modNum = _placement.numModules();
    for (unsigned i = 0; i < modNum; ++i) {
        int randX = rand()%((int)abs(_placement.boundryLeft())+(int)abs(_placement.boundryRight()));
        int randY = rand()%((int)abs(_placement.boundryBottom())+(int)abs(_placement.boundryTop()));
        /*x[i*2] = (double)randX-abs(_placement.boundryLeft());
        x[i*2+1] = (double)randY-abs(_placement.boundryBottom());*/
        x[i*2] = boundryXCent;
        x[i*2+1] = boundryYCent;
        _placement.module(i).setCenterPosition(x[i*2], x[i*2+1]);
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
    no.setStepSizeBound(_placement.boundryWidth() / 10); // user-specified parameter
    no.setX(x);             // set initial solution
    no.setNumIteration(20); // user-specified parameter

    for (size_t iter = 0; iter < 11; ++iter) {
        no.solve();             // Conjugate Gradient solver
        for (size_t i = 0; i < moduleNum; ++i) {
            _placement.module(i).setCenterPosition(no.x(2*i), no.x(2*i+1));
            x[2*i] = _placement.module(i).centerX();
            x[2*i+1] = _placement.module(i).centerY();
        }
        no.setX(x);
        ef.increaseLambda();
    }

    cout << "Current solution:" << endl;
    for (unsigned i = 0; i < no.dimension(); i++) {
        cout << "x[" << i << "] = " << no.x(i) << endl;
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
