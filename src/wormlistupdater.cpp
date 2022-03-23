//============================================================================
// Name        : wormlistupdater.cpp

// Author      : jason pitt
// Version     : 1.0
// Copyright   : june 2018
// Description : This application take a wormlist built/modified in the retrograde
// javascript app and finds the unix timestamps and saves the modfied wormlist.csv
//============================================================================

//stdlib
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

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


const Scalar SCALAR_BLACK = Scalar(0.0,0.0,0.0);
const Scalar SCALAR_WHITE = Scalar(255.0,255.0,255.0);
const Scalar SCALAR_BLUE = Scalar(255.0,0.0,0.0);
const Scalar SCALAR_GREEN = Scalar(0.0,255.0,0.0);
const Scalar SCALAR_RED = Scalar(0.0,0.0,255.0);


//Globals
int expID;
string datapath; //hold the path to all the robot data
//read in path from /usr/lib/cgi-bin/data_path
ifstream pathfile("/wormbot/cgi-bin/data_path");


ofstream debugger;
int numworms=0;



// C L A S S E S

class Experiment { //adapted from wormbot.cpp Well class
public:
		string email;
		string title;
		string investigator;
		string description;
		struct timeval starttime;
		long currentframe;
		int active;
		int status;
		string directory;
		int startingN;	   //number of worms put onto plate
		int startingAge;   //in days;
		int busy;
		int finished;
		long thisexpID;
		int xval;
		int yval;
		int plate;
		int rank;
		bool transActive;
		bool gfpActive;
		bool timelapseActive;
		bool dailymonitorActive;
		string wellname;
		string strain;

		 Experiment(int expnum){







		}//end constructor


		void printDescriptionFile(void){
				string filename;
				filename = directory + string("description.txt");
				//cout<<filename << endl;
				ofstream ofile(filename.c_str());

						ofile << "****************************************************************\n";
						ofile << title << endl;
						ofile << email<< endl;
						ofile << investigator<< endl;
						ofile << description<< endl;
						ofile << starttime.tv_sec<< endl;
						ofile << currentframe<< endl;
						ofile << strain << endl;
						ofile << active<< endl;
						ofile << directory<< endl;
						ofile << startingN<< endl;	   //number of worms put onto plate
						ofile << startingAge<< endl;   //in days;
						ofile << "expID:" << expID<< endl;
						ofile << xval<< endl;
						ofile << yval<< endl;
						ofile << plate<< endl;
						ofile << wellname<< endl;
						ofile << "****************************************************************\n";
						ofile << "::br::" << endl;


						ofile.close();



			}//end print description file

};

class Worm {
public:
	int x;
	int y;
	int currf;
	int number;
	float daysold;
	float minutesold;
	long secondsold;

	Worm(int sx, int sy, int curr, int wormnumber,long secs){

		x=sx;
		y=sy;
		currf=curr;
		number=wormnumber;
		daysold=0;
		minutesold=0;
		secondsold=secs;
		minutesold = ((float)secondsold) / 60.0f;
		daysold = ((float)secondsold) /86400.0f;


	}//end constructor

	Worm(string fileline){
        stringstream ss(fileline);
        string token;
        //cout << "<br>debug:" << fileline<<endl;
        getline(ss,token,',');
        x=atoi(token.c_str());
        getline(ss,token,',');
        y=atoi(token.c_str());
        getline(ss,token,',');
        currf=atoi(token.c_str());
        getline(ss,token,',');
        number=atoi(token.c_str());
        getline(ss,token,',');
        daysold=atof(token.c_str());
        getline(ss,token,',');
        minutesold=atof(token.c_str());
        //cout << "<br>debug:" << drawDiv();

		}//end constructor

	string printData(void){

		stringstream ss;
		ss << setprecision(5);
		ss << x << "," << y << "," << currf << "," << number << "," << daysold << "," << minutesold << endl;
		return (ss.str());


	}//end print data

	string drawDiv(int now){
		stringstream oss;
	      if (now >= currf) oss << ".w" << number <<" { position:absolute; opacity:0.75; font-size:20px; font-family:Impact, Charcoal, sans-serif; color:white; text-shadow: 2px 2px 4px #000000; background:transparent; left:" << (x-25)  << "px; top:" << (y-25) << "px; z-index:2;}" << endl;


		return ( oss.str());

	}//end drawDiv

