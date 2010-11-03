#pragma once

#include "ofxVectorMath.h"
#include "ofxXmlSettings.h"
#include "ofMain.h"

#include "MultiComputer.h"
#include "MultiScreen.h"
#include "MultiWindow.h"

class ofxMultiscreen : public ofBaseApp {
public:
	static const string appName, appDirectory;

	static vector<MultiComputer> computers;
	static bool master;
	static string hostname;
	static int display;
	static MultiWindow window;

	static void multiSetup();
	static void loadScreens(ofxXmlSettings& settings);
	static void startScreens();
	static void stopScreens();
	static void execute(string command);
	static void launch(string appName);

	static string getHostname();
	static int getDisplay();

	~ofxMultiscreen();

	void draw();

	virtual void drawInsideViewport() = 0;
	virtual void drawOutsideViewport() = 0;
};
