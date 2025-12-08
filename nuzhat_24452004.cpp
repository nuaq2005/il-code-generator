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
#define NEXT 30

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
            nextToken = NEXT;
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
        case DIGIT: {
             bool isFloat = false;
             addChar();
             getChar();
             while (charClass == DIGIT || charClass == '.') { //loops through digits to form the full lexeme
                if (charClass == '.') isFloat = true;
                addChar();
                getChar();
             } 
             nextToken = isFloat ? FLOAT_LIT : INT_LIT; //if decimal point found, it is float literal
             break;
        }

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
void addSymbol(bool isIdent, const string& name, const string& type, const string& value) {
    if (symbolCount < 20) {
        symbolTable[symbolCount].name = name;
        symbolTable[symbolCount].type = type;
        symbolTable[symbolCount].address = symbolCount; //simple address assignment
        
        symbolTable[symbolCount].value.int_value = stoi(value);
        symbolTable[symbolCount].value.float_value = stof(value);

        symbolCount++;
    } else {
        cout << "Symbol table full!" << endl;
    }
}

/* Lookup type of identifier in symbol table */
string lookupType(const string &name) {
    for (int i = 0; i < symbolCount; i++) {
        if (symbolTable[i].name == name) {
            return symbolTable[i].type;
        }
    }
    cout << "Undeclared identifier: " << name << "\n";
    return ""; // error type
}


/*Syntax Analyzer*/
Node* factor (){
    cout << step++ << ". Enter <factor> \n";
    Node* node = nullptr;
    //determine which path to take
    if (nextToken == IDENT || nextToken == INT_LIT || nextToken == FLOAT_LIT){
        node = new Node{nextToken, lexeme, nullptr, nullptr, "", ""}; //create node for factor
        
        //check symbol table for type
        if (nextToken == IDENT) {
            node->actual_type = lookupType(lexeme);
        } else if (nextToken == INT_LIT) {
            node->actual_type = "int";
        } else if (nextToken == FLOAT_LIT) {
            node->actual_type = "float";
        }

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
       
       Node* parent = new Node{op, opLex, left, right, "", ""};
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
        Node* parent = new Node{op, opLex, left, right, "", ""};
        left = parent; //update left to be the new parent node
    }
    cout << step++ << ". Exit <expr> \n";
    return left;
}

// assign ->  <ident> = <expression>;
Node* assign() {
    cout << step++ << ". Enter <assign> \n"; //every line begins here

    //first lexeme must be identifier
    if(nextToken != IDENT){ 
        cout << "Syntax Error \n";
    }

    Node* left = new Node{IDENT, lexeme, nullptr, nullptr, "", ""}; //create node for identifier
    lex(); 
    
    
    //second lexeme must be operator
    if(nextToken != ASSIGN_OP){
        cout << "Syntax Error \n";
    }

    string opLex = lexeme;
    lex(); 
  
    //third lexeme must be expression
    Node* right = expr(); //we enter expression, which takes us through the terms/factors/operations
    Node* root = new Node{ASSIGN_OP, opLex, left, right, "", ""}; //create root node for assignment

    cout << step++ << ". Exit <assign> \n";
    return root;
}

// <assign_list> -> {<ident> =} <assign>;
//edit tmrw
Node* assign_list(){
    cout << step++ << ". Enter <assign_list> \n";
    
    //process zero or more {IDENT =}
    Node* left = nullptr;

    if (nextToken == IDENT) {
        Node* identNode = new Node{IDENT, lexeme, nullptr, nullptr, "", ""}; //create node for identifier
        lex(); //consume identifier

        if (nextToken != ASSIGN_OP){
            cout << "Syntax Error: expected '=' \n";
        }

        string opLex = lexeme;
        lex(); //consume '='
        Node* right = assign_list(); //process the next assignment

        Node* assignNode = new Node{ASSIGN_OP, opLex, identNode, right, "", ""}; //create assignment node
        left = assignNode; //update left to be the assignment node

        return assignNode;
    }

    Node* assignNode = assign(); //process the final assignment
    cout << step++ << ". Exit <assign_list> \n";
    return assignNode;
}

/* <declare list> -> (<int> | <float>) <ident> [= <expr>] {, <ident> [= <expr>] };
this handles multiple declarations in one line separated by commas */

Node* declare (){
     //check for identifier
    if (nextToken != IDENT) {
        cout << "Expected identifier \n";
    }

    string identName = lexeme;
    
    Node* left = new Node{IDENT, identName, nullptr, nullptr, currentType, ""}; //create node for identifier
    lex(); //consume identifier

    //optional assignment [= <expr>]
    if (nextToken == ASSIGN_OP) {
        string opLex = lexeme;
        lex(); //consume '='
        Node* right = expr(); //get expression
        Node* assignNode = new Node{ASSIGN_OP, opLex, left, right, "", currentType}; //create assignment node
        
        //store value in the symbol table
        string type = currentType;
        if(right -> token == INT_LIT || right -> token == FLOAT_LIT) {
            addSymbol(false, identName, type, right -> lexeme);
        }
        left = assignNode; //update left to be the assignment node
    } else {
        //no assignment, just declaration
        addSymbol(true, identName, currentType, "0"); //default value 0
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
        Node* parent = new Node{COMMA_OP, ",", left, newIdent, "", ""};
        left = parent; //update left to be the new parent node
    }

    cout << step++ << ". Exit <declare_list> \n";
    return left;
}

