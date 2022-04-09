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
    input_blk >> str >> _outlineWidth >> _outlineHeight;    // Outline
    input_blk >> str >> _blkNum;                            // NumBlocks
    _blkArray.resize(_blkNum);
    input_blk >> str >> _termNum;                           // NumTerminals
    // Block info
    for (size_t i = 0; i < _blkNum; ++i) {
        input_blk >> str >> val1 >> val2;
        Block *block = new Block(str, val1, val2);
        _blkArray[i] = block;
        _name2Term[str] = block;
    }
    /* Terminal info */
    string buffer;                      // "terminal"   
    for (size_t i = 0; i < _termNum; ++i) {
        input_blk >> str >> buffer >> val1 >> val2;
        _name2Term[str] = new Terminal(str, val1, val2);;
    }
    /* input file: net */
    input_net >> str >> _netNum;
    _netArray.resize(_netNum);
    for (size_t i = 0; i < _netNum; ++i) {
        input_net >> str >> val1;       // NetDegree
        Net *net = new Net(val1);
        for (size_t j = 0; j < val1; ++j) {
            input_net >> str;
            net->getTermList()[j] = _name2Term[str];
        }
        _netArray[i] = net;
    }
}

void 
Floorplanner::calcCost(int& bdWidth, int& bdHeight)
{
    bdWidth = _bt->get_bd_box().first;
    bdHeight = _bt->get_bd_box().second;
    _areaCost = (double)bdWidth*bdHeight;
    _aspRatioCost = std::pow((_dsrAspRatio - ((double)bdHeight/bdWidth)), 2);
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
        it->getBlk()->setFL(it->get_l() == NULL ? NULL : it->get_l()->getBlk());
        it->getBlk()->setFR(it->get_r() == NULL ? NULL : it->get_r()->getBlk());
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
    size_t m = 2*(_blkNum + _netNum);
    int N = 100*_blkNum;   // user-defined number for inner loop
    int bdWidth, bdHeight;  
    double T;
    double r = 0.93;       // cool down parameter
    int rej = 0, uphill = 0, MT = 0;
    double prevCost, currCost, bestCost, deltaCost, avgDelta = 0;

    for(size_t i = 0; i < 2*m; ++i) {
        OP = rand() % 3;
        if (OP == 0)    _bt->op_swap2nodes(); else
        if (OP == 1)    _bt->op_rotate(); else 
        if (OP == 2)    _bt->op_deleteAndInsert();
        _bt->reset();
        _bt->computeCoord_dfs(_bt->getRoot(), 0);
        calcCost(bdWidth, bdHeight);
        if (i < m) {
            _areaNorm += _areaCost/(double)m;
            _wlNorm += _wlCost/(double)m;
            _aspRatioNorm += _aspRatioCost/(double)m;
            if (i == m-1) 
                prevCost = _alpha*(_areaCost/_areaNorm) + 0*(_wlCost/_wlNorm) + _gamma*(_aspRatioCost/_aspRatioNorm);
        } else {
            currCost = _alpha*(_areaCost/_areaNorm) + 0*(_wlCost/_wlNorm) + _gamma*(_aspRatioCost/_aspRatioNorm);
            deltaCost = abs(currCost - prevCost);
            avgDelta += deltaCost/(double)m;
            prevCost = currCost;
        }
    }
    _bt->clearRecover();
    T = abs(avgDelta/log(0.99));
    /********************************************************
    // Run Simulated Annealing
    *********************************************************/
    int outerCount = 0, innerCount = 0;
    int num[3] = {0};
    bestCost = INT_MAX;
    do {
        MT = rej = uphill = 0;
        do {
            OP = rand() % 3;
            if (OP == 0)    _bt->op_swap2nodes(); else
            if (OP == 1)    _bt->op_rotate();   else 
            if (OP == 2)    _bt->op_deleteAndInsert();
            _bt->reset();
            _bt->computeCoord_dfs(_bt->getRoot(), 0);
            calcCost(bdWidth, bdHeight);
            currCost = _alpha*(_areaCost/_areaNorm) + 0*(_wlCost/_wlNorm) + _gamma*(_aspRatioCost/_aspRatioNorm);
            deltaCost = currCost - prevCost;
            // Accept the neighbor
            if (deltaCost<=0 || (double)rand()/(RAND_MAX+1.0) < exp(-deltaCost/T)) {
                if (deltaCost>0) ++uphill;
                if (_alpha*(_areaCost)+_beta*(_wlCost) <= bestCost
                && bdWidth<=_outlineWidth && bdHeight<=_outlineHeight) {
                    ++num[OP];
                    recordBest();
                    bestCost = _alpha*(_areaCost)+_beta*(_wlCost);
                    _bestArea = _areaCost;
                    _bestWL = _wlCost;
                    _bestWidth = bdWidth;
                    _bestHeight = bdHeight;
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
    } while ((double)rej/MT < 0.95 && T > 0.00001);
    cout << "AREA: " << _bestArea << " WL: " << _bestWL << endl;
    cout << "bestCost: " << 0.5*(_bestArea)+0.5*(_bestWL) << endl;
    cout << num[0] << " " << num[1] << " " << num[2] << endl;
    plot(BEST_FLOORPLAN);
}

void Floorplanner::plot(bool type)
{
    //gnuplot preset
	fstream outgraph("./floorplan.gp", ios::out);
	outgraph << "reset\n";
    outgraph << "set terminal png\n";
    outgraph << "set output \"floorplan.png\"\n";
	outgraph << "set tics\n";
	outgraph << "unset key\n";
	outgraph << "set title \"The result of FloorPlanning\"\n";
    outgraph << "set size noratio" << "\n";
    int screen_left = -_outlineWidth*0.1;
    int screen_right = _outlineWidth*1.1;
    int screen_down = -_outlineHeight*0.1;
    int screen_up = _outlineHeight*1.1;
	int index = 1;
 	// write block info into floorplan.gp
    outgraph << "set object " << index++ << " rect from "
		<< screen_left << "," << screen_down << " to "
		<< screen_right << "," << screen_up << " fc rgb 'black'\n";

    if (type == CURRENT_FLOORPLAN) {
        _bt->plotNode(outgraph, index);
        _bt->plotContourLine(outgraph, index);
    }
    else if (type == BEST_FLOORPLAN) {
        for (vector<Block*>::iterator it = _blkArray.begin(); it != _blkArray.end(); ++it) {
            Block* blk = *it;
            int x0 = blk->getfx1(), y0 = blk->getfy1();
            int x1 = blk->getfx2(), y1 = blk->getfy2();
            int midX = blk->getfmx(), midY = blk->getfmy();
            outgraph << "set object " << index++ << " rect from " 
                    << x0 << "," << y0 << " to " << x1 << "," << y1 << " fs empty border fc rgb 'green'\n"
                    << "set label " << "\"" << blk->getName() << "\"" << " at " << midX << "," << midY << " center " << "font \",8\" tc rgb \"white\"\n";
            /* draw B* tree */
            if (blk->getFL() != NULL) {
                outgraph << "set arrow " << index++ << " from "
                << midX << "," << midY << " to " << blk->getFL()->getfmx() << "," << blk->getFL()->getfmy() << " nohead lc rgb \'red\'\n";
            }
            if (blk->getFR() != NULL) {
                outgraph << "set arrow " << index++ << " from "
                << midX << "," << midY << " to " << blk->getFR()->getfmx() << "," << blk->getFR()->getfmy() << " nohead lc rgb \'red\'\n";
            }
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
	outgraph << "plot [" << screen_left << ":" << screen_right << "]["
		     << screen_down << ":" << screen_up << "]\'line\' w l lt 2 lw 1\n";
	outgraph << "set terminal x11 persist\n";
	//outgraph << "replot\n";
	outgraph << "exit";
	outgraph.close();
	int gnuplot = system("gnuplot floorplan.gp");
}

void Floorplanner::writeOutputFile(double runtime)
{
    fstream file;
    file.open("output.rpt", ios::out);
    file << std::fixed << _alpha*_bestArea + _beta*_bestWL << endl;
    file << std::fixed << _bestWL << endl;
    file << std::fixed << _bestArea << endl;
    file << _bestWidth << ' ' << _bestHeight << endl;
    file << runtime << endl;
    for (auto& it : _blkArray) 
        file << it->getName() << '\t' << it->getfx1() << '\t' << it->getfy1() << '\t' << it->getfx2() << '\t' << it->getfy2() << endl;
}