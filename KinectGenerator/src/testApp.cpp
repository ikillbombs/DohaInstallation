#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
	ofSetVerticalSync(true);
	
	panel.setup("Control Panel", 5, 5, 280, 600);
	panel.addPanel("Contours");
	panel.addSlider("alpha", "alpha", .2, 0, 1);
	panel.addSlider("blurAmount", "blurAmount", 3, 0, 10, true);
	panel.addSlider("threshLevel", "threshLevel", 128, 0, 255, true);
	panel.addSlider("minArea", "minArea", 1000, 0, 100 * 100, true);
	panel.addSlider("maxArea", "maxArea", 10000, 32 * 32, 240 * 240, true);
	panel.addSlider("nConsidered", "nConsidered", 8, 1, 16, true);
	
	panel.addPanel("Camera");
	panel.addSlider("maxLen", "maxLen", 8, 0, 20);
	panel.addSlider("stepSize", "stepSize", 1, 1, 10, true);
	panel.addSlider("nearClipping", "nearClipping", 380, 0, 1024);
	panel.addSlider("farClipping", "farClipping", 600, 0, 1024);
	panel.addSlider("orthoScale", "orthoScale", .66, 0, 2);
	panel.addSlider("camx", "camx", 0, -1024, 1024);
	panel.addSlider("camy", "camy", -240, -1024, 1024);
	panel.addSlider("camz", "camz", -500, -1024, 1024);
	panel.addSlider("camrx", "camrx", 90, -180, 180);
	panel.addSlider("camry", "camry", 0, -180, 180);
	panel.addSlider("camrz", "camrz", 0, -180, 180);
	
	cam.setup();
	
	blur.allocate(640, 480);
	thresh.allocate(640, 480);
	
	setupOsc();
}

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

IplImage* toCv(unsigned char* pixels, int width, int height, int type) {
	int channels = type == OF_IMAGE_COLOR ? 3 : 1;
	IplImage* ipl = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, channels);
	cvSetData(ipl, pixels, channels * width);
	return ipl;
}

void testApp::updateOsc() {
	ofxOscMessage message;
	message.setAddress("mouse");
	
	vector<ofxCvBlob>& blobs = finder.blobs;
	for(int i = 0; i < blobs.size(); i++) {
		ofxVec2f curPoint(blobs[i].centroid);
	
		message.addFloatArg(curPoint.x);
		message.addFloatArg(ofMap(curPoint.y, 0, 1, .8, .9));
		
		message.addFloatArg(curPoint.x);
		message.addFloatArg(curPoint.y);
	}
	
	oscSender.sendMessage(message);
}

//--------------------------------------------------------------
void testApp::update() {
	cam.setMaxLen(panel.getValueF("maxLen"));
	cam.setStepSize(panel.getValueF("stepSize"));
	cam.setClipping(panel.getValueF("nearClipping"), panel.getValueF("farClipping"));
	cam.setOrthoScale(panel.getValueF("orthoScale"));
	cam.setPosition(ofxVec3f(panel.getValueF("camx"), panel.getValueF("camy"), panel.getValueF("camz")));
	cam.setRotation(ofxVec3f(panel.getValueF("camrx"), panel.getValueF("camry"), panel.getValueF("camrz")));
	
	int blurAmount = panel.getValueI("blurAmount");
	int threshLevel = panel.getValueI("threshLevel");
	int minArea = panel.getValueI("minArea");
	int maxArea = panel.getValueI("maxArea");
	int nConsidered = panel.getValueI("nConsidered");
	
	cam.update();
	if(cam.isFrameNew()) {		
		float alpha = panel.getValueF("alpha");
		float beta = 1 - alpha;
		IplImage* camIpl = toCv(cam.getPixels(), cam.getWidth(), cam.getHeight(), OF_IMAGE_GRAYSCALE);
		cvAddWeighted(camIpl, alpha, blur.getCvImage(), beta, 0, blur.getCvImage());
		blur.flagImageChanged();
		blur.blur(blurAmount * 2 + 1);
		
		thresh = blur;
		thresh.threshold(threshLevel);
		finder.findContours(thresh, minArea, maxArea, nConsidered, false);
		
		updateOsc();
	}
}

//--------------------------------------------------------------
void testApp::draw() {
	ofBackground(0, 0, 0);
	ofSetColor(255, 255, 255);
	cam.draw(0, 0);
	
	blur.draw(640, 0);
	ofEnableAlphaBlending();
	ofSetColor(255, 0, 0, 128);
	thresh.draw(640, 0);
	ofDisableAlphaBlending();
	
	finder.draw(0, 0);
}

void testApp::keyPressed(int key) {
}