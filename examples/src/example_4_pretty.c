/*  
    This is a refractor of example_4.c in which I did the following:

        This programm creates an hdf5 sile containing a group which contains two subgroups
        both subgroups each share a common dataset but also have their own datasets
        the shared dataset posses attributes

        --> I implemented the sharing as the dataset being in the first group and the second group having a link
    
    Now I want to do it again but this time I use structs to make the code more readable.
*/