
#include "vma.h"

int main(void)
{
	arena_t *my_arena = NULL;
	uint64_t addr, size;
	int8_t *catch = NULL, *permission;
	char *buffer = NULL;
	buffer = malloc(MAXSIZE * sizeof(char));
	catch=malloc(sizeof(char));

	DIE(!catch,"malloc failed");
	DIE(!buffer,"malloc failed");

	while(1) {
		scanf("%s",buffer);

		if(!strcmp(buffer,"ALLOC_ARENA")) {
			scanf("%lu", &size);
			my_arena = alloc_arena(size);
		}
		else if(!strcmp(buffer,"ALLOC_BLOCK")) {
			scanf("%lu %lu",&addr,&size);
			alloc_block(my_arena,addr,size);
		}
		else if(!strcmp(buffer,"FREE_BLOCK")) {
			scanf("%lu",&addr);
			free_block(my_arena,addr);
		}
		else if(!strcmp(buffer,"PMAP"))
			pmap(my_arena);

		else if(!strcmp(buffer,"DEALLOC_ARENA")) {
			dealloc_arena(my_arena);
			break;
		}
		else if(!strcmp(buffer,"READ")) {
			scanf("%lu %lu",&addr,&size);
			read(my_arena,addr,size);
		}
		else if(!strcmp(buffer,"WRITE")) {
			scanf("%lu %lu",&addr, &size);
			scanf("%c",(char*)catch);
			write(my_arena,addr,size,catch);
		}
		else if(!strcmp(buffer,"MPROTECT")) {
			scanf("%lu",&addr);
			mprotect(my_arena, addr, permission);
		}
		else {
			printf("Invalid command. Please try again.\n");
		}
	}
	free(catch);
	free(buffer);
    return 0;
}
