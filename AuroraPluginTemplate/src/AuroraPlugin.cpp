/*
    Copyright 2017 Nanoleaf Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

#include "AuroraPlugin.h"
#include "LayoutProcessingUtils.h"
#include "ColorUtils.h"
#include "DataManager.h"
#include "PluginFeatures.h"
#include "Logger.h"
#include <math.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

	void initPlugin();
	void getPluginFrame(Frame_t* frames, int* nFrames, int* sleepTime);
	void pluginCleanup();

#ifdef __cplusplus
}
#endif

#define MAX_SOURCES 1
#define ADJACENT_PANEL_DISTANCE 86.599995

typedef struct Source {
	double x, y;
	double vx, vy;
	int r, g, b;
	double lifetime = INT_MAX;	// MAX_INT?
} Source;

int numSources = 0;

LayoutData *layoutData = NULL;
Source sources[MAX_SOURCES];

void initSource(int index) {
	if (index < MAX_SOURCES) {
		printf("Creating source\n");

		int i = rand() % layoutData->nPanels;
		Panel *panel = &layoutData->panels[i];

		// TODO - check for a panel adjacent to the current one
		sources[index].x = panel->shape->getCentroid().x;
		sources[index].y = panel->shape->getCentroid().y;

		// TODO - find constant for scaling tempo
		// double magnitude = getTempo();
		double magnitude = 10;
		double direction = (rand() % 360) * M_PI / 180.0;
		sources[index].vx = magnitude * cos(direction);
		sources[index].vy = magnitude * sin(direction);
		// sources[index].r = rand() % 256;	// TODO - better way?
		// sources[index].g = rand() % 256;
		// sources[index].b = rand() % 256;

		sources[index].r = 255;	// TODO - better way?
		sources[index].g = 0;
		sources[index].b = 0;

		printf("%lf %lf %lf %lf %d %d %d\n",
			sources[index].x, sources[index].y, sources[index].vx, sources[index].vy,
			sources[index].r, sources[index].g, sources[index].b);

		numSources++;
	}
	// TODO - delete function
}

void deleteSource(int index) {
	printf("Deleting source\n");
	memmove(sources + index, sources + index + 1, sizeof(Source) * (numSources - index - 1));
	numSources--;
}

void propagateSource(Source *source) {
	// TODO - check macro for transition time
	source->x += source->vx * 0.05/1e0;
	source->y += source->vy * 0.05/1e0;
}


/**
 * @description: Initialize the plugin. Called once, when the plugin is loaded.
 * This function can be used to enable rhythm or advanced features,
 * e.g., to enable energy feature, simply call enableEnergy()
 * It can also be used to load the LayoutData and the colorPalette from the DataManager.
 * Any allocation, if done here, should be deallocated in the plugin cleanup function
 *
 */
void initPlugin() {
	layoutData = getLayoutData();
	enableBeatFeatures();
}

/**
 * @description: this the 'main' function that gives a frame to the Aurora to display onto the panels
 * To obtain updated values of enabled features, simply call get<feature_name>, e.g.,
 * getEnergy(), getIsBeat().
 *
 * If the plugin is a sound visualization plugin, the sleepTime variable will be NULL and is not required to be
 * filled in
 * This function, if is an effects plugin, can specify the interval it is to be called at through the sleepTime variable
 * if its a sound visualization plugin, this function is called at an interval of 50ms or more.
 *
 * @param frames: a pre-allocated buffer of the Frame_t structure to fill up with RGB values to show on panels.
 * Maximum size of this buffer is equal to the number of panels
 * @param nFrames: fill with the number of frames in frames
 * @param sleepTime: specify interval after which this function is called again, NULL if sound visualization plugin
 */
void getPluginFrame(Frame_t* frames, int* nFrames, int* sleepTime){
	if (getIsBeat()) {
		printf("Beat\n");
		initSource(numSources);
	}

	for (int iPanel = 0; iPanel < layoutData->nPanels; iPanel++) {
		frames[iPanel].panelId = layoutData->panels[iPanel].panelId;
		frames[iPanel].r = 0;
		frames[iPanel].g = 0;
		frames[iPanel].b = 0;
		for (int iSource = 0; iSource < MAX_SOURCES; iSource++) {
			double distance = Point::distance(layoutData->panels[iPanel].shape->getCentroid(), Point(sources[iSource].x, sources[iSource].y));
			double multiplier = 3 * ADJACENT_PANEL_DISTANCE - distance;
			frames[iPanel].r += sources[iSource].r * multiplier;
			frames[iPanel].g += sources[iSource].g * multiplier;
			frames[iPanel].b += sources[iSource].b * multiplier;
		}
	}

	*nFrames = layoutData->nPanels;

	for (int i = 0; i < numSources; i++) {
		propagateSource(&sources[i]);
		printf("%lf %lf\n", sources[i].x, sources[i].y);
	}

	for (int i = numSources - 1; i >= 0; i--) {
		if (pointInsideWhichPanel(layoutData, Point(sources[i].x, sources[i].y)) == -1) {
			deleteSource(i);
		}
	}
}

/**
 * @description: called once when the plugin is being closed.
 * Do all deallocation for memory allocated in initplugin here
 */
void pluginCleanup(){
	//do deallocation here
}
