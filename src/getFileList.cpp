//============================================================================
// Name        : getFileList.cpp

// Author      : jason pitt
// Version     : 1.0
// Copyright   : march 2022
// Description : This application builds a list of all the available data files to download
// 
//============================================================================

//stdlib
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <glob.h>

//cgicc
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

//boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

//openCV
#include <opencv2/opencv.hpp>
#include <opencv/cv.hpp>
#include <opencv2/videoio.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"



//namespace
using namespace std;
using namespace cgicc;
using namespace cv;

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

//Globals
long expID=0;
string datapath; //hold the path to all the robot data
//read in path from /usr/lib/cgi-bin/data_path
ifstream pathfile("/wormbot/cgi-bin/data_path");


ofstream debugger;

stringstream linkOutput;



// C L A S S E S


// F U N C T I O N S

string pad(int number){
	stringstream num;
	num << setfill('0') << setw(6) << number;
	return num.str();

}//end pad


string scanExperimentFiles(string fulldirectory, string searchpattern){
	glob_t glob_result;
	glob_t aligned_result;
	stringstream filelist;
	stringstream globpattern;
	globpattern  << fulldirectory.c_str() << searchpattern;
     
	cout <<"globpat:" << globpattern.str() << endl;

	glob(globpattern.str().c_str(),GLOB_TILDE,NULL,&glob_result);
	
	for (unsigned int i=0; i < glob_result.gl_pathc; i++){
		string trimmedFilename=glob_result.gl_pathv[i];
		size_t found = trimmedFilename.find_last_of('/'); 
		filelist << "<a href=\" " << string(glob_result.gl_pathv[i]) << "\" > " << trimmedFilename.substr(found+1) << "</a><BR>" << endl ;
		
	}//end for each glob
	

	return filelist.str();
	


}//end scanExperimentFiles


//////////// M A I N
int main(int argc,char **argv){

	cout << setprecision(5);

	getline(pathfile,datapath);
	pathfile.close();

	
	Cgicc cgi;

	cout << HTTPHTMLHeader() << endl;
        cout << html() << endl << endl;
	

	try {


	    expID = atol(string(cgi("expID")).c_str());
	cout << "expID=" << expID << endl; 



	} catch (exception& e){
		
		//return (0);

	}//end exception caught

	//testu
	//expID=590;

	stringstream exppath;

	exppath << datapath << expID << "/" ;


	//open output file
	ofstream ofile(exppath.str() + string("linkfile.html"));



	//rects
	ofile << scanExperimentFiles(exppath.str(), string("arect*"));

	//lifespan files
	ofile << scanExperimentFiles(exppath.str(), string("lifespanoutput*"));

	// movies
	ofile << scanExperimentFiles(exppath.str(), string("*.avi"));

	// description
	ofile << scanExperimentFiles(exppath.str(), string("description*"));

	// wormlist
	ofile << scanExperimentFiles(exppath.str(), string("wormlist*"));

	
	ofile.close();



	//output something so data calls success
		cout << HTTPHTMLHeader() << endl;
		cout << html() << head(title("WormBot Response")) << endl;
		cout << "</body></html>" << endl;


	return 0;
}