	string drawIcon(int now){
		stringstream oss;
		 if (now >= currf){
			 oss << "<img src=\"/dead.png\" class=\"w" << number << "\">";
			 oss << "<div class=\"w" << number << "\" >" << number << "</div>" << endl;
		 }//end if should be visible
        return ( oss.str());

	}//end drawIcon

	string drawFormField(int now){
	     stringstream oss;
	     if (now >= currf) oss << "<input type=\"checkbox\" name=\"w" << number <<"\"> Worm "<< number <<": days old:" << daysold << "<br>" << endl;
	     return (oss.str());
	}//end draw formField

};

class Day{
public:
	int numdead;
	int numalive;

	Day(void){
		numdead=0;
		numalive=0;
	}
};

class Lifespan{
public:
	vector <Day> days;
	float maxlifespan;
	int n;
	double mean;
	double median;
	vector <float> formedian; //death events


	Lifespan(vector<Worm>  myworms){
		//getmaxlifespan
		maxlifespan=0;
		n=0;
		double total=0;
		

		for (int i=0; i < myworms.size(); i++){
			if (myworms[i].daysold > maxlifespan) maxlifespan = myworms[i].daysold;
			formedian.push_back(myworms[i].daysold);
			n++;
		}//end for each worm

		for (int i=0; i < myworms.size(); i++){
					total += myworms[i].daysold;
		}//end for each worm
		if (n>0) mean = total/((double)n); else mean=0;

		if (n >0){
			sort (formedian.begin(), formedian.end());
			if (n%2==1){
				median = formedian[n/2];
			}else { //end if odd
				median = (formedian[n/2] + formedian[(n+1)/2])/2;

			}//else even
		} else median=0;






		for (int i=0; i <= (int)maxlifespan; i++){
		     Day today = Day();

		     for(int j=0; j < myworms.size(); j++){
		    	 if (i==(int)myworms[j].daysold) {
		    		 today.numdead++;
		    	 }//if died this day
		    	 if (i < (int)myworms[j].daysold){
		    		 today.numalive++;
		    	 }//end if not dead yet

		     }//end for each worm
		     days.push_back(today);

		}//end for each day

	}//end constructor

	string getOasisListHTML(void){
		stringstream oss;
		for(int i =0; i <= maxlifespan; i++){
			oss << i << "," << days[i].numdead << endl;

		}//end for each day
		return (oss.str());
	}//end get oasis list

	string getOasisListFloat(void){
		stringstream oss;
		for(int i =0; i < formedian.size(); i++){
			oss << formedian[i] << "," << 1 << endl;

		}//end for each day
		return (oss.str());
	}//end get oasis list



};


vector <Worm> wormlist;








// F U N C T I O N S

string pad(int number){
	stringstream num;
	num << setfill('0') << setw(6) << number;
	return num.str();

}//end pad

static void on_trackbar( int, void* )
{
   // resizeWindow("Simple Ass BS", 800, cannyHigh);
  // imshow( "Adjusted Out", adjustedOut );
}

