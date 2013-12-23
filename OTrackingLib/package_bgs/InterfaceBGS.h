#pragma once
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
// Summary: Header of the CodeBlock object.
// This abstract class will be overrided for each algorithm used to get the
// background sustraction.
// ******* Change History *******
// 07/10/2013 - DCZ - Inital Version
// ******* Change History *******

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/legacy/legacy.hpp"

class InterfaceBGS
{
public:
  virtual void process(const cv::Mat &img_input, cv::Mat &img_foreground, cv::Mat &img_background) = 0;
  virtual void setFile(const char file[255]){};
  virtual ~InterfaceBGS(){}

private:
  virtual void saveConfig() = 0;
  virtual void loadConfig() = 0;
};
