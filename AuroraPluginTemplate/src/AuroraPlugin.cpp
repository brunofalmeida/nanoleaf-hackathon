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
#include <algorithm>
#include <math.h>
using namespace std;
#define N_FFT_BINS 32
#ifdef __cplusplus
extern "C" {
#endif

	void initPlugin();
	void getPluginFrame(Frame_t* frames, int* nFrames, int* sleepTime);
	void pluginCleanup();

#ifdef __cplusplus
}
#endif

#define f0 440 // frequency of A about middle C (A_4)

FrameSlice_t* frameSlices = NULL;
int nFrameSlices = 0;
LayoutData* layoutData = NULL;
int angleRotate = 60; //rotate layout to 'horizontal'
/**
 * @description: Initialize the plugin. Called once, when the plugin is loaded.
 * This function can be used to enable rhythm or advanced features,
 * e.g., to enable energy feature, simply call enableEnergy()
 * It can also be used to load the LayoutData and the colorPalette from the DataManager.
 * Any allocation, if done here, should be deallocated in the plugin cleanup function
 *
 */
void initPlugin(){
	layoutData = getLayoutData();
	getFrameSlicesFromLayoutForTriangle(layoutData, &frameSlices, &nFrameSlices, rotateAuroraPanels(layoutData, &angleRotate));
	enableFft(N_FFT_BINS);
}

/**
 * @description: returns index within length of list
 * @param ind: value we wish to index into keys with, 9 + n, where 9 corresponds to index of A_4 and n is number of
 * half steps from A_4
 */
int wrapAroundIndex(int ind){
	if (ind < 0) {
		return 12 - ind;
	}
	else {
		return ind % 12;
	}
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

	uint8_t* fftBins = getFftBins();
	int sum = 0, sumAverage = 0;
	for (int i = 0; i < N_FFT_BINS; i++){
		sum = sum + fftBins[i];
		int average = i * fftBins[i];
		sumAverage = average + sumAverage;
	}

	double fn = 0;

	if (sumAverage!=0&&sum!=0){
		printf("%d %d\n", sumAverage,sum);
		double averageIndex = (double)sumAverage/sum;
		printf("%.2lf\n", averageIndex);
		fn = averageIndex*5500/32;
		printf("%.2lf Hz\n", fn);
	}

	int* keys[nFrameSlices]; // there should be 12

	for (int i = 0; i < nFrameSlices; i++){
		if (frameSlices[i].panelIds.size() > 0) {
			keys[i] = &frameSlices[i].panelIds[0];
		}
	}

	int n = (int)round( 12 * (log2(fn) - log2(f0)) ) % 12; //calculate the number of half steps away from reference f0 mod 12

	frames[0].panelId = 255;
	frames[0].r = 0;
	frames[0].g = 0;
	frames[0].b = 0;

	frames[1].panelId = *keys[wrapAroundIndex(9 + n)]]; // set this key on A is 9th key
	frames[1].r = 255;

	*nFrames = 2;
}

/**
 * @description: called once when the plugin is being closed.
 * Do all deallocation for memory allocated in initplugin here
 */
void pluginCleanup(){
	//do deallocation here
	freeFrameSlices(frameSlices);
}
