#	VIRTUAL MEMORY ALLOCATOR by Muntean Vlad-Andrei 315CA

For the simulation of a virtual memory allocator, the main.c file
contains a loop that accepts commands from a dynamically allocated input
buffer. The program can allocate a new memory arena, allocate a memory block
read and write in a block, change permissions and output arena information.

The "arena" holds general ifnormation, such as the total memory in
bytes, the used memory (see the used_mem of the arena_t structure from
the vma.h header file), length of block and minilock size
(size_block and size_minib) and also, a pointer to the list of nodes that
represent each block of memory. Likewise, every node holds information such
as the start address, the size and a pointer to the miniblock list. The
definite characteristic of the miniblock is a pointer to a buffer in which
the user can write and read information(rw_buffer), alongisde a field for
permissions of type RWX.


All commands are implemented in the vma.c file and are as follows:

	
## **ALLOC_ARENA**

This command dynamically allocates a new arena and initialises its
branches with the appropriate specifications (the used memory, length of
block and miniblock list are 0). The size in bytes is read from the terminal.

## **DEALLOC_ARENA**

This function takes as input only the base arena to be freed and it
loops through the whole list of blocks. For every block, the minilock list is
freed, one element by one, and then the list structure is freed with the
corresponding block (in case a writing command was previously called, the
read-write buffer is freed). After every element of the block list has been
freed, the list structure and the arena are freed and the program ends.

## **ALLOC_BLOCK**

The most complex function of the project, the start address and the
size of the miniblock we would like to add is given as a parameter alongside
our arena. First, the case in which the block list is empty is checked. For
that matter, we will allocate a specific block for our miniblock, initialise
the values correspondingly and add it to the head of the list.
If the list already has elements, it will loop to search for the
position of our new miniblock. If the miniblock has a common margin with
other miniblocks, it will simply be added into the miniblock list of the
corresponding node. Otherwise, a new block will be created that points to
the miniblock and will be added to the block list. For every addition of
a block/miniblock, the arena's used memory increases and the length of
the two lists.

A very important thing to check in this function is if the address
and size do not collide with other existing miniblocks. To avoid this,
I have created the 'check_valid_block()' function to determine wether the
miniblock is appropriate. If the return code is false, an error message is
printed and the miniblock is destroyed.

## **FREE_BLOCK**

This command approaches 3 cases in which a miniblock must be freed:
	
1. The miniblock is the lower or higher margin of the miniblock list.
Here, the miniblock is simply popped correspondingly and the miniblock
list's number decreases with the total used memory.
2. The miniblock is inside of the list. In this case, a new block is
createdto hold all of the miniblocks from the list that were in front
of the deleted miniblock.
3. The miniblock address is invalid, therefore an error message is
printed and the virtual memory is not modified in any way.
	
## **PMAP**

The PMAP command acts as a formatted print. It shows the total
allocated memory, the used memory (both in hexadecimal) and the number of
blocks and miniblocks. The function loops through every block, prints the
starting and ending addresses in hexadecimal and then prints every miniblock's
address in a format with its permissions.

## **MPROTECT**

The command changes the permission of a given miniblock's address. It
accepts multiple permissions on the same line, needing to apply al of them
accordingly. If only one permission is given, all other fields are erased. If
the address doesn't exist, an error is printed and all trailing characters will
be read with the 'full_read()' command, to avoid the misinterpretation of
possible arguments as commands.
For the 'R'(read permission), the block accepts the use of the READ
function on its buffer. The 'W' (write permission) gives privileges to the
WRITE command for the miniblock's buffer. (NOTE: in order for the READ and
WRITE commands to work, ALL miniblocks from a memory block must have the
permission, regardless of the position given to the command).

## **WRITE**

The WRITE command loops through every block's miniblock list to find
the given address for writing. If the address is invalid, an error message is
printed and the function stops. If an address is found, it must have the 'W'
permission in order to work. The starting position may not correspond to the
start address of the miniblock, therefore we will start with an offset during
the reading inside the buffer and the given message is written character by
character. If more characters are given than specified, they will be caught
with the 'full_read()' command.
If the given length exceeds the size from the original address to the
end of the miniblock, the message will be shortened to fit.

## **READ**

The READ command works if the address is valid and every miniblock from
the found block has the 'R' permission. Characters can be read from multiple
minilocks, therefore the position of the buffer's offset is nulled and the
next miniblock is accessed. If the length of the read is larger than the size
of the chosen memory area, only the existing characters are read until the end
of the miniblock list is reached.



##

This project was a fun way to better understand the concept of a doubly
linked list and the union of multiple lists at once. More than that, it was a
legitimate way to further my understandings about how the memory of a computer
is managed and updated. 
