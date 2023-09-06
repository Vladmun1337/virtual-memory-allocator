#include "vma.h"

arena_t *alloc_arena(const uint64_t size)
{
    arena_t *new_arena = NULL;

    new_arena = malloc(sizeof(arena_t));
    DIE(new_arena==NULL,"malloc failed");

    new_arena->arena_size=size;
    new_arena->alloc_list=NULL;

    new_arena->alloc_list=malloc(sizeof(list_t));
    DIE(new_arena->alloc_list==NULL,"malloc failed");

    new_arena->used_mem=0;
    new_arena->size_block=0;
    new_arena->size_minib=0;
    new_arena->alloc_list->head=NULL;

    return new_arena;
}

void dealloc_arena(arena_t *arena)
{
	block_t *tmp_block;
	miniblock_t *tmp_miniblock;
	while(arena->alloc_list->head!=NULL){
		tmp_block=arena->alloc_list->head;
		while(((list_t*)tmp_block->miniblock_list)->head!=NULL) {

			tmp_miniblock=((list_t*)tmp_block->miniblock_list)->head;
			free(tmp_miniblock->rw_buffer);

			((list_t*)tmp_block->miniblock_list)->head = 
			tmp_miniblock->next;

			free(tmp_miniblock);
		}

		free(tmp_block->miniblock_list);
		arena->alloc_list->head =tmp_block->next;
		free(tmp_block);

	}

	free(arena->alloc_list);
	free(arena);

}

block_t* create_block(uint64_t address, uint64_t size) {

	block_t *new_block=NULL;

	new_block=malloc(sizeof(block_t));
	DIE(new_block==NULL,"malloc failed");
	new_block->start_address = address;
	new_block->size = size;
	new_block->miniblock_list=malloc(sizeof(list_t));
	DIE(new_block->miniblock_list==NULL,"malloc failed");
	((list_t*)new_block->miniblock_list)->head=NULL;

	return new_block;
}

void add_nth_block(arena_t *arena, block_t *new_block, unsigned int n) {

	block_t *tmp=arena->alloc_list->head;

	if(n==0) {

		new_block->next = arena->alloc_list->head;
		if(new_block->next != NULL)
			((block_t*)arena->alloc_list->head)->prev = new_block;

		arena->alloc_list->head = new_block;
		new_block->prev =NULL;

	} else {
		for(unsigned int i=0; i<n-1; i++)
			tmp=tmp->next;
		new_block->next = tmp->next;
		if(tmp->next != NULL)
			tmp->next->prev = new_block;
		tmp->next = new_block;
		new_block->prev = tmp;
	}
	arena->size_block++;
}

int check_valid_block(block_t *base, uint64_t address, uint64_t size)
{
	uint64_t end, new_end;
	end= base->start_address + base->size;
	new_end= address + size;

	if(base->size == 0) {
		if(address == base->start_address)
			return 1;
		if(new_end == base->start_address)
			return 1;
	}
	if(address >= base->start_address && address < end)
		return 0;

	if(new_end > base->start_address && new_end <= end)
		return 0;

	if(address <= base ->start_address && new_end >= end)
		return 0;

	if(base->next!=NULL && address + size > base->next->start_address)
		return 0;

	return 1;
}

void check_first(arena_t *arena, miniblock_t *new,uint64_t address, uint64_t size)
{
	block_t *tmp=arena->alloc_list->head,*new_block;
	miniblock_t *aux=((list_t*)tmp->miniblock_list)->head;

	if(tmp!=NULL && check_valid_block(tmp,address,size))
		if(address+size == tmp->start_address) {

			new->next = aux;
			aux->prev = new;
			((list_t*)tmp->miniblock_list)->head = new;
			tmp->start_address -= size;
			tmp->size += size;

		} else {

			new_block=create_block(address,size);
			((list_t*)new_block->miniblock_list)->head = new;
			add_nth_block(arena,new_block,0);

		}
	else {
		printf("Invalid block\n");
			arena->used_mem -= new->size;
			free(new->rw_buffer);
			free(new);
			arena->size_minib--;
	}
}

