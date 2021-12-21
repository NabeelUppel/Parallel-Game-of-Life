A bash script called evaluation.sh will handle all the commands to compile and run the various implementations with varying input and number of threads.

The executable might need to me made executable this can be done by opening a terminal to folder and running "chmod +x evaluation.sh"
You can then run it using "./evaluation.sh"

The input is read from various text files found in the input folder.

Output and result files are moved automatically to the output and results folders respectively.

Old Input, Output and Results will be removed before evaluation.sh is ran.

The bash script was tested using a Linux system and I am unsure how well it work on other Unix systems like MacOS.