//make an updated current_contour.png
int Update_Contours(string filename, int lowthresh, int highthresh, int imagechannel){

				try{
							
							vector<int> compression_params;
							compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION); //(CV_IMWRITE_PXM_BINARY);
							compression_params.push_back(0);			
							Mat canny_output;
							Mat inputImg;
							inputImg = imread(filename.c_str());
							debugger << "empty image " << inputImg.empty() << endl; 
							debugger << "inputimg filename " << filename << endl;
							int blurval=4;

							vector<Mat> channels(3);
							
							if (imagechannel >= 0) {
								split(inputImg, channels);
								debugger << "split,";
								Mat invt; 
								bitwise_not (channels[imagechannel], invt ); //invert the image
								debugger << "bnot,";
								inputImg = invt.clone();

								debugger << "clone,";
								//blurval=0;
								imwrite(string("/wormbot/invertedimage.png").c_str(),inputImg);
								debugger << "write" << endl;
							}//end if color image
							else cvtColor( inputImg, inputImg, COLOR_BGR2GRAY );


							if (lowthresh < 0) lowthresh =0;
							if (highthresh < 0) highthresh =0;
							if (lowthresh > 255) lowthresh =255;
							if (highthresh > 255) highthresh =255;

							debugger << "low: " << lowthresh << " high: " << highthresh << endl;

							vector<vector<Point> > contours;

							vector<Vec4i> hierarchy;
							
							
							
							
							if (imagechannel == -1) equalizeHist(inputImg,inputImg);
							debugger << "equalize " << endl;
							//threshold(inputImg,inputImg, binthresh, 255, THRESH_BINARY_INV);
							if (blurval >0) blur( inputImg, inputImg, Size(blurval,blurval) );
						        debugger << "blur " << endl;

							
								stringstream testfile;
								testfile << datapath << expID << "/canny1.png";

								debugger << "cannyoutput " << testfile.str() << endl;
								
								//Canny( inputImg, canny_output, lowthresh, highthresh, 3);
								

							Mat element = getStructuringElement(MORPH_RECT,Size( 3, 3 ));
							erode( inputImg, inputImg,element);
							dilate( inputImg, inputImg,element);

							imwrite(testfile.str().c_str(), inputImg);
							
							
							Canny( inputImg, canny_output, lowthresh, highthresh, 3);
							//imwrite(testfile.str().c_str(), inputImg);
							
							
							

							findContours( canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point(0, 0) );
							
							Mat drawing = (Mat::zeros( canny_output.size(), CV_8UC1 ));

							
							debugger << "contour size: " << contours.size() << endl;
							for( size_t j = 0; j< contours.size(); j++ )
								 {
								  double length=arcLength(contours[j], true);
								  Rect boundRect=boundingRect(contours[j]);

								   Scalar color = SCALAR_WHITE;
								   drawContours( drawing, contours, (long)j, color, 2, 8, hierarchy, 0, Point() );


								 }

							

							stringstream outfilename;
						 	outfilename << datapath << expID << "/current_contrast.png";
			
						 	imwrite(outfilename.str().c_str(), drawing);
		}  catch (cv::Exception ex) {
			debugger << " output was bad" << endl;
			return (0);
		} //end caught exception trying to load
							
							
	return 1;
	  

}//end update contours





//gets the lifespan based on description.txt and the frame time
long getLifespan(string filename, long frametime){
	ifstream ifile(filename.c_str());
	string inputline;
	long expstarttime=0;
	int daysold=0;

	int i=0;
	while (getline(ifile,inputline)){
		if (i==5){
			expstarttime=atol(inputline.c_str());
		}//end process experiment start time
		if (i==11){
            daysold = atoi(inputline.c_str());
            cout << "daysold," << daysold << endl;
            cout << "expstarttime," << expstarttime << endl;
            cout << "frametime," << frametime << endl;
             return (frametime-(expstarttime -(86400 * daysold)));

		}//end process starting age
		i++;
	}


}//end getexperimentstarttime


long getFileCreationTime(string filename){
	struct stat attr;
	if (stat(filename.c_str(),&attr) == -1) return (-1); //if file not found return -1
	long time = attr.st_mtim.tv_sec;

	return (time);

}//end getFileCreationTime


float getAgeinDays(int framenum){

	stringstream oss;
	string fulldirectory;
	long frametime;
	oss << datapath << expID << "/";

		fulldirectory=oss.str();
		stringstream number;
		stringstream ss;
		number << setfill('0') << setw(6) << framenum;
		ss << fulldirectory << "frame" << number.str() <<".png";
		frametime = getFileCreationTime(ss.str());
		if (frametime == -1) return (-1); //if file not found
		ss.str("");
		ss << fulldirectory << "description.txt";
	return(((float)(getLifespan(ss.str(),frametime)))/86400.0f);
}//end getageinDays

int getAgeinMinutes(int framenum){

	stringstream oss;
	string fulldirectory;
	long frametime;
	oss << datapath << expID << "/";

		fulldirectory=oss.str();
		stringstream number;
		stringstream ss;
		number << setfill('0') << setw(6) << framenum;
		ss << fulldirectory << "frame" << number.str() <<".png";
		//debugger << ss.str() << ",";
		frametime = getFileCreationTime(ss.str());
		if (frametime == -1) return (-1);  //if file not found
		//debugger << "frametime: " << frametime;
		ss.str("");
		ss << fulldirectory << "description.txt";
	return(((float)(getLifespan(ss.str(),frametime)))/60);
}//end getageinDays


