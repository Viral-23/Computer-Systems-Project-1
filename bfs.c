#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int L;
int H;
int PN;

struct ArrayInfo {
    int* arr;
    int size;
};

// structure that holds average and maximum, returned in the dfs
struct info {
    double average;
    int maximum;
    int keysFound;
    char *keyFoundMessage;
};

char *string_append(char *s1, char *s2) {
    int s1_length = strlen(s1);
    int s2_length = strlen(s2);
    int size = s1_length + s2_length + 1;
    char *s = calloc(size, sizeof(char));

    strcpy(s, s1);
    strcat(s, s2);
    
    // printf("%s", s);

    return s;
}

struct ArrayInfo split_array(int *arr, int size, int parts, int section) {
    int start = section * size / parts;
    int end = (section + 1) * size / parts;
    int length = end - start;
    int *newArr = malloc(length * sizeof(int));

    for (int i = 0; i < length; i++) {
        newArr[i] = arr[start + i];
    }
    struct ArrayInfo ret = {newArr, length};
    return ret;
}

int max2(int num1, int num2) {
    if (num1 > num2) {
        return num1;
    } else {
        return num2;
    }
}

int power(int base, unsigned int exp) {
    int i, result = 1;
    for (i = 0; i < exp; i++)
        result *= base;
    return result;
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

char *findKeys(int arr[], int size, int keysFound, int shift) {
    //print the location of keys and return the number of keys found
    char *keyFoundMessage = malloc(sizeof(char) * 1024);
    *keyFoundMessage = '\0';
  
    for (int i = 0; i < size; i++) {
        if (keysFound == H) {
            break;
        }
        if (arr[i] == -1 && keysFound < H) {
            keysFound += 1;
            char *message = malloc(sizeof(char) * 1024);
            *message = '\0';  
            sprintf(message, "Hi I'm process %d and I found the hidden key in position A[%d]\n", getpid(), i + shift);
            keyFoundMessage = string_append(keyFoundMessage, message);
            free(message);
        }
    }
    // printf("%s", keyFoundMessage);
    return keyFoundMessage;
}


int randomNumber(int min, int max) 
{
	return (rand() % (max - min + 1)) + min;
}

int fileGenerator()
{
	FILE *fp; 
	fp = fopen ("number_List.txt", "w"); //create a file named fp

	int keysToPlace = 2; //number of keys that will be distributed through the list
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

struct info bfs(int PN, int sizeOfChildren, int timesCalled, int L, int *randNumbers, struct info ret, int shift) {
    
    pid_t pid[sizeOfChildren];
    
    int fdptoc[sizeOfChildren][2];
    int fdctop[sizeOfChildren][2];

    for (int i = 0; i < sizeOfChildren; i++) {
        pipe(fdptoc[i]);
        pipe(fdctop[i]);
    }

    for (int i = 0; i < sizeOfChildren; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            printf("Hi I'm process %d with return arg %d and my parent is %d.\n", getpid(), PN, getppid());
        
            close(fdptoc[i][1]);
            close(fdctop[i][0]);

            struct ArrayInfo part;
            read(fdptoc[i][0], &part.size, sizeof(int));
            part.arr = malloc(sizeof(int) * part.size);
            read(fdptoc[i][0], part.arr, sizeof(int) * part.size);
            int shiftArr[sizeOfChildren];
            read(fdptoc[i][0], &shiftArr, sizeof(int) * sizeOfChildren);

            // printf("pid: %d arr: \n", getpid());
            // for (int i = 0; i < part.size; i++) {
            //     printf("value: %d index: %d\n", part.arr[i] , i);
            // }
            // printf("\n");

            int futureProcesses = 0;
            for (int i = 0; i <= timesCalled + 1; i++) {
                futureProcesses += power(sizeOfChildren, i);
            }

            for (int j = 0; j < i; j++) {
                shift += shiftArr[j];
            }

            if (futureProcesses <= PN && part.size / sizeOfChildren > 1) {
                ret = bfs(PN, sizeOfChildren, timesCalled + 1, part.size, part.arr, ret, shift);
            }
            else {
                ret.average = findAvg(part.arr, part.size);
                ret.maximum = findMax(part.arr, part.size);
                ret.keyFoundMessage = string_append(ret.keyFoundMessage, findKeys(part.arr, part.size, ret.keysFound, shift));
                // printf("%s", ret.keyFoundMessage);
            }

            write(fdctop[i][1], &ret.average, sizeof(double));
            write(fdctop[i][1], &ret.maximum, sizeof(int));
            int size = sizeof(char) * strlen(ret.keyFoundMessage);
            write(fdctop[i][1], &size, sizeof(int));
            write(fdctop[i][1], ret.keyFoundMessage, size);
            
            free(part.arr);

            exit(0);
        }    
    }
    struct ArrayInfo partitions[sizeOfChildren];
    int shiftArr[sizeOfChildren];

    for (int i = 0; i < sizeOfChildren; i++) {
        close(fdptoc[i][0]);
        close(fdctop[i][1]);
        
        partitions[i] = split_array(randNumbers, L, sizeOfChildren, i);
        shiftArr[i] = partitions[i].size;
        write(fdptoc[i][1], &partitions[i].size, sizeof(int));
        write(fdptoc[i][1], partitions[i].arr, sizeof(int) * partitions[i].size);
    }
    for (int i = 0; i < sizeOfChildren; i++) {
        write(fdptoc[i][1], &shiftArr, sizeof(int) * sizeOfChildren);
    }

    for (int i = 0; i < sizeOfChildren; i++) {
        wait(NULL);
    }
    
    double temp = 0;
    int temp2 = 0;
    int size = 0;

    for (int i = 0; i < sizeOfChildren; i++) {
        read(fdctop[i][0], &temp, sizeof(double));
        ret.average += (temp * partitions[i].size);
        read(fdctop[i][0], &temp2, sizeof(int));
        ret.maximum = max2(ret.maximum, temp2);
        
        read(fdctop[i][0], &size, sizeof(int));
        char *buf = malloc(size + 1);
        read(fdctop[i][0], buf, size);
        buf[size] = '\0';

        
        // printf("%s", buf);

        ret.keyFoundMessage = string_append(ret.keyFoundMessage, buf);
        
        // printf("%s", ret.keyFoundMessage);

        free(buf);
        free(partitions[i].arr);
    }

    ret.average /= L;

    // printf("Hi i am the parent with pid %d\n", getpid());
    
    
    return ret;
}


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

    struct info data = {0.0 , 0 , 0, NULL};
    data.keyFoundMessage = malloc(sizeof(char));
    *data.keyFoundMessage = '\0';
    int sizeOfChildren = 4;
    data = bfs(PN, sizeOfChildren, 1, L, randNumbers, data, 0);
	fp = fopen("results.txt", "w");
	fprintf(fp, "%s %f\n", "Average is: ", data.average);
	fprintf(fp, "%s %d\n", "Maximum is: ", data.maximum);
    fprintf(fp, "%s %d\n", "Keys found is: ", H);

    int max_lines = H;
    int lines_printed = 0;

    char *ptr = data.keyFoundMessage;
    while (*ptr && lines_printed < max_lines) {
    if (*ptr == '\n') {
        lines_printed++;
    }
    fputc(*ptr, fp);
    ptr++;
}

    // fprintf(fp, "%s", data.keyFoundMessage);


	fclose(fp);  

    free(data.keyFoundMessage);
	return 0;
}