#pragma once

#include "MultiScreen.h"

class MultiWindow {
public:
	vector<MultiScreen> screens;

	MultiWindow() {
	}
	float getWidth() const {
		float width = 0;
		for(unsigned int i = 0; i < screens.size(); i++)
			width += screens[i].width;
		return width;
	}
	float getHeight() const {
		float height = 0;
		for(unsigned int i = 0; i < screens.size(); i++)
			height += screens[i].height;
		return height;
	}

	friend ostream& operator<<(ostream& out, const MultiWindow& window) {
		out << window.getWidth() << "x" << window.getHeight() << " { ";
		for(unsigned int i = 0; i < window.screens.size(); i++) {
			out << window.screens[i] << " ";
		}
		out << "}";
		return out;
	}
};
