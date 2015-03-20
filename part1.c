#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#define NUM_THRDS 5

DIR *dir; //directory stream
struct dirent *ent;  //directory entry structure
char *dir_name;
pthread_t threads[NUM_THRDS];
pthread_mutex_t mutex_lock;

void *readfiles(void *threadid);


int main (int argc, char *argv[]) {

	if (argc < 2) {
		printf("Not enough arguments supplied\n");
		return -1;
	}

	if (argc > 2) {
		printf("Too many arguements supplied\n");
		return -1;
	}

	dir_name = argv[1];
	if ((dir = opendir(dir_name)) == NULL) {
		printf("Fail to open directory %s\n", dir_name);
		return -1;
	}

	
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_mutex_init(&mutex_lock, NULL);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int rc;
	long t;
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

	for (t = 0; t < NUM_THRDS; t++)
	{
		pthread_join(threads[t], NULL);
	}

		
	closedir(dir);
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex_lock);
	pthread_exit(NULL);

}

void *readfiles(void *threadid) 
{
	long tid = (long)threadid;	
	FILE *file; // file stream
	char *line = NULL;
	size_t len = 1000;
	size_t read;

	char full_filename[256];

	pthread_mutex_lock(&mutex_lock);

		while((ent = readdir(dir)) != NULL) 
		{
			printf("ready to read file: %s\n\n\n", ent->d_name);
			if (ent->d_type == DT_REG)
				 {
				 	snprintf(full_filename, sizeof full_filename, "./%s%s\0", dir_name, ent->d_name);
				 	file = fopen(full_filename, "r");
				 	if (file == NULL) 
				 	{
				 		printf("File not found: %s\n", full_filename);
				 		exit(-1);
				 	} else
				 	{
				 		while ((read = getline(&line, &len, file)) != -1) 
				 		{
				 			printf("Retrieved line of length %d:\n", read);
				 			printf("%s", line);
				 		}
				 		fclose(file);
				 	}
				 }
		} 
	pthread_mutex_unlock(&mutex_lock);
}


