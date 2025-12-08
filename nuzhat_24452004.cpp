#include <iostream>
#include <fstream>
using namespace std;

/* Global declarations for variables */
int charClass;
char lexeme [100];
char reservedwords[100];
int nextChar; /* nextChar must be int to hold EOF */
int lexLen;
int token;
int nextToken;
int step; /* track each assignment for token and lexeme */
ifstream inputFile;

/* Function declarations */
void addChar();
void getChar();
void getNonBlank();
int lex();
int lookup(char ch);

/* NodePtr */
struct Node {
    int token;
    string lexeme;
    Node* left;
    Node* right;

    string actual_type; //to hold the actual type after type checking
    string expected_type; //to hold the expected type during type checking
};

/* Function declarations for syntax analyzer */
Node* expr();
Node* term();
Node* factor();
Node* assign();

/* Character classes*/
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99

/* Token Codes*/
#define INT_LIT 10
#define FLOAT_LIT 11
#define IDENT 12
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define LEFT_PAREN 27
#define RIGHT_PAREN 28
#define COMMA_OP 29

/* For assign */
#define INT_KEY 44
#define FLOAT_KEY 46

/* Symbol Table */
struct Symbol {
    bool is_ident = true;
    string name;
    string type;
    int address = 0;
    union {
        int int_value;
        float float_value;
    } value;
};

Symbol symbolTable[20]; //array to hold symbol table entries
int symbolCount = 0; //number of symbols in the table
string currentType; //to hold the type of the current declaration


/* lookup - look up operators and paranthesis and return token */
int lookup(char ch) {
    switch (ch) {
        case '(': 
            addChar();
            nextToken = LEFT_PAREN;
            break;
        case ')':
            addChar();
            nextToken = RIGHT_PAREN;
            break;
        case '+':
            addChar();
            nextToken = ADD_OP;
            break;
        case '-':
            addChar();
                nextToken = SUB_OP;
            break;
        case '*':
            addChar();
            nextToken = MULT_OP;
            break;
        case '/':
            addChar();
            nextToken = DIV_OP;
            break;
        case ',':
            addChar();
            nextToken = COMMA_OP;
            break;
        case '\n':
            addChar();
            nextToken = EOF;
            break;
        case '=':
            addChar();
            nextToken = ASSIGN_OP;
            break;
        default:
            addChar();
            nextToken = EOF;
            break;
    }
    return nextToken;
}


/* addChar - a function to add nextChar to lexeme */
void addChar() {
   if (lexLen <= 98) {
      lexeme[lexLen++] = nextChar;
      lexeme[lexLen] = 0;
   }
   else
      cout << "Error - lexeme is too long \n";
}

/* getChar - a function to get the next character of input 
and determine its character class */

void getChar() {

    if(inputFile.eof()) {
        charClass = EOF;
        nextChar = EOF;
        return;
    }

    nextChar = inputFile.get();

    if (isalpha(nextChar)) {
        charClass = LETTER;
    }
    else if (isdigit(nextChar)){
        charClass = DIGIT;
    } 
    else {
        charClass = UNKNOWN;
    }
}


   /* getNonBlank - a function to call getChar until it returns a non-blank character */
   void getNonBlank() {
    while (isspace(nextChar))
      getChar();
    }

    bool isKeyWord(const char* str, const char* keyword) {
        int i = 0;
        while (str[i] != '\0' && keyword[i] != '\0') { //iterates through lexeme and keyword from lex function
            if (str[i] != keyword[i]) { //if any character does not match, it is not a keyword
                return false;
            }
            i++;
        }
        return str[i] == '\0' && keyword[i] == '\0'; //returns true only if both strings end at the same time
    }

/* lex - a simple lexical analyzer for arithmetic expressions */

