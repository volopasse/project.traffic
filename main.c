#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <time.h>

//temp declaration for max no of cars. 
//later will be dynamic
#define MAX_CARS	  	100
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
void IntegrateStep();
int Initialize();

void InitializeCars();

int SortByPosAsc();
int SortByPosDsc();
void SortCarsByPos();

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
void MoveCar(vehicle *car);
void Move(float *,float *,float *);

//neighbours
void BuildNeighbourList();
void CleanNeighbourList();
void FindNeighbours();
void FindLaneNeighbours(vehicle *car);

//road statistics
void DensityHist(int nBins, int *hist);
float EnergyRatio();
float EnergyAct();
float EnergyMin();

//helpers
float RandBetween(int low, int high);

//variable declarations
vehicle *cars[MAX_CARS]; //TODO make a dynamic array
int carSlotsActive=0;
int maxID=-1;
int nCars=0;

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

//end of declrarations

int main(int argc, char const *argv[])
{
	//process args

	// if(argc!=2){
	// 	printf("Invalid number or arguments. Function supports only one \n");
	// 	return 0;
	// } 
		
	// if (argv[1][0]=='-'){
	// 	for (int i = 1; argv[1][i] !='\0' ; ++i){
	// 		switch(argv[1][i]){
	// 			case 'd':
	// 				outHist=1;
	// 				break;
	// 			case 'e':
	// 				outEnergy=1;
	// 				break;
	// 			case 'p':
	// 				outCars=1;
	// 				break;
	// 			case 'v':
	// 				outSS=1;
	// 				break;
	// 			default:
	// 				printf("option -%c is invalid\n", argv[1][i]);
	// 		}
	// 	}
		
	// }


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

	return 0;

}

//for debug purposes
int IntegrateDebug(){
	for (int i = 0; i < 5; ++i)
	{
		vehicle *newcar=CreateCar(maxID+1);
		maxID++;
		AddCar(newcar);	
	}

	printf("Number of Cars = %i\n", nCars);
	printf("Car Slots Active = %i\n", carSlotsActive);

	free(cars[0]);
	cars[0]=NULL;
	cars[1]->s=20;
	cars[2]->s=10;
	cars[3]->s=5;


	for (int i = 0; i < carSlotsActive; ++i)
	{	
		if (cars[i]!=NULL)
			printf(
				"|%3i|%3.2f|%3.2f|%3.2f|%3.2f|%4.2f|\n", 
				cars[i]->ID, cars[i]->s, cars[i]->v, 
				cars[i]->a, cars[i]->d, cars[i]->vDesired);		
		else
			printf("Empty\n");
	}

	

	BuildNeighbourList();

	//steps sequence
	for (int j = 0; j < nSteps; ++j){
		printf("Step %i\n", j);
		MoveAllCars();

		for (int k = 0; k < carSlotsActive; ++k){	
				if (cars[k]!=NULL)
				printf(
					"|%3i|%3.2f|%3.2f|%3.2f|%3.2f|%4.2f|\n", 
					cars[k]->ID, cars[k]->s, cars[k]->v, 
					cars[k]->a, cars[k]->d, cars[k]->vDesired);		
				else
					printf("Empty\n");
			}
	}

	
	vehicle *newcar=CreateCar(maxID+1);
	maxID++;
	AddCar(newcar);	
	cars[3]->s=0.5;


	printf("\n\n");

	for (int i = 0; i < carSlotsActive; ++i)
	{	
		if (cars[i]!=NULL)
			printf(
				"|%3i|%3.2f|%3.2f|%3.2f|%3.2f|%4.2f|\n", 
				cars[i]->ID, cars[i]->s, cars[i]->v, 
				cars[i]->a, cars[i]->d, cars[i]->vDesired);		
		else
			printf("Empty\n");
	}


	BuildNeighbourList();

	//steps sequence
	for (int j = 0; j < nSteps; ++j){
		printf("Step %i\n", j);
		MoveAllCars();

		for (int k = 0; k < carSlotsActive; ++k){	
				if (cars[k]!=NULL)
				printf(
					"|%3i|%3.2f|%3.2f|%3.2f|%3.2f|%4.2f|\n", 
					cars[k]->ID, cars[k]->s, cars[k]->v, 
					cars[k]->a, cars[k]->d, cars[k]->vDesired);		
				else
					printf("Empty\n");
			}
	}



	return 0;

}



