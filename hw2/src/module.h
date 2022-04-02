#ifndef MODULE_H
#define MODULE_H
#include <vector>
#include <string>
#include <climits>
#include <algorithm>
using namespace std;

class Terminal
{
public:
    // constructor and destructor
    Terminal(string& name, size_t x, size_t y) :
        _name(name), _x1(x), _y1(y), _x2(x), _y2(y) { }
    ~Terminal()  { }

    // basic access methods
    const string getName()  { return _name; }
    const size_t getX1()    { return _x1; }
    const size_t getX2()    { return _x2; }
    const size_t getY1()    { return _y1; }
    const size_t getY2()    { return _y2; }

    // set functions
    void setName(string& name) { _name = name; }
    void setx(int x1, int x2)  { _x1 = x1; _x2 = x2; }
    void sety(int y1, int y2)  { _y1 = y1; _y2 = y2; }
    void setPos(size_t x1, size_t y1, size_t x2, size_t y2) {
        _x1 = x1;   _y1 = y1;
        _x2 = x2;   _y2 = y2;
    }

protected:
    string      _name;      // module name
    size_t      _x1;        // min x coordinate of the terminal
    size_t      _y1;        // min y coordinate of the terminal
    size_t      _x2;        // max x coordinate of the terminal
    size_t      _y2;        // max y coordinate of the terminal
};

class Block : public Terminal
{
public:
    // constructor and destructor
    Block(string& name, int w, int h) :
        Terminal(name, 0, 0), _w(w), _h(h) { }
    ~Block() { }

    // basic access methods
    inline const size_t getWidth(bool rotate = false)  { return rotate? _h: _w; }
    inline const size_t getHeight(bool rotate = false) { return rotate? _w: _h; }
    inline const int getArea()  { return _h * _w; }
    inline static size_t getMaxX() { return _maxX; }
    inline static size_t getMaxY() { return _maxY; }
    int getfx1() {return _fx1;}
    int getfy1() {return _fy1;}
    int getfx2() {return _fx2;}
    int getfy2() {return _fy2;}
    double getfmx() {return (_fx1+_fx2)/2.0;}
    double getfmy() {return (_fy1+_fy2)/2.0;}
    Block* getFL()  {return _fl;}
    Block* getFR()  {return _fr;}

    // set functions
    void setFinalPos(int x1, int y1, int x2, int y2)    {_fx1=x1; _fy1=y1; _fx2=x2; _fy2=y2;}
    inline void setWidth(int w)            { _w = w; }
    inline void setHeight(int h)           { _h = h; }
    inline static void setMaxX(size_t x)   { _maxX = x; }
    inline static void setMaxY(size_t y)   { _maxY = y; }
    void setFL(Block* b)    {_fl = b;}
    void setFR(Block* b)    {_fr = b;}

    inline void rotate()    {std::swap(this->_w, this->_h);}
private:
    int             _w;         // width of the block
    int             _h;         // height of the block
    static size_t   _maxX;      // maximum x coordinate for all blocks
    static size_t   _maxY;      // maximum y coordinate for all blocks
    /* result info. */
    int             _fx1;        // final lower-left x
    int             _fy1;        // final lower-left y
    int             _fx2;        // final upper-right x
    int             _fy2;        // final upper-right y
    Block           *_fl, *_fr;  // final left and right child of each block
};


class Net
{
public:
    // constructor and destructor
    Net(int netDegree) : _netDegree(netDegree) { 
        _termList.resize(netDegree);
    }
    ~Net()  { }

    // basic access methods
    inline vector<Terminal*>& getTermList()   { return _termList; }
    inline int getNetDegree()                 { return _netDegree; }

    // other member functions
    inline double calcHPWL();

private:
    int                 _netDegree;
    vector<Terminal*>   _termList;  // list of terminals the net is connected to
};

inline double Net::calcHPWL()
{
    double minX = INT_MAX, minY = INT_MAX;
    double maxX = INT_MIN, maxY = INT_MIN;
    double midX, midY;
    for (size_t i = 0; i < _netDegree; ++i) {
        midX = (double)(_termList[i]->getX1()+_termList[i]->getX2()) / 2.0;
        midY = (double)(_termList[i]->getY1()+_termList[i]->getY2()) / 2.0;
        minX = std::min(minX, midX);
        minY = std::min(minY, midY);
        maxX = std::max(maxX, midX);
        maxY = std::max(maxY, midY);
    }
    return (maxX - minX) + (maxY - minY);
}
#endif  // MODULE_H
