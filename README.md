# du-command
Implementation of du command line in xv6

# Create a user program named “du” that prints a list of the files in the current directory (where the program was run), with their on-disk size in bytes.  The final line prints the sum of all of the sizes of the files within the directory.

## Expected Behaviors:
- Size is considered to be the byte count stored in the xv6 stat structure.
- Only T_FILE types should be included in the output and in the calculation of the directory size.
- This implementation (with no arguments or flags) should not recurse into subdirectories.
- Program could be able to be run from within any directory within xv6, and to report the files within that directory correctly.

## Flags/Args for du program
### Objective:
#### Add functionality to prior du program so that it can handle various combinations of the following flags/arguments: 
-k 
Report the number of blocks each entry is allocated in the filesystem (instead of bytes).

-t [threshold]
Filter files based on a lower-bound threshold of > [threshold] bytes.

[file | directory] 
Run the command on a specific file, or on a different directory than where it was originally called.
