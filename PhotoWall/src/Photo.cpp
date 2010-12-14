#include "Photo.h"

int Photo::photoWidth = 816;
int Photo::photoHeight = 612;

void Photo::setup(const ControlSurface& surface, float x, float y) {
	nw = &(surface.get(floor(x), floor(y)));
	ne = &(surface.get(ceil(x), floor(y)));
	sw = &(surface.get(floor(x), ceil(y)));
	se = &(surface.get(ceil(x), ceil(y)));
	baseColor.set(ofRandom(0, 1), ofRandom(0, 1), ofRandom(0, 1));
	img = PhotoManager::randomPhoto();
}

void Photo::update() {
	size = nw->distance(*se) + ne->distance(*sw);
	size /= 4;
	//size = size * size;
	//size /= 6 * 6;

	position = *nw + *ne + *sw + *se;
	position /= 4;

	ofxVec2f o1 = *sw - *se;
	ofxVec2f o2 = *nw - *ne;
	rotation = atan2f(o1.y, o1.x) + atan2f(o2.y, o2.x);
	rotation /= 2;

	brightness = ofMap(size, 200, 400, 1, 0, true);
}

void Photo::draw() const {
	glPushMatrix();

	glTranslatef(position.x, position.y, size / 10.);
	ofRotate(ofRadToDeg(rotation));

/*
	// frame
	glColor3f(1, 1, 1);
	glScalef(size * 16 / 9, size, 1);
	glBegin(GL_LINE_LOOP);
	glVertex2f(-.5, -.5);
	glVertex2f(-.5, .5);
	glVertex2f(.5, .5);
	glVertex2f(.5, -.5);
	glEnd();
*/
	// photo
	//glColor3f(brightness * baseColor.r, brightness * baseColor.g, brightness * baseColor.b);
	glScalef(.95, .95, 1);
	
	if(ofxMultiscreen::master) {
		glColor3f(baseColor.r, baseColor.g, baseColor.b);
		glBegin(GL_QUADS);
		glVertex2f(-.5, -.5);
		glVertex2f(-.5, .5);
		glVertex2f(.5, .5);
		glVertex2f(.5, -.5);
		glEnd();
	} else {
		glColor3f(1, 1, 1);
		img->bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(-.5, -.5);
		glTexCoord2f(0, photoHeight);
		glVertex2f(-.5, .5);
		glTexCoord2f(photoWidth, photoHeight);
		glVertex2f(.5, .5);
		glTexCoord2f(photoWidth, 0);
		glVertex2f(.5, -.5);
		glEnd();
		img->unbind();
	}

	glPopMatrix();
}

bool Photo::inside(const ofRectangle& window) const {
	return position.x > window.x && position.y > window.y &&
		position.x < window.x + window.width && position.y < window.y + window.height;
}