void loadWorms(string filename){
     ifstream ifile(filename.c_str());
     string inputline;
     numworms=0;

     while(getline(ifile,inputline)){
    	 Worm myworm(inputline);
    	 wormlist.push_back(myworm);
    	 numworms++;
     }//end while inputlines
}//end load worms

string getChannelName(string name){
	if(name =="gfp") return string("GFP");
	if(name =="bf") return string("frame");
	if(name =="cherry") return string("CHERRY");
	if(name =="uv") return string("UV");
	
	return string("notfound");
}//end getCHannelName

string getFFMPEGResolution(string name, bool docomplex){
	if (!docomplex){
		if(name =="1080") return string(" -vf scale=-1:1080 ");
		if(name =="720") return string(" -vf scale=-1:720 ");
		if(name =="480") return string(" -vf scale=-1:480 ");
		if(name =="full") return string(" ");
	}
	//complex option
	if(name =="1080") return string(" scale=-1:1080 ");
	if(name =="720") return string(" scale=-1:720 ");
	if(name =="480") return string(" scale=-1:480 ");
	if(name =="full") return string(" scale=-1:-1 ");
	

	
	return string(" ");


}//end getFFMPEGResolution

string buildMovie(string filename, int startframe, int endframe, string movChannel, bool drawDeadWorms, string mres){
	stringstream oss;
	stringstream ffmpeg;
	stringstream lastcomp;

	movChannel = getChannelName(movChannel);
	mres = getFFMPEGResolution(mres, drawDeadWorms);
	

	ffmpeg << "./ffmpeg -y -f image2 -start_number " << startframe
		<<" -i "<< filename << movChannel << "%06d.png ";

	if (wormlist.size() != 0 && drawDeadWorms) {

		for (int i=0; i < wormlist.size(); i++){
			ffmpeg << " -i /var/www/html/wormbot/img/dead.png "; //load the worm circle for each dead worm
		}
		//load the base timelapse movie
		ffmpeg << " -filter_complex \" [0:v] setpts=PTS-STARTPTS [base]; ";

		int counter=1;
		for (int i=0; i < wormlist.size(); i++){
			ffmpeg << " [" << counter+i << ":v] setpts=PTS-STARTPTS [dead" << counter+i << "]; ";
		}

		lastcomp << "[base]";

		for (int i=0; i < wormlist.size(); i++){

			int start =wormlist[i].currf-startframe;
			int end = endframe - startframe;

			ffmpeg << lastcomp.str() << "[dead" << counter +i << "] overlay="
			   << wormlist[i].x -25 << ":" << wormlist[i].y -25<< ":";
			ffmpeg << "enable='between(n," << start << "," << end << ")' ";

			lastcomp.str(""); //erase the last composite title

			if (i + 1 < wormlist.size()) {
				lastcomp << "[tmp" << i <<"] ";
				ffmpeg << lastcomp.str() << ";";
			} else { //last entry
				lastcomp << "[tmp" << i <<"] ";
				ffmpeg << lastcomp.str() << ";" << lastcomp.str() << mres << "[out]\" -map \"[out]\" ";
			}//else is last one

		}
		
		//ffmpeg << " \"";
	}

	ffmpeg << " -q:v 1 -vframes " << (endframe+1)-startframe << " ";
	if (!drawDeadWorms) ffmpeg << mres;
	ffmpeg << filename << expID <<"_" << movChannel << ".avi 2>&1 | tee /var/www/robot_data/ffmpegstdout.txt" << endl;

	system(ffmpeg.str().c_str());

	string fn = filename + "/tempffmpegcomand";
	ofstream ofile(fn.c_str());
	ofile << ffmpeg.str() <<endl;
	ofile.close();

	oss << "worked";

	return (oss.str());
}//end buildmovie

string printWormLifespan(string title){
	stringstream oss;
	if (wormlist.empty()) return oss.str();



	Lifespan ls(wormlist);

	oss << "N," << ls.n <<  endl;
		oss << "Mean, " << ls.mean <<  endl;
		oss << "Median," << ls.median << endl;
		oss << "Max, " << ls.maxlifespan  << endl <<endl;

		oss << "%" <<  title << endl;
	oss << ls.getOasisListHTML();
	oss << "\n\n\n";
	oss << "%" <<  title << endl;
	oss << ls.getOasisListFloat();




	return (oss.str());
}//end printwormlifespan

