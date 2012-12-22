#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "linkedlist.h"
#include "traffic.h"

int main(int argc, char const *argv[])
{

	Initialize();
	Integrate();

	fclose(logFile);
	
	return 0;
}

int Initialize(){
	//default parameters



	char ex='n';
	
	ReadInputFile();
	
	printf("Road length:\t%4.1f\n", roadLength);
    printf("Time step:\t%4.1f\n", dt);
    printf("Steps:\t%4i\n", nSteps);
    printf("Loops:\t%4i\n", nLoops);
    printf("Mean velocity:\t%4.1f\n", meanVelocity);
    printf("Velocity deviation:\t%4.1f\n", devVelocity);
    printf("Output type:\t%c\n", outSwitch);
    printf("New Car Probability:\t %.1f%%\n", pNewCar*100);
	
	printf("Do you wish to run the sumilation with these parameters? (y/n):");
	scanf("%c", &ex);

	if(ex=='n'){
		printf("The simulation will now exit. Please restart after editing parameters.\n");
		exit(0);
	}

	switch(outSwitch){
    	case 'd':
    		out = fopen("histogram.out", "w");
    		break;
    	case 'v':
    		out = fopen("velocity_dev.out", "w");

    		break;
    	case 'e':
    		out = fopen("energy_excess.out", "w");
    		break;
    	case 'p':
    		out = fopen("cars.out", "w");
    		break;
    }

    logFile=fopen(logFileName,"a");
    time_t now;
    time(&now);

    fprintf(logFile, "Simulation started at %s\n", ctime(&now));


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
	listPushFront(cars, car);
}	


//output function
void Output(int i){

		switch(outSwitch){
		case 'd':
			hist=calloc(N_BINS,sizeof(int));		
			DensityHist(N_BINS, hist);
			for (int i = 0; i < N_BINS; ++i){
				fprintf(out, "%i\t%i\n",i,hist[i]);
			}
			fprintf(out, "\n");
			free(hist);
			break;
		case 'v':	
			fprintf(out, "Loop %i\tAverage SS\t%5.2f\n", i, AvgSumOfSquares());
			break;
		case 'p':
			fprintf(out, "Loop %i\n", i);
			PrintAllCars();
			break;
		case 'e':	
			fprintf(out, "Loop %i\tEnergy Excess\t%5.2f\n", i, EnergyRatio());
			break;
	}
}


void PrintCar(vehicle *car){
	assert(car!=NULL);
	fprintf(out,
			"|%3i|%3.2f|%3.2f|%3.2f|%3.2f|%4.2f|\n", 
			car->ID, car->s, car->v, 
			car->a, car->d, car->vDesired);
}

void PrintAllCars(){

	ListNode *_node=NULL;
	ListNode *C=NULL;
	vehicle *temp=NULL;
	for(C=_node = cars->first; _node!=NULL; C=_node=_node->next){
		temp=_node->value;
		PrintCar(temp);

	}

}


//Movement functions

void MoveAllCars(){
	float vDelta;
	float aActual;
	vehicle *car=NULL;
	vehicle *frontCar=NULL;

	LIST_FOREACH(cars, first, next, cur){
		car=cur->value;

		if (cur->next!=NULL) {
			frontCar=cur->next->value;
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
		} else if (frontCar->s - car->s > car->v/SAFE_DIST_RATIO) {
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

void CleanUpCars(){
	vehicle *temp;
	LIST_FOREACH(cars, first, next, cur){
		temp=cur->value;
		if (temp->s > roadLength) {
			listPopEnd(cars);
		}
	}
}


//helper functions

float RandBetween(int low, int high){
	return (float)rand()/(float)RAND_MAX * (high-low)+low;
}


void ReadInputFile(){
	char tempLine[100];

	printf("Loading parameters...\n");

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

void UseDefaults(){
	roadLength=100;
	dt=0.1;
	nSteps=10;
	nLoops=100;
	meanVelocity=3;
	devVelocity=1;
	pNewCar=0.5;
	outSwitch='p';
}

//road statistics functions


void DensityHist(int nBins, int *hist){
	float binSize=roadLength/nBins;
	int bin;
	vehicle *temp;

	LIST_FOREACH(cars, first, next, cur){
		temp=cur->value;
		bin=(int)(temp->s / binSize);
		hist[bin]++;
		
	}
}


float AvgSumOfSquares(){
	float sumSquares=0;
	vehicle *temp;
	LIST_FOREACH(cars, first, next, cur){
		temp=cur->value;
		sumSquares+=pow(temp->vDesired - temp->v, 2);
	}
	return cars->count>0 ? sumSquares/cars->count : 0;
}

float EnergyRatio(){
	return (float)(EnergyAct()/EnergyMin()-1);
}


float EnergyAct(){
	float e =0;
	ListNode *cur_i=NULL;
	ListNode *cur_j=NULL;

	vehicle *temp_i;
	vehicle *temp_j;

	for (cur_i = cars->first; cur_j!=NULL; cur_i=cur_i->next) {
		temp_i=cur_i->value;
		for (cur_j = cur_i->next; cur_j!=NULL; cur_j=cur_j->next) {
				temp_j=cur_j->value;
				e += 1 / pow(temp_i->s - temp_j->s,2) ;
		}
	}

	return cars->count>1 ? e : 1;
}


float EnergyMin(){
	float e =0;

	float slice=roadLength/(cars->count - 1);

	if (cars->count>1) {
		for (int  i = 0; i < cars->count; ++i) {
			for (int j = i+1; j < cars->count; ++j)	{
				e += 1 / pow(slice*(i-j),2) ;
			}
		}
	}
	
	return cars->count>1 ? e : 1;
}
