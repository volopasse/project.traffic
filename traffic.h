#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "linkedlist.h"

#define CAR_LENGTH		5
#define N_BINS			20
#define SAFE_DIST_CONST	8.4

typedef struct vehicle{
	int ID;
	float s,v,a,d; //position, velocity, accelartion, deceleration
	float vDesired;
	struct vehicle *neighbours[3][3];
} vehicle;

//function declarations
int Integrate();
int Initialize();

//initialization
vehicle *CreateCar(int ID);
void DestroyCar(vehicle *car);
void AddCar(vehicle *car);

void CleanUpCars();


//input
void ReadInputFile();
int ProcessArgs(int count, char const *args[]);
void PrintParameters();
void UseDefaults();


//movement-related
void MoveAllCars();
void Move(float *,float *,float *);
float SafeDistance(vehicle *car);

//helpers
float RandBetween(int low, int high);

//variable declarations

//io-related
extern char inputFileName[100];
FILE *in;
extern char logFileName[100];
extern FILE *logFile;

extern int maxID;
extern int logEvents;

//simulation parameters
float roadLength;
float dt;
int nLoops;
int nSteps;
float meanVelocity;
float devVelocity;
float pNewCar;

//cars list
List *cars;

#endif