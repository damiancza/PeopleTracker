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
// Summary: Source of the ObjectTracking object 
// ******* Change History *******
// 07/10/2013 - DCZ - Inital Version
// ******* Change History *******

#include "ObjectTracking.h"
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <ctime>

using namespace std;
using namespace cvb;
using namespace cv;

Obj::Obj() : 
	x(0),
	y(0),
	tl_x(0),
	tl_y(0),
	Area(0),
	width(0),
	heigth(0)
{
}

Object::Object() : 
	id(0),
	uniqueID(0),
	apparenceNumber(0),
	efi(0),
	lastSeen(0),
	prevx(0),
	prevy(0)
{
}

ObjectTracking::ObjectTracking() : 
	firstTime(true), 
	minArea(500), 
	maxArea(20000), 
	prcVarSize(20), 
	prcVarDist(100), 
	debugBlob(false), 
	showBlobMask(true), 
	showOutput(true), 
	keepFileTrack(true),
	nframes(0),
	uniqueIdCounter(0),
	m_iSavePathFileNFrames(0),
	m_iFramesPathCounter(0),
	pathsCounter(1),
	statisticTrack(false),
	m_iFrameRateCounter(0),
	m_iFrameRate(25)//por defecti
{
	//Font size
	double hScale=0.5;
	double vScale=0.5; 
	int    lineWidth=1;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, hScale, vScale, 0, lineWidth);
	//Initialize the object array
	for(int i=0; i<SIZE_LST; ++i)
	{
		list[i].id=0;
	}
	morphKernel = NULL;
}

ObjectTracking::~ObjectTracking()
{
	std::cout << "~ObjectTracking()" << std::endl;
}

const cvb::CvTracks ObjectTracking::getTracks()
{
  return tracks;
}

void ObjectTracking::UpdateFilePath(int iCantObjs, IplImage *fr)
{
	/*
	paths json file model
	=====================
	{
		"frames" :
		[{		
			"frame": 1",
			"imagesrc" : "1.jpg",
			"objects" : 2
		},
		{
			"frame": 2",
			"imagesrc" : "2.jpg",
			"objects" : 4
		},
		{
			"frame": 3",
			"imagesrc" : "3.jpg",
			"objects" : 5
		}]
	}

	*/

	if(m_iFramesPathCounter <= m_iSavePathFileNFrames)
	{
		m_iFramesPathCounter++;

		//Save the file
		char pathImg[255];

		//format the image name
		sprintf((char*)&pathImg,imageFrameFileName.c_str(),nframes);

		cvSaveImage(pathImg ,fr); 

		Frame frm;
		frm.frame = nframes;
		frm.imagesrc = pathImg;
		
		frm.objects = iCantObjs;

		frames.push_back(frm);

		if(m_iFramesPathCounter == m_iSavePathFileNFrames)
		{
			char pathJson[255];

			//Format the Json name
			sprintf((char*)&pathJson,pathFileName.c_str(),pathsCounter);
			//update the frame file
			std::ofstream file;
			file.open(pathJson);

			std::ostringstream cout;

			/*
			{
			   "frames":[
				  {
					 "frame":1,
					 "imagesrc":"1.jpg",
					 "objects":2
				  },
				  {
					 "frame":2,
					 "imagesrc":"2.jpg",
					 "objects":4
				  },
				  {
					 "frame":3,
					 "imagesrc":"3.jpg",
					 "objects":5
				  }
			   ]
			}
			*/
			cout << "{" << endl;
			cout << "\"frames\":[" << endl;
			int iCount = 0;

			for (std::list<Frame>::iterator it=frames.begin(); it != frames.end(); ++it,++iCount)
			{
				Frame frm;
				string date;

				frm = *it;
				cout << "{" << endl;
				cout << "\"frame\":" << frm.frame << "," << endl;
				cout << "\"imagesrc\":\"" << frm.frame << ".jpg\"," << endl;
				cout << "\"datetime\":\"" << frm.datetime << "\"," << endl;
				cout << "\"objects\":" << frm.objects << endl;

				//if is the last, no comma and out the loop
				if(iCount+1 == frames.size())
					cout << "}" << endl;
				else
					cout << "}," << endl;
			}

			cout << "]" << endl;
			cout << "}" << endl;

			file << cout.str();

			file.close();
			cout.flush();
		}
	}

	m_iFrameRateCounter = 0;
}

