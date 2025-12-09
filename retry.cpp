#include <iostream>
#include <fstream>
using namespace std;

/* Global declarations for variables */
int charClass;
char lexeme [100];
int nextChar; /* nextChar must be int to hold EOF */
int lexLen;
TokenType token;
TokenType nextToken;
ifstream inputFile;

enum CharacterClass {
    LETTER, DIGIT, UNKNOWN, DOT, END_OF_FILE
};

enum TokenType {
    T_EOF, T_INT_CONST, T_FLOAT_CONST, T_IDENT,T_INT_KEY, T_FLOAT_KEY,
    T_ASSIGN, T_ADD, T_SUB, T_MULT, T_DIV,
    T_LEFT_PAREN, T_RIGHT_PAREN, T_COMMA, T_NEXT
}; 

struct Node {
    TokenType token;
    string lexeme;
    Node* left;
    Node* right;
    string actual_type; //to hold the actual type after type checking
    string expected_type; //to hold the expected type during type checking
};

/* Function declarations for syntax analyzer */


/* Function declarations for lexical analyzer */
TokenType lex();
TokenType lookup(char ch);
void addChar();
void getChar();
void getNonBlank();

/* Symbol Table*/
struct Symbol {
    bool is_ident;
    string name;
    string type;
    union {
        int int_value;
        float float_value;
    } rvalue;
    int lvalue;
};

Symbol symbolTable[20]; //array to hold symbol table entries
int symbolCount = 0; //number of symbols in the table

/* Add symbol to symbol table */
void addSymbol (bool isIdent, const string &lexeme, const string &type) {
    for(int i = 0; i < symbolCount; i++) {
        if (symbolTable[i].name == lexeme) {
            return; //symbol already exists
        }
    }   
    symbolTable[symbolCount].is_ident = isIdent;
    symbolTable[symbolCount].name = lexeme;
    symbolTable[symbolCount].type = type;
    symbolTable[symbolCount].rvalue.int_value = 0; //initialize rvalue
    symbolTable[symbolCount].rvalue.float_value = 0.0; //initialize rvalue
    symbolTable[symbolCount].lvalue = rand(); //initialize lvalue
    symbolCount++;
}

/* Lexical Analyzer Simple*/

