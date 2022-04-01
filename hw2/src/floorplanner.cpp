#include "floorplanner.h"
#define CURRENT_FLOORPLAN   0
#define BEST_FLOORPLAN      1
using namespace std;

void 
Floorplanner::parseInput(fstream& input_blk, fstream& input_net)
{
    string str;
    int val1, val2;

    /* input file: block */
    // Outline
    input_blk >> str >> _outlineWidth >> _outlineHeight;
    // NumBlocks
    input_blk >> str >> _blkNum;
    _blkArray.resize(_blkNum);
    // NumTerminals
    input_blk >> str >> _termNum;

    // Block info
    for (size_t i = 0; i < _blkNum; ++i) {
        input_blk >> str >> val1 >> val2;
        Block *block = new Block(str, val1, val2);
        _blkArray[i] = block;
        //_termArray.push_back(block);
        _name2Term[str] = block;
    }

    // Terminal info
    string buffer;  // "terminal"
    for (size_t i = 0; i < _termNum; ++i) {
        input_blk >> str >> buffer >> val1 >> val2;
        //_termArray.push_back(terminal);
        _name2Term[str] = new Terminal(str, val1, val2);;
    }

    /* input file: net */
    input_net >> str >> _netNum;
    _netArray.resize(_netNum);
    for (size_t i = 0; i < _netNum; ++i) {
        input_net >> str >> val1;   // NetDegree
        Net *net = new Net(val1);
        for (size_t j = 0; j < val1; ++j) {
            input_net >> str;
            net->getTermList()[j] = _name2Term[str];
        }
        _netArray[i] = net;
    }
}

void 
Floorplanner::calcCost()
{
    double bdWidth = _bt->get_bd_box().first;
    double bdHeight = _bt->get_bd_box().second;
    _areaCost = bdWidth*bdHeight;
    _aspRatioCost = abs((_dsrAspRatio - (bdHeight/bdWidth)));
    _wlCost = 0.0;
    for (auto it : _netArray) 
        _wlCost += it->calcHPWL();
}

void 
Floorplanner::recordBest()
{
    vector<Node*>& nodeArray = _bt->getNodeArray();
    for (auto& it : nodeArray) {
        it->getBlk()->setFinalPos(it->getx(),it->gety(),
                                it->getx()+it->getWidth(), it->gety()+it->getHeight());
    }
}

void 
Floorplanner::floorplan()
{
    _bt->initialize();
    /********************************************************
    // calculate normalized value of area and wirelength 
    *********************************************************/
    short OP;
    size_t m = _blkNum + _netNum + 100;
    int N = 100*_blkNum;   // user-defined number for inner loop
    double T;
    double r = 0.92;       // cool down parameter
    int rej = 0, uphill = 0, MT = 0;
    double prevCost, currCost, bestCost, deltaCost, avgDelta = 0;

    for(size_t i = 0; i < 2*m; ++i) {
        OP = rand() % 3;
        if (OP == 0)    _bt->op_rotate(); else
        if (OP == 1)    _bt->op_swap2nodes(); else 
        if (OP == 2)    _bt->op_deleteAndInsert();
        _bt->reset();
        _bt->computeCoord_dfs(_bt->getRoot(), 0);
        calcCost();
        if (i < m) {
            _areaNorm += _areaCost/(double)m;
            _wlNorm += _wlCost/(double)m;
            _aspRatioNorm += _aspRatioCost/(double)m;
            if (i == m-1) {
                prevCost = _alpha*(_areaCost/_areaNorm) + _beta*(_wlCost/_wlNorm) + _gamma*(_aspRatioCost/_aspRatioNorm);
                bestCost = prevCost;
                recordBest();
            }
        } else {
            currCost = _alpha*(_areaCost/_areaNorm) + _beta*(_wlCost/_wlNorm) + _gamma*(_aspRatioCost/_aspRatioNorm);
            if (currCost < bestCost) {
                recordBest();
                bestCost = currCost;
            }
            deltaCost = abs(currCost - prevCost);
            avgDelta += deltaCost/(double)m;
            prevCost = currCost;
        }
    }
    _bt->clearRecover();
    T = abs(avgDelta/log(0.99));
    cout << T << endl;
    /********************************************************
    // Run Simulated Annealing
    *********************************************************/
    int outerCount = 0, innerCount = 0;
    do {
        MT = rej = uphill = 0;
        do {
            OP = rand() % 3;
            if (OP == 0)    _bt->op_rotate(); else
            if (OP == 1)    _bt->op_swap2nodes(); else 
            if (OP == 2)    _bt->op_deleteAndInsert();
            _bt->reset();
            _bt->computeCoord_dfs(_bt->getRoot(), 0);
            //_bt->computeCoord_bfs(); 
            calcCost();
            currCost = _alpha*(_areaCost/_areaNorm) + _beta*(_wlCost/_wlNorm) + _gamma*(_aspRatioCost/_aspRatioNorm);
            deltaCost = currCost - prevCost;
            // Accept the neighbor
            //cout << "outer: " << outerCount << "   " << exp(-deltaCost/T) << endl;
            if (deltaCost<=0 || (double)rand()/(RAND_MAX+1.0) < exp(-deltaCost/T)) {
                if (deltaCost>0) ++uphill;
                if (currCost < bestCost) {
                    recordBest();
                    bestCost = currCost;
                }
                prevCost = currCost;
                _bt->clearRecover();
            } else {
                _bt->recover();
                ++rej;
            }
            ++MT;
            innerCount++;
        } while (uphill <= N && MT <= 2*N);
        T*=r;
        ++outerCount;
        innerCount = 0;
    } while ((double)rej/(double)MT < 0.95 && T > 0.00001);

    cout << "bestCost: " << bestCost << endl;
    plot(BEST_FLOORPLAN);
}