double medianMat(Mat Input){    
    Input = Input.reshape(0,1); // spread Input Mat to single row
    vector<double> vecFromMat;
    Input.copyTo(vecFromMat); // Copy Input Mat to vector vecFromMat
    nth_element(vecFromMat.begin(), vecFromMat.begin() + vecFromMat.size() / 2, vecFromMat.end());
    return vecFromMat[vecFromMat.size() / 2];
}



string getStats(int x, int y, int w, int h, int f, string chan, unsigned char bgv) {

	stringstream ss,filename;

	filename << datapath << expID << "/";
	
	if (chan == string("gfp")) filename << "GFP" << pad(f) << ".png";
	if (chan == string("bf")) filename << "frame" << pad(f) << ".png";
	if (chan == string("cherry")) filename << "CHERRY" << pad(f) << ".png";
	if (chan == string("uv")) filename << "UV" << pad(f) << ".png";

	debugger << "arect filename:" << filename.str() << endl;
	
	Mat thisImg = imread(filename.str());
	Rect rec(x,y,w,h);
	Mat recIm = thisImg(rec).clone(); //clone out the Rect

	if(chan== string("bf")) {

		cvtColor(recIm, recIm, cv::COLOR_BGR2GRAY);


		double min,max;

		//output min and max
		minMaxLoc(recIm, &min, &max);
		ss << min << "," << max << ",";


		//output mean
		ss << mean(recIm)[0] << ",";

		//calculate media pixel value
		double medianVal= medianMat(recIm);
		ss << medianVal << ",";
		

		//calculate median background from area * min
		double backgroundVol = (double)recIm.rows * (double)recIm.cols * min;
		ss << backgroundVol << ",";


		//output sum area, ie. volume

		double totvol = sum(recIm)[0];
		ss << totvol << ",";

		//output sum - bg
		ss << totvol - backgroundVol << ",";

		

		

		
		 int histSize = 256; // bin size
		 float range[] = { 0, 256} ;
		 const float* histRange = { range };

		 bool uniform = true;
		 bool accumulate = false;

		 Mat hist;

		 int channels[] = {0};

		 /// Compute the histograms:
		 calcHist( &recIm, 1, channels, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );

		 //output histogram
		 //0-255
		for( int i = 0; i < histSize; i++ ){
			ss << hist.at<float>(i) << ",";

		}//end for each value
		return ss.str();
		
	}//end if brightfield
	if(chan== string("gfp") || chan== string("cherry") || chan== string("uv") ) {
		int channel=0; //default UV blue;

		Mat bgr[3];   
		split(recIm,bgr);
		
		if (chan== string("gfp")) channel=1;
		if (chan== string("cherry")) channel=2;


		double min,max;

		//output min and max
		minMaxLoc(bgr[channel], &min, &max);
		ss << min << "," << max << ",";


		//output mean
		ss << mean(bgr[channel])[0] << ",";

		//calculate media pixel value
		double medianVal= medianMat(bgr[channel]);
		ss << medianVal << ",";
		
		//calculate median background from area * min
		double backgroundVol = (double)bgr[channel].rows * (double)bgr[channel].cols * min;
		ss << backgroundVol << ",";

		//output sum area, ie. volume

		double totvol = sum(bgr[channel])[0];
		ss << totvol << ",";

		


		

		//output sum - bg
		ss << totvol - backgroundVol << ",";




		
		 int histSize = 256; // bin size
		 float range[] = { 0, 256} ;
		 const float* histRange = { range };

		 bool uniform = true;
		 bool accumulate = false;

		 Mat hist;

		 int channels[] = {0};

		 /// Compute the histograms:
		 calcHist( &bgr[channel], 1, channels, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );

		 //output histogram
		 //0-255
		for( int i = 0; i < histSize; i++ ){
			ss << hist.at<float>(i) << ",";

		}//end for each value
		return ss.str();
		
	}//end if fluor

	/*
	    vector<Mat> bgr_planes;
	    split( recIm, bgr_planes );
	    int histSize = 256;
	    float range[] = { 0, 256 }; //the upper boundary is exclusive
	    const float* histRange[] = { range };
	    bool uniform = true, accumulate = false;
	    Mat b_hist, g_hist, r_hist;
	    calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, histRange, uniform, accumulate );
	    calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, histRange, uniform, accumulate );
	    calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, histRange, uniform, accumulate );
	*/
	return ss.str();

}//end getStats



