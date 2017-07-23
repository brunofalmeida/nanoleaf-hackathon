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
#include <stdio.h>

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
	double x, y;                // origin, const
	double v;                   // velocity, const
	double rad;                 // radius of explosion, var
	double lifetime;	          // lifetime of source, var
	int r, g, b;                // red, green, blue
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

		// TODO adjust
		sources[index].v = 1000;
		sources[index].rad = 50;
		sources[index].lifetime = 10;

		sources[index].r = rand() % 256;	// TODO - different based on frequency
		sources[index].g = rand() % 256;
		sources[index].b = rand() % 256;

		numSources++;
	}
}

void deleteSource(int index) {
	printf("Deleting source\n");
	memmove(sources + index, sources + index + 1, sizeof(Source) * (numSources - index - 1));
	numSources--;
}

void propagateSource(Source *source) {
	// TODO - check macro for transition time
	source->rad += source->v * 0.05;
	source->lifetime --;
	printf("%.2lf %.1lf\n", source->rad, source->lifetime);
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

	for (int i =0; i<numSources;i++){
		if (sources[i].lifetime<0){
			deleteSource(i);
		}
	}
	if (getIsBeat()) {
		printf("beat\n");
		initSource(numSources);
	}

	for (int iPanel = 0; iPanel < layoutData->nPanels; iPanel++) {
		frames[iPanel].panelId = layoutData->panels[iPanel].panelId;
		frames[iPanel].r = 0;
		frames[iPanel].g = 0;
		frames[iPanel].b = 0;
		frames[iPanel].transTime=5;
		for (int iSource = 0; iSource < numSources; iSource++) {
			double dist = Point::distance(layoutData->panels[iPanel].shape->getCentroid(), Point(sources[iSource].x, sources[iSource].y));
			if (abs(dist - sources[iSource].rad) <= 50) {
				frames[iPanel].r = sources[iSource].r;
				frames[iPanel].g = sources[iSource].g;
				frames[iPanel].b = sources[iSource].b;
				frames[iPanel].transTime = 0;
			} else {
				frames[iPanel].transTime = 3;
			}
		}
	}

	*nFrames = layoutData->nPanels;

	for (int i = 0; i < numSources; i++) {
		propagateSource(&sources[i]);
	}
}

/**
 * @description: called once when the plugin is being closed.
 * Do all deallocation for memory allocated in initplugin here
 */
void pluginCleanup(){
	//do deallocation here
}
