# MagNum - Programming Language
MagNum is a "Work In Progress" interpreted multi-paradigm programming language written in C++.
It is the direct evolution of EMERALD, another interpreted programming language I created, you can find it [here](https://github.com/Davi0k/EMERALD).

## Compiling the driver code
Just use your C++ compiler and compile the "mangum.cpp" file which contains the driver code for the interpreter

## Running a "MagNum" program
Execute this command using your shell in a folder which contains the language source:
```
.\magnum.exe *.mag
```

* ### A program that calculates a number's respective fibonacci sequence value and factorial:
```
/*
    A simple calculator made using control-flow statements
    11/28/2019
*/

set x, y, result #Declaring some global variables
set operator

#Using "while" statement to allow more than one operation
while true: {
    print 'Insert two numbers:'
    x = number(get); y = number(get) #Getting the user input and converting it to "Number" type

    print 'Insert an operator (+, -, *, /):'
    operator = get

    if operator == '+': result = x + y
    if operator == '-': result = x - y
    if operator == '*': result = x * y
    if operator == '/': result = x / y

    print 'The result is:' @ string(result) #Printing the result on the terminal
}
```

* ### A program that calculates a number's respective fibonacci sequence value and factorial:
```
#Defining a recursive function to calculate the fibonacci number
define fibonacci(n) {
    if (n == 0) or (n == 1): return n 
    else: return fibonacci(n - 1) + fibonacci(n - 2) 
}

#Defining a recursive function to calculate the factorial
define factorial(n) {
    if n > 0: return n * factorial(n - 1) 
    else: return 1
}

while true: {
    print 'Insert a number:'

    set num: number(get) #Getting a "Number" value from user's input
    print 'Its respective fibonacci number is: ' + string(fibonacci(num)) 
    print 'Its factorial is: ' + string(factorial(num))
}
```

## MagNum Sheet
```
/*
    Welcome to MagNum, developer 
    NB: this is only a cheat-sheet for the language, please do not execute

    This is a multi-line comment
    Very beautiful, don't you think?
*/

# I think we can start declaring a variable and initializing it with 'void' literal (explicity, because every uinitialized vars is set to 'void')
set variable: void

variable = 500.5 - 499.5 # "Number" type, it merges integer and decimal numbers all together
variable = "Hello" @ 'World' + `!` # "String" type, I guess that you are wondering what the freak that operator is, right?
variable = not((true or false and true) is false) != !((true | false & true) == false) # "Boolean" type, do you prefer readable words or operators?

print variable # Very simple 'print' statement, you can print whatever you want

set input: get # Let's handle some user inputs, obviously 'get' will return a "String" variable
input = number(input) # Ops... Did you want a "Number"?
input = -string(input) # Well, maybe you really wanted a "String"... Reversed?

print number; print string # Output: <Standard library function> - Yes, this language also has a STL

print length(input) # Gets the length of a "String" variable

if variable: {
    /*
        Some instruction inside a scoped-block
        If the number of instructions is one, you can write it directly
    */
}
else: empty # Have you ever heard of 'pass' key-word in Python? Well, 'empty' is the same

# A simple counter implemented with 'while' construct

variable = 0

while variable < 5: {
    print variable + 1
    variable += 1
}

for(set i: 0; i < 5; i++) print i + 1 # The same counter but implemented by a good developer

# A simple recursive function to calculate a number's factorial
define factorial(n) {
    if n > 0: 
        return n * factorial(n - 1) 
    else: return 1
}

print factorial(100) # Please, don't insert big-ass numbers, it is always a recursive algorithm

define PI: return 3.14159 # Single-statement function, very beautiful, huh? Constants are now at a new whole level
define circumference(radius) print 2 * radius * PI()

circumference(4)

exit # Actually, this statement is useless, but I think you understood the spirit
```
