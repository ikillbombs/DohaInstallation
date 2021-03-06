#include "testApp.h"

void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);

	ofxXmlSettings settings;
	settings.loadFile("osc.xml");
	string address = settings.getValue("address", "255.255.255.255");
	int port = settings.getValue("port", 8888);

	if(master) {
		cout << "Connecting to " << address << ":" << port << endl;
		oscSender.setup(address, port);
		startScreens();
	}	
	cout << "Listening on port " << port << endl;
	oscReceiver.setup(port);
}

void testApp::update() {
	while(oscReceiver.hasWaitingMessages()) {
		ofxOscMessage message;
		oscReceiver.getNextMessage(&message);
		string address = message.getAddress();
		if(address.compare("mouse") == 0) {
			int mx = message.getArgAsInt32(0);
			int my = message.getArgAsInt32(1);
			ofPoint cur(mx, my);
			points.push_back(cur);
		} else if(address.compare("reset") == 0) {
			points.clear();
		}
	}
}

void testApp::drawLocal() {
	ofBackground(0, 0);
	ofSetColor(255, 255, 255);
	glBegin(GL_LINE_STRIP);
	for(unsigned int i = 0; i < points.size(); i++) {
		ofPoint& cur = points[i];
		glVertex2f(cur.x, cur.y);
	}
	glEnd();
}

void testApp::drawOverlay() {
}

void testApp::keyPressed(int key) {
}

void testApp::keyReleased(int key) {
}

void testApp::mouseMoved(int x, int y) {
	x *= 4;
	y *= 4;

	if(master) {
		ofxOscMessage message;
		message.setAddress("mouse");
		message.addIntArg(x);
		message.addIntArg(y);
		oscSender.sendMessage(message);
	}
}

void testApp::mouseDragged(int x, int y, int button) {
}

void testApp::mousePressed(int x, int y, int button) {
	if(master) {
		ofxOscMessage message;
		message.setAddress("reset");
		oscSender.sendMessage(message);
	}
}

void testApp::mouseReleased(int x, int y, int button) {
}

void testApp::windowResized(int w, int h) {
}

