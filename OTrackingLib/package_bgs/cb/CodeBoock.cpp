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
// Summary: Source of the CodeBoock object 
// ******* Change History *******
// 07/10/2013 - DCZ - Inital Version
// ******* Change History *******

#include "CodeBoock.h"

CodeBoock::CodeBoock() : 
firstTime(true),
nframes(0),
m_iMinVal(3),
m_iMaxVal(10),
m_iFramesSecBG(100)
{
	loadConfig();

	morphKernel1 = cvCreateStructuringElementEx(3, 3, 1, 1, CV_SHAPE_RECT, NULL);
	morphKernel2 = cvCreateStructuringElementEx(5, 5, 3, 3, CV_SHAPE_RECT, NULL);
	
    model = cvCreateBGCodeBookModel();

    //Cambiar thresholds to default values
    model->modMin[0] = m_iMinVal;
    model->modMin[1] = model->modMin[2] = m_iMinVal;

    model->modMax[0] = m_iMaxVal;
    model->modMax[1] = model->modMax[2] = m_iMaxVal;
    model->cbBounds[0] = model->cbBounds[1] = model->cbBounds[2] = m_iMaxVal;

    rawImage = 0;
    ImaskCodeBook = 0;

	bkgImg = NULL;

	std::cout << "CodeBoock()" << std::endl;
}

CodeBoock::~CodeBoock()
{
    cvReleaseStructuringElement(&morphKernel1); 
	cvReleaseStructuringElement(&morphKernel2);
	cvReleaseImage(&ImaskCodeBook);
	cvReleaseImage(&bkgImg);

	std::cout << "~CodeBoock()" << std::endl;
}

std::string& CodeBoock::sReplaceAll(std::string& sS, 
                         const std::string& sWhat, 
                         const std::string& sReplacement)
{
    size_t pos = 0, fpos;
    while ((fpos = sS.find(sWhat, pos)) != std::string::npos)
    {
        sS.replace(fpos, sWhat.size(), sReplacement);
        pos = fpos + sReplacement.size();
    }
    return sS;
}

void CodeBoock::process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel)
{
	if(img_input.empty())
		return;

	++nframes;

	//********bg image support**********

	if(nframes < m_iFramesSecBG)
	{
		//if exists an image for the bg we use that one
		//instead of create the bg based on frames
		if(bkgImg)
			rawImage = bkgImg;
		else
			rawImage = new IplImage(img_input);
	}
	else
	{
		rawImage = new IplImage(img_input);
	}
	//********Fin Soporte imagen de fondo**********

	// CodeBoock init
	if( nframes == 1)
	{
		ImaskCodeBook = cvCreateImage( cvGetSize(rawImage), IPL_DEPTH_8U, 1 );
		cvSet(ImaskCodeBook,cvScalar(255));
	}

	if((nframes<m_iFramesSecBG))
	{
		cvBGCodeBookUpdate( model, rawImage );
		return;
	}
	else
		cvBGCodeBookDiff( model, rawImage, ImaskCodeBook );
		
	if(nframes==100)
		cvBGCodeBookClearStale( model, model->t/2 );

	cvMorphologyEx(ImaskCodeBook, ImaskCodeBook, NULL, morphKernel1, CV_MOP_ERODE, 1);
	cvMorphologyEx(ImaskCodeBook, ImaskCodeBook, NULL, morphKernel2, CV_MOP_DILATE, 1);

    cv::Mat img_foreground(ImaskCodeBook);
    cv::Mat img_bkg(ImaskCodeBook);

	//Copy the MAT file to the IplImages
    img_foreground.copyTo(img_output);
    img_bkg.copyTo(img_bgmodel);

    delete rawImage;
}

void CodeBoock::saveConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/CodeBoock.xml", 0, CV_STORAGE_WRITE);

  cvWriteInt(fs, "MinVal", m_iMinVal );
  cvWriteInt(fs, "MaxVal", m_iMaxVal );
  cvWriteInt(fs, "bkgFrames", m_iFramesSecBG );

  cvReleaseFileStorage(&fs);
}

void CodeBoock::loadConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/CodeBoock.xml", 0, CV_STORAGE_READ);

  m_iMinVal = cvReadIntByName(fs, 0, "MinVal", 25);
  m_iMaxVal = cvReadIntByName(fs, 0, "MaxVal", 50);
  m_iFramesSecBG = cvReadIntByName(fs, 0, "bkgFrames", 100);

  cvReleaseFileStorage(&fs);
}

void CodeBoock::setFile(const char file[255])
{
	//En caso que exista imagen de fondo, tomamos el patron por la imagen y despues la captura
	std::string backg(file);

	sReplaceAll(backg, ".avi",".png");
	sReplaceAll(backg, ".mp4",".png");
	
	bkgImg = cvLoadImage(backg.c_str());
}
