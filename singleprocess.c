#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int L;
int H;
int PN;

int findMax(int arr[], int size) {
    int max = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

double findAvg(int arr[], int size) {
    long long sum = 0;
    double avg = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    avg = (double) sum / (double) size;

    return avg;
}

int findKeys(int arr[], int size, int keysFound) {
	//print the location of keys and return the number of keys found
		for (int i = 0; i < size; i++)
		{
		        if (arr[i] == -1 && keysFound < H) {
                keysFound+=1;
				printf("Hi I'm process %d and I found the hidden key %d in position A[%d] \n", getpid(), keysFound, i);
        		}
		}
    printf("\n");
    return keysFound;
}

int randomNumber(int min, int max) 
{
	return (rand() % (max - min + 1)) + min;
}

int fileGenerator()
{
	FILE *fp; 
	fp = fopen ("number_List.txt", "w"); //create a file named fp

	int keysToPlace = 50; //number of keys that will be distributed through the list
	int rMax = 10; //maximum number to randomly generate
	int rMin = 0; //minimum number to randomly generate
	int fileEntry;
	if (L<keysToPlace)
	{
		printf("L < keysToPlace\n");
		printf("list size must be greater than number of keys to place\n");
		printf("L = %d, keysToPlace = %d\n", L, keysToPlace);
		printf("exiting\n");
		exit(0);
	}

    srand(time(NULL));

    int *randNumbers = malloc(L * sizeof(int));

	//generate initial random number array
	for (int i = 0; i < L; i++) {
        randNumbers[i] = randomNumber(rMin, rMax);
    }

    //populate the random array with keys
    while (keysToPlace > 0) {
        int placeKey = randomNumber(0, L - 1);
        if (randNumbers[placeKey] != -1) {
            randNumbers[placeKey] = -1;
            keysToPlace--;
        }
    }

    //print values to file
    for (int i = 0; i < L; i++) {
        fprintf(fp, "%d\n", randNumbers[i]); 
    }

    //fprintf(fp, "%s %d\n", "Length of array is: ", L); 

    fclose(fp);
return 0;
}

// structure that holds average and maximum, returned in the dfs
struct info {
    double average;
    int maximum;
    int keysFound;
};

int main( int argc, char *argv[] )  {
		
	if (argc != 4)	
	{
		printf("Expected 3 arguments (L,H,PN)\n exiting\n");		
		exit(0);
	}
	char *a1 = argv[1];
	char *a2 = argv[2];
	char *a3 = argv[3];

	L = atoi(a1);
	H = atoi(a2);
	PN = atoi(a3);

	//printf( "\nL: %d \nH: %d \nPN: %d \n", L, H, PN);
	
	fileGenerator();

	int randNumbers[L];
	int i=0;
	int num;

	FILE *fp;
	fp = fopen("number_List.txt", "r");

	while(fscanf(fp, "%d", &num) > 0) {
		randNumbers[i] = num;
		i++;		
	}	
	
	fclose(fp);

	 
    struct info data = {0,0,0};

    data.average = findAvg(randNumbers, L);
    data.maximum = findMax(randNumbers, L);
    data.keysFound = findKeys(randNumbers, L, 0);

	fp = fopen("results.txt", "w");
	fprintf(fp, "%s %f\n", "Average is: ", data.average);
	fprintf(fp, "%s %d\n", "Maximum is: ", data.maximum);
    fprintf(fp, "%s %d\n", "Keys found is: ", data.keysFound);

	fclose(fp);  

	return 0;
}