void ObjectTracking::process(const cv::Mat &img_input, const cv::Mat &img_mask, cv::Mat &img_output)
{
	if(img_input.empty() || img_mask.empty())
		return;

	loadConfig();

	if(firstTime)
		saveConfig();

	++nframes;

	IplImage* frame = new IplImage(img_input);
	cvConvertScale(frame, frame, 1, 0);

	if( nframes == 1)
	{
		SetSizes(frame->width, frame->height);
	}
	
	IplImage* segmentated = new IplImage(img_mask);
  
	morphKernel = cvCreateStructuringElementEx(5, 5, 1, 1, CV_SHAPE_RECT, NULL);
	cvMorphologyEx(segmentated, segmentated, NULL, morphKernel, CV_MOP_OPEN, 1);

	//if(showBlobMask)***DEBUG***
	//cvShowImage("MASK", segmentated); ***DEBUG***

	IplImage* labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);

	cvb::CvBlobs blobs;
	unsigned int result = cvb::cvLabel(segmentated, labelImg, blobs);
  
	//cvSaveImage("mask.jpg" ,segmentated); ***DEBUG***

	cvb::cvFilterByArea(blobs, minArea, maxArea);

	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		Obj ob1;
		//center pos
		ob1.x=(unsigned int)it->second->centroid.x;
		ob1.y=(unsigned int)it->second->centroid.y;
		//real pos
		ob1.tl_x = (unsigned int)it->second->contour.startingPoint.x;
		ob1.tl_y = (unsigned int)it->second->contour.startingPoint.y;
		//Area
		ob1.Area=it->second->area;
		//put a definitive size
		ob1.width=it->second->maxx-it->second->minx;
		ob1.heigth=it->second->maxy-it->second->miny;
	
		//cout << "Sizes= W:" << ob1.w << " H:" << ob1.h << endl;
		
		Update(ob1,nframes);
	}

	//Render the image
	int iObjects = RenderImage(frame);

	//keep statictic track
	if(statisticTrack)
	{
		m_iFrameRateCounter++;
		if(m_iFrameRateCounter == m_iFrameRate)//uno por segundo
		{
			UpdateFilePath(iObjects,frame);
		}
	}
	
	if(debugBlob)
		cvb::cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_ANGLE|CV_BLOB_RENDER_TO_STD);

	cvb::cvUpdateTracks(blobs, tracks, 200., 5);

	if(debugTrack)
		cvb::cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX|CV_TRACK_RENDER_TO_STD);

	if(keepFileTrack)
		cvSaveImage(trackFileName.c_str() ,frame);

	if(showOutput)
		cvShowImage("INPUT", frame);

	cv::Mat img_result(frame);
	img_result.copyTo(img_output);

	cvReleaseImage(&labelImg);
	delete frame;
	delete segmentated;
	cvReleaseBlobs(blobs);
	cvReleaseStructuringElement(&morphKernel);

	firstTime = false;
}

void ObjectTracking::saveConfig()
{
	CvFileStorage* fs = cvOpenFileStorage("./config/ObjectTracking.xml", 0, CV_STORAGE_WRITE);

	cvWriteInt(fs, "minArea", minArea);
	cvWriteInt(fs, "maxArea", maxArea);
	cvWriteInt(fs, "debugTrack", debugTrack);
	cvWriteInt(fs, "debugBlob", debugBlob);
	cvWriteInt(fs, "showBlobMask", showBlobMask);
	cvWriteInt(fs, "showOutput", showOutput);
	cvWriteInt(fs, "keepFileTrack", keepFileTrack);
	cvWriteString(fs,"trackFfileName", trackFileName.c_str());

	cvWriteInt(fs, "savePathFileNFrames", m_iSavePathFileNFrames);
	cvWriteString(fs,"pathsIndexFileName", pathsIndexFileName.c_str());
	cvWriteString(fs,"pathFileName",pathFileName.c_str());
	cvWriteString(fs,"imageFrameFileName",imageFrameFileName.c_str());
	cvWriteString(fs,"pathFramesFileName",pathFramesFileName.c_str());
	cvWriteInt(fs, "statisticTrack", statisticTrack);

	cvReleaseFileStorage(&fs);
}

