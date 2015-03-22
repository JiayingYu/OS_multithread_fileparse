/*
*Author: Jiaying YU
*Fileparser part1: a concurrent file parser implemented with pthread library
*and count the unique ip addresses
*take the directory name as argument
* please compile both parse2.c and strmap.c to run the program
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include "strmap.h"
#define NUM_THRDS 5

DIR *dir; //directory stream

char *dir_name;
pthread_t threads[NUM_THRDS]; //initialize the threads
pthread_mutex_t mutex_lock; //mutex lock for sychronization
int ip_num; //number of unique ip addresses
StrMap *map; //create a string hash table to store ip addresses

void *readfiles(void *threadid);


int main (int argc, char *argv[]) {
	int mapsize = 500;

	if (argc < 2) {
		printf("Please enter directory name\n");
		return -1;
	}

	if (argc > 2) {
		printf("Too many arguements supplied\n");
		return -1;
	}

	//open directory
	dir_name = argv[1];
	if ((dir = opendir(dir_name)) == NULL) {
		printf("Fail to open directory %s\n", dir_name);
		return -1;
	}

	//create map
	map = sm_new(mapsize);
	if (map == NULL)
	{
		printf("Fail to create map\n");
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
	for (t=0; t<NUM_THRDS; t++) 
	{
		printf("In main: creating thread %ld\n", t);
		rc = pthread_create(&threads[t], &attr, readfiles, (void *) t);
		if (rc) 
		{
			printf("Error; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	//wait for the threads to complete their job
	for (t = 0; t < NUM_THRDS; t++)
	{
		pthread_join(threads[t], NULL);
	}

	printf("IP counts: %d\n", ip_num);

	//clean up
	closedir(dir);
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex_lock);
	pthread_exit(NULL);

}

//read files concurrently
void *readfiles(void *threadid){
	struct dirent *ent;  //directory entry structure
	long tid = (long)threadid;	//thread id
	FILE *file; // file stream
	char *line = NULL; //pointer to
	size_t len = 1000; // the length of bytes getline will allocate
	size_t read;

	char full_filename[256]; //file name

	
	
		//print files under directory
	while(1) {
		if ((ent = readdir(dir)) != NULL) {
			//get mutex lock to prevent other thread to read the same file at the same time
			pthread_mutex_lock(&mutex_lock); 
			printf("Reading file: %s in thread #%ld\n", ent->d_name, tid);
			if (ent->d_type == DT_REG){
				 // Create the absolute path of the filename
				 snprintf(full_filename, sizeof full_filename, "./%s%s\0", dir_name, ent->d_name);
				 file = fopen(full_filename, "r");
				 if (file == NULL) {
				 	printf("File not found: %s\n", full_filename);
				 	exit(-1);
				 } else {
				 	char *ip; //ip address
				 	while ((read = getline(&line, &len, file)) != -1) 
				 	{
				 		//take ip address as the first token of each line
				 		ip = strtok(line, " ");
				 		//if the ip does not exists in the map, put it into the map and increment the ip count
				 		if (!sm_exists(map, ip)) {
				 			sm_put(map, ip, "");
				 			ip_num++;
				 		}
				 	}
				 	fclose(file);
				}
			}
			//release the lock
			pthread_mutex_unlock(&mutex_lock);
		} else {
			//if all the files under the directory has been read, break while loop and finish the process
			break;
		}
	} 
	
}


