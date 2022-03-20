#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include "cell.h"
#include "net.h"
#include "partitioner.h"
using namespace std;

#define L 0
#define R 1
#define G2I(GAIN)  (GAIN+_maxPinNum)
#define I2G(INDEX) (INDEX-_maxPinNum)

void Partitioner::parseInput(fstream& inFile)
{
    string str;
    // Set balance factor
    inFile >> str;
    _bFactor = stod(str);

    // Set up whole circuit
    while (inFile >> str) {
        if (str == "NET") {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
            int netId = _netNum;
            _netArray.push_back(new Net(netName));
            _netName2Id[netName] = netId;
            while (inFile >> cellName) {
                if (cellName == ";") {
                    tmpCellName = "";
                    break;
                }
                else {
                    // a newly seen cell
                    if (_cellName2Id.count(cellName) == 0) {
                        int cellId = _cellNum;
                        _cellArray.push_back(new Cell(cellName, 0, cellId));
                        _cellName2Id[cellName] = cellId;
                        _cellArray[cellId]->addNet(netId);
                        _cellArray[cellId]->incPinNum();
                        _netArray[netId]->addCell(cellId);
                        ++_cellNum;
                        tmpCellName = cellName;
                        _maxPinNum = max(_maxPinNum, _cellArray[cellId]->getPinNum());
                    }
                    // an existed cell
                    else {
                        if (cellName != tmpCellName) {
                            assert(_cellName2Id.count(cellName) == 1);
                            int cellId = _cellName2Id[cellName];
                            _cellArray[cellId]->addNet(netId);
                            _cellArray[cellId]->incPinNum();
                            _netArray[netId]->addCell(cellId);
                            tmpCellName = cellName;
                            _maxPinNum = max(_maxPinNum, _cellArray[cellId]->getPinNum());
                        }
                    }
                }
            }
            ++_netNum;
        }
    }
    return;
}

inline bool
Partitioner::isBalanced(Node *swpNode) 
{
    Cell* swpCell = _cellArray[swpNode->getId()];
    double lowerBound = (1.0-getBFactor()) / 2.0 * (double)getCellNum();
    double upperBound = (1.0+getBFactor()) / 2.0 * (double)getCellNum();
    int sizeOfF = _partSize[swpCell->getPart()] - 1;
    int sizeOfT = _partSize[!swpCell->getPart()] + 1;
    return (((double)sizeOfF>=lowerBound) && ((double)sizeOfF<=upperBound)) && (((double)sizeOfT>=lowerBound)&&((double)sizeOfT<=upperBound));
}

inline void 
Partitioner::bucketAddNode(Node* node, size_t key)
{
    if (_bList[G2I(key)] != NULL) {
        Node *root_node = _bList[G2I(key)];
        Node *last_node = root_node->getPrev();
        last_node->setNext(node);
        node->setPrev(last_node);
        root_node->setPrev(node);
    } else {
        _bList[G2I(key)] = node;
        node->setPrev(node);
    }
    return;
}
inline void 
Partitioner::bucketEraseNode(Node* node, size_t key)
{
    if (node->getPrev() == node) {                  // []: node
        _bList[G2I(key)] = NULL;
    } else if (_bList[G2I(key)] == node) {          // []: node X
        Node *newRoot = node->getNext();
        newRoot->setPrev(node->getPrev());
        _bList[G2I(key)] = newRoot;
    } else if (node->getNext() == NULL){            // []: X node
        node->getPrev()->setNext(NULL);
        _bList[G2I(key)]->setPrev(node->getPrev());
    } else {                                        // []: X node X
        node->getPrev()->setNext(node->getNext());
        node->getNext()->setPrev(node->getPrev());
    }
    node->setPrev(NULL);
    node->setNext(NULL);
    return;
}