void ObjectTracking::loadConfig()
{
	CvFileStorage* fs = cvOpenFileStorage("./config/ObjectTracking.xml", 0, CV_STORAGE_READ);

	minArea = cvReadIntByName(fs, 0, "minArea", 500);
	maxArea = cvReadIntByName(fs, 0, "maxArea", 20000);

	debugTrack = cvReadIntByName(fs, 0, "debugTrack", false);
	debugBlob = cvReadIntByName(fs, 0, "debugBlob", false);
	showBlobMask = cvReadIntByName(fs, 0, "showBlobMask", false);
	showOutput = cvReadIntByName(fs, 0, "showOutput", true);
	keepFileTrack = cvReadIntByName(fs, 0, "keepFileTrack", true);
	trackFileName = cvReadStringByName(fs, 0, "trackFfileName", "labeltest.jpg");
	statisticTrack = cvReadIntByName(fs, 0, "statisticTrack", true);

	m_iSavePathFileNFrames = cvReadIntByName(fs, 0, "savePathFileNFrames", 0);
	pathsIndexFileName = cvReadStringByName(fs, 0, "pathsIndexFileName", 0);
	pathFileName = cvReadStringByName(fs, 0, "pathFileName", 0);
	pathFramesFileName = cvReadStringByName(fs, 0, "pathFramesFileName", 0);
	imageFrameFileName = cvReadStringByName(fs, 0, "imageFrameFileName","");

	cvReleaseFileStorage(&fs);
}

int ObjectTracking::RenderImage(IplImage *fr)
{
	CvScalar col;
	col=cvScalar(255,255,255,0);
	CvScalar col2;
	col2=cvScalar(255,0,0,0);
	CvScalar color;

	//Remove unseen object from the array
	removeOld();

	//Cantidad de objetos en el frame
	int iCantObjects = 0;

	//Loop through array and renderize the objects on the image
	for(int i=0; i<SIZE_LST; ++i)
	{
		if((list[i].id!=0)&&(list[i].apparenceNumber>5))
		{
			if(list[i].lastSeen==currentFrame) 
			{
				RenderObject(fr,i,col);
				CvPoint point;
				point.x = list[i].x;
				point.y = list[i].y;

				//add the position to the path file
				trobjs[list[i].uniqueID].push_back(getStagePoint(list[i].x,list[i].y));

				CvRect rect = cvRect(list[i].tl_x - (list[i].width / 2) < 0? list[i].tl_x - (list[i].width / 2): list[i].tl_x, list[i].tl_y,
							list[i].tl_x + list[i].width >= fr->width ? (fr->width - 1)  - list[i].tl_x : list[i].width -1,
							list[i].tl_y + list[i].heigth >= fr->height ? (fr->height - 1) - list[i].tl_y :  list[i].heigth -1);

				//Increment of each object on the frame
				iCantObjects++;
			}
			else
			{
				if((currentFrame-list[i].lastSeen)<10)
				{
					RenderObject(fr,i,col2);
				}
			}
		}
	}

	//Return the total objects of each frame
	return iCantObjects;
}

void ObjectTracking::Update(Obj ob, int nfr)
{
	currentFrame=nfr;
	int nrXY, nrS, nrCOL, nrDX;
	int id=0;
	nrXY=findObjectByXY(ob.x,ob.y,id);
	nrS=checkObjectBySize(ob.width,ob.heigth,ob.Area,id);
	if(nrS==0) 
		nrXY=0;
	if(nrXY==1)
		updateObject(ob,id);
	if(nrXY==0)
		addObject(ob);
}

