#include "ObjectCounter.h"
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

//Namespace used to define the line in case that weren´t defined
namespace FAV1
{
	IplImage* img_input1 = 0;
	IplImage* img_input2 = 0;
	int roi_x0 = 0;
	int roi_y0 = 0;
	int roi_x1 = 0;
	int roi_y1 = 0;
	int numOfRec = 0;
	int startDraw = 0;
	bool roi_defined = false;
	bool use_roi = true;
	void ObjectCounter_on_mouse(int evt, int x, int y, int flag, void* param)
	{
		if(!use_roi)
			return;

		if(evt == CV_EVENT_LBUTTONDOWN)
		{
			if(!startDraw)
			{
				roi_x0 = x;
				roi_y0 = y;
				startDraw = 1;
			}
			else
			{
				roi_x1 = x;
				roi_y1 = y;
				startDraw = 0;
				roi_defined = true;
			}
		}

		if(evt == CV_EVENT_MOUSEMOVE && startDraw)
		{
			//redraw selection
			img_input2 = cvCloneImage(img_input1);
			cvLine(img_input2, cvPoint(roi_x0,roi_y0), cvPoint(x,y), CV_RGB(255,0,255),3);
			cvShowImage("ObjectCounter", img_input2);
			cvReleaseImage(&img_input2);
		}
	}
}

ObjectCounter::ObjectCounter(): 
firstTime(true), 
	showOutput(true), 
	key(0), 
	countAB(0), 
	countBA(0), 
	showAB(0), 
	keepFileTrack(true)

{
	std::cout << "ObjectCounter()" << std::endl;
}

ObjectCounter::~ObjectCounter()
{
	std::cout << "~ObjectCounter()" << std::endl;
}

void ObjectCounter::setInput(const cv::Mat &i)
{
	img_input = i;
}

void ObjectCounter::setTracks(const cvb::CvTracks &t)
{
	tracks = t;
}

ObjectPosition ObjectCounter::getObjectPosition(const CvPoint2D64f centroid)
{
	ObjectPosition objectPosition = VP_NONE;
	
	if(LaneOrientation == LO_HORIZONTAL)
	{
		if(centroid.x < FAV1::roi_x0)
		{
			objectPosition = VP_A;
		}
		
		if(centroid.x > FAV1::roi_x0)
		{
			objectPosition = VP_B;
		}
	}

	if(LaneOrientation == LO_VERTICAL)
	{
		if(centroid.y > FAV1::roi_y0)
		{
			objectPosition = VP_A;
		}
	  
		if(centroid.y < FAV1::roi_y0)
		{
			objectPosition = VP_B;
		}
	}

	return objectPosition;
}

void ObjectCounter::processObjects()
{
	if(img_input.empty())
		return;

	img_w = img_input.size().width;
	img_h = img_input.size().height;

	loadConfig();

	//--------------------------------------------------------------------------

	if(FAV1::use_roi == true && FAV1::roi_defined == false && firstTime == true)
	{
		do
		{
			FAV1::img_input1 = new IplImage(img_input);
			cvShowImage("ObjectCounter", FAV1::img_input1);
			cvSetMouseCallback("ObjectCounter", FAV1::ObjectCounter_on_mouse, NULL);
			key = cvWaitKey(0);
			delete FAV1::img_input1;
			
			if(FAV1::roi_defined)
			{
				std::cout << "Counting line defined (" << FAV1::roi_x0 << "," << FAV1::roi_y0 << "," << FAV1::roi_x1 << "," << FAV1::roi_y1 << ")" << std::endl;
				break;
			}
			else
				std::cout << "Counting line undefined!" << std::endl;
		}while(1);
	}

	if(FAV1::use_roi == true && FAV1::roi_defined == true)
		cv::line(img_input, cv::Point(FAV1::roi_x0,FAV1::roi_y0), cv::Point(FAV1::roi_x1,FAV1::roi_y1), cv::Scalar(0,0,255),3);

	bool ROI_OK = false;

	if(FAV1::use_roi == true && FAV1::roi_defined == true)
		ROI_OK = true;

	if(ROI_OK)
	{
		LaneOrientation = LO_NONE;

		if(abs(FAV1::roi_x0 - FAV1::roi_x1) < abs(FAV1::roi_y0 - FAV1::roi_y1))
			LaneOrientation = LO_HORIZONTAL;

		if(abs(FAV1::roi_x0 - FAV1::roi_x1) > abs(FAV1::roi_y0 - FAV1::roi_y1))
			LaneOrientation = LO_VERTICAL;
	}

	if(showOutput)
	{
		IplImage* imgInput = new IplImage(img_input);
		cvShowImage("INPUT", imgInput);
		delete imgInput;
	}

	if(firstTime)
		saveConfig();

	firstTime = false;
}

