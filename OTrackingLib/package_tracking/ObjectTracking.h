// Copyright (c) 2013 All Right Reserved, Globant
// All other rights reserved.
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Author: Damian Czajkovski
// Email: damian.czajkovski@globant.com
// Date: 07/10/2013
// Summary: Header of the ObjectTracking object 
// ******* Change History *******
// 07/10/2013 - DCZ - Inital Version
// ******* Change History *******
#pragma once

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <math.h>
#include <ctime>
#include "cvblob/cvblob.h"

using namespace std;

#define SIZE_LST	100  //Max amount of objects supported by frame 

//Struct used to keep track of the amount of objects by frame
typedef struct Frame
{
	int frame;
    std::string imagesrc;
	std::string datetime;
    int objects;
}
Frame;

//This struct is used to keep the path of each object
typedef struct StagePoint
{
	int frame;
    int x;
    int y;
}
StagePoint;

//List of renderized tracks
typedef std::map<int, CvPoint> ObjTracks;

//Maps of lists of coordenates of each object
typedef std::map<int,std::list<StagePoint>> TrackingObj;

//List of objects by frame
typedef std::list<Frame> lstFrames;

//Base object, to keef the coordinate and size (area) of each single object
class Obj
{
public:
	Obj();
	~Obj(){};
	unsigned int tl_x;
	unsigned int tl_y;
	unsigned int x;
	unsigned int y;
	unsigned int Area;
	unsigned int width;
	unsigned int heigth;
};

//We keep on this stage the tracking information of each object
class Object:public Obj
{
public:
	Object();
	~Object(){};

	unsigned int id;				//if id is equal to zero doesn´t exists
	unsigned int uniqueID;			//This unique number is used to enumerate the renderized objects
	unsigned int apparenceNumber;	//If appeared more than 5 times, keep track of it
	unsigned int efi;				//Flag to indicate if the object was edited, found and interpreted
	unsigned int lastSeen;			//Last seen, if is greather than 50 remove it
	int prevx;						//previous x pos
	int prevy;						//previous y pos
};

class __declspec(dllexport) ObjectTracking
{
public:
	ObjectTracking();
	~ObjectTracking();

	void process(const cv::Mat &img_input, const cv::Mat &img_mask, cv::Mat &img_output);

	const cvb::CvTracks getTracks();

	//Localization operations
	void Update(Obj oblob, int nfr);
	int RenderImage(IplImage *fr);
	void SetSizes(unsigned int x, unsigned int y){maxX=x; maxY=y;};
	void WriteObjToFile();
	void SetFrameRate(int rate) {m_iFrameRate = rate;};

private:
	//Configuration
	bool firstTime;
	int minArea;			// Min area (to filter with)
	int maxArea;			// Max area (to filter with)
	int prcVarDist;			// prc variations distance between frames
	int prcVarSize;			// prc variacion of area between frames
	bool debugTrack;		// Renderize flag
	bool debugBlob;			// Renderize flag
	bool showBlobMask;		// Renderize flag
	bool showOutput;		// Renderize flag
	bool keepFileTrack;		// Renderize flag

	std::string  trackFileName;	//Image to be saved
	int m_iSavePathFileNFrames;	//Number of frames to save for statistics 
	std::string pathsIndexFileName;	// Json
	std::string pathFileName;		// Json
	std::string pathFramesFileName;	// Json
	std::string imageFrameFileName;	// Json
	bool statisticTrack;    // Keep statistic track

	//Counters
	int m_iFramesPathCounter;  
	int pathsCounter;

	void saveConfig();
	void loadConfig();

	//Kernel conversion to blur the image to aislate the blobs 
	IplConvKernel* morphKernel;

	//font to renderize the number of each id
	CvFont font;
	unsigned int maxX;
	unsigned int maxY;
	
	//Object Tracks for counting purposes
	cvb::CvTracks tracks;

	//List for the movement statistics
	lstFrames frames;

	//Array of object for the capture
	Object list[SIZE_LST];

	//Counters
	long nframes;
	long currentFrame;
	int uniqueIdCounter;

	//map objects
	TrackingObj trobjs;

	//to control the frame rate
	int m_iFrameRateCounter;
	int m_iFrameRate;

private:
	//Methods
	int findObjectByXY(int x, int y, int &id);
	int checkObjectBySize(unsigned int width, unsigned int heigth, unsigned int S, int &id);
	void updateObject(Obj ob, int id);
	void addObject(Obj ob);
	void removeOld();
	void RenderObject(IplImage *fr,int id,CvScalar color);
	StagePoint getStagePoint( int x, int y);
	void UpdateFilePath(int iCantObjs, IplImage *fr);
};

