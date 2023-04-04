#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

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

int randomNumber(int min, int max) 
{
	return (rand() % (max - min + 1)) + min;
}

int max(int num1, int num2) {
    if (num1 > num2) {
        return num1;
    } else {
        return num2;
    }
}


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

int findKeys(int arr[], int size, int H, int keysFound) {
	//print the location of keys and return the number of keys found
	for (int i = 1; i < size; i++) 
	{
		while(keysFound < H)
		{
		        if (arr[i] == -1) 
			{
				printf("Hi I'm process %d and I found the hidden key in position A[%d]", getpid(), i);
				keysFound+=1;
        		}
		}
	}
    return keysFound;
}

// structure that holds average and maximum, returned in the dfs
struct info {
    double average;
    int maximum;
};

// dfs method, keeps spawning children if PN not reached or array cannot be split anymore
struct info dfs(int PN, int L, int *randNumbers) {
    double average;
    int maximum;
    struct info ret = { 0.0, 0 };
    int fd[2][2];

    pipe(fd[0]);
    pipe(fd[1]);

    pid_t pid = fork();
   
    if (pid == 0) {
        printf("Hi I'm process %d with return arg %d and my parent is %d.\n", getpid(), PN, getppid());
        close(fd[0][1]);
        close(fd[1][0]);
        int arrRightSize = L - L/2;
        int *arrRight = (int *)malloc(sizeof(int) * arrRightSize);
        read(fd[0][0], arrRight, sizeof(int) * arrRightSize);
        if (PN > 2 && arrRightSize > 2) {
            dfs(PN - 1, arrRightSize, &arrRight[0]);
        }
        ret.average = findAvg(arrRight, arrRightSize);
        ret.maximum = findMax(arrRight, arrRightSize);
        
        write(fd[1][1], &ret.average, sizeof(double));
        write(fd[1][1], &ret.maximum, sizeof(int));
        
        // printf("arrRight: ");
        // for (int i = 0; i < arrRightSize; i++) {
        //     printf("%d ", arrRight[i]);
        // }
        // printf("\n");

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
        read(fd[1][0], &childAverage, sizeof(double));
        read(fd[1][0], &childMax, sizeof(int));
       
        int *arrLeft = left(randNumbers, L);
        double parentAverage = findAvg(arrLeft, L/2);
        int parentMax = findMax(arrLeft, L/2);

        ret.average = (parentAverage * (L/2) + childAverage * (L - L/2))/L;
        ret.maximum = max(parentMax, childMax);

        // printf("arrLeft: ");
        // for (int i = 0; i < L/2; i++) {
        //     printf("%d ", arrLeft[i]);
        // }
        // printf("\n");
        
        free(arrLeft);
        free(arrRight);

    }
    return ret;
}

int main( int argc, char *argv[] )  {

	FILE *fp; 
	fp = fopen ("number_List", "w"); //create a file named fp

	int keysToPlace = 50; //number of keys that will be distributed through the list
	int rMax = 10; //maximum number to randomly generate
	int rMin = 0; //minimum number to randomly generate
	
	int keysFound = 0;
	
	if (argc !=4)	
	{
		printf("Incorrect argcount/n 3 arguments required (L,H,PN)\n exiting\n");		
		exit(0);
	}
	char *a1 = argv[1];
	char *a2 = argv[2];
	char *a3 = argv[3];

	int L = atoi(a1);
	int H = atoi(a2);
	int PN = atoi(a3);

	//printf( "\nL: %d \nH: %d \nPN: %d \n", L, H, PN);

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

    fprintf(fp, "%s %d\n", "Length of array is: ", L); 

    fclose(fp);

    struct info data = dfs(PN, L, randNumbers);
    free(randNumbers);
    printf("%s %f\n", "Average is: ", data.average); 
    printf("%s %d\n", "Maximum is: ", data.maximum); 

	return 0;
}