void merge_blocks(arena_t* arena,block_t* dest, block_t* source)
{
	miniblock_t *last,*source_head;
	last=((list_t*)dest->miniblock_list)->head;
	source_head=((list_t*)source->miniblock_list)->head;
	while(last->next!=NULL)
		last=last->next;

	last->next=source_head;
	source_head->prev=last;

	free(source->miniblock_list);

	dest->next=source->next;
	dest->size+=source->size;
	if(source->next!=NULL)
		source->next->prev=dest;
	free(source);
	arena->size_block--;
}
void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	block_t *new_block=NULL, *tmp;
	miniblock_t *new_miniblock=NULL, *curr_mini;
	int count;

	if(address < arena->arena_size && address + size <= arena->arena_size) {
		new_miniblock=malloc(sizeof(miniblock_t));
		DIE(new_miniblock==NULL,"malloc failed");

		new_miniblock->start_address=address;
		new_miniblock->size=size;

		new_miniblock->perm=6;
		new_miniblock->next = new_miniblock->prev = NULL;
		new_miniblock->rw_buffer=NULL;

		arena->used_mem += new_miniblock->size;
		arena->size_minib++;

		
		if(arena->alloc_list->head==NULL) {
			new_block=create_block(address,size);

			((list_t*)new_block->miniblock_list)->head = new_miniblock;
			add_nth_block(arena,new_block,0);
		}

		else {

			tmp=arena->alloc_list->head;
			count=1;
			if(address + size <= tmp->start_address) {
				check_first(arena,new_miniblock,address,size);
				return;
			}

			while(tmp->next != NULL && address+size > tmp->next->start_address + tmp->next->size) {

				tmp=tmp->next;
				count++;
			}

			if(check_valid_block(tmp,address,size)==0){
				printf("This zone was already allocated.\n");
				arena->used_mem -= new_miniblock->size;
				free(new_miniblock->rw_buffer);
				free(new_miniblock);
				arena->size_minib--;
			}
					

			else if(address == tmp->start_address + tmp->size){
				
				curr_mini=((list_t*)tmp->miniblock_list)->head;

				while(curr_mini->next!=NULL)
					curr_mini=curr_mini->next;

				curr_mini->next=new_miniblock;
				new_miniblock->prev= curr_mini;
				tmp->size+=size;
				if(tmp->next!=NULL && address + size == tmp->next->start_address) {
					merge_blocks(arena,tmp,tmp->next);
				}

			}
			else if(tmp->next!=NULL && address+ size == tmp->next->start_address) {
				curr_mini=((list_t*)tmp->next->miniblock_list)->head;
				new_miniblock->next= curr_mini;
				curr_mini->prev=new_miniblock;
				((list_t*)tmp->next->miniblock_list)->head= new_miniblock;
				tmp->next->start_address -= size;
				tmp->next->size += size;
			}
			else {
				new_block=create_block(address,size);
				((list_t*)new_block->miniblock_list)->head = new_miniblock;
				add_nth_block(arena,new_block,count);
			}
		}
	}
	else if(address >= arena->arena_size) 
		printf("The allocated address is outside the size of arena\n");
	
	else if(address + size > arena->arena_size)
		printf("The end address is past the size of the arena\n");

}
void destroy_miniblock(arena_t *arena, block_t *base, miniblock_t *mini)
{
	block_t *new_block;
	uint64_t next_addr;
	if(mini->prev==NULL) {
		base->start_address += mini->size;
		base->size -= mini->size;

		((list_t*)base->miniblock_list)->head = mini->next;
		if(mini->next != NULL)
			mini->next->prev = NULL;

	}
	else if(mini->next==NULL) {
		base->size -= mini->size;
		mini->prev->next = mini->next;
	}
	else {
		next_addr = mini->next->start_address;
		new_block=create_block(next_addr, base->start_address + base->size - next_addr);
		((list_t*)new_block->miniblock_list)->head = mini->next;

		mini->next->prev=NULL;
		mini->prev->next=NULL;

		base->size -= mini->size;
		base->size -= new_block->size;

		new_block->next = base->next;
		if(base->next != NULL)
			base->next->prev = new_block;
		new_block->prev = base;
		base->next = new_block;
		arena->size_block++;
	}

	
	arena->used_mem -= mini->size;
	arena->size_minib--;

	if(mini->rw_buffer != NULL)
		free(mini->rw_buffer);
	free(mini);


}

