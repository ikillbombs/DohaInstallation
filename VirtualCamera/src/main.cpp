#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 2 * 640, 480, OF_WINDOW);
	ofRunApp(new testApp());
}
