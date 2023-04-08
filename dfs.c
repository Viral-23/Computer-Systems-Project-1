#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int L;
int H;
int PN;

int* left(int *arr, int size) {
    int mid = size / 2;
    int *left = (int*) malloc(mid * sizeof(int));

    for(int i = 0; i < mid; i++) {
        left[i] = arr[i];
    }

    return left;
}

int* right(int *arr, int size) {
    int mid = size / 2;
    int *right = (int*) malloc((size - mid) * sizeof(int));

    for(int i = mid; i < size; i++) {
        right[i - mid] = arr[i];
    }

    return right;
}

int max2(int num1, int num2) {
    if (num1 > num2) {
        return num1;
    } else {
        return num2;
    }
}


int findMax(int arr[], int size) {
    int maximum = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > maximum) {
            maximum = arr[i];
        }
    }
    return maximum;
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

int findKeys(int arr[], int size, int keysFound, int shift) {
	//print the location of keys and return the number of keys found
		for (int i = 0; i < size; i++)
		{
		        if (arr[i] == -1 && keysFound < H) {
                keysFound+=1;
				printf("Hi I'm process %d and I found the hidden key %d in position A[%d] \n", getpid(), keysFound, i + shift);
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

	int keysToPlace = 10; //number of keys that will be distributed through the list
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

// dfs method, keeps spawning children if PN not reached or array cannot be split anymore
struct info dfs(int PN, int L, int *randNumbers, struct info ret, int shift) {
    int prevShift = shift;
    int firstPN = PN;
    double average = ret.average;
    int maximum = ret.maximum;
    int keysFound = ret.keysFound;
    int fd[2][2];

    pipe(fd[0]);
    pipe(fd[1]);

    pid_t pid = fork();
   
    if (pid == 0) {
        printf("Hi I'm process %d with return arg %d and my parent is %d.\n", getpid(), PN, getppid());
        close(fd[0][1]);
        close(fd[1][0]);
        int arrLeftSize = L/2;
        int arrRightSize = L - arrLeftSize;
        int *arrRight = (int *)malloc(sizeof(int) * arrRightSize);
        read(fd[0][0], arrRight, sizeof(int) * arrRightSize);

        printf("pid: %d arrRight: \n", getpid());
        for (int i = 0; i < arrRightSize; i++) {
            printf("value: %d index: %d\n", arrRight[i] , i);
        }
        printf("\n");

        if (PN == 2) {
            ret.average = findAvg(arrRight, arrRightSize);
            ret.maximum = findMax(arrRight, arrRightSize);
            ret.keysFound = findKeys(arrRight, arrRightSize, ret.keysFound, shift + arrLeftSize);
        }
        
        if (PN > 2 && arrRightSize > 1) {
            ret = dfs(PN - 1, arrRightSize, &arrRight[0], ret, shift + arrLeftSize);
        }
    
        
        write(fd[1][1], &average, sizeof(double));
        write(fd[1][1], &maximum, sizeof(int));
        write(fd[1][1], &keysFound, sizeof(int));



        free(arrRight);

        exit(0);

    }
    else {
        close(fd[0][0]);
        close(fd[1][1]);
        int *arrRight = right(randNumbers, L);
        write(fd[0][1], arrRight, sizeof(int) * (L - L/2));

        wait(NULL);

        double childAverage;
        int childMax;
        int childKeysFound;
        read(fd[1][0], &childAverage, sizeof(double));
        read(fd[1][0], &childMax, sizeof(int));
        read(fd[1][0], &childKeysFound, sizeof(int));

        int *arrLeft = left(randNumbers, L);

        printf("pid: %d arrLeft: \n", getpid());
        for (int i = 0; i < L/2; i++) {
            printf("value: %d index: %d\n", arrLeft[i] , i);
        }
        printf("\n");

        double parentAverage = findAvg(arrLeft, L/2);
        int parentMax = findMax(arrLeft, L/2);
        int parentKeysFound = 0;
        // if (PN = firstPN)
        //     parentKeysFound = findKeys(arrLeft, L/2, childKeysFound, 0);
        // else
        parentKeysFound = findKeys(arrLeft, L/2, childKeysFound, prevShift);

        ret.average = (parentAverage * (L/2) + childAverage * (L - L/2))/L;
        ret.maximum = max2(parentMax, childMax);
        ret.keysFound = parentKeysFound;
        
        free(arrLeft);
        free(arrRight);

    }
    return ret;
}

int main( int argc, char *argv[] )  {
	
	int keysFound = 0;
	
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

	struct info data = {0.0 , 0 , 0};
    data = dfs(PN, L, randNumbers, data, 0);
    //free(randNumbers);

    //printf("%s %f\n", "Average is: ", data.average); 
    //printf("%s %d\n", "Maximum is: ", data.maximum); 


    // data.average = findAvg(randNumbers, L);
    // data.maximum = findMax(randNumbers, L);
    // data.keysFound = findKeys(randNumbers, L, 0);

	fp = fopen("results.txt", "w");
	fprintf(fp, "%s %f\n", "Average is: ", data.average);
	fprintf(fp, "%s %d\n", "Maximum is: ", data.maximum);
    fprintf(fp, "%s %d\n", "Keys found is: ", data.keysFound);

	fclose(fp);  

	return 0;
}