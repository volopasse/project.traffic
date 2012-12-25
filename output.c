#include "output.h"

FILE *out;
FILE *logFile;
char logFileName[100]="log.dat";

int *hist;

FILE *OutputOpen(char outSwitch){
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

	return out;
}

void Output(int i){

		if (out==NULL) out=OutputOpen(outSwitch);

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


void LogOpen(){
	logFile=fopen(logFileName,"a");
}

void LogEvent(const char *logMsg){
	if(logFile==NULL) LogOpen();
	fprintf(logFile, "%s\n", logMsg);
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