void destroy_block(arena_t* arena, block_t *base)
{
	if(base->prev ==NULL) {
		arena->alloc_list->head = base->next;
		if(base->next !=NULL)
			base->next->prev=NULL;
	}
	else {
		base->prev->next = base->next;
		if(base->next != NULL)
			base->next->prev = base->prev;
	}
	arena->size_block--;
	free(base->miniblock_list);
	free(base);
}

void free_block(arena_t *arena, const uint64_t address)
{
	block_t *curr_block=arena->alloc_list->head;
	miniblock_t *mini;
	int ok=0;
	uint64_t end_addr;

	while(curr_block!=NULL) {
		
		end_addr = curr_block->start_address + curr_block->size;

		if(curr_block->start_address <= address && address <= end_addr) {
			mini=((list_t*)curr_block->miniblock_list)->head;

			while(mini!=NULL) {

				if(mini->start_address == address) {
					destroy_miniblock(arena,curr_block,mini);
					ok=1;
					
					if(((list_t*)curr_block->miniblock_list)->head==0)
						destroy_block(arena,curr_block);

					return;
				}
				mini=mini->next;
			}
		}

		curr_block = curr_block->next;
	}
	if(ok==0)
		printf("Invalid address for free.\n");
}

int check_perm(block_t *curr, uint8_t permission)
{
	miniblock_t *mini = ((list_t*)curr->miniblock_list)->head;
	uint8_t catch;

	while(mini != NULL) {

		catch = mini->perm & permission;
		if(catch == 0)
			return 0;

		mini = mini->next;
	}

	return 1;
}
void full_read(char carac)
{
	while(carac!='\n')
		scanf("%c",&carac);
}
void read(arena_t *arena, uint64_t address, uint64_t size)
{
	block_t *curr_block;
	miniblock_t *curr_mini;
	uint64_t end, count;
	char carac = 0;
	if(arena->size_minib == 0 || arena->size_block == 0) {
		printf("Invalid address for read.\n");
		full_read(carac);
		return;
	}
	curr_block = arena->alloc_list->head;
	while(curr_block != NULL) {
		end = curr_block->start_address + curr_block->size;

		if(address >= curr_block->start_address && address <= end)
			break;
		curr_block = curr_block->next;
	}
	if(curr_block == NULL) {
		printf("Invalid address for read.\n");
		return;
	}

	if(check_perm(curr_block,4)==0) {
		printf("Invalid permissions for read.\n");
		full_read(carac);
		return;
	}

	curr_mini = ((list_t*)curr_block->miniblock_list)->head;

	while(curr_mini !=NULL) {
		end=curr_mini->start_address + curr_mini->size;

		if(address >= curr_mini->start_address && address <= end)
			break;
		curr_mini = curr_mini->next;
	}

	if(curr_mini->rw_buffer == NULL) {
		printf("Invalid address for read.\n");
		return;
	}
	end = curr_block->start_address + curr_block->size;
	if(size > end - address) {
		printf("Warning: size was bigger than the block size. ");
		printf("Reading %lu characters.\n",end - address);
		size = end - address;
	}

	count = address - curr_mini->start_address;

	while(size != 0 && ((char*)curr_mini->rw_buffer)[count] != 0) {
		size--;
		printf("%c",((char*)curr_mini->rw_buffer)[count]);
		count++;

		if(count == curr_mini->size && size != 0) {
			count=0;
			curr_mini = curr_mini->next;
		}
	}
	printf("\n");
}

