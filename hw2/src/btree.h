#ifndef BTREE_H
#define BTREE_H
#include <iostream>
#include <random>
#include <list>
#include <tuple>
#include "module.h"

typedef pair<pair<int, int>, int> Interval;
enum {L, R, P, SWAP, ROTATE, DEL_INS, ROOT};

class Node 
{
    friend class B_Tree;
public:
    Node() {}
    Node(Block* blk) : _blk(blk), _l(NULL), _r(NULL), _p(NULL) {}
    ~Node() {}

    int getWidth()      { return _blk->getWidth(); }
    int getHeight()     { return _blk->getHeight(); }
    int getArea()       { return getWidth() * getHeight(); }
    int getx()          { return _x; }
    int gety()          { return _y; }
    Node* get_l()       { return _l; }
    Node* get_r()       { return _r; }
    Block* getBlk()     { return _blk; }
    string getName()    { return _blk->getName(); }
    void setx(int x)    { _x = x; _blk->setx((2*x+_blk->getWidth())/2.0); }
    void sety(int y)    { _y = y; _blk->sety((2*y+_blk->getHeight())/2.0); }
private:
    int     _x, _y;
    Node    *_l, *_r, *_p;
    Block*  _blk;
};

typedef tuple<short, Node*, Block*, short, Node*> Info;
class B_Tree 
{
public:
    B_Tree(vector<Block*>& blkArray, int outlineW, int outlineH) : 
    _nodeNum(blkArray.size()), _outlineW(outlineW), _outlineH(outlineH) {
        srand(time(NULL));
        _nodeArray.resize(blkArray.size());
        for (size_t i = 0, i_end = blkArray.size(); i < i_end; ++i) 
            _nodeArray[i] = new Node(blkArray[i]);
        std::sort(_nodeArray.begin(), _nodeArray.end(), 
        [](Node* a, Node* b) {return a->getArea() > b->getArea();});
        
        _ctl.push_front({{0, INT_MAX}, 0});
        _boundingBox.first = _boundingBox.second = 0;
    }
    ~B_Tree() {}

    /* access func */
    Node* getRoot()                 { return _root; }
    vector<Node*>& getNodeArray()   { return _nodeArray; }
    pair<int, int> get_bd_box()     { return _boundingBox; }

    /* contour line operation */
    int updateContourLine(Node*);
    void printContourLine();

    /* B* tree operation */
    void initialize();
    void reset();
    void printTree(Node*);
    void insertNode(Node*, Node*);
    void computeCoord_dfs(Node*, int);
    void computeCoord_bfs();
    Node* deleteNode(Node*);

    /* Perturbation */
    void op_swap2nodes();
    void op_rotate();
    void op_deleteAndInsert();

    /* recover operation */
    void recover();
    void clearRecover() { _rec.clear(); }

    /* draw */
    void plotContourLine(fstream&, int&);
    void plotNode(fstream&, int&);
private:
    int             _outlineW;
    int             _outlineH;
    int             _nodeNum;       // number of tree node
    pair<int, int>  _boundingBox;   // record maximum x/y coord.
    Node*           _root;          // root of B* tree
    vector<Node*>   _nodeArray;     // node array of B* tree
    list<Interval>  _ctl;           // contour line
    list<Info>      _rec;           // record changes and recover to previous tree
};

#endif