void ObjectCounter::processTracks()
{
  if(img_input.empty())
    return;

  img_w = img_input.size().width;
  img_h = img_input.size().height;

  loadConfig();

  //--------------------------------------------------------------------------

  if(FAV1::use_roi == true && FAV1::roi_defined == false && firstTime == true)
  {
    do
    {
      FAV1::img_input1 = new IplImage(img_input);
      cvShowImage("ObjectCounter", FAV1::img_input1);
      cvSetMouseCallback("ObjectCounter", FAV1::ObjectCounter_on_mouse, NULL);
      key = cvWaitKey(0);
      delete FAV1::img_input1;

		if(FAV1::roi_defined)
		{
			std::cout << "Counting line defined (" << FAV1::roi_x0 << "," << FAV1::roi_y0 << "," << FAV1::roi_x1 << "," << FAV1::roi_y1 << ")" << std::endl;
			cvDestroyWindow("ObjectCounter");
			break;
		}

	}while(1);
  }

  if(FAV1::use_roi == true && FAV1::roi_defined == true)
    cv::line(img_input, cv::Point(FAV1::roi_x0,FAV1::roi_y0), cv::Point(FAV1::roi_x1,FAV1::roi_y1), cv::Scalar(0,0,255),3);
  
  bool ROI_OK = false;
  
  if(FAV1::use_roi == true && FAV1::roi_defined == true)
    ROI_OK = true;

  if(ROI_OK)
  {
    LaneOrientation = LO_NONE;

    if(abs(FAV1::roi_x0 - FAV1::roi_x1) < abs(FAV1::roi_y0 - FAV1::roi_y1))
      LaneOrientation = LO_HORIZONTAL;

    if(abs(FAV1::roi_x0 - FAV1::roi_x1) > abs(FAV1::roi_y0 - FAV1::roi_y1))
      LaneOrientation = LO_VERTICAL;
  }

  //--------------------------------------------------------------------------

  for(std::map<cvb::CvID,cvb::CvTrack*>::iterator it = tracks.begin() ; it != tracks.end(); it++)
  {
    cvb::CvID id = (*it).first;
    cvb::CvTrack* track = (*it).second;

    CvPoint2D64f centroid = track->centroid;
    //----------------------------------------------------------------------------

    if(track->inactive == 0)
    {
      if(positions.count(id) > 0)
      {
        std::map<cvb::CvID, ObjectPosition>::iterator it2 = positions.find(id);
        ObjectPosition old_position = it2->second;

        ObjectPosition current_position = getObjectPosition(centroid);

        if(current_position != old_position)
        {
          if(old_position == VP_A && current_position == VP_B)
		  {
            countAB++;
			//Save the file
			char pathImg[255];
			//Format the file name on a file to keep track of who or what crossed the line
			sprintf((char*)&pathImg,imageInFileName.c_str(),countAB);
			IplImage* imgIn = new IplImage(img_input);
			cvSaveImage(pathImg , imgIn); 
		  }

          if(old_position == VP_B && current_position == VP_A)
		  {
            countBA++;
			//Save the file
			char pathImg[255];
			//Format the file name on a file to keep track of who or what crossed the line
			sprintf((char*)&pathImg,imageOutFileName.c_str(),countBA);
			IplImage* imgIn = new IplImage(img_input);
			cvSaveImage(pathImg , imgIn); 
		  }

          positions.erase(positions.find(id));
        }
      }
      else
      {
        ObjectPosition peoplePosition = getObjectPosition(centroid);

        if(peoplePosition != VP_NONE)
          positions.insert(std::pair<cvb::CvID, ObjectPosition>(id,peoplePosition));
      }
    }
    else
    {
      if(positions.count(id) > 0)
        positions.erase(positions.find(id));
    }

    //----------------------------------------------------------------------------

    if(points.count(id) > 0)
    {
      std::map<cvb::CvID, std::vector<CvPoint2D64f>>::iterator it2 = points.find(id);
      std::vector<CvPoint2D64f> centroids = it2->second;
      
      std::vector<CvPoint2D64f> centroids2;
      if(track->inactive == 0 && centroids.size() < 30)
      {
        centroids2.push_back(centroid);
      
        for(std::vector<CvPoint2D64f>::iterator it3 = centroids.begin() ; it3 != centroids.end(); it3++)
        {
          centroids2.push_back(*it3);
        }

        for(std::vector<CvPoint2D64f>::iterator it3 = centroids2.begin() ; it3 != centroids2.end(); it3++)
        {
          cv::circle(img_input, cv::Point((*it3).x,(*it3).y), 3, cv::Scalar(255,255,255), -1);
        }
      
        points.erase(it2);
        points.insert(std::pair<cvb::CvID, std::vector<CvPoint2D64f>>(id,centroids2));
      }
      else
      {
        points.erase(it2);
      }
    }
    else
    {
      if(track->inactive == 0)
      {
        std::vector<CvPoint2D64f> centroids;
        centroids.push_back(centroid);
        points.insert(std::pair<cvb::CvID, std::vector<CvPoint2D64f>>(id,centroids));
      }
    }

    //cv::waitKey(0);
  }
  
  //--------------------------------------------------------------------------
	std::ostringstream countAb;
	std::string scountAb;

	std::ostringstream countBa;
	std::string scountBa;

	countAb << "A->B:" << countAB << ", B->A" << countBA;
	scountAb = countAb.str();
	countBa << "B->A:" << countBA;
	scountBa = countBa.str();

  if(showOutput)
  {
    IplImage* imgInput = new IplImage(img_input);

	if(keepFileTrack)
	{
		std::ofstream file;
		file.open(trackJsonFileName.c_str());


		std::ostringstream count;

		//{	
		//  "in": 2,
		//  "out": 1
		//}

		count << "{" << endl;
		count << "\"out\": " << countBA << "," << endl;
		count << "\"in\": " << countAB << endl;
		count << "}" << endl;

		file << count.str();

		file.close();

		cvSaveImage(trackFileName.c_str() ,imgInput);
	}
    cvShowImage("INPUT", imgInput);
	delete imgInput;
  }
  //
  if(firstTime)
    saveConfig();

  firstTime = false;
}

