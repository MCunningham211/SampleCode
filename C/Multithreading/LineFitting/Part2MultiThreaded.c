#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <float.h>

double* yValues;
int numEntries = 0;

struct Line {
	double slope;
	double intersept;
};

sem_t accuracyProtector;
struct Line* BestFit;
double accuracy = DBL_MAX;

struct Line *findLine(double x1, double y1, double x2, double y2){
	struct Line* thisline = malloc(sizeof(struct Line));
	if(x2 > x1){
		thisline -> slope = (y2 - y1)/(x2 - x1);
		thisline -> intersept = y1 - (x1*(thisline -> slope));
	}else{
		thisline -> slope = (y1 - y2)/(x1 - x2);
		thisline -> intersept = y1 - (x1*(thisline -> slope));
	}
	return thisline;
}

double difference(double slope, double intersept, double x, double y){
	int hold = ((slope * x) + intersept) - y;
	if(hold < 0)return -hold;
	return hold;
}

void *lineFit(void* i){
	int value = *((int *) i);
	free(i);
	struct Line* testLine;
	double fit;
	for(int i = value + 1; i < numEntries; i++){
		testLine = findLine((double)value, yValues[value], (double)i, yValues[i]);
		fit = 0;
		for(int j = 0; j < numEntries; j++)fit = fit + difference(testLine -> slope, testLine -> intersept, j, yValues[j]);
		//printf("SUM: %G  Slope: %G  Intersept: %G\n",fit,testLine -> slope, testLine -> intersept);
		if(fit < accuracy){
			sem_wait(&accuracyProtector);
			if(fit < accuracy){
				accuracy = fit;
				free(BestFit);
				BestFit = testLine;
			}else{
				free(testLine);
			}
			sem_post(&accuracyProtector);
		}else{
			free(testLine);
		}
	}
	pthread_exit(NULL);
}

void read_csv(int row, char *filename, double *data){ //This function was adapted from this code https://gist.github.com/amirmasoudabdol/f1efda29760b97f16e0e
	FILE *file;
	file = fopen(filename, "r");
	int i = 0;
	char line[128];
	char* tok;
	char* ptr;
	fgets(line, 128, file);
	while (fgets(line, 128, file) && (i < row))
    {
		tok = strtok(line, ",");
		tok = strtok(NULL, ",");
		data[i] = strtod(tok, &ptr);
		//printf("%G\n", data[i]);
		i++;
    }
}

int main(int argc, char *argv[])//https://stackoverflow.com/questions/19232957/pthread-create-and-passing-an-integer-as-the-last-argument
{			  
	char *hold = argv[1];
	int num = atoi(hold);
	hold = argv[2];
  	printf("Number of Values: %d\n",num);
	numEntries = num;
	sem_init(&accuracyProtector,0,1);
	yValues = (double *)malloc(numEntries * sizeof(double));
	read_csv(numEntries, hold, yValues);
	pthread_t entriesThread[numEntries - 1];
	for(int q = 0; q < numEntries - 1; q++){
		int *arg = malloc(sizeof(*arg));
        *arg = q;
		pthread_create(&(entriesThread[q]), NULL, lineFit, arg);
	}
	for(int q = 0; q < numEntries - 1; q++){
		pthread_join(entriesThread[q], NULL);
	}	
	
	printf("Accuracy: %G  Slope: %G  Intersept: %G\n",accuracy,BestFit -> slope, BestFit -> intersept);
	return 0;
}