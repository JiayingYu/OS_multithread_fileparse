/*
*Author: Jiaying YU
*Fileparser part1: a concurrent file parser implemented with pthread library
*take the directory name as argument
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#define NUM_THRDS 5 //number of threads

DIR *dir; //directory stream

char *dir_name;
pthread_t threads[NUM_THRDS]; //initialize the threads
pthread_mutex_t mutex_lock; //mutex lock for sychronization

void *readfiles(void *threadid);


int main (int argc, char *argv[]) {

	if (argc < 2) {
		printf("Plase provide directory name\n");
		return -1;
	}

	if (argc > 2) {
		printf("Too many arguements supplied\n");
		return -1;
	}

	//open the directory
	dir_name = argv[1];
	if ((dir = opendir(dir_name)) == NULL) {
		printf("Fail to open directory %s\n", dir_name);
		return -1;
	}

	
	//set up threads attributes 
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	//set up mutex attributes
	pthread_mutex_init(&mutex_lock, NULL);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int rc;
	long t;
	//create 5 threads
	for (t=0; t<NUM_THRDS; t++) {
		printf("In main: creating thread %ld\n", t);
		rc = pthread_create(&threads[t], &attr, readfiles, (void *) t);
		if (rc) {
			printf("Error; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	//wait for the threads to complete their job
	for (t = 0; t < NUM_THRDS; t++)	{
		pthread_join(threads[t], NULL);
	}

	//clean up
	closedir(dir);
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex_lock);
	pthread_exit(NULL);

}

//read files concurrently
void *readfiles(void *threadid) {
	struct dirent *ent;  //directory entry structure
	long tid = (long)threadid;	//thread id
	FILE *file; // file stream
	char *line = NULL;  //pointer to
	size_t len = 1000; // the length of bytes getline will allocate
	size_t read;

	char full_filename[256]; //file name

	//get mutex lock to prevent other thread to read the same file at the same time
	
	
	while (1) {
		//print files under directory
		if ((ent = readdir(dir)) != NULL) {
			pthread_mutex_lock(&mutex_lock);
			printf("Reading file: %s in thread #%d\n", ent->d_name, tid);
			if (ent->d_type == DT_REG) { 	// Check if the list is a regular file
				// Create the absolute path of the filename
				snprintf(full_filename, sizeof full_filename, "./%s%s\0", dir_name, ent->d_name);
				file = fopen(full_filename, "r");
				if (file == NULL) {
				 	printf("File not found: %s\n", full_filename);
				 	exit(-1);
				 } else {
				 	// Print out each line in the file
				 	while ((read = getline(&line, &len, file)) != -1){
				 		//we do not print file content to make screen output clean and concise
				 		//printf("Retrieved line of length %d:\n", read);
				 		//printf("%s", line);
				 	}
				 	fclose(file);
				}				 
			}
			//release lock
			pthread_mutex_unlock(&mutex_lock);
		} else {
			break;
		}
	}
}


