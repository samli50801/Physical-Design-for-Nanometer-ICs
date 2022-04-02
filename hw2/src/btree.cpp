#include <queue>
#include <assert.h>
#include <fstream>
#include "btree.h"
using namespace std;

#define LCHILD(idx) ((2*idx)+1)
#define RCHILD(idx) ((2*idx)+2)

void B_Tree::initialize()
{
    /* initialize B* tree */
    queue<size_t> q;
    _root = _nodeArray[0];
    q.push(0);
    while (!q.empty()) {
        size_t root = q.front();
        q.pop();
        size_t l = LCHILD(root), r = RCHILD(root);
        if (l < _nodeNum) {
            _nodeArray[root]->_l = _nodeArray[l];
            _nodeArray[l]->_p = _nodeArray[root];
            q.push(l);
        }
        if (r < _nodeNum) {
            _nodeArray[root]->_r = _nodeArray[r];
            _nodeArray[r]->_p = _nodeArray[root];
            q.push(r);
        }
    }
}

void B_Tree::reset()
{
    _ctl.clear();
    _ctl.push_front({{0, INT_MAX}, 0});
    _boundingBox.first = _boundingBox.second = 0;
}

Node* B_Tree::deleteNode(Node* delNode)
{
    if (delNode == NULL) return NULL;

    /* delete */
    if (delNode->_l == NULL && delNode->_r == NULL) {         // delete leaf node
        if (delNode->_p->_l == delNode) {
            _rec.push_front(Info(delNode->_p, DEL_INS, NULL, L, delNode->_p->_l));
            delNode->_p->_l = NULL;
        }
        else {
            _rec.push_front(Info(delNode->_p, DEL_INS, NULL, R, delNode->_p->_r));
            delNode->_p->_r = NULL;
        }
        _rec.push_front(Info(delNode, DEL_INS, NULL, P, delNode->_p));
        delNode->_p = NULL;
    } else if (delNode->_l == NULL || delNode->_r == NULL) {  // delete one child node
        Node *childOfDelete = (delNode->_l == NULL) ? delNode->_r : delNode->_l;
        if (delNode == _root) {
            _rec.push_front(Info(_root, ROOT, NULL, 0, NULL));
            _root = childOfDelete;
        }
        else if (delNode->_p->_l == delNode) {
            _rec.push_front(Info(delNode->_p, DEL_INS, NULL, L, delNode->_p->_l));
            delNode->_p->_l = childOfDelete;
        }
        else {
            _rec.push_front(Info(delNode->_p, DEL_INS, NULL, R, delNode->_p->_r));
            delNode->_p->_r = childOfDelete;
        }
        _rec.push_front(Info(childOfDelete, DEL_INS, NULL, P, childOfDelete->_p));
        childOfDelete->_p = delNode->_p;
        _rec.push_front(Info(delNode, DEL_INS, NULL, P, delNode->_p));
        _rec.push_front(Info(delNode, DEL_INS, NULL, L, delNode->_l));
        _rec.push_front(Info(delNode, DEL_INS, NULL, R, delNode->_r));
        delNode->_p = NULL;
        delNode->_l = NULL;
        delNode->_r = NULL;
    } else {                                                   // delete two child node
        Node* replace = rand()%2 ? delNode->_r : delNode->_l;
        _rec.push_front(Info(delNode, SWAP, delNode->_blk));
        _rec.push_front(Info(replace, SWAP, replace->_blk));
        std::swap(delNode->_blk, replace->_blk);
        return deleteNode(replace);
    }
    return delNode;
}

inline void B_Tree::insertNode(Node* node, Node* root)
{
    if (rand() % 2) {
        if (root->_l != NULL) {
            if (rand() % 2) {
                _rec.push_front(Info(node, DEL_INS, NULL, L, node->_l));
                node->_l = root->_l;
            }
            else {
                _rec.push_front(Info(node, DEL_INS, NULL, R, node->_r));
                node->_r = root->_l;
            }
            _rec.push_front(Info(root->_l, DEL_INS, NULL, P, root->_l->_p));
            root->_l->_p = node;
        }
        _rec.push_front(Info(root, DEL_INS, NULL, L, root->_l));
        root->_l = node;
    } else {
        if (root->_r != NULL) {
            if (rand() % 2) {
                _rec.push_front(Info(node, DEL_INS, NULL, L, node->_l));
                node->_l = root->_r;
            }
            else {
                _rec.push_front(Info(node, DEL_INS, NULL, R, node->_r));
                node->_r = root->_r;
            }
            _rec.push_front(Info(root->_r, DEL_INS, NULL, P, root->_r->_p));
            root->_r->_p = node;
        }
        _rec.push_front(Info(root, DEL_INS, NULL, R, root->_r));
        root->_r = node;
    }
    _rec.push_front(Info(node, DEL_INS, NULL, P, node->_p));
    node->_p = root;
}

void B_Tree::op_swap2nodes()
{
    /* select two swapped node randomly */
    Node *node1, *node2;
    do {
        node1 = _nodeArray[rand() % _nodeNum];
        node2 = _nodeArray[rand() % _nodeNum];
    } while (node1 == node2);
    _rec.push_front(Info(node1, SWAP, node1->_blk));
    _rec.push_front(Info(node2, SWAP, node2->_blk));
    std::swap(node1->_blk, node2->_blk);
}

void B_Tree::op_rotate()
{
    Node* node = _nodeArray[rand() % _nodeNum];
    _rec.push_front(Info(NULL, ROTATE, node->_blk, 0, NULL));
    node->_blk->rotate();
}