//////////// M A I N
int main(int argc,char **argv){

	cout << setprecision(5);

	getline(pathfile,datapath);
	pathfile.close();

	string updatepath = datapath + "updatedebug";
	debugger.open(updatepath.c_str());

	stringstream wormfilename;
	 Cgicc cgi;

	cout << HTTPHTMLHeader() << endl;
        cout << html() << endl << endl;
	 int moviestart,moviestop=0;
	 int highthresh,lowthresh =0;
	 int currframe=837;
	string mchan="";
	string ctchan="";
	unsigned char bg = 0; //hold the background value

	try {


	      string foo = cgi("deadworms");
	      string bar = cgi("analrects");	
		ctchan = cgi("ctchan");
	      expID = atoi(string(cgi("expID")).c_str());
	      moviestart = atoi(string(cgi("startmovie")).c_str());
	      moviestop = atoi(string(cgi("stopmovie")).c_str());
	      highthresh = atoi(string(cgi("highthresh")).c_str());
    	  lowthresh = atoi(string(cgi("lowthresh")).c_str());
    	  currframe = atoi(string(cgi("currframe")).c_str());
		mchan = cgi("mchan");  
		string bgstyle = cgi("bgstyle");  
		
		   
		debugger << "L:" << lowthresh << " " << cgi("lowthresh") << " H: " << highthresh  << " " << cgi("highthresh") <<  
				"EXPID:" << expID << " movie start:" << moviestart << " movie stop:" << moviestop << " mchan:" << mchan << endl;

		time_t logtime;
		time(&logtime);

		stringstream logfilename;
		logfilename << datapath << expID << "/" << logtime << "_updatelog";
		ofstream logfile(logfilename.str().c_str());
		
		logfile << "IP:" << cgi.getEnvironment().getRemoteAddr() << endl;
		logfile << "USER:" << cgi.getEnvironment().getRemoteUser() << endl;
		logfile << "HOST:" << cgi.getEnvironment().getRemoteHost() << endl;
		logfile << "EXPID:" << expID << endl;
		logfile << "deadworms:" << foo << endl;
		logfile.close();

	      wormfilename << datapath << expID << "/wormlist.csv";
	      ofstream wormfile(wormfilename.str().c_str());
	      stringstream readcgi;
		  readcgi << foo;

		  ptree pt;
		  read_json (readcgi, pt);

		  string tboost(datapath + "updateboost");
		  ofstream testboost(tboost.c_str());
		  testboost << "movstart:" << moviestart << ",moviestop:" << moviestop << endl;
		  write_json(testboost,pt);

		  testboost.close();



		  BOOST_FOREACH(const ptree::value_type &v, pt.get_child("")) {
			  
			//check to see if the frame exists
			if (getAgeinDays(v.second.get<int>("deathframe")) == -1) continue; //skip the invalid worm
			  
		      	  wormfile  <<  (int)v.second.get<float>("x") << ",";
		      	  wormfile <<  (int)v.second.get<float>("y") << ",";
		      	  wormfile  <<  v.second.get<int>("deathframe") << ",";
		      	  wormfile  <<  v.second.get<int>("number") << ",";
		      	 
		      		  wormfile <<   getAgeinDays(v.second.get<int>("deathframe")) << ",";
		      		  wormfile <<   getAgeinMinutes(v.second.get<int>("deathframe")) << endl;
		      	 


		   }//end boostforeach

		  wormfile.close();


		//read in analysis rectangles
			  stringstream rectfilename;
			  rectfilename << datapath << expID << "/arects_" << time(0) << ".csv";
		    	  ofstream rfile(rectfilename.str().c_str());
		     	  stringstream Rreadcgi;
			  Rreadcgi << bar;

			  ptree rpt;
			  read_json (Rreadcgi, rpt);

		           //output column header		
			   rfile << "x,y,width,height,frame,ID,channel,Days, Minutes, Min, Max, Mean, Median, BgVol, Vol, Vol-BgVol" << endl;

			   BOOST_FOREACH(const ptree::value_type &v, rpt.get_child("")) {
				
			  }//end find background if it exists first

			  BOOST_FOREACH(const ptree::value_type &v, rpt.get_child("")) {
				  
				//check to see if the frame exists
				if (getAgeinDays(v.second.get<int>("f")) == -1) continue; //skip the invalid rects
				  	  int x = (int)v.second.get<float>("x");
					  int y = (int)v.second.get<float>("y");
					  int w = (int)v.second.get<float>("w");
					  int h = (int)v.second.get<float>("h");
					  int f = v.second.get<int>("f");
					  string c = v.second.get<string>("c");
				      	  rfile  <<  x << ",";
				      	  rfile <<  y << ",";
					  rfile  <<  w << ",";
				      	  rfile <<  h << ",";
				      	  rfile  <<  f << ",";
				      	  rfile  <<  v.second.get<string>("id") << ",";
					  rfile  <<  c << ",";
				      	 
			      		  rfile <<   getAgeinDays(f) << ",";
			      		  rfile <<   getAgeinMinutes(f) << ",";

					  rfile << getStats(x,y,w,h,f,c,bg);

					  rfile << endl;
			      	 


			   }//end boostforeach

		//	Rect rec(x,y,w,h);
		//frames.push_back(thisImg(rec).clone());

			  rfile.close();


		//end analysis rects



	} catch (exception& e){
		
		//return (0);

	}//end exception caught


	//build the wormlist
	stringstream wormpath;
	wormpath << datapath << expID << "/";
	loadWorms(wormfilename.str());


	if (cgi.queryCheckbox("buildMovie")){

		buildMovie(wormpath.str(),moviestart,moviestop, mchan, cgi.queryCheckbox("drawDead"), cgi("mres") );

	}//end if want to build a new movie

	if (cgi.queryCheckbox("updatecontours")){
		stringstream currimgfilename;
		stringstream number;
		string channelName;
		int procchan =-1;
		if (ctchan == "bf") {channelName="/frame"; procchan=-1;}
		else if (ctchan == "gfp") {channelName="/GFP"; procchan=1;}
		else if (ctchan == "cherry") {channelName="/CHERRY"; procchan=2;}
		else if (ctchan == "uv") {channelName="/UV"; procchan=0;}
		number << setfill('0') << setw(6) << currframe;
		currimgfilename << datapath << expID << channelName << number.str() << ".png";
		debugger << "img filename " << currimgfilename.str() << " img channel:" << procchan << endl;
		Update_Contours(currimgfilename.str(),lowthresh, highthresh, procchan);

	}//end if update contours



	//Dump data to lifespanoutput_expID.csv


	//dump description into lifespan output
	stringstream filename,ofilename;
	filename << datapath << expID << "/description.txt";
	ofilename << datapath << expID << "/lifespanoutput_" << expID << ".csv";
	ifstream ifile(filename.str().c_str());
	ofstream ofile(ofilename.str().c_str());

	debugger << "out filename: " << ofilename.str() << " descfilename: " << filename.str() << endl;
	string aline;

	int c=0;

	string dtitle;
	while (getline(ifile,aline)){
		switch(c){
		case 0:
			break;
		case 1:
			ofile << "title," << aline << endl;
			dtitle=aline;
			break;
		case 2:
			ofile << "email," << aline << endl;
			break;
		case 3:
			ofile << "investigator," << aline << endl;
			break;
		case 4:
			ofile << "description," << aline << endl;
			break;
		case 5:
			ofile << "unix epoch start time," << aline << endl;
			break;
		case 6:
			ofile << "last experiment frame number,"  << aline << endl;
			break;
		case 7:
			ofile << "strain,"  << aline << endl;
			break;
		case 9:
			ofile << "experiment directory,"  << aline << endl;
			break;
		case 10:
			ofile << "starting N," << aline << endl;
			break;
		case 11:
			ofile << "starting Age in days," << aline << endl;
			break;
		case 12:
			ofile << "expID," << expID << endl;
			break;
		case 15:
			ofile << "plate number," << aline << endl;
			break;
		case 16:
			ofile << "well," << aline << endl;
			break;




		}//end switch


		c++;
	}//end while lines in description
	ifile.close();

	ofile << printWormLifespan(dtitle);


	ofile.close();

	debugger.close();

	//output something so data calls success
		cout << HTTPHTMLHeader() << endl;
		cout << html() << head(title("WormBot Response")) << endl;
		cout << "</body></html>" << endl;


	return 0;
}