void write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{

	block_t *curr_block = arena->alloc_list->head;
	miniblock_t *curr_mini;
	uint64_t end, count, start = size;
	char carac = 0;
	if(data == NULL)
		data = malloc(sizeof(char));

	while(curr_block != NULL) {
		end= curr_block->start_address + curr_block->size;
		if(address >= curr_block->start_address && address <= end)
			break;
		curr_block = curr_block->next;
	}
	if(curr_block == NULL) {
		printf("Invalid address for write.\n");
		full_read(*(char*)data);
		return;
	}

	if(check_perm(curr_block,2)==0) {
		printf("Invalid permissions for write.\n");
		full_read(carac);
		return;
	}
	
	if(start > end - address) {
		printf("Warning: size was bigger than the block size. ");
		printf("Writing %lu characters.\n",end - address);
		start = end - address;
	}
	curr_mini = ((list_t*)curr_block->miniblock_list)->head;

	while(curr_mini !=NULL) {
		end=curr_mini->start_address + curr_mini->size;

		if(address >= curr_mini->start_address && address <= end)
			break;
		curr_mini = curr_mini->next;
	}
	curr_mini->rw_buffer = calloc(1,curr_mini->size);
	count = address - curr_mini->start_address;


	while(start != 0) {

		scanf("%c",(char*)data);

		memcpy(curr_mini->rw_buffer+count,(char*)data,sizeof(char));
		start--;
		count++;

		if(count == curr_mini->size && start != 0) {
			count=0;
			curr_mini = curr_mini->next;
			curr_mini->rw_buffer = calloc(1,curr_mini->size);
			DIE(curr_mini->rw_buffer==NULL,"calloc failed");
		}
	}

	full_read(*(char*)data);
}

void print_permission(miniblock_t *mini)
{
	uint8_t permission = 4, catch;

	while(permission > 0) {

		catch = mini->perm & permission;

		if(catch == 0)
			printf("-");
		else {
			if(permission == 4)
				printf("R");
			else if(permission == 2)
				printf("W");
			else
				printf("X");
		}

		permission = permission >> 1;
	}
	printf("\n");
}

void pmap(const arena_t *arena)
{
	block_t *curr_block = arena->alloc_list->head;
	miniblock_t *curr_miniblock;
	int count_block=1, count_miniblock;
	uint64_t end_addr, end_miniblock;

	printf("Total memory: 0x%lX bytes\n",arena->arena_size);
	printf("Free memory: 0x%lX bytes\n",arena->arena_size - arena->used_mem);
	printf("Number of allocated blocks: %lu\n",arena->size_block);
	printf("Number of allocated miniblocks: %lu\n",arena->size_minib);

	while(curr_block!=NULL) {
		printf("\nBlock %d begin\n",count_block++);
		end_addr = curr_block->start_address + curr_block->size;
		printf("Zone: 0x%lX - 0x%lX\n",curr_block->start_address, end_addr);

		curr_miniblock=((list_t*)curr_block->miniblock_list)->head;
		count_miniblock=1;
		while(curr_miniblock != NULL) {
			printf("Miniblock %d:",count_miniblock++);
			end_miniblock= curr_miniblock->start_address + curr_miniblock->size;

			printf("\t\t0x%lX\t\t-\t\t0x%lX\t\t| ",curr_miniblock->start_address, end_miniblock);
			print_permission(curr_miniblock);
			curr_miniblock=curr_miniblock->next;
		}
		printf("Block %d end\n",count_block-1);

		curr_block=curr_block->next;
	}
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	block_t *curr_block = arena->alloc_list->head;
	miniblock_t * mini;
	char *perm=NULL;
	uint64_t end;
	
	while(curr_block != NULL) {
		end = curr_block->start_address + curr_block->size;
		if(address >= curr_block->start_address && address <= end)
			break;
		curr_block = curr_block->next;
	}
	if(curr_block == NULL) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	permission = malloc(MAXSIZE * sizeof(char));
	mini = ((list_t*)curr_block->miniblock_list)->head;

	while(mini != NULL) {
		end= mini->start_address + mini->size;
		if(address >= mini->start_address && address <= end)
			break;
		mini = mini->next;
	}

	fgets((char *)permission,MAXSIZE,stdin);
	perm = strtok((char*)permission, " \n");

	while(perm) {
		if(!strcmp(perm,"PROT_NONE"))
			mini->perm = 0;
		else if(!strcmp(perm,"PROT_READ"))
			mini->perm |= 4;
		else if(!strcmp(perm,"PROT_WRITE"))
			mini->perm |= 2;
		else if(!strcmp(perm,"PROT_EXEC"))
			mini->perm |= 1;

		perm = strtok(NULL," \n");
	}
	free(permission);
}
