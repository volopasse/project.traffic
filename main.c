#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#define MAXCARS 1000
#define NSTEPS 10
#define CARLENGTH 4.0
#define NBINS 20

struct car
{
	int id;
	float pos;
	float vel;
	float vel_d;
	float acc;
	float dcc;
	float sd;
} car_act[MAXCARS];

int cars[MAXCARS];
int empty[MAXCARS];

int number_of_cars;
int count_ID;


char Inputfile[100] = "Input.dat";
FILE *in;

FILE *out;

char Logfile[100] = "Log.dat";
FILE *logfile;

// Simulation parameters

double t_step;
double road_length;
int n_loops;
double mean_velocity;
double velocity_deviation;

char output_type;


// Functions

void CarAdd(void);
void CarMove(int);
void CarPrint(int);
void CarSort();
void DensityHistogram(int *);
void Initialize(void);
void Integrate(int, char);

float Energy(void);
float EnergyMin(void);
float KinematicsPos(float, float, float);
float KinematicsVel(float, float);
float VelocityDeviation(void);

int CarCompare(const void *,const void *);
int CarRemove(int);
int NeighborFront(int);

// ******************************************************************************************************************************************************************************************************************
// *					Main
// ******************************************************************************************************************************************************************************************************************


main()
{
	Initialize();
	Integrate(n_loops, output_type);

	fclose(logfile);
}

// ******************************************************************************************************************************************************************************************************************
// * 					Functions
// ******************************************************************************************************************************************************************************************************************