//Define a radio 0f 20 pct of tolerance between the distance of one object to the other
#define radio 20

int ObjectTracking::findObjectByXY(int x, int y, int &id)
{
	int number=0;
	int prevDist=101;
	int disty=0;
	int distx=0;
	int radius=radio;
	for(int i=0; i<SIZE_LST; ++i)
	{
		if(list[i].id!=0)
		{
			distx=list[i].x-x;
			if(distx<0) 
				distx*=-1;
			
			disty=list[i].y-y;
			if(disty<0) 
				disty*=-1;

			radius=radio+floor((double)((10/7)*(currentFrame-list[i].lastSeen)+(60/7)));

			if((distx+disty)<radius)
			{
				if((distx+disty)<prevDist)
				{
					prevDist=distx+disty;
					number=1;
					id=i+1;
				}
			}
		}
	}
	return number;
}

int ObjectTracking::checkObjectBySize(unsigned int w, unsigned int h, unsigned int S, int &id)
{
	int disth=0;
	int distw=0;
	int distS=0;

	//this variable is the max variation of the area of
	//an object, by default was 256 but for an HD vide frame 
	//the variation should be greather or at least calculate it
	//based on the size of the object
	//In this case I´m using a variation of the 20 pct
	//this rate was not added to the config file
	int iMaxVar = ((list[id-1].width * list[id-1].heigth) * prcVarSize) / 100;//256;

	disth=list[id-1].heigth-h;
	
	if(disth<0) 
		disth*=-1;
	
	distw=list[id-1].width-w;
	
	if(distw<0) 
		distw*=-1;
		
	distS=list[id-1].Area-S;
			
	if(distS<0) 
		distS*=-1;

	if( ((distw+disth)<prcVarDist) && (distS<iMaxVar) )
	{
		return 1;
	}
	return 0;
}

void ObjectTracking::updateObject(Obj ob, int id)
{

	if(list[id-1].efi>1)
	{
		//Get the centered and the top left pos
		list[id-1].tl_x=ob.tl_x;
		list[id-1].tl_y=ob.tl_y;
		list[id-1].x=(unsigned int)(list[id-1].x+ob.x)/2;
		list[id-1].y=(unsigned int)(list[id-1].y+ob.y)/2;
		list[id-1].Area+=ob.Area;
		list[id-1].width=(unsigned int)(0.5*max(ob.width,list[id-1].width)+0.5*min(ob.width,list[id-1].width));
		list[id-1].heigth=(unsigned int)(0.5*max(ob.heigth,list[id-1].heigth)+0.5*min(ob.heigth,list[id-1].heigth));
	}
	else
	{
		list[id-1].tl_x=ob.tl_x;
		list[id-1].tl_y=ob.tl_y;
		list[id-1].x=ob.x;
		list[id-1].y=ob.y;
		list[id-1].Area=ob.Area;
		list[id-1].width=ob.width;
		list[id-1].heigth=ob.heigth;
		++list[id-1].apparenceNumber;
		list[id-1].lastSeen=currentFrame;
		list[id-1].efi=2;
	}

}

void ObjectTracking::addObject(Obj ob)
{
	//Increment the counter
	uniqueIdCounter++;

	int i=0;
	while(list[i].id!=0)
	{
		++i;
		if(i>SIZE_LST) return;
	}
	list[i].id=i+1;
	
	list[i].tl_x=ob.tl_x;
	list[i].tl_y=ob.tl_y;

	list[i].x=ob.x;
	list[i].y=ob.y;
	list[i].Area=ob.Area;

	list[i].width=ob.width;
	list[i].heigth=ob.heigth;
	list[i].apparenceNumber=1;
	list[i].lastSeen=currentFrame;
	list[i].efi=1;
	list[i].uniqueID = uniqueIdCounter;
}

