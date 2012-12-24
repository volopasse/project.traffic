#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "traffic.h"
#include "linkedlist.h"

extern FILE *out;
extern char outSwitch;

FILE *OutputOpen(char outSwitch);
void Output(int);

void PrintCar(vehicle *car);
void PrintAllCars();

float AvgSumOfSquares();
float EnergyRatio();
float EnergyAct();
float EnergyMin();
void DensityHist(int nBins, int *hist);


#endif