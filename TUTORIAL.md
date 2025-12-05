***Version MC-Beta-01***

**Basic syntax**:
    
    In MC whole program starts from the `start ()` function. (Note: if you write any other functions below "start" function they will be ignored)

    In MC to create your own function type: `<"name"> (<"arguments">)`. And declaring function, type '{' and '}' to declare where start and end of your function.
    <"name"> - is name of your function. General recommendation is to not include special characters into the name, like: "`'"~!@#$%?&:;^*-+=(){}<>\|/".
    <"arguments"> - is name of your variables you pass into the function separated with ',' character. (Note: 1. Variables you pass needs to be declared. 2. Names of arguments must be the same as names of the variables you are passing.)

    MC allows this operands: '+', '-', '*', '/', '%', '&', '|', '^'. Aswell as variable assign operators: "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=".
    
    MC have condition operators listed here: "==", "!=", "<", "<=", ">", ">=".

    In MC to initialize variable you need to pass `<"type"> <"name"> = <"default value">` (Note: All fields are required to fill in)
    <"type"> - is type of your variable it determines what you can do with variable, like for integers MC have: "int8_t", "int16_t", "int32_t", "int64_t". Aswell as for strings: "string8_t", "string16_t", "string32_t", "string64_t". (Note: Right now string type can only be used for printing characters)
    <"name"> - is name of your variable. General recommendation is to not include special characters into the name, like: "`'"~!@#$%?&:;^*-+=(){}<>\|/".
    <"default value"> - is default value that corresponds to the data type you choose: For integers can only be digits and operans that are listed in first sentance. For strings you can declare with one string without spaces using this syntax "text", or you can declare string with spaces using this syntax " text1 text2 ".
    
    In MC programming language there are 6 built in functions: print, println, read, goto, goto_if, return.

    In MC programming language all programs required to have `return <"number">`. If you don't write `return` you will get segmentation fault. 

**Examples**:

    To write simple Hello World program in MC programming language do the following:
    1. Create file that ends with ".mc" extension.
    2. write that code into the file:
    ```
    start () {                     <- where all starts 
        println " Hello World! "   <- print string "Hello world" with newline
        return 0                   <- exit with error code 0
    }
    ```
    3. Compile the code by doing `mclang <"file_name.mc">`. 
    <"file_name.mc"> - is name of the file you created in the previous steps.

**Reference**:
    
    * `**print any_type**` - prints characters to the screen. Accepts strings, integer variables, string variables. (Note: for variables syntax is: `print (variable)`, and for strings syntax is: `print "string"`.)
    * `**println any_type**` - same as above, but with newline added.
    * `**read (string_var, digit)**` - first argument is string variable where input will be stored, second argument is number of characters to read from input.
    * `**goto function**` - goes to existing function.
    * `**goto_if (digit_or_digit_var condition_operator digit_or_digit_var, function)**` - goes to existing function if condition is meet.
    * `**return digit_or_digit_var**` - exits program with error code you entered. (Note: for variables syntax is: `return (variable)`, and for digits syntax is: `return digit`.)

