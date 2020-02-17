
#include <iostream>
#include <vector>
#include <utility>
#include <map>
#include "CodeBuilder.hpp"
using namespace std;

string cmdToString(Command cmd){
    switch(cmd.commandType){
        case GET: return string("GET");
        case PUT: return string("PUT");
        case LOAD: return string("LOAD ")+ to_string(cmd.value);
        case STORE: return string("STORE ")+ to_string(cmd.value);
        case LOADI: return string("LOADI ")+ to_string(cmd.value);
        case STOREI: return string("STOREI ")+ to_string(cmd.value);
        case ADD: return string("ADD ")+ to_string(cmd.value);
        case SUB: return string("SUB ")+ to_string(cmd.value);
        case SHIFT: return string("SHIFT ")+ to_string(cmd.value);
        case INC: return string("INC");
        case DEC: return string("DEC");
        case JUMP: return string("JUMP ")+ to_string(cmd.value);
        case JPOS: return string("JPOS ")+ to_string(cmd.value);
        case JZERO: return string("JZERO ")+ to_string(cmd.value);
        case JNEG: return string("JNEG ")+ to_string(cmd.value);
        case HALT: return string("HALT");
    }
}

//----------------- DIRECT PARSERS -----------------


    void __end(){
        addCommand(HALT);
    }
    void __initValueOne(){
        string name = "1";
        Variable var1;
        createNumVar(&var1,name);
        variables.insert(make_pair(name,var1));
        addCommand(SUB,0);
        addCommand(INC);
        storeBuffer(name);
    }
    void __declareVariable(string name,int line){
        if(variables.find(name) != variables.end()){
            error("Already declared: " +name,line);
            exit(1);
        }    
        else{
            Variable var;
            createSingleVar(&var,name);
            
        }

    }

    
    void __declareArray(string name,string start,string stop,int line){
        if(variables.find(name) != variables.end()){
            error("Already declared: " +name,line);
            exit(1);
        }else if (stoll(start) > stoll(stop)){
            error("Incorrect array range: ("+start+":"+stop+")",line);
            exit(1);
        } else {
            ArrayBuilder arr;
            arr.name = name;
            arr.start = stoll(start);
            arr.stop = stoll(stop);
            arrayBuilders.push_back(arr);
            //adding arrays at the end for optimal memory handling (I hope)
        }
    }
    void __declareNumber(string name){
        declareNumber(name);

    }
    void __read(string varName,int line){

        checkDeclare(varName,line);
        if(variables[varName].type == ITER){
            error("Variable is an iterator: "+varName,line);
            exit(1);
        }


        addCommand(GET);
        if(variables[varName].type == ARR){
            assignArray();
            return;
        }

        storeBuffer(varName);
        

    }
    void __write(string varName,int line){

        checkDeclAndInit(varName,line);

         if(variables[varName].type == ARR){
            varName = getArrayValue();
        } 
        loadBuffer(varName);
         
       
        addCommand(PUT);
    }
   void __assign(string varName,int line){
        checkDeclare(varName,line);
        if(variables[varName].type == ARR){
            assignArray();
            return;
        }
        if(variables[varName].type == ITER){
            error("Variable is an iterator: "+varName,line);
            exit(1);
        }
        
        storeBuffer(varName);
        
    }

    void __endif(){
        
        for(long long idx :jumps.top().commands){
            commands[idx].value = cmdOffset;
            
        }
        jumps.pop();
    }
    void __if_else(){
        
        for(long long idx :jumps.top().commands){
            commands[idx].value = cmdOffset+1;
            
        }
        
        jumps.pop();
        createJump(JUMP);
    }
    void __while_do(){
        createLoop();
    }
    void __end_while(){
        addCommand(JUMP,loops.top().cmdOffset);
        loops.pop();
        for(long long idx :jumps.top().commands){
            commands[idx].value = cmdOffset;
        }
        jumps.pop();
    }
    void __do_while(){
        createLoop();
    }
    void __end_do(){

        addCommand(JUMP,loops.top().cmdOffset);
        loops.pop();
        for(long long idx :jumps.top().commands){
            commands[idx].value = cmdOffset;
        }
        jumps.pop();

    }
    void __for(string i, string a, string b,int line){

        if(variables.find(i) != variables.end()){
            error("Loop iterator already declared: "+i,line);
            exit(1);
        }    
            
        startOperationHandler(&a,&b,line);
            
        Variable iterator;
        iterator.name = i;
        iterator.size = 1;
        iterator.memoryIdx = memoryOffset++;
        iterator.type = ITER;
        iterator.init = true;
        variables.insert(make_pair(iterator.name,iterator));

        Variable target;
        createImplicitVar(&target);

        loadBuffer(a);
        storeBuffer(iterator.name);
        loadBuffer(b);
        storeBuffer(target.name);

        createForLoop(iterator.name,target.name);

        addCommand(LOAD,getVarAddr(iterator.name));
        addCommand(SUB,getVarAddr(target.name));
    }
    void __for_to(string i, string a, string b,int line){
        checkForToRange(a,b,line);
        __for(i,a,b,line);
        createJump(JPOS);
    }
    void __for_downto(string i, string a, string b,int line){
        checkForDowntoRange(a,b,line);
       __for(i,a,b,line);
        createJump(JNEG);
        
    }
    void __end_for_to(){  // load inc store jump
        Loop loop = loops.top();
        loops.pop();
        loadBuffer(loop.iteratorName);
        addCommand(INC);
        addCommand(STORE,getVarAddr(loop.iteratorName));
        addCommand(JUMP, loop.cmdOffset);
        
        for(long long idx :jumps.top().commands){
            commands[idx].value = cmdOffset;
        }
        jumps.pop();

        variables.erase(loop.iteratorName);

    }
    void __end_for_downto(){ // load dec store jump
       Loop loop = loops.top();
        loops.pop();
        loadBuffer(loop.iteratorName);
        addCommand(DEC);
        addCommand(STORE,getVarAddr(loop.iteratorName));
        addCommand(JUMP, loop.cmdOffset);
        
        for(long long idx :jumps.top().commands){
            commands[idx].value = cmdOffset;
        }
        jumps.pop();

        variables.erase(loop.iteratorName);

    }
    void __buildArrays(){
        for(const ArrayBuilder & builder: arrayBuilders){
            Variable array;
            array.init = false;
            array.memoryIdx = memoryOffset;
            array.name = builder.name;
            array.size = builder.stop - builder.start + 1; //a(1:3) is 3 elements, not 2
            array.start = builder.start;
            array.type = ARR;

            memoryOffset +=array.size; 
            variables.insert(make_pair(array.name,array));

        }

    }
    void __pid(string name, int line){
        if(variables.find(name) == variables.end())
            return;
        if (variables[name].type == ARR){
            error("Variable is an array: " + name,line);
            exit(1);
        }

        
    }

    void __arrayPid(string arrayName,string idxName,int line){
        checkDeclare(arrayName,line);
        checkDeclAndInit(idxName,line);
        if(variables[arrayName].type != ARR){
            error("Variable is not an array: " + arrayName,line);
            exit(1);
        }
        Variable var;
        createImplicitVar(&var);

        long long offset = variables[arrayName].memoryIdx 
                            - variables[arrayName].start;
        
        declareNumber(to_string(offset));
        addCommand(LOAD,getVarAddr(to_string(offset)));
        addCommand(ADD,getVarAddr(idxName));
        storeBuffer(var.name);
        
        Array array;
        array.type = PID; 
        array.name = arrayName;
        array.memoryVar = var.name;
        arrays.push(array);

    }
    void __arrayNumber(string arrayName,string idxName,int line){
        checkDeclare(arrayName,line);
        if(variables[arrayName].type != ARR){
            error("Variable is not an array: " + arrayName,line);
            exit(1);
        }     
        if(stoll(idxName) < variables[arrayName].start
        ||  stoll(idxName) > variables[arrayName].start + variables[arrayName].size - 1){
            error("Incorrect array index: "+ idxName + "in array: "+ arrayName,line);
            exit(1);
        }
 

        if(variables.find(arrayName + "$" + idxName) == variables.end()){
            long long offset = variables[arrayName].memoryIdx 
                        + stoll(idxName)
                        - variables[arrayName].start;

            Variable var;
            var.name = arrayName + "$" + idxName;
            var.size = 1;
            var.init = true;
            var.type = SINGLE;
            var.memoryIdx = offset;
            variables.insert(make_pair(var.name,var));
        }
        Array array;
        array.type = DIRECT;
        array.name = arrayName;
        array.memoryVar = arrayName + "$" + idxName;
        arrays.push(array);
        
    }


