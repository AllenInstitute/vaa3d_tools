/* getlevel0data_func.h
 * a plugin to get level 0 data
 * 10/10/2018 : by Yang Yu
 */
 
#ifndef __NEURONRECON_FUNC_H__
#define __NEURONRECON_FUNC_H__

//
#include <QtGlobal>
#include <vector>
#include "v3d_interface.h"
#include "v3d_message.h"
#include <iostream>
#include "basic_4dimage.h"
#include "algorithm"
#include <map>
#include <cmath>
#include <climits>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <string>
#include <tuple>
#include <stack>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;

enum  axis { vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0 };

// cube
class Cube
{
public:
    Cube();
    ~Cube();

public:
    int offset_D; // z

    string fileName; // 000000_000000_000000.tif
    string filePath; // ourdir/RESXXxXXxXX/000000/000000_000000/000000_000000_000000.tif
    unsigned int depth; // 256
    bool toBeCopied;
};

// folder
class YXFolder
{
public:
    YXFolder();
    ~YXFolder();

public:
    int offset_V; // y
    int offset_H; // x

    unsigned short lengthFileName; // 25 len("000000_000000_000000.tif") + 1
    unsigned short lengthDirName; // 21 len("000000/000000_000000") + 1

    string dirName; // 000000/000000_000000
    string xDirPath; // ourdir/RESXXxXXxXX/000000
    string yDirPath; // ourdir/RESXXxXXxXX/000000/000000_000000

    unsigned int height, width; // 256x256

    map<int,Cube> cubes;
};

// layer
class Layer
{
public:
    Layer();
    ~Layer();

public:
    unsigned short rows, cols; // floor(dim_V/height)+1, floor(dim_H/width)+1
    unsigned int ncubes;
    float vs_x, vs_y, vs_z; // voxel sizes
    unsigned int dim_V, dim_H, dim_D; // dimensions y, x, z

    string layerName; // outdir/RESXXxXXxXX

    map<string, YXFolder> yxfolders; // <dirName, YXFolder>
};

// swc
class Point
{
public:
    Point(float a, float b, float c);
    ~Point();

public:
    float x,y,z;
};

typedef vector<Point> PointCloud;

// nodes of tree
class Block
{
public:
    Block(string fn, long xoff, long yoff, long zoff, long sx, long sy, long sz);
    ~Block();

public:
    string filepath;
    long offset_x, offset_y, offset_z;
    long size_x, size_y, size_z;
};

typedef map<long, Block> OneScaleTree; // offset_z*dimx*dimy+offset_y*dimx+offset_x

//
class QueryAndCopy
{
public:
    QueryAndCopy(string swcfile, string inputdir, string outputdir, float ratio);
    ~QueryAndCopy();
public:
    int readSWC(string filename, float ratio);
    int readMetaData(string filename);

    int copyblock(string filepath, string outputdir);
    int makeDir(string dirname);

    int query(long x, long y, long z);

public:
    OneScaleTree tree;
    PointCloud pc;

public:
    // mdata.bin
    float org_V, org_H, org_D; // offsets (0, 0, 0)
    axis reference_V, reference_H, reference_D; // vertical, horizonal, depth
    float mdata_version; // 2

    unsigned int color, bytesPerVoxel; //
    unsigned int cubex, cubey, cubez;

    Layer layer;
};


// functions
bool getlevel0data_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);


#endif