void
Partitioner::initialPartition() {

    _bList.resize((_maxPinNum * 2) + 1);
    for (auto& it : _bList)
        it = NULL;

    /* initial partition */
    for (int i = 0; i < _cellNum; ++i) {
        bool side = i < _cellNum/2;
        _cellArray[i]->setPart(side);
        ++_partSize[side];
        vector<int>& netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
            _netArray[netList[j]]->incPartCount(side);
        }
    }
    /* initial gain */
    for (size_t i = 0; i < _netNum; ++i) {

        if (_netArray[i]->getPartCount(L) != 0 && _netArray[i]->getPartCount(R) != 0)
            ++_cutSize;

        vector<int>& cellList = _netArray[i]->getCellList();
        for (vector<int>::iterator c = cellList.begin(); c != cellList.end(); ++c) {
            Cell* cell = _cellArray[*c];
            if (_netArray[i]->getPartCount(cell->getPart()) == 1)
                cell->incGain();
            if (_netArray[i]->getPartCount(!cell->getPart()) == 0)
                cell->decGain();
        }
    }

    /* initial bucket list */
    for (vector<Cell*>::iterator c = _cellArray.begin(); c != _cellArray.end(); ++c) {
        Cell *cell = *c;
        _maxGain = (_maxGain, cell->getGain());
        bucketAddNode(cell->getNode(), cell->getGain());
    }
    return;
}

Cell* 
Partitioner::getSwapCell()
{
    Node *node;
    for (int i = _bList.size()-1; i >= 0; --i) {

        node = _bList[i];

        if (node == NULL) 
            continue;

        while (node != NULL && !isBalanced(node)) 
            node = node->getNext();

        if (node != NULL) {
            _maxGain = I2G(i);
            return _cellArray[node->getId()];
        } 
    }
    return NULL;
}

void 
Partitioner::bucketUpdate(Cell* cell, int diff) 
{
    int old_gain = cell->getGain();
    int new_gain = cell->getGain() + diff;
    Node *node = cell->getNode();

    /* remove old value */
    bucketEraseNode(node, old_gain);
    /* add new value */
    bucketAddNode(node, new_gain);
    return;
}

void 
Partitioner::updateCellGain(Cell* swpCell) 
{   
    bool swpPart = swpCell->getPart();
    vector<int>& netList = swpCell->getNetList();

    // erase swap node from the bucket
    bucketEraseNode(swpCell->getNode(), swpCell->getGain());

    for (size_t i = 0, i_end = netList.size(); i < i_end; ++i) {
        Net *net = _netArray[netList[i]];

        int beforeT = net->getPartCount(!swpPart);

        /* check critical nets before the move */
        if (beforeT == 0) {
            vector<int>& cellList = net->getCellList();
            for (vector<int>::iterator cidx = cellList.begin(); cidx != cellList.end(); ++cidx) {
                if (!_cellArray[*cidx]->getLock()) {
                    bucketUpdate(_cellArray[*cidx], 1);
                    _cellArray[*cidx]->incGain();
                }

            }
        } else if (beforeT == 1) {
            vector<int>& cellList = net->getCellList();
            for (vector<int>::iterator cidx = cellList.begin(); cidx != cellList.end(); ++cidx) {
                if (!_cellArray[*cidx]->getLock() && _cellArray[*cidx]->getPart() != swpPart) {
                    bucketUpdate(_cellArray[*cidx], -1);
                    _cellArray[*cidx]->decGain();
                }
            }
        }

        /* change F(n) and T(n) to reflect the move */
        net->decPartCount(swpPart);
        net->incPartCount(!swpPart);

        /* check critical nets after the move */
        int afterF = net->getPartCount(swpPart);
        if (afterF == 0) {
            vector<int>& cellList = net->getCellList();
            for (vector<int>::iterator cidx = cellList.begin(); cidx != cellList.end(); ++cidx) {
                if (!_cellArray[*cidx]->getLock())  {
                    bucketUpdate(_cellArray[*cidx], -1);
                    _cellArray[*cidx]->decGain();
                }
            }
        } else if (afterF == 1) {
            vector<int>& cellList = net->getCellList();
            for (vector<int>::iterator cidx = cellList.begin(); cidx != cellList.end(); ++cidx) {
                if (!_cellArray[*cidx]->getLock() && _cellArray[*cidx]->getPart() == swpPart) {
                    bucketUpdate(_cellArray[*cidx], 1);
                    _cellArray[*cidx]->incGain();
                }
            }
        }
    }
    return;
}

