#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "linkedlist.h"
#include "traffic.h"
#include "output.h"

char outSwitch;
int maxID=-1;
char inputFileName[100]="Input.dat";


int main(int argc, char const *argv[])
{

	ProcessArgs(argc, argv);

	Initialize();
	Integrate();

	return 0;
}

int ProcessArgs(int count, char const *args[]) {
	for (int i = 1; i < count; ++i)
	{
		if (strcmp(args[i],"-i")==0) {
			strcpy(inputFileName,args[i+1]);
			i++;
			continue;
		} 
	}

	return 0;
}


int Initialize() {
	//default parameters

	char ex='n';
	
	ReadInputFile();
	PrintParameters();
	
	printf("Do you wish to run the sumilation with these parameters? (y/n):");
	scanf("%c", &ex);

	if(ex=='n'){
		printf("The simulation will now exit. Please restart after editing parameters.\n");
		exit(0);
	}

    time_t now;
    time(&now);

    #define BUF 1024
    char logMsg[BUF];
    snprintf(logMsg, BUF, "Simulation started at %s\n", ctime(&now));
    LogEvent(logMsg);
    
    cars=listCreate();

	return 0;
}


int Integrate(){
	//loops sequence
	printf("Running simulation...\n");
	
	for (int i = 0; i < nLoops; ++i){

		//steps sequence
		for (int j = 0; j < nSteps; ++j){
			MoveAllCars();
			CleanUpCars();
		}

		if (RandBetween(0,1)<pNewCar){
			vehicle *newcar=CreateCar(maxID+1);
			maxID++;
			AddCar(newcar);	

			// fprintf(logFile, "Car %i added on loop %i\n", newcar->ID, i);
		}
		
		Output(i);
	}

	return 0;
}


vehicle *CreateCar(int ID){
	vehicle *car=malloc(sizeof(vehicle));
	assert(car!=NULL);

	(*car).ID=ID;
	(*car).s=0;
	(*car).v=RandBetween(meanVelocity - devVelocity,
						meanVelocity + devVelocity);
	(*car).a=RandBetween(4,4);
	(*car).d=-RandBetween(8,8);
	(*car).vDesired=RandBetween(meanVelocity - devVelocity,
								meanVelocity + devVelocity);

	return car;
}

void DestroyCar(vehicle *car){
	assert(car!=NULL);
	free(car);
}

void AddCar(vehicle *car){
	assert(car!=NULL);
	listPushEnd(cars, car);
}	

//Movement functions

void MoveAllCars(){
	float vDelta;
	float aActual;
	vehicle *car=NULL;
	vehicle *frontCar=NULL;

	LIST_FOREACH(cars, first, next, cur){
		car=cur->value;

		if (cur->prev!=NULL) {
			frontCar=cur->prev->value;
		} else {
			frontCar=NULL;
		}
		
		if (frontCar==NULL) {
			vDelta=car->vDesired - car->v;
			if (vDelta>0) 
				aActual=car->a;
			else if (vDelta<0) 
				aActual=car->d;
			else aActual=0;
		} else if (frontCar->s - car->s > SafeDistance(car)) {
			vDelta = car->vDesired - car->v;
			if (vDelta>0) 
				aActual=car->a;
			else if (vDelta<0) 
				aActual=car->d;
			else aActual=0;
		} else {
			aActual=car->d;
		}
		
		Move(&(car->s), &(car->v), &aActual);
		
	}


	// accident check
	if (frontCar!=NULL){
		if (car->s >= frontCar->s){
			car->s=frontCar->s;
			car->v=0; frontCar->v=0;
			car->a=0; frontCar->a=0;
		}
	}

}

void Move(float *ps,float *pv,float *pa){
	//cars not allowed to move backwards
	*ps=*ps + fmax(*pv * dt + 0.5 * (*pa) * pow(dt,2), 0);
	*pv=fmax(*pv + *pa * dt ,0);
}

float SafeDistance(vehicle *car){
	return pow(car->v,2)/(2*SAFE_DIST_CONST) + car->v * dt;
}

void CleanUpCars(){
	vehicle *temp;
	LIST_FOREACH(cars, first, next, cur){
		temp=cur->value;
		if (temp->s > roadLength) {
			listPopFront(cars);
		}
	}
}

//helper functions

float RandBetween(int low, int high){
	return (float)rand()/(float)RAND_MAX * (high-low)+low;
}


void ReadInputFile(){
	char tempLine[100];

	printf("Loading parameters from %s ...\n", inputFileName);

	in = fopen(inputFileName, "r");
	if (!in)
	{
		printf("%s not found.\nLoading default paramaters...\n", inputFileName);
		UseDefaults();
		return;
	}
	assert(in!=NULL);

	fgets(tempLine,99,in);
	fscanf(in, "%*s%f\n", &roadLength);
	fscanf(in, "%*s%f\n", &pNewCar);
	fscanf(in, "%*s%f\n", &dt);
	fscanf(in, "%*s%f\n", &meanVelocity);
	fscanf(in, "%*s%f\n", &devVelocity);
	fscanf(in, "%*s%i\n", &nLoops);
	fscanf(in, "%*s%i\n", &nSteps);
	fscanf(in, "%*s\t%c\n", &outSwitch);

	fclose(in);
}

void PrintParameters() {
	printf("Road length:\t%4.1f\n", roadLength);
    printf("Time step:\t%4.1f\n", dt);
    printf("Steps:\t%4i\n", nSteps);
    printf("Loops:\t%4i\n", nLoops);
    printf("Mean velocity:\t%4.1f\n", meanVelocity);
    printf("Velocity deviation:\t%4.1f\n", devVelocity);
    printf("Output type:\t%c\n", outSwitch);
    printf("New Car Probability:\t %.1f%%\n", pNewCar*100);
}

void UseDefaults() {
	roadLength=100;
	dt=0.1;
	nSteps=10;
	nLoops=100;
	meanVelocity=3;
	devVelocity=1;
	pNewCar=0.5;
	outSwitch='p';
}