void Floorplanner::plot(bool type)
{
    //gnuplot preset
	fstream outgraph("./floorplan.gp", ios::out);
	outgraph << "reset\n";
	outgraph << "set tics\n";
	outgraph << "unset key\n";
	outgraph << "set title \"The result of FloorPlanning\"\n";
	outgraph << "set size " << 1 << ", " << 1 << "\n";
	int index = 1;
 	// wirte block info into floorplan.gp
    outgraph << "set object " << index++ << " rect from "
		<< -_outlineWidth << "," << -_outlineHeight << " to "
		<< _outlineWidth*2 << "," << _outlineHeight*2 << " fc rgb 'black'\n";

    if (type == CURRENT_FLOORPLAN) {
        _bt->plotNode(outgraph, index);
        _bt->plotContourLine(outgraph, index);
    }
    else if (type == BEST_FLOORPLAN) {
        for (vector<Block*>::iterator it = _blkArray.begin(); it != _blkArray.end(); ++it) {
            Block* blk = *it;
            int x0 = blk->getfx1(), y0 = blk->getfy1();
            int x1 = blk->getfx2(), y1 = blk->getfy2();
            outgraph << "set object " << index++ << " rect from " 
                    << x0 << "," << y0 << " to " << x1 << "," << y1 << " fs empty border fc rgb 'green'\n";
        }
    }
 
    /* draw outlune */
    fstream outline("line", ios::out);   
    outgraph << "set arrow " << index++ << " from "
			<< 0 << "," << 0 << " to " << _outlineWidth << "," << 0 << " nohead lc rgb \'yellow\'\n"
            << "set arrow " << index++ << " from "
			<< _outlineWidth << "," << 0 << " to " << _outlineWidth << "," << _outlineHeight << " nohead lc rgb \'yellow\'\n"
            << "set arrow " << index++ << " from "
			<< _outlineWidth << "," << _outlineHeight << " to " << 0 << "," << _outlineHeight << " nohead lc rgb \'yellow\'\n"
            << "set arrow " << index++ << " from "
			<< 0 << "," << _outlineHeight << " to " << 0 << "," << 0 << " nohead lc rgb \'yellow\'\n";
    
    outgraph << "set style line 1 lc rgb \"red\" lw 3\n";
	outgraph << "set border ls 1\n";
	outgraph << "set terminal png\n";
	outgraph << "set output \"floorplan.png\"\n";
	outgraph << "plot [" << -_outlineWidth << ":" << _outlineWidth*2 << "]["
		     << -_outlineHeight << ":" << _outlineHeight*2 << "]\'line\' w l lt 2 lw 1\n";
	outgraph << "set terminal x11 persist\n";
	outgraph << "replot\n";
	outgraph << "exit";
	outgraph.close();
	int gnuplot = system("gnuplot floorplan.gp");
}