//----------------- MEMORY & VARIABLES -----------------

    void clearBuffer(){
        addCommand(SUB,0);
    }
    void createNumVar(Variable* var,string name){
        var->name=name;
        var->size = 1;
        var->memoryIdx = memoryOffset++;
        var->init = false;
        var->type = NUM;
        
    }
    void createSingleVar(Variable* var,string name){
        var->name=name;
        var->size = 1;
        var->memoryIdx = memoryOffset++;
        var->init = false;
        var->type = SINGLE;  
        variables.insert(make_pair(var->name,*var));
    }
    string createImplicitVar(Variable* var){
        var->name="$Implicit" + to_string(implicitVarOffset++);
        var->size = 1;
        var->memoryIdx = memoryOffset++;
        var->init = false;
        var->type = SINGLE;
        variables.insert(make_pair(var->name,*var));
        return var->name;
    }

    void declareNumber(string name){
        if(name == "1")
            return;
        if(variables.find(name) == variables.end()){
            Variable var;
            createNumVar(&var,name);
            var.init = true;
            variables.insert(make_pair(name,var));
        }
        initNumber(stoll(name));
        storeBuffer(name); 
    }
    void initNumber(long long number){
        addCommand(SUB, 0);
        createNumber(number);
    }

    void createNumber(long long number){
        if(number > 24 || number < -24) {
            createNumber(number / 2);
            addCommand(SHIFT,getVarAddr("1"));     
            if(number % 2 != 0){
                if(number > 0)
                    addCommand(INC);
                else
                    addCommand(DEC);
            }               
        } else {
            if(number > 0)
                for(long long i =0;i<number;i++)
                    addCommand(INC); 
            else
                for(long long i =0;i>number;i--){
                    addCommand(DEC); 
                    
                }               
        }
    }

    void loadBuffer(string varName){
        addCommand(LOAD,variables[varName].memoryIdx);
    }

    void storeBuffer(string varName){
        addCommand(STORE,variables[varName].memoryIdx);
        variables[varName].init = true;
    }

    long long getVarAddr(string varName){
        return variables[varName].memoryIdx;
    }

    void checkInitialize(string name,int line){
        if(variables[name].type == ARR)
            return; 
        if(variables[name].init == false){
            error("WARN: variable may not be initialized: " +name,line);
        }
    }
    void checkDeclare(string name,int line){
        if(variables.find(name) == variables.end()){
            error("Variable not declared: " + name,line);
            exit(1);
        }
    

    }
    void checkDeclAndInit(string name,int line){
        checkDeclare(name,line);
        checkInitialize(name,line);
    }
    void handleArrays(string* a,string* b){
        if(variables[*b].type == ARR)
            *b = getArrayValue();
        if(variables[*a].type == ARR)
            *a = getArrayValue();
    }
    string getArrayValue(){ 
        if(arrays.top().type == DIRECT){
            string name = arrays.top().memoryVar;
            arrays.pop();
            return name;
        }
        string varWithAddr = arrays.top().memoryVar;
             
        arrays.pop();
        addCommand(LOADI,getVarAddr(varWithAddr));
        Variable var;
        createImplicitVar(&var);

        addCommand(STORE,getVarAddr(var.name));
        return var.name;     
        
    }
    void assignArray(){
        if(arrays.top().type == DIRECT){
            string directAddr = arrays.top().memoryVar;
            arrays.pop();
            addCommand(STORE,getVarAddr(directAddr));      
            return;
        }
        string varWithAddr = arrays.top().memoryVar;
        arrays.pop();
        addCommand(STOREI,getVarAddr(varWithAddr));
        
        
    }
    void startOperationHandler(string* a,string* b,int line){
        checkDeclAndInit(*a,line);
        checkDeclAndInit(*b,line);
            
        handleArrays(a,b);
    }