/* lookup - look up operators and paranthesis and return token */
TokenType lookup(char ch) {
    switch (ch) {
        case '(': 
            addChar();
            nextToken = T_LEFT_PAREN;
            break;
        case ')':
            addChar();
            nextToken = T_RIGHT_PAREN;
            break;
        case '+':
            addChar();
            nextToken = T_ADD;
            break;
        case '-':
            addChar();
                nextToken = T_SUB;
            break;
        case '*':
            addChar();
            nextToken = T_MULT;
            break;
        case '/':
            addChar();
            nextToken = T_DIV;
            break;
        case ',':
            addChar();
            nextToken = T_COMMA;
            break;
        case '\n':
            addChar();
            nextToken = T_NEXT;
            break;
        case '=':
            addChar();
            nextToken = T_ASSIGN;
            break;
        case '.':
            addChar();
            nextToken = DOT;
            break;
        default:
            addChar();
            nextToken = T_EOF;
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
    } else {
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

    TokenType lex() {
        lexLen = 0;
        getNonBlank();
        switch (charClass) {
            case LETTER: 
                addChar();
                getChar();
                while (charClass == LETTER || charClass == DIGIT) {
                    addChar();
                    getChar();
                } //loops through letters and digits to form the full lexeme
                //if first lex is a letter, it goes through if statements to see if it is a keyword, if not it is treated as identifier
            if (isKeyWord(lexeme, "int")){
                nextToken = T_INT_KEY;
            } else if (isKeyWord(lexeme, "float")) {
                nextToken = T_FLOAT_KEY;
            } else {
                nextToken = T_IDENT; //if not a keyword, it is an identifier
            }
            break;
        case DIGIT: {
             bool isFloat = false;
             addChar();
             getChar();
             while (charClass == DIGIT || charClass == DOT ) { //loops through digits to form the full lexeme
                if (charClass == DOT) isFloat = true;
                addChar();
                getChar();
             } 
             nextToken = isFloat ? T_FLOAT_CONST : T_INT_CONST; //if decimal point found, it is float literal
             break;
            }
        }
        cout << "Next token is: " << nextToken << ", Next lexeme is " << lexeme << endl;
    }

/* Node */
struct Node {
    TokenType token;
    string lexeme;
    Node* left;
    Node* right;
    string actual_type; //to hold the actual type after type checking
    string expected_type; //to hold the expected type during type checking
};

/* IL CODE GENERATOR */
void generateIL(Node* node) {
    if (!node) return;

    //post-order traversal
    generateIL(node->left);
    generateIL(node->right);

    //generate IL code based on node type
    string iorf = (node->actual_type == "int") ? "i" : "f";
        cout << iorf;
        switch (node->token) {
            case T_ADD:
                cout << "add \n";
                break;
            case T_SUB:
                cout << "sub \n";
                break;
            case T_MULT:
                cout << "mul \n";
                break;
            case T_DIV:
                cout << "div \n";
                break;
            case T_ASSIGN:
                cout << "assign \n";
                break;
            case T_IDENT:
            case T_INT_CONST:
            case T_FLOAT_CONST:
                cout << "push " << node->lexeme << "\n";
                break;
            default:
                break;
        }
}

/* Expected / Computed Type (pre-order)*/
void determineExpectedType(Node* node) {
    if (node == nullptr) return;

    //pre-order traversal
    if (node->token == T_ADD || node->token == T_SUB || node->token == T_MULT || node->token == T_DIV) {
        //set expected type for children
        node->left->expected_type = node->expected_type;
        node->right->expected_type = node->expected_type;
    } else if (node->token == T_ASSIGN) {
        //for assignment, left child expected type is the type of the identifier
        node->left->expected_type = node->left->actual_type;
        //right child expected type is the same as left child's expected type
        node->right->expected_type = node->left->expected_type;
    }

    determineExpectedType (node->left);
    determineExpectedType (node->right);  
}

/* Actual Type (post-order)*/
void determineActualType(Node* node) {
    if (node == nullptr) return;

    //post-order traversal
    determineActualType (node->left);
    determineActualType (node->right);

    //determine actual type based on children
    if (node->token == T_ADD || node->token == T_SUB || node->token == T_MULT || node->token == T_DIV) {
        string leftType = node->left ? node->left->actual_type : "";
        string rightType = node->right ? node->right->actual_type : "";

        if (leftType == "float" || rightType == "float") {
            node->actual_type = "float";
        } else if (leftType == "int" && rightType == "int") {
            node->actual_type = "int";
        } else {
            node->actual_type = "unknown";
            cout << "Type Error: incompatible types for operator " << node->lexeme << "\n";
        }
    } else if (node->token == T_INT_CONST) {
        node->actual_type = "int";
    } else if (node->token == T_FLOAT_CONST) {
        node->actual_type = "float";
    } else if (node->token == T_IDENT) {
        //lookup type from symbol table
        for (int i = 0; i < symbolCount; i++) {
            if (symbolTable[i].name == node->lexeme) {
                node->actual_type = symbolTable[i].type;
                return;
            }
        }
        node->actual_type = "unknown";
        cout << "Undeclared identifier: " << node->lexeme << "\n";
    }
}

/* Parse Tree*/
// <program> --> <declare_list> {<assign_list>  | <declare_list> }
// <declare_list> --> (int | float) <ident> [= <expr>] {, <ident> [= <expr>] }
// <assign_list> --> {<ident> = } <assign> (eg. l = k = i + j)
// <assign> --> <ident> = <assign>
// <expr> --> <term> {(+ | -) <term>}
// <term> --> <factor> {(* | /) <factor>}
// <factor> --> <ident> | <int_lit> | <float_lit> | ( <expr> )

Node* factor (){
    Node *node = nullptr;    
    if(nextToken == T_IDENT || nextToken == T_INT_CONST || nextToken == T_FLOAT_CONST){
        Node* node = new Node{nextToken, lexeme, nullptr, nullptr, "", ""}; //create node for factor
        lex(); //next token
        return node;
    } else if (nextToken == T_LEFT_PAREN){
        lex(); //get past '('
        Node* node = expr(); //go to expression
        if (nextToken == T_RIGHT_PAREN){
            lex(); //get past ')'
        } else {
            cout << "Syntax Error: Missing ')' \n";
        }
        return node;
    } else {
        cout << "Syntax Error: Invalid Factor \n";
        return nullptr;
    }
}

Node* term (){
    Node* left = factor();
    lex(); //get the operator
    while(nextToken == T_MULT || nextToken == T_DIV) {
        TokenType op = nextToken;
        string opLex = lexeme;
        lex(); //get the operator
    
        Node* right = factor(); //get the next factor
        Node* parent = new Node{op, opLex, left, right, "", ""};
        left = parent; //update left to be the new parent node
    }

    return left;
}

Node* expr (){
    //print first term
    Node* left = term();

    //then we can have 0 or more (+ | - | *) term
    while (nextToken == T_ADD|| nextToken == T_SUB) {
        TokenType op = nextToken;
        string opLex = lexeme;
        lex(); //get the operator
    
        Node* right = term(); //get the next term
        Node* parent = new Node{op, opLex, left, right, "", ""};
        left = parent; //update left to be the new parent node
    }

    return left;
}

Node* assign (){
    if(nextToken != T_IDENT) {
        cout << "Syntax Error: expected identifier \n";
    }

    string name = lexeme;
    Node* left = new Node{T_IDENT, name, nullptr, nullptr, "", ""};
    lex(); //consume identifier

    if (nextToken != T_ASSIGN){
        cout << "Syntax Error: expected '=' \n";
    }
    string op = lexeme;
    lex(); //consume '='
    Node* right = assign(); //process the next assignment
    Node* root = new Node{T_ASSIGN, op, left, right, "", ""}; //create root node for assignment
    return root;
}

Node* assign_list (){
    if (nextToken == T_NEXT) {
        lex(); //consume newline
        return nullptr; //empty declaration list
    }
    while (nextToken == T_IDENT){
        string name = lexeme;
        lex(); //consume identifier
        if (nextToken != T_ASSIGN){
            cout << "Syntax Error: expected '=' \n";
        }
        string op = lexeme;
        lex(); //consume '='
        Node* right = assign(); //process the next assignment
    }

    Node* root = assign(); //process the final assignment
    return root;
}

Node* declare_list (){
   if (nextToken == T_NEXT) {
        lex(); //consume newline
        return nullptr; //empty declaration list
   }

   if(nextToken != T_INT_KEY && nextToken != T_FLOAT_KEY) {
        cout << "Expected type keyword \n";
    }
    string type = nextToken == T_INT_KEY ? "int" : "float";
    lex();

    //process first identifier (and optional assignment)
    if(nextToken != T_IDENT) {
        cout << "Expected identifier \n";
    }
    string name = lexeme;
    Node* left = new Node{T_IDENT, name, nullptr, nullptr, type, ""}; //create node for identifier
    lex(); //consume identifier

    //check for assignment (optional)
    if (nextToken == T_ASSIGN){
        string op = lexeme;
        lex(); //consume '='
        Node* right = expr();
        Node* assignNode = new Node{T_ASSIGN, op, left, right, "", ""}; //create assignment node, left is ident, right is expr
    }

    //add symbol to symbol table
    addSymbol (true, name, type);

}

Node* program (){
    
    declare_list();
    while(nextToken != T_EOF) {
        if (nextToken == T_INT_KEY || nextToken == T_FLOAT_KEY) {
            declare_list();
        } else {
            assign_list();
        }
    }
}


void main () {
    /* Open input file and process contents */
   inputFile.open("front.in");

   if (!inputFile.is_open()) {
      cout << "ERROR - cannot open front.in \n";
      return;
   } else {
    getChar();
    lex();
    program();
   }

   return;
}