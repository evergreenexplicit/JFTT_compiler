
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include <stack>
using namespace std;

//------------------ ENUMS ------------------
 
enum VariableType {SINGLE,NUM,ARR,ITER};
enum CommandType {GET,PUT,LOAD,STORE,LOADI,STOREI,ADD,SUB,SHIFT,INC,DEC,JUMP,JPOS,JZERO,JNEG,HALT};
enum ArrayType {DIRECT,PID};
//------------------ CURRENT STATE VARIABLES ------------------

long long cmdOffset = 0;
long long memoryOffset = 1;
long long implicitVarOffset = 0;
long long errors = 0;
//------------------ STRUCTS ------------------

typedef struct {
    std::string name;
    VariableType type;
    long long memoryIdx;
    bool init;
    long long start;
    long long size;
} Variable;

typedef struct{
    std::string name;
    long long start;
    long stop;
}ArrayBuilder;

typedef struct{
    CommandType commandType;
    long long value;
} Command; 
typedef struct{
    string name;
    string memoryVar;
    long long memoryIdx;
    ArrayType type;
} Array;
typedef struct {
    vector<long long> commands;
} Jump;
typedef struct {
    long long cmdOffset;
    string iteratorName;
    string targetName;
} Loop;
//------------------ COLLECTIONS ------------------
std::stack<Jump> jumps;
std::map<string, Variable> variables;
std::vector<ArrayBuilder> arrayBuilders;
std::vector<Command> commands;
std::stack<Array> arrays;
std::stack<Loop> loops;
string cmdToString(Command cmd);

//----------------- DIRECT PARSERS -----------------

    void __end();
    void __initValueOne();
    void __declareVariable(string name,int line);
    void __declareArray(string name,string start,string stop,int line);
    void __declareNumber(string name);
    void __read(string varName,int line);
    void __write(string varName,int line);
    void __assign(string varName,int line);
    void __endif();
    void __if_else();
    void __while_do();
    void __end_while();
    void __do_while();
    void __end_do();
    void __for(string i, string a, string b,int line);
    void __for_to(string i, string a, string b,int line);
    void __for_downto(string i, string a, string b,int line);
    void __end_for_to();
    void __end_for_downto();
    void __buildArrays();
    void __pid(string name, int line);
    void __arrayPid(string arrayName,string idxName,int line);
    void __arrayNumber(string arrayName,string idxName,int line);


//----------------- MEMORY & VARIABLES -----------------

    void clearBuffer();
    void createNumVar(Variable* var,string name);
    void createSingleVar(Variable* var,string name);
    string createImplicitVar(Variable* var);
    void declareNumber(string name);
    void initNumber(long long number);
    void createNumber(long long number);
    void loadBuffer(string varName);
    void storeBuffer(string varName);
    long long getVarAddr(string varName);
    void checkInitialize(string name,int line);
    void checkDeclare(string name,int line);
    void checkDeclAndInit(string name,int line);
    void handleArrays(string* a,string* b);
    string getArrayValue();
    void assignArray();
    void startOperationHandler(string* a,string* b,int line);
//----------------- INSTRUCTION FLOW -----------------
    void createJump(CommandType cmd);
    void createJump(CommandType cmd_1,CommandType cmd_2);
    void createLoop();
    void createForLoop(string iterator,string target);
    
//----------------- COMMANDS -----------------

    void addCommand(CommandType commandType, long long value);
    void addCommand(CommandType commandType);

//----------------- EXPRESSIONS AND CONDITIONS -----------------
    void __value(string a,int line);
    void __plus(string a,string b,int line);
    void __minus(string a,string b,int line);
    void __times(string a,string b,int line);
    void __div(string a,string b,int line);
    void __mod(string a,string b,int line);

    void __condEQ(string a, string b,int line);
    void __condNEQ(string a, string b,int line);
    void __condLE(string a, string b,int line);
    void __condGE(string a, string b,int line);
    void __condLEQ(string a, string b, int line);
    void __condGEQ(string a, string b,int line);


//----------------- ERROR HANDLING -----------------

    void error(string text, int line);
    void error(string text);

    bool isNumber(const std::string &str) ;
    void checkForToRange(string a,string b, int line);
    void checkForDowntoRange(string a,string b, int line);
    