void Initialize(void)
{
	printf("Initializing...\n\n");

	number_of_cars = 0;
	count_ID = 0;
	int i;

	for(i = 0; i < MAXCARS; i++)
	{
		empty[i] = -1;
		cars[i] = -1;
	}	

	empty[0] = 0;

	// Initialising the logfile

	logfile = fopen(Logfile, "a");

	time_t current;
	time(&current);

	fprintf(logfile, "*******************************************************************\n");
	fprintf(logfile, "\tSimulation started on %s\n", ctime(&current));
	fprintf(logfile, "*******************************************************************\n\n\n");

	// Reading the Input file
	// It's handy to declare a buffer string for all the mess

	in = fopen(Inputfile, "r");
	char *line;
	line = calloc(100,sizeof(char));

    printf("**************************************************************\n\n");

	printf("Reading data from %s\n\n", Inputfile);

	fgets(line,99,in);
    fscanf(in, "%s\t%lf\n", line,	&road_length);
    fscanf(in, "%s\t%lf\n", line,	&t_step);
    fscanf(in, "%s\t%lf\n", line,	&mean_velocity);
    fscanf(in, "%s\t%lf\n", line,	&velocity_deviation);
    fscanf(in, "%s\t%s\n", line,	&output_type);
    fscanf(in, "%s\t%d\n", line,	&n_loops);

    fclose(in);
    free(line);

    printf("Road length:\t%4.1f\n", road_length);
    printf("Time step:\t%4.1f\n", t_step);
    printf("Loops:\t%4i\n", n_loops);
    printf("Mean velocity:\t%4.1f\n", mean_velocity);
    printf("Velocity deviation:\t%4.1f\n", velocity_deviation);
    printf("Output type:\t%c\n\n", output_type);

    switch(output_type){
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



    printf("**************************************************************\n\n");

    printf("END INIT!\n\n");
}

void Integrate(int loops, char print)
{
	int i,n,j,count,tmp[MAXCARS],histogram[NBINS];
	int *hist_p = &histogram[0];

	DensityHistogram(hist_p);

	for(i = 0; i < NSTEPS*loops; i++){
				
		count = 0;

		for(n = 0; n < number_of_cars; n++){
			if(car_act[cars[n]].pos >= road_length){
				count += 1;
				tmp[count] = n;
				}
		}

		if(count > 0)
			for(j = 1; j <= count; j++)
				CarRemove(tmp[j]);

		if((i%(4*NSTEPS)) == 0)
			CarAdd();

		CarSort();
		
		if(i%NSTEPS == 0){
			switch(print){
			case 'd':
				DensityHistogram(hist_p);
				for(n = 0; n < NBINS; n++){
					fprintf(out,"%i\t%i\n", n, histogram[n]);
					histogram[n] = 0;
				}
				fprintf(out,"\n");
				break;

			case 'v':
				fprintf(out,"Step %i Velocity deviation = %f\n", i/NSTEPS, VelocityDeviation());
				break;

			case 'p':
				fprintf(out,"Step %i\n", i/NSTEPS);
				break;
			case 'e':
				fprintf(out,"Step %i Energy excess = \t%5.2f\n", i/NSTEPS, Energy());
				break;
			}		
		}

		for(n = 0; n < number_of_cars; n++){
			if(i%NSTEPS == 0 && print == 'p')
				CarPrint(n);
			CarMove(n);
		}
		
		if(i%NSTEPS == 0 && print == 'p')
			fprintf(out,"\n");
	}
}

void DensityHistogram(int *bins)
{
	int i;
	float step = road_length / NBINS;
	int pos;

	for(i = 0; i < number_of_cars; i++){
		pos = (int) (car_act[i].pos / step);
		(*(bins+pos))++;
	}
}

float VelocityDeviation(void)
{
	int i;
	float tmp_v = 0;

	for(i = 0; i < number_of_cars; i++)
		tmp_v += (car_act[i].vel - car_act[i].vel_d)*(car_act[i].vel - car_act[i].vel_d);

	return tmp_v/number_of_cars;
}

void CarPrint(int index)
{	
	int i = cars[index];
	fprintf(out,"%3i\t%4.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n", car_act[i].id, car_act[i].pos, car_act[i].vel, car_act[i].vel_d, car_act[i].acc, car_act[i].dcc, car_act[i].sd);
}

void CarMove(int index)
{
	int i = cars[index];
	
	int neighbor = NeighborFront(index);

	int n = cars[neighbor];

	if(car_act[n].pos - car_act[i].pos < ( CARLENGTH*car_act[i].vel/car_act[i].sd ) && car_act[n].pos - car_act[i].pos > 0.0){
		car_act[i].pos = KinematicsPos(car_act[i].pos, car_act[i].vel, car_act[i].dcc);
		car_act[i].vel = KinematicsVel(car_act[i].vel, car_act[i].dcc);
	}
	else{
		if(car_act[i].vel < car_act[i].vel_d){
			car_act[i].pos = KinematicsPos(car_act[i].pos, car_act[i].vel, car_act[i].acc);
			car_act[i].vel = KinematicsVel(car_act[i].vel, car_act[i].acc);
		}
		else if(car_act[i].vel = car_act[i].vel_d){
			car_act[i].pos = KinematicsPos(car_act[i].pos, car_act[i].vel, 0.0);
			car_act[i].vel = KinematicsVel(car_act[i].vel, 0.0);
		}
		else if(car_act[i].vel > car_act[i].vel_d){
			car_act[i].pos = KinematicsPos(car_act[i].pos, car_act[i].vel, car_act[i].dcc);
			car_act[i].vel = KinematicsVel(car_act[i].vel, car_act[i].dcc);
		}
	}
}

float KinematicsPos(float pos, float v, float a)
{
	pos = pos + v*t_step + 0.5*a*t_step*t_step;
	return pos;
}

float KinematicsVel(float v, float a)
{
	v = ((v + a*t_step) > 0) ? (v + a*t_step) : 0;
	return v;
}

int NeighborFront(int id)
{
	struct car sub = car_act[cars[id]];
	float min = road_length;
	int min_id,n;
	float distance;

	min_id = 0;

	for(n = 0; n < number_of_cars; n++){
		struct car ob = car_act[cars[n]];
		if(n != id && (distance = ob.pos - sub.pos) > 0 && distance < min){
			min = distance;
			min_id = n;
		}
	}

	return min_id;
}

void CarAdd(void)
{
	int i;
	int vacancy = empty[1];


	if(vacancy < 0){
		vacancy = number_of_cars;
	}
	else
	{
		for(i = 1; empty[i] >= 0; i++)
			empty[i] = empty[i+1];
	}

	car_act[vacancy].id = count_ID;
	car_act[vacancy].pos = 0.0;
	car_act[vacancy].vel = (rand()%((int)(velocity_deviation*2.0)) + (mean_velocity - velocity_deviation));
	car_act[vacancy].vel_d = (rand()%(int)(velocity_deviation*2.0) + (mean_velocity - velocity_deviation));
	car_act[vacancy].acc = 4.0;
	car_act[vacancy].dcc = -8.0;
	car_act[vacancy].sd = 2.7;

	cars[number_of_cars] = vacancy;

	number_of_cars++;
	count_ID++;
}

int CarRemove(int index)
{
	int i = 1;
	int tmp;
	
	while(empty[i-1] >= 0){
		tmp = i;
		i++;
	}
	
	empty[tmp] = cars[index];

	cars[index] = -1;

	CarSort();

	number_of_cars--;
}

int CarCompare(const void *a,const void *b)
{
	return ( *(int*)b - *(int*)a );
}

void CarSort()
{
	size_t cars_len=sizeof(cars)/sizeof(int);
	qsort(cars,cars_len,sizeof(int),CarCompare);
}

float Energy()
{
	int i,j;
	float e = .0;


	if (number_of_cars <= 1){
			e = 1.0;
			return e;
	}
	else{
		for (i = 0; i < number_of_cars; i++)
			for (j = i; j < number_of_cars; j++){
				if (i != j)
					e += ( 1.0 / ( (car_act[cars[i]].pos - car_act[cars[j]].pos) * (car_act[cars[i]].pos - car_act[cars[j]].pos) ));
			}
	return ( ( (e/EnergyMin()) - 1));
	// return e;
	}
}

float EnergyMin()
{
	int i,j;
	float e = .0;

	if (number_of_cars <= 1){
			e = 1.0;
			return e;
	}
	else{
		double slice = road_length / (number_of_cars - 1);

		for (i = 0; i < number_of_cars; i++)
			for (j = i; j < number_of_cars; j++){
				if (i != j)
					e += ( 1 / ( (slice*(i-j))*(slice*(i-j) ) ) );
			}
	return e;
	}
}



















