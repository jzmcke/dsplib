#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NUM_THREADS 10
#define MAX_COUNT 1000000

typedef struct file_state_s
{
	int count;
	int b_thread_locked;
	FILE *p_file;
} file_state;

void
inc_and_print(file_state *p_state);

int
main
	(int argc
	,char **argv
	)
{
	int i;
	file_state state;
	pthread_t a_thread_ID[MAX_NUM_THREADS];
	int *ap_count[MAX_NUM_THREADS];
	state.p_file = fopen("/home/joe/Desktop/test.txt","w");
	state.count = 0;
	state.b_thread_locked = 0;
	for (i=0; i<MAX_NUM_THREADS; i++)
	{
		pthread_create(&a_thread_ID[i], NULL, (void*)inc_and_print, &state);
	}
	for (i=0; i<MAX_NUM_THREADS; i++)
	{
		pthread_join(a_thread_ID[i], (void**)&ap_count[i]);
		printf("Done! %d\n", *(ap_count[i]));
		printf("state.count = %d\nstate.b_thread_locked = %d\n", state.count, state.b_thread_locked);
	}
	fclose(state.p_file);
	return 0;
}

void
inc_and_print(file_state *p_state)
{
	while (p_state->count < MAX_COUNT)
	{
		if(p_state->b_thread_locked != 1)
		{
			p_state->b_thread_locked = 1;
			//printf("Thread write\n");
			(p_state->count)+=1;
			fprintf(p_state->p_file, "%d\n", p_state->count);
			p_state->b_thread_locked = 0;
		}
		else
		{
			//printf("Thread idle\n");
			int i=0;
			i++;
		}		
	}
	pthread_exit((void*)&p_state->count);
}