//----------------- INSTRUCTION FLOW -----------------
    void createJump(CommandType cmd){
        Jump jump;
        addCommand(cmd);
        jump.commands.push_back( cmdOffset-1 );
        jumps.push(jump);
    }
    void createJump(CommandType cmd_1,CommandType cmd_2){
        Jump jump;
        addCommand(cmd_1);
        jump.commands.push_back( cmdOffset-1 );
        addCommand(cmd_2);
        jump.commands.push_back( cmdOffset-1 );
        jumps.push(jump);
    }
    void createLoop(){
        Loop loop;
        loop.cmdOffset = cmdOffset;
        loops.push(loop);
    }
    void createForLoop(string iterator,string target){
        Loop loop;
        loop.cmdOffset = cmdOffset;
        loop.iteratorName = iterator;
        loop.targetName = target;
        loops.push(loop);
    }
    



//----------------- COMMANDS -----------------

    void addCommand(CommandType commandType, long long value){
        Command cmd;
        cmd.commandType = commandType;
        cmd.value = value;
        commands.push_back(cmd);
        cmdOffset++;
    }
    void addCommand(CommandType commandType){
        Command cmd;
        cmd.commandType = commandType;
        commands.push_back(cmd);
        cmdOffset++;
    }

//----------------- EXPRESSIONS AND CONDITIONS -----------------
    void __value(string a,int line){ 
        checkDeclAndInit(a,line);
            
        if(variables[a].type == ARR)
            a = getArrayValue();

        addCommand(LOAD,getVarAddr(a));
        
    }
    void __plus(string a,string b,int line){ 

        startOperationHandler(&a,&b,line);


        loadBuffer(a);
        addCommand(ADD,getVarAddr(b));
        
    }
    void __minus(string a,string b,int line){
        startOperationHandler(&a,&b,line);

        loadBuffer(a);
        addCommand(SUB,getVarAddr(b));
        
    }
    void __times(string a,string b,int line){

        startOperationHandler(&a,&b,line);

        Variable var1;
        createImplicitVar(&var1);
        Variable var2;
        createImplicitVar(&var2);

        Variable varResult;
        createImplicitVar(&varResult);
        Variable varSign;
        createImplicitVar(&varSign);
        declareNumber("-1");

        //handling negative numers
        addCommand(SUB,0); 
        storeBuffer(varSign.name);
        storeBuffer(varResult.name);
        addCommand(LOAD,getVarAddr(a));
        addCommand(JPOS,cmdOffset+6);
        addCommand(SUB,getVarAddr(a));
        addCommand(INC);
        addCommand(STORE,getVarAddr(varSign.name));
        addCommand(DEC);
        addCommand(SUB,getVarAddr(a));
        addCommand(STORE,getVarAddr(var1.name));

        addCommand(LOAD,getVarAddr(b));
        addCommand(JPOS,cmdOffset+9);
        addCommand(SUB,getVarAddr(b));
        addCommand(SUB,getVarAddr(b));
        addCommand(STORE,getVarAddr(var2.name));
        addCommand(SUB,0);
        addCommand(DEC);
        addCommand(ADD,getVarAddr(varSign.name));
        addCommand(STORE,getVarAddr(varSign.name));
        addCommand(JUMP,cmdOffset+2);
        addCommand(STORE,getVarAddr(var2.name));
       


        //multiplying algorithm
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(JZERO,cmdOffset+15); 
        addCommand(SHIFT,getVarAddr("-1")); 
        addCommand(SHIFT,getVarAddr("1"));
        addCommand(SUB,getVarAddr(var1.name));
        addCommand(JZERO, cmdOffset+4); 
        addCommand(LOAD,getVarAddr(var2.name));
        addCommand(ADD,getVarAddr(varResult.name));
        addCommand(STORE,getVarAddr(varResult.name));
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(SHIFT, getVarAddr("-1"));
        addCommand(STORE,getVarAddr(var1.name));
        addCommand(LOAD,getVarAddr(var2.name));
        addCommand(SHIFT,getVarAddr("1"));
        addCommand(STORE,getVarAddr(var2.name));
        addCommand(JUMP,cmdOffset-15);
        addCommand(LOAD,getVarAddr(varSign.name));
        addCommand(JZERO,cmdOffset+4);
        addCommand(SUB,0);
        addCommand(SUB,getVarAddr(varResult.name));
        addCommand(JUMP,cmdOffset+2);
        addCommand(LOAD,getVarAddr(varResult.name));
    }
    void __div(string a,string b,int line){ 
       startOperationHandler(&a,&b,line);



        Variable var1;
        createImplicitVar(&var1);
        Variable var2;
        createImplicitVar(&var2);

        Variable varDividend;
        createImplicitVar(&varDividend);

        Variable varResult;
        createImplicitVar(&varResult);
        Variable varSign;
        createImplicitVar(&varSign);
        Variable varMultiple;
        createImplicitVar(&varMultiple);
        declareNumber("-1");
    
        //handling signs
        addCommand(SUB,0);
        storeBuffer(varSign.name);
        storeBuffer(varResult.name);
        addCommand(INC);
        addCommand(STORE,getVarAddr(varMultiple.name));
        addCommand(LOAD,getVarAddr(a));
        addCommand(JPOS,cmdOffset+6);
        addCommand(SUB,getVarAddr(a));
        addCommand(INC);
        addCommand(STORE,getVarAddr(varSign.name));
        addCommand(DEC);
        addCommand(SUB,getVarAddr(a));
        addCommand(STORE,getVarAddr(var1.name));
        addCommand(STORE,getVarAddr(varDividend.name));

        addCommand(LOAD,getVarAddr(b));
        addCommand(JZERO,cmdOffset+51);
        addCommand(JPOS,cmdOffset+9);
        addCommand(SUB,getVarAddr(b));
        addCommand(SUB,getVarAddr(b));
        addCommand(STORE,getVarAddr(var2.name));
        addCommand(SUB,0);
        addCommand(DEC);
        addCommand(ADD,getVarAddr(varSign.name));
        addCommand(STORE,getVarAddr(varSign.name));
        addCommand(JUMP,cmdOffset+2);
        addCommand(STORE,getVarAddr(var2.name));
       
       //div algorithm
        addCommand(LOAD,getVarAddr(var2.name));
        addCommand(SUB,getVarAddr(varDividend.name));
        addCommand(INC);
        addCommand(JPOS,cmdOffset+8);
        addCommand(LOAD,getVarAddr(var2.name));
        addCommand(SHIFT,getVarAddr("1"));
        addCommand(STORE,getVarAddr(var2.name));
        addCommand(LOAD,getVarAddr(varMultiple.name));
        addCommand(SHIFT,getVarAddr("1"));
        addCommand(STORE,getVarAddr(varMultiple.name));
        addCommand(JUMP,cmdOffset-10);
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(SUB,getVarAddr(var2.name));
        addCommand(JNEG,cmdOffset+7);
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(SUB,getVarAddr(var2.name));
        addCommand(STORE,getVarAddr(var1.name));
        addCommand(LOAD,getVarAddr(varResult.name));
        addCommand(ADD,getVarAddr(varMultiple.name));
        addCommand(STORE,getVarAddr(varResult.name));
        addCommand(LOAD,getVarAddr(var2.name));
        addCommand(SHIFT,getVarAddr("-1"));
        addCommand(STORE,getVarAddr(var2.name));
        addCommand(LOAD,getVarAddr(varMultiple.name));
        addCommand(SHIFT,getVarAddr("-1"));
        addCommand(STORE,getVarAddr(varMultiple.name));
        addCommand(JPOS,cmdOffset-15);
        addCommand(JNEG,cmdOffset-16);

        //loading result, considering edge cases with negative numbers
        addCommand(LOAD,getVarAddr(varSign.name));
        addCommand(JZERO,cmdOffset+10);
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(JZERO,cmdOffset+5);
        addCommand(SUB,0);
        addCommand(SUB,getVarAddr(varResult.name));
        addCommand(DEC);
        addCommand(JUMP,cmdOffset+5);
        addCommand(SUB,0);
        addCommand(SUB,getVarAddr(varResult.name));
        addCommand(JUMP,cmdOffset+2);
        addCommand(LOAD,getVarAddr(varResult.name));
        




        
        
    }
    void __mod(string a,string b,int line){ 
        startOperationHandler(&a,&b,line);



        Variable var1;
        createImplicitVar(&var1);
        Variable var2;
        createImplicitVar(&var2);
        Variable varDividend;
        createImplicitVar(&varDividend);
        Variable varResult;
        createImplicitVar(&varResult);
        Variable varSign1;
        createImplicitVar(&varSign1);
        Variable varSign2;
        createImplicitVar(&varSign2);
        Variable varMultiple;
        createImplicitVar(&varMultiple);
        declareNumber("-1");

        //handling signs
        addCommand(SUB,0);
        storeBuffer(varSign1.name);
        storeBuffer(varSign2.name);
        storeBuffer(varResult.name);
        addCommand(INC);
        addCommand(STORE,getVarAddr(varMultiple.name));
        addCommand(LOAD,getVarAddr(a));
        addCommand(JPOS,cmdOffset+6);
        addCommand(SUB,getVarAddr(a));
        addCommand(INC);
        addCommand(STORE,getVarAddr(varSign1.name));
        addCommand(DEC);
        addCommand(SUB,getVarAddr(a));
        addCommand(STORE,getVarAddr(var1.name));
        addCommand(STORE,getVarAddr(varDividend.name));

        addCommand(LOAD,getVarAddr(b));
        addCommand(JZERO,cmdOffset+57);
        addCommand(JPOS,cmdOffset+6);
        addCommand(SUB,getVarAddr(b));
        addCommand(INC);
        addCommand(STORE,getVarAddr(varSign2.name));
        addCommand(DEC);
        addCommand(SUB,getVarAddr(b));
        addCommand(STORE,getVarAddr(var2.name));
    

        //mod algorithm
        addCommand(LOAD,getVarAddr(var2.name));
        addCommand(SUB,getVarAddr(varDividend.name));
        addCommand(INC);
        addCommand(JPOS,cmdOffset+8);
        addCommand(LOAD,getVarAddr(var2.name));
        addCommand(SHIFT,getVarAddr("1"));
        addCommand(STORE,getVarAddr(var2.name));
        addCommand(LOAD,getVarAddr(varMultiple.name));
        addCommand(SHIFT,getVarAddr("1"));
        addCommand(STORE,getVarAddr(varMultiple.name));
        addCommand(JUMP,cmdOffset-10);
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(SUB,getVarAddr(var2.name));
        addCommand(JNEG,cmdOffset+7);
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(SUB,getVarAddr(var2.name));
        addCommand(STORE,getVarAddr(var1.name));
        addCommand(LOAD,getVarAddr(varResult.name));
        addCommand(ADD,getVarAddr(varMultiple.name));
        addCommand(STORE,getVarAddr(varResult.name));
        addCommand(LOAD,getVarAddr(var2.name));
        addCommand(SHIFT,getVarAddr("-1"));
        addCommand(STORE,getVarAddr(var2.name));
        addCommand(LOAD,getVarAddr(varMultiple.name));
        addCommand(SHIFT,getVarAddr("-1"));
        addCommand(STORE,getVarAddr(varMultiple.name));
        addCommand(JPOS,cmdOffset-15);
        addCommand(JNEG,cmdOffset-16);


        //handling mod edge cases and getting result
        //if rest = 0 finish
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(JZERO,cmdOffset + 20);
        
        addCommand(LOAD,getVarAddr(varSign1.name));
        addCommand(JPOS, cmdOffset+8);
        addCommand(LOAD,getVarAddr(varSign2.name));
        addCommand(JPOS, cmdOffset+3);
        //pos a, pos b
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(JUMP,cmdOffset+14);
        //pos a, neg b
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(ADD,getVarAddr(b));
        addCommand(JUMP,cmdOffset+11);
        
        
        addCommand(LOAD,getVarAddr(varSign2.name));
        addCommand(JPOS, cmdOffset+7);
        //neg a, pos b
        addCommand(LOAD,getVarAddr(var1.name));
        addCommand(SUB,getVarAddr(b));
        addCommand(STORE,getVarAddr(var1.name));
        addCommand(SUB,getVarAddr(var1.name));
        addCommand(SUB,getVarAddr(var1.name));
        addCommand(JUMP,cmdOffset+3);
        //neg a, neg b
        addCommand(SUB,0);
        addCommand(SUB,getVarAddr(var1.name));
      
    }

    void __condEQ(string a, string b,int line){
        startOperationHandler(&a,&b,line);

        addCommand(LOAD,getVarAddr(a));
        addCommand(SUB,getVarAddr(b));

        createJump(JPOS,JNEG);
              
    }
    void __condNEQ(string a, string b,int line){
        startOperationHandler(&a,&b,line);

        addCommand(LOAD,getVarAddr(a));
        addCommand(SUB,getVarAddr(b));

        createJump(JZERO);       
    }
    void __condLE(string a, string b,int line){
        startOperationHandler(&a,&b,line);

        addCommand(LOAD,getVarAddr(a));
        addCommand(SUB,getVarAddr(b));

        addCommand(INC);
        createJump(JPOS);       
    }
    void __condGE(string a, string b,int line){
        startOperationHandler(&a,&b,line);

        addCommand(LOAD,getVarAddr(a));
        addCommand(SUB,getVarAddr(b));

        addCommand(DEC);    
        createJump(JNEG);   
    }
    void __condLEQ(string a, string b, int line){
        startOperationHandler(&a,&b,line);

        addCommand(LOAD,getVarAddr(a));
        addCommand(SUB,getVarAddr(b));
        
        createJump(JPOS);
    }
    void __condGEQ(string a, string b,int line){
        startOperationHandler(&a,&b,line);

        addCommand(LOAD,getVarAddr(a));
        addCommand(SUB,getVarAddr(b));
        
        createJump(JNEG);
    }


//----------------- ERROR HANDLING -----------------

    void error(string text, int line){
        cout << "Line:" << line << "; " << text << endl;
    }
    void error(string text){
        cout <<  text << endl;
    }

    bool isNumber(const std::string &str) {
        return str.find_first_not_of("0123456789") == std::string::npos;
    }
    void checkForToRange(string a,string b, int line){
        if(!isNumber(a)
        || !isNumber(b))
            return;

        if(stoll(a) > stoll(b)){
            error("Incorrect for range: "+a + " TO " + b,line);
            exit(1);
        }
    }
    void checkForDowntoRange(string a,string b, int line){
        if(!isNumber(a)
        || !isNumber(b))
            return;

        if(stoll(a) < stoll(b)){
            error("Incorrect for range: "+a + " DOWNTO " + b,line);
            exit(1);
        }
    }