int Integrate(){
	//loops sequence
	printf("Running simulation...\n");
	
	for (int i = 0; i < nLoops; ++i){


		CleanNeighbourList();
		BuildNeighbourList();

	
		//steps sequence
		for (int j = 0; j < nSteps; ++j){
			MoveAllCars();
		}

		if (RandBetween(0,1)<pNewCar && carSlotsActive<MAX_CARS){
			vehicle *newcar=CreateCar(maxID+1);
			maxID++;
			AddCar(newcar);	

			fprintf(logFile, "Car %i added on loop %i\n", newcar->ID, i);
		}
		
		Output(i);

		CleanUpCars();
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
	int i=0;
	do{
		if (cars[i]==NULL){
			cars[i]=car;
			return;
		}
		i++;
	} while (i < carSlotsActive); //TODO <= or < ?? may introduce a bug
	cars[carSlotsActive]=car;
	carSlotsActive++;
	nCars++;

	/* 	HACK rerun neighbour list. Otherwise if car was recently dropped and 
		replaced by another, the neighbour logic breaks */
	BuildNeighbourList();
}	

//cleans up the empty slots in the middle of the array
void CleanUpCars(){
	for (int i = 0; i < carSlotsActive; ++i){
		//trim and rerun
		if (cars[carSlotsActive-1]==NULL){
			carSlotsActive--;
			i--;
			continue;
		}
		//swap and continue
		else if (cars[i]==NULL){
			cars[i]=cars[carSlotsActive-1];
			cars[carSlotsActive]=NULL;
			carSlotsActive--;
		}
	}
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
	for (int i = 0; i < carSlotsActive; ++i)
	{	
		if (cars[i]!=NULL)
			PrintCar(cars[i]);
		else
			fprintf(out,"Empty\n");
	}
}

void BuildNeighbourList(){
	for (int i = 0; i < carSlotsActive; ++i){
		if (cars[i]!=NULL) FindLaneNeighbours(cars[i]);
	}
}

void CleanNeighbourList(){
	for (int i = 0; i < carSlotsActive; ++i){
		for (int j = 0; j < 3; ++j){
			for (int k = 0; k < 3; ++k){
				cars[i]->neighbours[j][k]=NULL;
			}
		}
	}
}

void FindNeighbours(){

}

void FindLaneNeighbours(vehicle *car){
	float sDelta=roadLength;	//HACK starting deltas set to max value possible
	float sFwdDelta=roadLength;
	float sBckDelta=roadLength;
	assert(car!=NULL);

	for (int i = 0; i < carSlotsActive; ++i){
		if (cars[i]==NULL) continue; //skip if the slot is empty
		if (cars[i]==car) continue;  //skip if looking at self

		sDelta=cars[i]->s - car->s;
		if (sDelta>0){
			if (sDelta<sFwdDelta){
				sFwdDelta=sDelta;
				car->neighbours[0][0]=cars[i];
			}
		} 
		else if (sDelta<0){
			if (abs(sDelta)<sBckDelta){
				sBckDelta=abs(sDelta);
				car->neighbours[0][2]=cars[i];
			}
		}
	}
}


//Movement functions

void MoveAllCars(){
	for (int i = 0; i < carSlotsActive; ++i){
		if (cars[i]!=NULL){ 
			MoveCar(cars[i]);
			if ((*cars[i]).s>roadLength) {
				DestroyCar(cars[i]);
				cars[i]=NULL;
				nCars--;
			}
		}
	}
}

void MoveCar(vehicle *car){
	float aActual;
	float vDelta;
	vehicle *frontCar; //assigned for better readability

	assert(car!=NULL);
	frontCar=car->neighbours[0][0];

	//separate checks to avoid memory error
	if (frontCar==NULL)
	{
		vDelta=car->vDesired-car->v;
		if (vDelta>0) 
			aActual=car->a;
		else if (vDelta<0) 
			aActual=car->d;
		else aActual=0;
	}
	else if (frontCar->s - car->s > car->v/SAFE_DIST_RATIO)
	{
		vDelta=(*car).vDesired-(*car).v;
		if (vDelta>0) 
			aActual=(*car).a;
		else if (vDelta<0) 
			aActual=(*car).d;
		else aActual=0;
	}
	else{
		aActual=(*car).d;
	}

	Move(&(*car).s, &(*car).v, &aActual);

	//accident check
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

	for (int i = 0; i < carSlotsActive; ++i){	
		if (cars[i]!=NULL){
			bin=(int)(cars[i]->s / binSize);
			hist[bin]++;
		}
	}
}


float AvgSumOfSquares(){
	float sumSquares=0;
	for (int i = 0; i < carSlotsActive; ++i){	
		if (cars[i]!=NULL){
			sumSquares+=pow(cars[i]->vDesired - cars[i]->v, 2);
		}
	}
	return nCars>0 ? sumSquares/nCars : 0;
}

float EnergyRatio(){
	return (float)(EnergyAct()/EnergyMin()-1);
}


float EnergyAct(){
	float e =0;
	if (nCars>1){
		for (int  i = 0; i < carSlotsActive; ++i){
			for (int j = i+1; j < carSlotsActive; ++j)	{
				if (cars[i]!=NULL && cars[j]!=NULL){
					e += 1 / pow(cars[i]->s - cars[j]->s,2) ;
				}
			}
		}
	}
	else{
		e=1;
	}

	return e;
}


float EnergyMin(){
	float e =0;

	if (nCars>1){
		float slice=roadLength/(nCars-1);
		for (int  i = 0; i < nCars; ++i){
			for (int j = i+1; j < nCars; ++j)	{
				e += 1 / pow(slice*(i-j),2) ;
			}
		}
	}
	else{
		e=1;
	}

	return e;
}



int SortByPosAsc(const void *a,const void *b){
	vehicle **pa=(vehicle **)a;
	vehicle **pb=(vehicle **)b;
	return (int)((*pa)->s - (*pb)->s);
}

int SortByPosDsc(const void *a,const void *b){
	vehicle **pa=(vehicle **)a;
	vehicle **pb=(vehicle **)b;
	return (int)((*pb)->s - (*pa)->s);
}

void SortCarsByPos(){
	//printf("Sorting Cars by Position...\n");
	qsort(cars,carSlotsActive,sizeof(vehicle*),SortByPosDsc);
}