// <proj_start> --> <assign_list>  | <declare_list> 
Node* proj_start() {
    cout << step++ << ". Enter <proj_start> \n";

    Node* root = nullptr;

    if (nextToken == INT_KEY || nextToken == FLOAT_KEY) {
        root = declare_list();
    } else {
        root = assign_list();
    }

    cout << step++ << ". Exit <proj_start> \n";
    return root;
} 

void computeActualTypes(Node* node) {
    if (node == nullptr || node->actual_type.empty()) return;

    // Post-order traversal
    computeActualTypes(node->left);
    computeActualTypes(node->right);

    // Process current node
    if (node->token == ASSIGN_OP) {
        // Assignment: left child is identifier, right child is expression
        node->actual_type = node->left->actual_type; // type of assignment is type of identifier
    } 
    else if( node -> token == ADD_OP || node -> token == SUB_OP || 
             node -> token == MULT_OP || node -> token == DIV_OP) {
        // Arithmetic operations: determine type based on children
        string leftType = node->left->actual_type;
        string rightType = node->right->actual_type;

        if (leftType == "float" || rightType == "float") {
            node->actual_type = "float"; // if either operand is float, result is float
        } else {
            node->actual_type = "int"; // otherwise, result is int
        }
    }
    // Leaf nodes (IDENT, INT_LIT, FLOAT_LIT) already have actual_type set during parsing
}

/* After syntax tree (Post-Order) we have to compute Expected types (Pre-Order)*/
void computeExpectedTypes(Node* node, const string& expectedType) {
    if (node == nullptr) return;

    node->expected_type = expectedType; //I dont think I need this line

    // Pre-order processing
    if (node->token == ASSIGN_OP) {
        string idType = node->left->actual_type; //left child is identifier
        computeExpectedTypes(node->left, idType); //identifier expected type is its own type
    } else if( node -> token == ADD_OP || node -> token == SUB_OP || 
        node -> token == MULT_OP || node -> token == DIV_OP) {
            string childExpectedType = expectedType;
            computeExpectedTypes(node->left, childExpectedType);
            computeExpectedTypes(node->right, childExpectedType);
    } else { //for commas 
        computeExpectedTypes(node->left, expectedType);
        computeExpectedTypes(node->right, expectedType);
    }

    //leaf node
    if( node -> token == IDENT || node -> token == INT_LIT || node -> token == FLOAT_LIT) {
        node -> expected_type = expectedType;
        return;
    }

    computeExpectedTypes(node->left, expectedType);
    computeExpectedTypes(node->right, expectedType);
}

void generateIL(Node* node){
    if (node == nullptr) return;

    if(node-> token == ASSIGN_OP){
        cout << "push addr(" << node->left->lexeme << ")\n";

        // Recursively read RHS expression
        if(node -> right) generateIL(node -> right);

        //tyoe conversion is I.actualType != I.expectedType
        if (node -> actual_type != node -> expected_type) {
            if (node -> actual_type == "int" && node -> expected_type == "float") {
                cout << "itof " <<  "\n";
            } else if (node -> actual_type == "float" && node -> expected_type == "int") {
                cout << "ftoi " <<  "\n";
            }
        }

        cout << "assign \n";
        return;
    }

    // Post-order traversal
    if (node->left) generateIL(node->left);
    if (node->right) generateIL(node->right);

    // Process leaf node
    if (node->token == IDENT) {
        cout << "push " << node->lexeme << "\n";
    } else if (node->token == INT_LIT || node->token == FLOAT_LIT) {
        cout << "push " << node->lexeme << "\n";
    }
    
    // arithmetic ops
    if (node->token >= ADD_OP && node->token <= DIV_OP) {
        string leftType = node->left->actual_type;
        string rightType = node->right->actual_type;
        string resultType = node->actual_type;

        if(leftType == "int" && resultType == "float") {
            cout << "itof \n";
        }

        string iorf = (node->actual_type == "int") ? "i" : "f";
        cout << iorf;
        switch (node->token) {
            case ADD_OP:
                cout << "add \n";
                break;
            case SUB_OP:
                cout << "sub \n";
                break;
            case MULT_OP:
                cout << "mul \n";
                break;
            case DIV_OP:
                cout << "div \n";
                break;
        }
    }
}

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
        Node* statement_root = nullptr;
        while (nextToken == NEXT) { //every line, start new project
            lex(); //consume newline
        }
        statement_root = proj_start();
        if(statement_root == nullptr) {
            break; // Exit if no more statements
        }
        computeActualTypes(statement_root);
        computeExpectedTypes(statement_root, ""); //no expected type at root
        generateIL(statement_root);
    } while (nextToken != EOF);
   }
   inputFile.close();

   cout << "\n=== FINAL SYMBOL TABLE ===" << endl;
    for (int i = 0; i < symbolCount; i++) {
        string type = symbolTable[i].type;
        cout << symbolTable[i].name << "\tType: " << type << "\tAddress: " << symbolTable[i].address << "\tValue: " << (type == "int" ? (symbolTable[i].value.int_value) : (symbolTable[i].value.float_value)) << endl;
    }

   return 0;
}