void ObjectCounter::saveConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/ObjectCounter.xml", 0, CV_STORAGE_WRITE);

  cvWriteInt(fs, "showOutput", showOutput);
  cvWriteInt(fs, "showAB", showAB);
  
  cvWriteInt(fs, "fav1_use_roi", FAV1::use_roi);
  cvWriteInt(fs, "fav1_roi_defined", FAV1::roi_defined);
  cvWriteInt(fs, "fav1_roi_x0", FAV1::roi_x0);
  cvWriteInt(fs, "fav1_roi_y0", FAV1::roi_y0);
  cvWriteInt(fs, "fav1_roi_x1", FAV1::roi_x1);
  cvWriteInt(fs, "fav1_roi_y1", FAV1::roi_y1);
  cvWriteInt(fs, "keepFileTrack", keepFileTrack);
  cvWriteString(fs,"trackFileName", trackFileName.c_str());
  cvWriteString(fs,"trackJsonFileName", trackJsonFileName.c_str());
  cvWriteString(fs,"imageInFileName", imageInFileName.c_str());
  cvWriteString(fs,"imageOutFileName", imageOutFileName.c_str());

 
  cvReleaseFileStorage(&fs);
}

void ObjectCounter::loadConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/ObjectCounter.xml", 0, CV_STORAGE_READ);

  showOutput = cvReadIntByName(fs, 0, "showOutput", true);
  showAB = cvReadIntByName(fs, 0, "showAB", 1);
  keepFileTrack = cvReadIntByName(fs, 0, "keepFileTrack", true);
  
  FAV1::use_roi = cvReadIntByName(fs, 0, "fav1_use_roi", true);
  FAV1::roi_defined = cvReadIntByName(fs, 0, "fav1_roi_defined", false);
  FAV1::roi_x0 = cvReadIntByName(fs, 0, "fav1_roi_x0", 0);
  FAV1::roi_y0 = cvReadIntByName(fs, 0, "fav1_roi_y0", 0);
  FAV1::roi_x1 = cvReadIntByName(fs, 0, "fav1_roi_x1", 0);
  FAV1::roi_y1 = cvReadIntByName(fs, 0, "fav1_roi_y1", 0);
  trackFileName = cvReadStringByName(fs, 0, "trackFileName", "labeltestc.jpg");
  trackJsonFileName = cvReadStringByName(fs, 0, "trackJsonFileName", "data.json");
  imageInFileName = cvReadStringByName(fs, 0, "imageInFileName", "In%i.jpg");
  imageOutFileName = cvReadStringByName(fs, 0, "imageOutFileName", "Out%i.jpg");
  cvReleaseFileStorage(&fs);
}
