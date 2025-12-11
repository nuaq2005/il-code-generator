#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

/* Global declarations for variables */
int charClass;
char lexeme [100];
int nextChar; /* nextChar must be int to hold EOF */
int lexLen;
int idx;
ifstream inputFile;

enum CharacterClass {
    LETTER, DIGIT, UNKNOWN, DOT, END_OF_FILE
};

enum TokenType {
    T_EOF, T_INT_CONST, T_FLOAT_CONST, T_IDENT,T_INT_KEY, T_FLOAT_KEY,
    T_ASSIGN, T_ADD, T_SUB, T_MULT, T_DIV,
    T_LEFT_PAREN, T_RIGHT_PAREN, T_COMMA, T_DOT, T_NEXT, T_UNKNOWN
}; 

TokenType list[50];
TokenType nextToken;
TokenType lookup(char ch);
TokenType lex();
void addChar();
struct Node {
    TokenType token;
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
Node* assign_list();
void declare_list();
Node* program();
string currentType; //to hold the type of the current declaration

/* Function declarations for lexical analyzer */
TokenType lex();
TokenType lookup(char ch);
void addChar();
void getChar();
void getNonBlank();

/* Stack for IL Generation */
struct ILStack {
    string items[100];
    int top;
    
    ILStack() : top(-1) {}
    
    void push(const string& item) {
        if (top < 99) {
            items[++top] = item;
        }
    }
    
    string pop() {
        if (top >= 0) {
            return items[top--];
        }
        return "";
    }
    
    string peek() {
        if (top >= 0) {
            return items[top];
        }
        return "";
    }
    
    bool isEmpty() {
        return top == -1;
    }
};

ILStack ilStack;

/* Symbol Table*/
struct Symbol {
    bool is_ident;
    string name;
    string type;
    struct {
        int int_value;
        float float_value;
    } rvalue;
    int lvalue;
};

void addSymbol(bool isIdent,const string &lexeme,const string &type, const string &value);
Symbol symbolTable[20]; //array to hold symbol table entries
int symbolCount = 0; //number of symbols in the table

/* Add symbol to symbol table */
void addSymbol (bool isIdent, const string &lexeme, const string &type,const string &value) {
    for(int i = 0; i < symbolCount; i++) {
        if (symbolTable[i].name == lexeme) {
            return; //symbol already exists
        }
    }   
    symbolTable[symbolCount].is_ident = isIdent;
    symbolTable[symbolCount].name = lexeme;
    symbolTable[symbolCount].type = type;
    if(type == "int") symbolTable[symbolCount].rvalue.int_value = stoi(value);
    else symbolTable[symbolCount].rvalue.float_value = stof(value);
    symbolTable[symbolCount].lvalue = rand(); //initialize lvalue
    symbolCount++;
}

void modifyType (const string &lexeme, const string &type) {
    for(int i = 0; i < symbolCount; i++) {
        if (symbolTable[i].name == lexeme) {
            symbolTable[i].type = type;
            return;
        }
    }   
    cout << "Undeclared identifier: " << lexeme << "\n";
}

void modifyValue (const string &lexeme, const string &value) {
    for(int i = 0; i < symbolCount; i++) {
        if (symbolTable[i].name == lexeme) {
            if(symbolTable[i].type == "int") {
                symbolTable[i].rvalue.int_value = stoi(value);
            } else if(symbolTable[i].type == "float") {
                symbolTable[i].rvalue.float_value = stof(value);
            }
            return;
        }
    }   
    cout << "Undeclared identifier: " << lexeme << "\n";
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
        case EOF:
            nextToken = T_EOF;
            lexeme[0] = 'E';
            lexeme[1] = 'O';
            lexeme[2] = 'F';
            lexeme[3] = 0;
            break;
        default:
            addChar();
            nextToken = T_UNKNOWN;
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
    } else if (nextChar == '.'){
        charClass = DOT;
    } else {
        charClass = UNKNOWN;
    }
}


   /* getNonBlank - a function to call getChar until it returns a non-blank character */
   void getNonBlank() {
    while (isspace(nextChar) && nextChar != '\n')
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
        case UNKNOWN: //if char is not letter or digit, it is either operator, parenthesis or unknown
             lookup(nextChar);
             if (nextToken != T_EOF)
             getChar(); //changed
             break;
        case EOF:
             nextToken = T_EOF;
                lexeme[0] = 'E';
                lexeme[1] = 'O';
                lexeme[2] = 'F';
                lexeme[3] = 0;
             break;
        }
        cout << "Next token is: " << nextToken << ", Next lexeme is " << lexeme << endl;
        return nextToken;
    }

    /* Is Next Function to peek ahead */
