#ifndef IMGMANAGER_H
#define IMGMANAGER_H

#include <string>
#include <unordered_map>

#include <boost\filesystem.hpp>

#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "basic_4dimage.h"
#include "v3d_interface.h"

struct registeredImg
{
	registeredImg(string imgFullName);

	string imgAlias;
	string imgFullPathName;
	Image4DSimple* thisImg4DPtr;
	unsigned char* imgData1D;
	long int dims[4];
	int datatype;
};

class ImgManager
{
public: 
	/********* Constructors *********/
	ImgManager() {};
	ImgManager(string wholeImgName);
	/********************************/

	/********* IO *********/
	static bool img1Ddumpster(Image4DSimple* imgPtr, unsigned char*& data1D, long int dims[4], int datatype);
	static inline bool saveimage_wrapper(const char* filename, unsigned char* pdata, V3DLONG sz[4], int datatype);
	
	unordered_map<string, registeredImg> imgDataBase;
	/**********************/

	/********* Methods for generating binary masks from SWC files *********/
	void swc2Mask_2D(string swcFileName, long int dims[2], unsigned char*& mask1D); // Generate a 2D mask based on the corresponding "SWC slice."
	bool getMarkersBetween(vector<MyMarker>& allmarkers, MyMarker m1, MyMarker m2);

	void detectedNodes2mask_2D(QList<NeuronSWC>* nodeListPtr, long int dims[2], unsigned char*& mask1D);
	/**********************************************************************/

	/********* Assemble all SWC masks together as an "SWC mip mask." *********/
	void MaskMIPfrom2Dseries(string path);                       
	/*************************************************************************/

	/********* Dessemble image/stack into tiles. This is for Caffe's memory leak issue *********/
	static void imgSliceDessemble(string imgName, int tileSize);
	/*******************************************************************************************/
};

inline bool ImgManager::saveimage_wrapper(const char* filename, unsigned char pdata[], V3DLONG sz[4], int datatype)
{
	if (!pdata)
	{
		cerr << "input array not valid" << endl;
		return false;
	}

	if (!filename || !sz)
	{
		v3d_msg("some of the parameters for simple_saveimage_wrapper() are not valid.", 0);
		return false;
	}

	ImagePixelType dt;
	if (datatype == 1) dt = V3D_UINT8;
	else if (datatype == 2) dt = V3D_UINT16;
	else if (datatype == 4) dt = V3D_FLOAT32;
	else
	{
		v3d_msg(QString("the specified save data type in simple_saveimage_wrapper() is not valid, dt=[%1].").arg(datatype), 0);
		return false;
	}

	Image4DSimple* outimg = new Image4DSimple;
	if (outimg) outimg->setData(pdata, sz[0], sz[1], sz[2], sz[3], dt);
	else
	{
		v3d_msg("Fail to new Image4DSimple for outimg.");
		return false;
	}
	outimg->saveImage(filename);
	
	return true;
}

#endif