This is line fitting problem that I solved. We are given a set of data and the goal is to find the best fit line
for the points. Running the included commands will allow you to see a veriety of best filt lines including for 
a very large dataset. The original problem is n^3, however, I break it up into individual treads which each run 
in n^2. This is much faster assuming it is being run on something that is god at multithreading. In this case,
the program is heavly relient on the opperating system being able to effectively handel the creating and hendeling 
of threads, as this implementation has the potentioal to flood the OS as it will create a thread for each datapoint.