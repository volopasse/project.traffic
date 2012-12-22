#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "linkedlist.h"

//temp declaration for max no of cars. 
//later will be dynamic
#define SAFE_DIST_RATIO	2.7
#define CAR_LENGTH		5
#define N_BINS			20

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

//flow statistics
float AvgSumOfSquares();

//output-related
void ReadInputFile();
void UseDefaults();
void Output(int);
void PrintCar(vehicle *car);
void PrintAllCars();

//movement-related
void MoveAllCars();
void Move(float *,float *,float *);

//road statistics
void DensityHist(int nBins, int *hist);
float EnergyRatio();
float EnergyAct();
float EnergyMin();

//helpers
float RandBetween(int low, int high);

//variable declarations
int maxID=-1;

//io-related
char inputFileName[100]="Input.dat";
FILE *in;
FILE *out;
char logFileName[100]="log.dat";
FILE *logFile;
char outSwitch;
int logEvents=1;

int outHist;
int outCars;
int outEnergy;
int outSS;

//simulation parameters
float roadLength;
float dt;
int nLoops;
int nSteps;
float meanVelocity;
float devVelocity;
float pNewCar;

int *hist;

List *cars;


#endif