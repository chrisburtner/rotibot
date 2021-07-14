# define commands to use for compilation
CXX = g++
CFLAGS = -v -g -w -Wall -Wpedantic -O0 -std=c++11




OPENCV_LIB = -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core
PYLON_LIB = -L/opt/pylon/lib -lpylonbase -Wl,-rpath=/opt/pylon/lib -lpylonutility -lGenApi_gcc_v3_1_Basler_pylon -lGCBase_gcc_v3_1_Basler_pylon

# libraries
LIB = -L/usr/local/lib -lserial -L/usr/lib/x86_64-linux-gnu -L/opt/pylon/lib

# -lcgicc

# includes
INC = -I/usr/include -I/usr/local/include -I/opt/pylon/include 

grab: src/grab.cpp
	$(CXX) $(CFLAGS) $(INC) src/grab.cpp $(LIB) $(OPENCV_LIB) $(PYLON_LIB) -o bin/grab


alignerd: src/alignerd.cpp
	$(CXX) $(CFLAGS) src/alignerd.cpp $(LIB) $(OPENCV_LIB) -o bin/alignerd

controller: src/controller.cpp
	$(CXX) $(CFLAGS) $(INC) src/controller.cpp $(LIB) $(OPENCV_LIB) $(PYLON_LIB) -o bin/controller

rapidscan: src/rapidscan.cpp
	$(CXX) $(CFLAGS) src/rapidscan.cpp $(LIB) $(OPENCV_LIB) -o bin/rapidscan

colorpicker: src/colorpicker.cpp
	$(CXX) $(CFLAGS) src/colorpicker.cpp $(LIB) $(OPENCV_LIB) -o bin/colorpicker


experimentbrowser: src/experimentbrowser.cpp
	$(CXX) $(CFLAGS) src/experimentbrowser.cpp -lcgicc -o bin/experimentbrowser

backcatalog: src/backcatalog.cpp
	$(CXX) $(CFLAGS) src/backcatalog.cpp -lcgicc -o bin/backcatalog

wormbotstatus: src/wormbotstatus.cpp
	$(CXX) $(CFLAGS) src/wormbotstatus.cpp -lcgicc -o bin/wormbotstatus

marker: src/marker.cpp
	$(CXX) $(CFLAGS) src/marker.cpp $(OPENCV_LIB) -lcgicc -o bin/marker

scheduler: src/scheduler.cpp
	$(CXX) $(CFLAGS) src/scheduler.cpp -lcgicc -o bin/scheduler

bulkuploader: src/bulkuploadMini.cpp
	$(CXX) $(CFLAGS) src/bulkuploadMini.cpp -lcgicc -o bin/bulkuploader

cgiccretro: src/cgiccretro.cpp
	$(CXX) $(CFLAGS) src/cgiccretro.cpp $(LIB) $(OPENCV_LIB) -lcgicc -o bin/cgiccretro

wormlistupdater: src/wormlistupdater.cpp
	$(CXX) $(CFLAGS) src/wormlistupdater.cpp $(LIB) $(OPENCV_LIB) -lcgicc -o bin/wormlistupdater

cropFrames: src/cropFrames.cpp
	$(CXX) $(CFLAGS) src/cropFrames.cpp $(LIB) $(OPENCV_LIB) -lcgicc -o bin/cropFrames

experimentuploader: src/experimentuploader.cpp
	$(CXX) $(CFLAGS) src/experimentuploader.cpp $(LIB) $(OPENCV_LIB) -lcgicc -o bin/experimentuploader

plateExplorer: src/plateExplorer.cpp
	$(CXX) $(CFLAGS) src/plateExplorer.cpp $(LIB) $(OPENCV_LIB) -lcgicc -o bin/plateExplorer

graphmaker: src/graphmaker.cpp
	$(CXX) $(CFLAGS) src/graphmaker.cpp $(LIB) $(OPENCV_LIB) -lcgicc -o bin/graphmaker

trainingSetExport: src/trainingSetExport.cpp
	$(CXX) $(CFLAGS) src/trainingSetExport.cpp $(LIB) $(OPENCV_LIB) -lcgicc -o bin/trainingSetExport



all: alignerd controller experimentbrowser marker scheduler cgiccretro wormlistupdater colorpicker wormbotstatus experimentuploader plateExplorer graphmaker backcatalog cropFrames trainingSetExport bulkuploader rapidscan



clean:
	rm bin/*


%.o: %.cpp
	echo heyo
	cd src
	$(CXX) $(CFLAGS) -c $(INC) $<
