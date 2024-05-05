# C---Compiler

* This is a full compiler a subset language of C, called C--
* To compile it, in the command line type 
* * make compile
* To run it, you can select any of the files within C-- Test Cases directory or create your own
* There is a driver file which uses command line flags to run any part, if any, on the compiler, these flags being
* * no flags == just parse the file
* * --chk_decl == semantic checking
* * --print_ast == display the Abstract Syntax Tree of the input file
* * --gen_code == will print in standard out assembly code to be used in a SPIM environment
* An example to run the file
* * ./compile --chk_decl --print_ast --gen_code < fileName > MIPS.s
* * spim -f MIPS.s (assuming you have SPIM installed)