int lex() {
   lexLen = 0;
   getNonBlank();
   switch (charClass) {
      /* Parse identifiers */
      case LETTER: 
         addChar();
         getChar();
         while (charClass == LETTER || charClass == DIGIT) {
            addChar();
            getChar();
         } //loops through letters and digits to form the full lexeme
         //if first lex is a letter, it goes through if statements to see if it is a keyword, if not it is treated as identifier
         if (isKeyWord(lexeme, "int")){
            nextToken = INT_KEY;
         } else if (isKeyWord(lexeme, "float")) {
            nextToken = FLOAT_KEY;
         } else {
            nextToken = IDENT; //if not a keyword, it is an identifier
         }
         break;

        /* Parse integer literals */
        case DIGIT:
             addChar();
             getChar();
             while (charClass == DIGIT) {
                addChar();
                getChar();
                if (charClass == '.') { //checks for decimal point to identify float literals
                    addChar();
                    getChar();
                    while (charClass == DIGIT) { //after decimal point, must have digits
                        addChar();
                        getChar();
                    }
                    nextToken = FLOAT_LIT; //if decimal point found, it is float literal
                    return nextToken;
                }
             } //if first lex is a digit, it is treated as integer literal, loop breaks once a non-digit is found
             nextToken = INT_LIT;
             break;

        /* Parentheses and operators */
        case UNKNOWN: //if char is not letter or digit, it is either operator, parenthesis or unknown
             lookup(nextChar);
             if (nextToken != EOF) getChar(); //changed
             break;

        /* EOF */
        case EOF:
             nextToken = EOF;
             lexeme[0] = 'E';
             lexeme[1] = 'O';
             lexeme[2] = 'F';
             lexeme[3] = 0;
             break;
    } /* End of switch */
    cout << step++ << ". Next token is: " << nextToken << ", Next lexeme is " << lexeme << endl;
   return nextToken;
} /* End of function lex */

/* Add symbols to table */
void addSymbol(const string& name, const string& type) {
    if (symbolCount < 20) {
        symbolTable[symbolCount].name = name;
        symbolTable[symbolCount].type = type;
        symbolTable[symbolCount].address = symbolCount; //simple address assignment
        symbolCount++;
    } else {
        cout << "Symbol table full!" << endl;
    }
}

/*Syntax Analyzer*/
Node* factor (){
    cout << step++ << ". Enter <factor> \n";
    Node* node = nullptr;
    //determine which path to take
    if (nextToken == IDENT || nextToken == INT_LIT || nextToken == FLOAT_LIT){
        node = new Node{nextToken, lexeme, nullptr, nullptr};
        lex(); //next token
    } else if (nextToken == LEFT_PAREN){
        lex(); //get past '('
        step++;
        node = expr(); //go to expression
        if (nextToken == RIGHT_PAREN){
            lex(); //get past ')'
        } else {
            cout << "Syntax Error \n";
        }
    } else {
        cout << "Syntax Error \n";
    }
    cout << step++ << ". Exit <factor> \n";
    return node;
}

Node* term (){
    cout << step++ << ". Enter <term> \n";

    //first we must have a factor
    Node* left = factor();

    //then we can have 0 or more (* | /) factor
    while (nextToken == MULT_OP || nextToken == DIV_OP) {
        int op = nextToken;
        string opLex = lexeme;
        lex(); //get the operator

       Node* right = factor(); //get the next factor
       Node* parent = new Node{op, opLex, left, right};
       left = parent; //update left to be the new parent node
    }
    cout << step++ << ". Exit <term> \n";
    return left;
}

/* expr
 Parses strings in the language generated by the rule:
 <expr> -> <term> {(+ | -) <term>}
 */

Node* expr() {
    cout << step++ << ". Enter <expr> \n";

    //print first term
    Node* left = term();

    //then we can have 0 or more (+ | - | *) term
    while (nextToken == ADD_OP || nextToken == SUB_OP) {
        int op = nextToken;
        string opLex = lexeme;
        lex(); //get the operator
    
        Node* right = term(); //get the next term
        Node* parent = new Node{op, opLex, left, right};
        left = parent; //update left to be the new parent node
    }
    cout << step++ << ". Exit <expr> \n";
    return left;
}

// assign ->  <ident> = <expression>;
Node* assign() {
    cout << step++ << ". Enter <assign> \n"; //every line begins here

    // Optional: byte, short, int, or long
    if (nextToken == INT_KEY || nextToken == FLOAT_KEY) {
        lex(); // consume the type keyword
    }

    //first lexeme must be identifier
    if(nextToken != IDENT){ 
        cout << "Syntax Error \n";
    }

    Node* left = new Node{IDENT, lexeme, nullptr, nullptr};
    lex(); 
    

    string opLex;
    //second lexeme must be operator
    if(nextToken == ASSIGN_OP){
        int op = ASSIGN_OP;
        opLex = lexeme;
        lex(); 
    } else {
        cout << "Syntax Error \n";
    }



    //third lexeme must be expression
    Node* right = expr(); //we enter expression, which takes us through the terms/factors/operations
    Node* root = new Node{ASSIGN_OP, opLex, left, right};

    cout << step++ << ". Exit <assign> \n";
    return root;
}

