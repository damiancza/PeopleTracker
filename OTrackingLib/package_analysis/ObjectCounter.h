#pragma once

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"

#include "../package_tracking/cvblob/cvblob.h"
#include "../package_tracking/ObjectTracking.h"

using namespace std;

enum LineOrientation
{
  LO_NONE       = 0,
  LO_HORIZONTAL = 1,
  LO_VERTICAL   = 2
};

//Object position towards the line
enum ObjectPosition
{
  VP_NONE = 0,
  VP_A  = 1,
  VP_B  = 2
};

class __declspec(dllexport) ObjectCounter
{
private:
	bool firstTime;
	bool showOutput;

	int key;				//Wait key used to define the line
	cv::Mat img_input;		// 
	cvb::CvTracks tracks;	//For the object counter we use the cvBlob tracks instead 
							//the objects because the tracks has more tolerance of moving ojbects

	std::map<cvb::CvID, std::vector<CvPoint2D64f>> points;
	
	LineOrientation LaneOrientation;
	std::map<cvb::CvID, ObjectPosition> positions;
	long countAB;
	long countBA;
	int img_w;
	int img_h;
	int showAB;
	bool keepFileTrack;
	std::string  trackFileName;
	std::string  trackJsonFileName;
	std::string  imageInFileName;
	std::string  imageOutFileName;

public:
  ObjectCounter();
  ~ObjectCounter();

  void setInput(const cv::Mat &i);
  void setTracks(const cvb::CvTracks &t);
  void setObjects(const ObjTracks &to);
  void processObjects();
  void processTracks();

  long getAbCount() {return countAB;};
  long getBaCount() {return countBA;};
  void reStartCounter() {countAB = 0; countBA = 0;};

private:
  ObjectPosition getObjectPosition(const CvPoint2D64f centroid);

  void saveConfig();
  void loadConfig();
};
