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

int main() {

	FILE *fp; 
	fp = fopen ("number_List", "w"); //create a file named fp
	int L, H, PN;
	int keysToPlace = 2; //number of keys that will be distributed through the list
	int rMax = 10; //maximum number to randomly generate
	int rMin = 0; //minimum number to randomly generate
	int fileEntry;

	//collect inputs from user
	printf("\nEnter L, the number of entries in the list: ");
	scanf("%d", &L);

	printf("\nEnter H, the number of hidden keys to find: ");
	scanf("%d", &H);

	printf("\nEnter PN, the number of processes to use: ");
	scanf("%d", &PN);
	
	printf( "\nL: %d \nH: %d \nPN: %d \n", L, H, PN);

    srand(time(NULL));

    int randNumbers[L];

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

    fprintf(fp, "%s %lu\n", "Length of array is: ", sizeof(randNumbers)/sizeof(randNumbers[0])); 

    fclose(fp);

    
    double average;
    int maximum;
    int fd[2][2];

    pipe(fd[0]);
    pipe(fd[1]);

    pid_t pid = fork();
   
    if (pid == 0) {
        close(fd[0][1]);
        close(fd[1][0]);
        int *arrRight = (int *)malloc(sizeof(int) * (L - L/2));
        read(fd[0][0], arrRight, sizeof(int) * (L - L/2)); 
        average = findAvg(arrRight, L - L/2);
        maximum = findMax(arrRight, L - L/2);
        
        write(fd[1][1], &average, sizeof(double));
        write(fd[1][1], &maximum, sizeof(int));
        
        printf("arrRight: ");
        for (int i = 0; i < L - L/2; i++) {
            printf("%d ", arrRight[i]);
        }
        printf("\n");

        // printf("%s %f\n", "Child average is: ", average); 
        // printf("%s %d\n", "Child maximum is: ", maximum); 

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

        average = (parentAverage * (L/2) + childAverage * (L - L/2))/L;
        maximum = max(parentMax, childMax);

        printf("arrLeft: ");
        for (int i = 0; i < L/2; i++) {
            printf("%d ", arrLeft[i]);
        }
        printf("\n");
        
        // printf("%s %f\n", "Parent average is: ", parentAverage); 
        // printf("%s %d\n", "Parent maximum is: ", parentMax); 
        
        free(arrLeft);
        free(arrRight);

    }
    
    printf("%s %f\n", "Average is: ", average); 
    printf("%s %d\n", "Maximum is: ", maximum); 

	return 0;
}