void Partitioner::moveCellPerIter()
{
    _cutSize -= _maxAccGain;
    _partSize[L] = 0;
    _partSize[R] = 0;

    for (int i = 0; i <= _bestMoveNum; ++i) 
        _moveStack[i]->move();

    /* clean net's part */
    for (size_t i = 0, i_end = _netArray.size(); i < i_end; ++i)
        _netArray[i]->resetPartCount();

    /* reset net's part */
    for (size_t i = 0; i < _cellNum; ++i) {
        bool part = _cellArray[i]->getPart();
        /* clean GAIN */
        _cellArray[i]->setGain(0);
        /* recalculate partSize */ 
        ++_partSize[part];
        vector<int>& netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) 
            _netArray[netList[j]]->incPartCount(part);
    }

    /* initial gain */
    for (size_t i = 0; i < _netNum; ++i) {

        vector<int>& cellList = _netArray[i]->getCellList();
        for (vector<int>::iterator c = cellList.begin(); c != cellList.end(); ++c) {
            Cell* cell = _cellArray[*c];
            if (_netArray[i]->getPartCount(cell->getPart()) == 1)
                cell->incGain();
            if (_netArray[i]->getPartCount(!cell->getPart()) == 0)
                cell->decGain();
        }
    }

    /* initial bucket list */
    if (_maxAccGain > 0) {
        for (vector<Cell*>::iterator c = _cellArray.begin(); c != _cellArray.end(); ++c) {
            Cell *cell = *c;
            _maxGain = (_maxGain, cell->getGain());
            bucketAddNode(cell->getNode(), cell->getGain());
        }
    }
    
}

void Partitioner::partition()
{
    initialPartition();
    //reportBucket();
    cout << "initial _cutSize: " << _cutSize << endl;

    do {

        _accGain = 0;
        _maxAccGain = 0;
        _bestMoveNum = -1;
        _moveStack.clear();

        for (size_t i = 0; i < _cellNum; ++i)
            _cellArray[i]->unlock();

        for (int i = 0; i < _cellNum; ++i) {

            Cell* swpCell = getSwapCell();
            _moveStack.push_back(swpCell);
            swpCell->lock();

            --_partSize[swpCell->getPart()];
            ++_partSize[!swpCell->getPart()];

            updateCellGain(swpCell);

            _accGain += _maxGain;
            if (_accGain > _maxAccGain) {
                _maxAccGain = _accGain;
                _bestMoveNum = i;
            }
            //reportBucket();
        }
        //cout << "final _accGain: " << _accGain << endl;
        //cout <<"_maxAccGain: " << _maxAccGain << endl;
        //cout << "_bestMoveNum: " << _bestMoveNum << endl;
        moveCellPerIter();
        //reportBucket();

    } while (_maxAccGain > 0);
}

void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[L] << endl;
    /*for (size_t i = 0; i < _cellNum; ++i) 
        if (_cellArray[i]->getPart() == L)
            cout << _cellArray[i]->getName() << " ";
    cout << endl;*/
    cout << " Cell Number of partition B: " << _partSize[R] << endl;
    /*for (size_t i = 0; i < _cellNum; ++i) 
        if (_cellArray[i]->getPart() == R)
            cout << _cellArray[i]->getName() << " ";*/
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int> cellList = _netArray[i]->getCellList();
        for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j) {
            cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int> netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
            cout << setw(8) << _netArray[netList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportBucket() const
{
    cout << "======================Bucket=====================" << endl;
    for (int i = 0; i < _bList.size(); ++i) {
        cout << I2G(i) << ": ";
        Node* n = _bList[i];
        while (n != NULL) {
            cout << _cellArray[n->getId()]->getName() << ' ';
            n = n->getNext();
        }
        cout << endl;
    }
    cout << "=================================================" << endl;
}

void Partitioner::writeResult(fstream& outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}
