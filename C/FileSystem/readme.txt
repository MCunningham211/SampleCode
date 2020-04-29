This is a project I completed for my opperating systems class. The task was to implement a sample file system. 
It sould be possible to build and run this using tha make file porvided. The idea is that I have a file called 
VDisk which I can readt to and write from. THere are a number of improvements I could have made, but the basic 
functionalety is there though it does not support command line opperations so for the moment it simply runs using 
functions calls from main. With that said, implementing linux commands such as ls, touch, MKdir, unlink or many others
would simply involve making function calls to the existiong functions. THis file system is designed to store data in a set of 
4096 bit blocks, each containing 512 bytes. This made tracking the sysem blocks very easy as the 512 bytes allowed for
one block was enough to create a bit vector that could track whether or not all the blocks were used using one bit set to
true or false each. Each file had the blocks it used reference using an Inode that took up 32 bits, with one layer of indirection
in case the file was large. The result of this design is thatbhte file system is roughly 2 megabytes, and a max file size of
about 136 kilobytes. There are a couple oter limitations that I used to make the system simpler too, so each folder can contain a 
maximum of 16 files, though sub folders are possible. Additionally, the file system can contain a maximum of 127 files.