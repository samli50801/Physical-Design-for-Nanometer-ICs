#ifndef NET_H
#define NET_H

#include <vector>
using namespace std;

class Net
{
public:
    // constructor and destructor
    Net(string& name) :
        _name(name) {
        _partCount[0] = 0; _partCount[1] = 0;
    }
    ~Net()  { }

    // basic access methods
    inline string getName()           const { return _name; }
    inline int getPartCount(int part) const { return _partCount[part]; }
    inline vector<int>& getCellList() { return _cellList; }

    // set functions
    inline void setName(const string name) { _name = name; }
    inline void setPartCount(int part, const int count) { _partCount[part] = count; }

    // modify methods
    inline void incPartCount(int part)     { ++_partCount[part]; }
    inline void decPartCount(int part)     { --_partCount[part]; }
    inline void addCell(const int cellId)  { _cellList.push_back(cellId); }
    inline void resetPartCount()           { _partCount[0]=0; _partCount[1]=0; }

private:
    int             _partCount[2];  // Cell number in partition A(0) and B(1)
    string          _name;          // Name of the net
    vector<int>     _cellList;      // List of cells the net is connected to
};

#endif  // NET_H
