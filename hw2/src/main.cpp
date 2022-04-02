#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include "floorplanner.h"
using namespace std;

int main(int argc, char** argv)
{
    fstream input_blk, input_net, output;
    double alpha;

    if (argc == 5) {
        alpha = stod(argv[1]);
        input_blk.open("input_pa2/" + (string)argv[2], ios::in);
        input_net.open("input_pa2/" + (string)argv[3], ios::in);
        output.open(argv[4], ios::out);
        if (!input_blk) {
            cerr << "Cannot open the input file \"" << argv[2]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!input_net) {
            cerr << "Cannot open the input file \"" << argv[3]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!output) {
            cerr << "Cannot open the output file \"" << argv[4]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
    }
    else {
        cerr << "Usage: ./Floorplanner <alpha> <input block file> " <<
                "<input net file> <output file>" << endl;
        exit(1);
    }

    double beta = 0.2;
    Floorplanner* fp = new Floorplanner(alpha, beta, input_blk, input_net);
    fp->floorplan();
    return 0;
}
