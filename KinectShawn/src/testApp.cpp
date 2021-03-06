#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup() {
	//kinect.init(true);  //shows infrared image
	kinect.init();
	kinect.setVerbose(true);
	kinect.open();

	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThresh.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);

//	nearThreshold = 230;
//	farThreshold  = 70;
	bThreshWithOpenCV = true;
	
	ofSetFrameRate(30);

	angle = -90;
	kinect.setCameraTiltAngle(angle);
	
	// start from the front
	pointCloudRotationY = 180;
	
	drawPC = false;
	setupOsc();
	
	//load settings for near far threshold, flipped color, angle?
	cout << "loading mySettings.xml" << endl;
	
	//we load our settings file
	//if it doesn't exist we can still make one
	//by hitting the 's' key
	if( imageSettings.loadFile("settings.xml") ){
		cout << "settings.xml loaded!" << endl;
	}else{
		cout << "unable to load mySettings.xml check data/ folder" << endl;
	}

	nearThreshold = imageSettings.getValue("options:nearThreshold", 230);
	farThreshold = imageSettings.getValue("options:farThreshold", 70);
	blobSize = imageSettings.getValue("options:blobSize", 400);
	
	kinect.getCalibration().setClippingInCentimeters(100, 500);
}

//--------------------------------------------------------------
void testApp::setupOsc() {
	ofxXmlSettings settings;
	if(!settings.loadFile("osc.xml")) {
		ofLog(OF_LOG_ERROR, "testApp::setupOsc(): couldn't load osc.xml");
	}
	string address = settings.getValue("address", "255.255.255.255");
	int port = settings.getValue("port", 8888);
	
    cout << "Broadcasting to " << address << ":" << port << endl;
    oscSender.setup(address, port);
}

//--------------------------------------------------------------
void testApp::update()
{
	ofBackground(100, 100, 100);
	
	kinect.update();
	if(kinect.isFrameNew())	// there is a new frame and we are connected
	{

		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
			
		//we do two thresholds - one for the far plane and one for the near plane
		//we then do a cvAnd to get the pixels which are a union of the two thresholds.	
		if( bThreshWithOpenCV ){
			grayThreshFar = grayImage;
			grayThresh = grayImage;
			grayThresh.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThresh.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		}else{
		
			//or we do it ourselves - show people how they can work with the pixels
		
			unsigned char * pix = grayImage.getPixels();
			int numPixels = grayImage.getWidth() * grayImage.getHeight();

			for(int i = 0; i < numPixels; i++){
				if( pix[i] < nearThreshold && pix[i] > farThreshold ){
					pix[i] = 255;
				}else{
					pix[i] = 0;
				}
			}
		}

		//update the cv image
		grayImage.flagImageChanged();

		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayImage, blobSize, (kinect.width*kinect.height)/2, 20, false);
		
		
		ofxOscMessage message;
		message.setAddress("mouse");
		
		for (int i = 0; i < contourFinder.blobs.size(); i++){
			//cerr<< contourFinder.blobs[i].centroid;
			curPoint.set( contourFinder.blobs[i].centroid );
			
//			curPoint.normalize();
//			message.addFloatArg(curPoint.x);
//			message.addFloatArg(curPoint.y);

			message.addFloatArg(ofMap(curPoint.x, 0.0, 640.0, 0.0, 1.0, true));
			message.addFloatArg(ofMap(curPoint.y, 0.0, 480.0, 0.0, 1.0, true));
			
		}
		oscSender.sendMessage(message);
	}
}

//--------------------------------------------------------------
void testApp::draw() {
	ofSetColor(255, 255, 255);
	if(drawPC){
		ofPushMatrix();
		ofTranslate(420, 320);
		// we need a proper camera class
		drawPointCloud();
		ofPopMatrix();
	}else{
		kinect.drawDepth(0, 0, 640 * 2, 480 * 2);
		kinect.draw(420, 10, 400, 300);

		grayImage.draw(10, 320, 400, 300);
		contourFinder.draw(10, 320, 400, 300);
		
	}
	

	ofSetColor(255, 255, 255);
	stringstream reportStream;
	reportStream //<< "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
				 //				 << ofToString(kinect.getMksAccel().y, 2) << " / " 
				 //				 << ofToString(kinect.getMksAccel().z, 2) << endl
				 //<< "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
				 //<< "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
				 << "set near threshold " << nearThreshold << " (press: + -)" << endl
				 << "set far threshold " << farThreshold << " (press: < >)" << endl
				 << "set blob size " << blobSize << " (press: [ ]) " 
				 << "num blobs found " << contourFinder.nBlobs << endl
				 << "press 's' to save these settings" << endl;
				 //	<< ", fps: " << ofGetFrameRate() << endl
				 //<< "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl
				 //<< "press UP and DOWN to change the tilt angle: " << angle << " degrees";
	ofDrawBitmapString(reportStream.str(),20,666);
}

void testApp::drawPointCloud() {
	ofScale(400, 400, 400);
	int w = 640;
	int h = 480;
	ofRotateY(pointCloudRotationY);
	glBegin(GL_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			ofPoint cur = kinect.getWorldCoordinateFor(x, y);
			ofColor color = kinect.getCalibratedColorAt(x,y);
			glColor3ub((unsigned char)color.r,(unsigned char)color.g,(unsigned char)color.b);
			glVertex3f(cur.x, cur.y, cur.z);
		}
	}
	glEnd();
}

//--------------------------------------------------------------
void testApp::exit() {
	//kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
		break;
		case'p':
			drawPC = !drawPC;
			break;
	
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			imageSettings.setValue("options:farThreshold", farThreshold);
			break;
		case '<':		
		case ',':		
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			imageSettings.setValue("options:farThreshold", farThreshold);
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			imageSettings.setValue("options:nearThreshold", nearThreshold);
			break;
		case '-':		
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			imageSettings.setValue("options:nearThreshold", nearThreshold);
			break;

		case '[':
		case '{':
			blobSize-=10;
			if (blobSize < 0) blobSize = 0;
			imageSettings.setValue("options:blobSize", blobSize);
			break;
		case ']':
		case '}':
			blobSize+=10;
			if (blobSize > 307200) blobSize = 307200;
			imageSettings.setValue("options:blobSize", blobSize);
			break;
			
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
		case 'o':
			kinect.setCameraTiltAngle(angle);	// go back to prev tilt
			kinect.open();
			break;
		case 'c':
			kinect.setCameraTiltAngle(0);		// zero the tilt
			kinect.close();
			break;
		case 's':
			imageSettings.saveFile("settings.xml");
			cout << "saved xml settings" << endl;
			break;
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;

		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {
	pointCloudRotationY = x;
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}