bool isNext(int tokenType) {
    int currentPos = inputFile.tellg(); // Save current position
    int savedCharClass = charClass;
    int savedNextChar = nextChar;
    int savedLexLen = lexLen;
    char savedLexeme[100];
    strcpy(savedLexeme, lexeme);
    TokenType savedNextToken = nextToken;

    lex(); // Get the next token
    bool isMatch = (nextToken == tokenType);
    // Restore saved state
    inputFile.clear(); // Clear any EOF flags
    inputFile.seekg(currentPos);
    charClass = savedCharClass;
    nextChar = savedNextChar;
    lexLen = savedLexLen;
    strcpy(lexeme, savedLexeme);
    nextToken = savedNextToken;

    return isMatch;
}

/* Intermediate Code Generation */
void generateIL(Node* node){
    if (!node) return; //skip declarations

    if(node-> token == T_ASSIGN){
        cout << "push addr(" << node->left->lexeme << ")\n";
        ilStack.push("addr(" + node->left->lexeme + ")");

        // Recursively read RHS expression
        if(node -> right) generateIL(node -> right);

        //tyoe conversion is I.actualType != I.expectedType
        if (node -> actual_type != node -> expected_type) {
            if (node -> actual_type == "int" && node -> expected_type == "float") {
                cout << "itof " <<  "\n";
                modifyType(node->lexeme, "float");
            } else if (node -> actual_type == "float" && node -> expected_type == "int") {
                cout << "ftoi " <<  "\n";
                modifyType(node->lexeme, "int");
            }
        }

        cout << "assign \n";
        return;
    }

    // Post-order traversal
    if (node->left) generateIL(node->left);
    if (node->right) generateIL(node->right);

    // Process leaf node
    if (node->token == T_IDENT) {
        cout << "push " << node->lexeme << "\n";
        ilStack.push(node->lexeme);

    } else if (node->token == T_INT_CONST || node->token == T_FLOAT_CONST) {
        cout << "push " << node->lexeme << "\n";
        ilStack.push(node->lexeme);
    }
    
    // arithmetic ops
    if (node->token >= T_ADD && node->token <= T_DIV) {
        string leftType = node->left->expected_type;
        string rightType = node->right->expected_type;
        string resultType = node->expected_type;

        if(leftType == "int" && resultType == "float") {
            cout << "itof \n";
        }

        string iorf = (node->expected_type == "int") ? "i" : "f";
        cout << iorf;
        string opd1, opd2;
        switch (node->token) {
            case T_ADD:
                cout << "add \n" << node-> right-> lexeme << " " << node-> left-> lexeme << "\n";
                opd1 = ilStack.peek();
                ilStack.pop();
                opd2 = ilStack.peek();
                ilStack.pop();
                ilStack.push( opd2 + "+" + opd1);
                break;
            case T_SUB:
                cout << "sub \n" << node-> right-> lexeme << " " << node-> left-> lexeme << "\n";
                opd1 = ilStack.peek();
                ilStack.pop();
                opd2 = ilStack.peek();
                ilStack.pop();
                ilStack.push( opd2 + "-" + opd1);
                break;
            case T_MULT:
                cout << "mul \n" << node-> right-> lexeme << " " << node-> left-> lexeme << "\n";
                opd1 = ilStack.peek();
                ilStack.pop();
                opd2 = ilStack.peek();
                ilStack.pop();
                ilStack.push( opd2 + "*" + opd1);
                break;
            case T_DIV:
                cout << "div \n" << node-> right-> lexeme << " " << node-> left-> lexeme << "\n";
                opd1 = ilStack.peek();
                ilStack.pop();
                opd2 = ilStack.peek();
                ilStack.pop();
                ilStack.push( opd2 + "/" + opd1);
                break;
        }
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
    cout << "determineActualType called for token: " << node->lexeme << "\n";
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

/*Syntax Analyzer*/
Node* factor (){
    Node* node = nullptr;
    //determine which path to take
    if (nextToken == T_IDENT || nextToken == T_INT_CONST || nextToken == T_FLOAT_CONST){
        node = new Node{nextToken, lexeme, nullptr, nullptr, "", ""}; //create node for factor
        
        //check symbol table for type
        if (nextToken == T_IDENT) {
            node->actual_type = lookupType(lexeme);
        } else if (nextToken == T_INT_CONST) {
            node->actual_type = "int";
        } else if (nextToken == T_FLOAT_CONST) {
            node->actual_type = "float";
        }

        lex(); //next token

    } else if (nextToken == T_LEFT_PAREN){
        lex(); //get past '('
        node = expr(); //go to expression
        if (nextToken == T_RIGHT_PAREN){
            lex(); //get past ')'
        } else {
            cout << "Syntax Error: Missing ')' \n";
        }
    } else {
        cout << "Syntax Error: Invalid Factor \n";
    }
    return node;
}

Node* term (){

    //first we must have a factor
    Node* left = factor();

    //then we can have 0 or more (* | /) factor
    while (nextToken == T_MULT || nextToken == T_DIV) {
        TokenType op = nextToken;
        string opLex = lexeme;
        lex(); //get the operator
       Node* right = factor(); //get the next factor
       
       Node* parent = new Node{op, opLex, left, right, "", ""};
       left = parent; //update left to be the new parent node
    }
 
    return left;
}

/* expr
 Parses strings in the language generated by the rule:
 <expr> -> <term> {(+ | -) <term>}
 */

Node* expr() {
 
    //print first term
    Node* left = term();

    //then we can have 0 or more (+ | - | *) term
    while (nextToken == T_ADD || nextToken == T_SUB) {
        TokenType op = nextToken;
        string opLex = lexeme;
        lex(); //get the operator
    
        Node* right = term(); //get the next term
        Node* parent = new Node{op, opLex, left, right, "", ""};
        left = parent; //update left to be the new parent node
    }

    return left;
}

// assign ->  <ident> = <expression>;
Node* assign() {

    //first lexeme must be identifier
    cout << list[idx - 2] << " " << T_IDENT << "\n";
    if(list[idx - 2] != T_IDENT){ 
        cout << "Syntax Error \n";
    }

    string name = lexeme;

    Node* left = new Node{T_IDENT, name, nullptr, nullptr, "", ""}; //create node for identifier
    left -> actual_type = lookupType(name); //get type from symbol table
    
    cout << list[idx - 2] << " " << T_IDENT << "\n";
    //second lexeme must be operator
    if(list [idx - 1] != T_ASSIGN){
        cout << "Syntax Error \n";
    }

    string opLex = lexeme;
  
    //third lexeme must be expression
    Node* right = expr(); //we enter expression, which takes us through the terms/factors/operations
    Node* root = new Node{T_ASSIGN, opLex, left, right, "", ""}; //create root node for assignment
    return root;
}

// <assign_list> -> {<ident> =} <assign>;
//edit tmrw
Node* assign_list(){

    //process zero or more {IDENT =}
    Node* left = nullptr;

    if (nextToken == T_IDENT) {
        string name = lexeme;
        Node* identNode = new Node{T_IDENT, name, nullptr, nullptr, "", ""}; //create node for identifier
        list[idx++] = nextToken; //store token in list
        lex(); //consume identifier

        if (nextToken != T_ASSIGN){
            cout << "Syntax Error: expected '=' \n";
        }

        string opLex = lexeme;
        list[idx++] = nextToken; //store token in list
        lex(); //consume '='
        Node* right = assign_list(); //process the next assignment

        Node* root = new Node{T_ASSIGN, opLex, identNode, right, "", ""}; //create assignment node
        left = root; //update left to be the assignment node

        return root;
    }

    Node* root = assign(); //process the final assignment
    return root;
}


void declare_list (){
    cout << "declare_list called \n";
    
    if (nextToken == T_NEXT) {
        lex();
        return;
    }

    if(nextToken != T_INT_KEY && nextToken != T_FLOAT_KEY) {
        cout << "Expected type keyword \n";
        return;
    }

    string type = (nextToken == T_INT_KEY) ? "int" : "float";
    lex();

    if(nextToken != T_IDENT) {
        cout << "Expected identifier \n";
        return;
    }

    string name = lexeme;
    lex();

    // Check for assignment (optional)
    if (nextToken == T_ASSIGN){
        lex(); // consume '='
        Node* right = expr();
        
        if(right->token == T_FLOAT_CONST || right->token == T_INT_CONST) {
            addSymbol(false, name, type, right->lexeme);
        } else {
            addSymbol(true, name, type, "0");
        }
        
        delete right; // Clean up - don't need the tree
    } else {
        addSymbol(true, name, type, "0");
    }

    // Handle comma-separated declarations
    while(nextToken == T_COMMA){
        lex();
        if(nextToken != T_IDENT) {
            cout << "Expected identifier after ',' \n";
            return;
        }
        
        string newname = lexeme;
        lex();

        if (nextToken == T_ASSIGN){
            lex(); // consume '='
            Node* right = expr();
            
            if(right->token == T_FLOAT_CONST || right->token == T_INT_CONST) {
                addSymbol(false, newname, type, right->lexeme);
            } else {
                addSymbol(true, newname, type, "0");
            }
            
            delete right; // Clean up
        } else {
            addSymbol(true, newname, type, "0");
        }
    }
    
    cout << "declare_list returning \n";
}

Node* program (){
    cout << "program called \n";
    
    Node* root = nullptr;
    Node* current = nullptr;

    // Process all declaration and assignment statements
    while(nextToken != T_EOF) {
        // Skip newlines
        while(nextToken == T_NEXT) {
            lex();
        }
        
        if (nextToken == T_EOF) break;
        
        Node* stmt = nullptr;
        
        // Check if it's a declaration or assignment
        if (nextToken == T_INT_KEY || nextToken == T_FLOAT_KEY) {
            declare_list();
        } else if (nextToken == T_IDENT) {
            stmt = assign_list();
        } else {
            cout << "Syntax Error: unexpected token\n";
            break;
        }
        
        if (stmt) {
            if (!root) {
                root = stmt;
                current = stmt;
            } else {
                // Chain statements together
                Node* connector = new Node{T_NEXT, "\\n", current, stmt, "", ""};
                current = connector;
                root = connector;
            }
        }
    }
    
    cout << "program returning \n";
    return root;
} 


int main () {
    
    inputFile.open("front.in");

    if (!inputFile.is_open()) {
        cout << "ERROR - cannot open front.in\n";
        return 1;
    }

    getChar();
    lex();
    Node* root = program();

    if (root){
        root-> expected_type = ""; //top-level has no expected type
        determineExpectedType(root);
        determineActualType(root);
        cout << "\n=== IL CODE ===\n";
        generateIL(root);
    }
    
    
    inputFile.close();

    cout << "\n=== SYMBOL TABLE ===\n";
    for (int i = 0; i < symbolCount; i++) {
        string type = symbolTable[i].type;
        cout << symbolTable[i].name << "\tType: " << type 
             << "\tAddress: " << symbolTable[i].lvalue << "\tValue: ";

        if (type == "int") {
            cout << symbolTable[i].rvalue.int_value << endl;
        } else {
            cout << symbolTable[i].rvalue.float_value << endl;
        }
    }
    return 0;
}
