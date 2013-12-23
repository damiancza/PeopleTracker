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
// Summary: Header of the CodeBlock object 
// ******* Change History *******
// 07/10/2013 - DCZ - Inital Version
// ******* Change History *******
#pragma once

#include <iostream>
#include "../InterfaceBGS.h"

using namespace std;
using namespace cv;

class __declspec(dllexport) CodeBoock : public InterfaceBGS
{
public:
  bool firstTime;
private:
	IplConvKernel* morphKernel1;
	IplConvKernel* morphKernel2;
	CvBGCodeBookModel* model;
    IplImage* rawImage;
    IplImage *ImaskCodeBook;

	//Parameters for the threshold of the image (config)
	int m_iMinVal;
	int m_iMaxVal;
	//Frames p sec to create the background (config)
	int m_iFramesSecBG;

	//frame counter
	long nframes;

	//Image used to store the picture for the background (in case of using an image for bg)
	IplImage* bkgImg;
public:
  CodeBoock();
  ~CodeBoock();

  //BGS Main process
  virtual void process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel);

  //Used to set a video file to capture the objs
  //if an image is used, must have the same name of the video (ie: video.avi -> video.jpg)
  virtual void setFile(const char file[255]);

  //There´s no string replace on std lib
  std::string& sReplaceAll(std::string& sS, const std::string& sWhat, const std::string& sReplacement);

private:
  void saveConfig();
  void loadConfig();
};