void ObjectTracking::removeOld()
{
	for(int i=0; i<SIZE_LST; ++i)
	{
		if(list[i].id!=0)
		{
			if(list[i].lastSeen!=currentFrame)
			{
				if(list[i].apparenceNumber<5)
				{
					if((list[i].lastSeen+5)<currentFrame)
					{
						list[i].id=0;
					}
				}
				else{
					if((list[i].lastSeen+50)<currentFrame)
					{
						list[i].id=0;
					}
				}
				if(list[i].x<0)
				{
					list[i].x=0; list[i].prevx=0;
				}
				if(list[i].x>maxX-2)
				{
					list[i].x=maxX-1; list[i].prevx=maxX-1;
				}
				if(list[i].y<0)
				{
					list[i].y=0; list[i].prevy=0;
				}
				if(list[i].y>maxY-2)
				{
					list[i].y=maxY-1; list[i].prevy=maxY-1;
				}
			}
		}
		list[i].efi=1;
	}
}

void ObjectTracking::RenderObject(IplImage *fr, int i,CvScalar color)
{
	char id[20];

	sprintf(id,"%d",list[i].uniqueID);
	cvCircle(fr,cvPoint(list[i].x,list[i].y),ceil(pow((double)(list[i].Area),0.5)),color,1);
	cvPutText(fr,id,cvPoint(list[i].x,list[i].y), &font, color);
	id[0]='\0'; id[1]='\0'; 
}

StagePoint ObjectTracking::getStagePoint( int x, int y)
{
    StagePoint p;

    p.x = x;
    p.y = y;
	p.frame = nframes;

    return p;
}

void ObjectTracking::WriteObjToFile()
{
	/*
	{
	   "objects":[
		  {
			 "id":1,
			 "frames":[
				{
				"frame":1,
				"x":345,
				"y":123
				},
				{
				"frame":2,
				"x":347,
				"y":124
				},
				{
				"frame":3,
				"x":357,
				"y":134
				}
			 ]
		  },
		  {
			 "id":2,
			 "frames":[
				{
				"frame":1,
				"x":355,
				"y":133
				},
				{
				"frame":2,
				"x":357,
				"y":134
				},
				{
				"frame":3,
				"x":367,
				"y":144
				}
			]
		  },
		  {
			 "id":3,
			 "frames":[
				{
				"frame":1,
				"x":355,
				"y":113
				},
				{
				"frame":2,
				"x":357,
				"y":114
				},
				{
				"frame":3,
				"x":357,
				"y":114
				}
			]
		  }
	   ]
	}
	*/

	std::ofstream file;
	file.open(pathFramesFileName);

	/*
	   {
	   "objects":[
	*/
	std::ostringstream stream;
	stream << "{ \"size\":" << trobjs.size() << ",";
	stream << "\"totalframes\":" << nframes << ",";
	stream << "\"objects\":[" << endl;
	file << stream.str();
	stream.str("");

	int iFrCount = 0;


	for (TrackingObj::const_iterator it=trobjs.begin();it!=trobjs.end();++it,++iFrCount)
	{
		int id = (*it).first;
		std::list<StagePoint> points=(*it).second;

		/*
		  {
			 "id":1,
			 "frames":[
		*/
		stream << "{ \"id\":" << id << ",";
		stream << "\"frames\":[";
		file << stream.str();
		stream.str("");

		std::list<StagePoint>::const_iterator iterator;

		int iCount = 0;

		for (iterator = points.begin(); iterator != points.end(); ++iterator,++iCount) 
		{
			/*
				{
				"x":355,
				"y":113
				},
			*/
			StagePoint point = *iterator;
			//sline << point.frame << "," << point.x << "," << point.y << "|";
			stream << "{ \"frame\":" << point.frame << ",\"x\":" << point.x << ",\"y\":" << point.y;

			if(iCount+1 == points.size())
				stream << "}" << endl;
			else
				stream << "}," << endl;
			
			file << stream.str();
			stream.str("");
		}
		file << "]" << endl;

		if(iFrCount+1 == trobjs.size())
			file << "}" << endl;
		else
			file << "}," << endl;
		
		file << endl;
	}

	file << "]" << endl;
	file << "}" << endl;

	file.close();
}