void B_Tree::op_deleteAndInsert()
{
    Node *delNode = deleteNode(_nodeArray[rand() % _nodeNum]);
    Node *insNode;
    do insNode = _nodeArray[rand() % _nodeNum]; while (insNode == delNode);
    insertNode(delNode, insNode);

}

int B_Tree::updateContourLine(Node* node)
{
    int start = node->getx(), end = node->getx() + node->getWidth();
    int maxHeight = 0;
    Interval newIntv = {{start, end}, 0};

    list<Interval>::iterator p = _ctl.end(), q = _ctl.begin();

    for (list<Interval>::iterator i = _ctl.begin(); i != _ctl.end(); ++i) {
        Interval it = *i;
        if (start < it.first.second && it.first.first < end)
            maxHeight = std::max(maxHeight, it.second);
        if (start >= it.first.first && start < it.first.second)
            p = i;
        if (end > it.first.first && end <= it.first.second) {
            q = ++i;
            break;
        }
    }
    assert(p!=_ctl.end() && q!=_ctl.begin());

    newIntv.second = maxHeight + node->getHeight();
    list<Interval>::iterator it = p;
    while (it != q) {
        //   -----  intv
        // ---------it
        if (start > (*it).first.first && end < (*it).first.second) {
            int org_s = (*it).first.first;
            int org_h = (*it).second;
            (*it).first.first = end;
            it = _ctl.insert(it, newIntv);
            _ctl.insert(it, {{org_s, start}, org_h});
            ++it;
        }
        // -----  intv
        //   -------it
        else if (start <= (*it).first.first && end < (*it).first.second) {
            (*it).first.first = end;
            if (it == p) _ctl.insert(it, newIntv);
        }
        //     -----  intv
        // -------it
        else if (start > (*it).first.first && end >= (*it).first.second) {
            int temp_s = (*it).first.first;
            int temp_h = (*it).second;
            (*it).first.first = start;
            (*it).first.second = end;
            (*it).second = newIntv.second;
            _ctl.insert(it, {{temp_s, start}, temp_h});
        }
        // --------  intv
        //  -----it
        else if (start <= (*it).first.first && end >= (*it).first.second) {
            if (it == p)
                _ctl.insert(it, newIntv);
            it = _ctl.erase(it);
            continue;
        }
        ++it;
    }
    return maxHeight;
}

void B_Tree::computeCoord_dfs(Node* root, int x)
{
    if (root == NULL) return;
    root->setx(x);
    root->sety(updateContourLine(root));
    _boundingBox.first = std::max(_boundingBox.first, root->getx() + root->getWidth());
    _boundingBox.second = std::max(_boundingBox.second, root->gety() + root->getHeight());
    computeCoord_dfs(root->_l, root->getx() + root->getWidth());
    computeCoord_dfs(root->_r, root->getx());
    return;
}

void B_Tree::computeCoord_bfs()
{
    _root->setx(0);
    queue<Node*> q;
    q.push(_root);

    while (!q.empty()) {
        Node* node = q.front();
        q.pop();
        node->sety(updateContourLine(node));
        if (node->_l != NULL) {
            node->_l->setx(node->getx() + node->getWidth());
            q.push(node->_l);
        }
        if (node->_r != NULL) {
            node->_r->setx(node->getx());
            q.push(node->_r);
        }
        _boundingBox.first = std::max(_boundingBox.first, node->getx() + node->getWidth());
        _boundingBox.second = std::max(_boundingBox.second, node->gety() + node->getHeight());
    }
}

void B_Tree::recover()
{
    while (!_rec.empty()) {
        Info it = _rec.front();
        switch (it._op) {
        case SWAP:
            it._node->_blk = it._blk;
            break;
        case ROTATE:
            it._blk->rotate();
            break;
        case DEL_INS:
            if (it._pntDir == L) it._node->_l = it._pntNode; else
            if (it._pntDir == R) it._node->_r = it._pntNode; else
            if (it._pntDir == P) it._node->_p = it._pntNode;
            break;
        case ROOT:
            _root = it._node;
            break;
        }
        _rec.pop_front();
    }
}

void B_Tree::printTree(Node* root)
{
    if (root == NULL) return;
    printTree(root->_l);
    printTree(root->_r);
    cout << root->getName() << endl;
    return;
}

void B_Tree::printContourLine()
{
    for (list<Interval>::iterator i = _ctl.begin(); i != _ctl.end(); ++i) {
        Interval it = *i;
        cout << "Interval: " << it.first.first << " " << it.first.second << " " << it.second << endl;
    }
}

void B_Tree::plotContourLine(fstream& outgraph, int& index) {
    for (auto it : _ctl)
        outgraph << "set arrow " << index++ << " from "
			<< it.first.first << "," << it.second << " to " << it.first.second << "," << it.second << " nohead lc rgb \'red\'\n";
}

void B_Tree::plotNode(fstream& outgraph, int& index) {
    for (vector<Node*>::iterator it = _nodeArray.begin(); it != _nodeArray.end(); ++it) {
        Node* node = *it;
        int x0 = node->getx(), y0 = node->gety();
        int x1 = x0 + node->getWidth(), y1 = y0 + node->getHeight();
        outgraph << "set object " << index++ << " rect from " 
		  		<< x0 << "," << y0 << " to " << x1 << "," << y1 << " fs empty border fc rgb 'green'\n"
                << "set label " << "\"" << node->getName() << "\"" << " at " << (x0+x1)/2.0 << "," << (y0+y1)/2.0 << " center " << "font \",8\" tc rgb \"white\"\n";
    }
}