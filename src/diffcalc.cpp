///////////////////////////////////////////
// diffcalc v0.1
// calculates the difference between adjacent images to determine movement over time
// usage:
// diffcal -e expID
//
// output diff.csv
//////////////////////////////////////////


//includes
 
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <sstream>
#include <stdlib.h>
#include <inttypes.h>
#include <glob.h>
#include <vector>
#include <fcntl.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <boost/algorithm/string.hpp> // include Boost, a C++ library
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>


#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <cstdio>
#include <errno.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <vector>
#include <stdio.h>



//openCV
#include <opencv2/opencv.hpp>
#include <opencv/cv.hpp>
#include <opencv2/videoio.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"


#include <iostream>
#include <stdio.h>
#include <stdlib.h>


#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace cgicc;
using namespace cv;


//GLOBALS

long expID;
bool writeImages = false;


long getNumFrames(long exp, string ichan){

	glob_t glob_result;
	
	
	long numframes =0;

	stringstream globpattern;
	globpattern  << "/wormbot/" << exp << "/" << ichan << string("*.png");
	glob(globpattern.str().c_str(),GLOB_TILDE,NULL,&glob_result);

	for (unsigned int i=0; i < glob_result.gl_pathc; i++){
		
		numframes++;
	}//end for each glob
	return numframes;

}//end getNumFrames


string padded(long c) {

	stringstream number;
	number << setfill('0') << setw(6) << c;

	return number.str();
}//end padded


//compare adjacent frame and extract optical difference, returns CSV line
string compareFrames(long exp,string chan, float threshold, long index){
	
	double diffsum=0;
	
	//generate filename
		stringstream firstname, secondname;
		firstname << "/wormbot/" << exp << "/" << chan << padded(index) << ".png" ;
		secondname << "/wormbot/" << exp << "/" << chan << padded(index+1) << ".png" ;
		

		//read in image files
		Mat start=imread(firstname.str());
		Mat end=imread(secondname.str());

	cv::Mat diffImage;
        cv::absdiff(start, end, diffImage);
	Mat element = getStructuringElement(MORPH_RECT,Size( 3, 3 ));
	erode( diffImage, diffImage,element);
	dilate( diffImage, diffImage,element);

        cv::Mat foregroundMask = cv::Mat::zeros(diffImage.rows, diffImage.cols, CV_8UC1);

       
        float dist;
/*
	    for(int j=0; j<diffImage.rows; ++j)
		for(int i=0; i<diffImage.cols; ++i)
		{
		   // cv::Vec3b pix = diffImage.at<cv::Vec3b>(j,i);

		  //  dist = (pix[0]*pix[0] + pix[1]*pix[1] + pix[2]*pix[2]);
		   // dist = sqrt(dist);

			dist = (float)diffImage.at<uchar>(j,i);

		    if(dist>threshold)
		    {
		        foregroundMask.at<unsigned char>(j,i) = 255;
			diffsum++;
		    }
		}
*/
	cv::threshold(diffImage, foregroundMask, threshold, 255, THRESH_BINARY);
	diffsum = sum(foregroundMask)[0];

	stringstream ss;
	ss << index << "," << diffsum;

	stringstream sss;
	sss << "/wormbot/" << exp << "/mask_" << chan << padded(index) << ".png" ;
	
      if (writeImages) imwrite(sss.str().c_str(),foregroundMask);
	return ss.str();


}//end compareFrames



int main(int argc, char* argv[])
{

    //check for the input parameter correctness
    if(argc < 2) {
        cerr <<"Incorrect input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }
    
   
	long expID;
	stringstream ss;
	string imageChan;
	float thresh;
	
	


    for (int i=1; i < argc; i+=2){
	char sw = argv[i][1];
	//cout << "arg i=" << sw << endl;
	
	switch (sw) {
		

		case 'e':
		case 'E':
			expID = atoi(argv[i+1]);
		break;


		case 'c':
		case 'C':
			imageChan = argv[i+1]; 
		break;

		case 't':
		case 'T':
			thresh = atof(argv[i+1]);
		break;

		case 'w':
		case 'W':
			char ans = argv[i+1][0];
			if (ans == 'y' || ans == 'Y'){
			 writeImages=true;
			  cout << "saving images" << endl;
			}
		break;

			

	}//end argument swtch


    }// end for each argument

	ss << "/wormbot/" << expID << "/diffcal.csv" ;
	ofstream ofile(ss.str().c_str());

	int numframes=0; 

	numframes = getNumFrames(expID,imageChan);

	cout << "N=" << numframes << endl;


	for (int i=0; i < numframes-1; i+=2){
		cout << ((float)i / (float)numframes) * 100.0f << "percent complete" <<  endl;
		ofile << compareFrames(expID, imageChan, thresh, i ) << endl;


	}//end for each frame pair

	ofile.close();


}//end main

