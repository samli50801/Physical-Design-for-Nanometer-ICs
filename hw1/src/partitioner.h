#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <fstream>
#include <vector>
#include <map>
#include<queue>
#include "cell.h"
#include "net.h"
using namespace std;

class Partitioner
{
public:
    // constructor and destructor
    Partitioner(fstream& inFile) :
        _cutSize(0), _netNum(0), _cellNum(0), _maxPinNum(0), _bFactor(0),
        _accGain(0), _maxAccGain(0), _iterNum(0) {
        parseInput(inFile);
        _partSize[0] = 0;
        _partSize[1] = 0;
    }
    ~Partitioner() {
        clear();
    }

    // basic access methods
    inline int getCutSize() const          { return _cutSize; }
    inline int getNetNum() const           { return _netNum; }
    inline int getCellNum() const          { return _cellNum; }
    inline double getBFactor() const       { return _bFactor; }
    inline int getPartSize(int part) const { return _partSize[part]; }

    // modify method
    inline bool isBalanced(Node *swpNode);
    void parseInput(fstream& inFile);
    void initialPartition();
    Cell* getBestCell();
    inline void bucketAddNode(Node* node, size_t key);
    inline void bucketEraseNode(Node* node, size_t key);
    void bucketUpdate(Cell*, int diff);
    void updateCellGain(Cell* swpCell);
    void moveCellPerIter();
    void partition();

    // member functions about reporting
    void printSummary() const;
    void reportNet() const;
    void reportCell() const;
    void reportBucket() const;
    void writeResult(fstream& outFile);

    // DEBUG
    bool verifyAnswer();

private:
    int                 _cutSize;       // cut size
    int                 _initCutSize;   // initial cut size
    int                 _partSize[2];   // size (cell number) of partition A(0) and B(1)
    int                 _netNum;        // number of nets
    int                 _cellNum;       // number of cells
    int                 _maxPinNum;     // Pmax for building bucket list
    int                 _maxGain;
    double              _bFactor;       // the balance factor to be met
    double              _lowerBound;    // lower bound of balance
    double              _upperBound;    // upper bound of balance
    Node*               _maxGainCell;   // pointer to max gain cell
    vector<Net*>        _netArray;      // net array of the circuit
    vector<Cell*>       _cellArray;     // cell array of the circuit
    vector<Node*>       _bList;         // bucket list of partition A(0) and B(1)
    map<string, int>    _netName2Id;    // mapping from net name to id
    map<string, int>    _cellName2Id;   // mapping from cell name to id

    int                 _accGain;       // accumulative gain
    int                 _maxAccGain;    // maximum accumulative gain
    int                 _moveNum;       // number of cell movements
    int                 _iterNum;       // number of iterations
    int                 _bestMoveNum;   // store best number of movements
    int                 _unlockNum[2];  // number of unlocked cells
    vector<Cell*>       _moveStack;     // history of cell movement

    // Clean up partitioner
    void clear();
};

#endif  // PARTITIONER_H