// <assign_list> -> {<ident> =} <assign>;
//edit tmrw
Node* assign_list(){
    cout << step++ << ". Enter <assign_list> \n";

    //process zero or more {IDENT =}
    Node* left = nullptr;
    while (nextToken == IDENT) {
        Node* identNode = new Node{IDENT, lexeme, nullptr, nullptr};
        lex(); //consume IDENT

        //must be followed by '='
        if (nextToken == ASSIGN_OP) {
            string opLex = lexeme;
            lex(); //consume '='
            Node* assignNode = new Node{ASSIGN_OP, opLex, identNode, nullptr};
            if (left == nullptr) {
                left = assignNode; //first assignment
            } else {
                Node* parent = new Node{COMMA_OP, ",", left, assignNode};
                left = parent; //link to previous assignments
            }
        } else {
            cout << "Syntax Error \n";
        }
    }

    //process final <assign>
    Node* finalAssign = assign();
    if (left == nullptr) {
        left = finalAssign; //only one assignment
    } else {
        Node* parent = new Node{COMMA_OP, ",", left, finalAssign};
        left = parent; //link final assignment
    }

    cout << step++ << ". Exit <assign_list> \n";
    return left;
}

/* <declare list> -> (<int> | <float>) <ident> [= <expr>] {, <ident> [= <expr>] };
this handles multiple declarations in one line separated by commas */

Node* declare (){
     //check for identifier
    if (nextToken != IDENT) {
        cout << "Expected identifier \n";
    }

    string identName = lexeme;
    addSymbol(identName, currentType); //add identifier to symbol table with current type
    Node* left = new Node{IDENT, identName, nullptr, nullptr}; //create node for identifier
    lex(); //consume identifier

    //optional assignment [= <expr>]
    if (nextToken == ASSIGN_OP) {
        string opLex = lexeme;
        lex(); //consume '='
        Node* right = expr(); //get expression
        Node* assignNode = new Node{ASSIGN_OP, opLex, left, right};
        left = assignNode; //update left to be the assignment node
    }
    return left;
}

Node* declare_list() {
    cout << step++ << ". Enter <declare_list> \n";

    //first lexeme must be type keyword
    if (nextToken != INT_KEY && nextToken != FLOAT_KEY) {
        cout << "Expected type keyword \n";
    } 

    if (nextToken == INT_KEY) {
        currentType = "int";
    } else if (nextToken == FLOAT_KEY) {
        currentType = "float";
    }

    lex(); //consume type keyword

    //process first identifier (and optional assignment)
    Node* left = declare();

    //handle additional identifiers separated by commas {, IDENT [= <expr>]}
    while (nextToken == COMMA_OP) {
        lex(); //consume ','
        Node* newIdent = declare(); //process the next identifier (and optional assignment)
        //link the new identifier (or assignment) to the left node
        Node* parent = new Node{COMMA_OP, ",", left, newIdent};
        left = parent; //update left to be the new parent node
    }

    cout << step++ << ". Exit <declare_list> \n";
    return left;
}

/* <proj_start> --> <assign_list>  | <declare_list> 
void proj_start() {
    cout << step++ << ". Enter <proj_start> \n";

    if (nextToken == INT_KEY || nextToken == FLOAT_KEY) {
        declare_list();
    } else {
        assign_list();
    }

    cout << step++ << ". Exit <proj_start> \n";
} */

int main () {
   /* Open input file and process contents */
   inputFile.open("front.in");

   if (!inputFile.is_open()) {
      cout << "ERROR - cannot open front.in \n";
      return 1;
   } else {
    getChar();
    lex();
    do {
    if (nextToken == INT_KEY || nextToken == FLOAT_KEY)
        declare_list();
    else
        assign();
        cout << '\n';
    } while (nextToken != EOF);
   }
   inputFile.close();
   return 0;
}