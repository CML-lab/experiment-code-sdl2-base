#ifndef TGTFRAME_H
#define TGTFRAME_H
#pragma once

// Data type used to store trial data for data writer
struct TargetFrame
{
	int trial;

	float startx;
	float starty;

	float tgtx;
	float tgty;

	float tgtx2;
	float tgty2;

	int trace;
	int path;
	int region;

	int score;

	int TrType;

	float velmin;
	float velmax;

	int PSstatus;

	char key;

	int lat;

};

#endif
