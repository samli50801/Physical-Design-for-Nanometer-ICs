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
        _name(name), _x(x), _y(y) { }
    ~Terminal()  { }

    // basic access methods
    const string getName()  { return _name; }
    const double getX()    { return _x; }
    const double getY()    { return _y; }

    // set functions
    void setName(string& name) { _name = name; }
    void setx(double x)  { _x = x; }
    void sety(double y)  { _y = y; }
    
protected:
    string      _name;      // module name
    double      _x;        // center x coordinate of the terminal
    double      _y;        // center y coordinate of the terminal
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
    void setFL(Block* b)    {_fl = b;}
    void setFR(Block* b)    {_fr = b;}

    inline void rotate()    {std::swap(this->_w, this->_h);}
private:
    int             _w;         // width of the block
    int             _h;         // height of the block
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
    for (size_t i = 0; i < _netDegree; ++i) {
       	minX = std::min(minX, _termList[i]->getX());
        minY = std::min(minY, _termList[i]->getY());
        maxX = std::max(maxX, _termList[i]->getX());
       	maxY = std::max(maxY, _termList[i]->getY());
    }
    return (maxX - minX) + (maxY - minY);
}
#endif  // MODULE_H
