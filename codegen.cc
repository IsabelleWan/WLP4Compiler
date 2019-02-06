#include<iostream>
#include<vector>
#include<sstream>
#include"codegen.h"
//#include"therules.h"
using namespace std;

string type_of(Node &n);

int the_rule2(string s);

//convert int to string
string to_str(int i){
  ostringstream oss;
  oss << i;
  return oss.str();
}
void CodeGenerator::push_reg(string s){
  output.emplace_back("sw " + s + ", -4($30)");
  output.emplace_back("sub $30, $30, $4");
}

void CodeGenerator::pop_reg(string s){
  output.emplace_back("lw " + s + ", 0($30)");
  output.emplace_back("add $30, $30, $4");
}

void CodeGenerator::save_regs(){
  push_reg("$8");
  push_reg("$29");
}

void CodeGenerator::restore_regs(){
  pop_reg("$29");
  pop_reg("$8");
}

//call function previously stored in register
void CodeGenerator::func_call(string s){
  output.emplace_back("sw $31, -4($30)");
  output.emplace_back("sub $30, $30, $4");
  output.emplace_back("jalr " + s);
  output.emplace_back("lw $31, 0($30)");
  output.emplace_back("add $30, $30, $4");
}

void CodeGenerator::traverse(Node & n){
  int rv = the_rule2(n.get_rule());
  if (rv == 0){traverse(n.get_child_at(1));}
  if (rv == 1 || rv == 12 || rv == 13 || rv == 16){traverse(n.get_child_at(0));
               traverse(n.get_child_at(1));}
  if (rv == 2 || rv == 6 ||rv == 7){
    if (rv == 7) ++paramNum[curFunc]; 
    traverse(n.get_child_at(0));}
  if (rv == 8) {
    ++paramNum[curFunc];
    traverse(n.get_child_at(0));
    traverse(n.get_child_at(2));}
  if (rv == 3){//set offset tbl for procedures
    counter.emplace_back(0);
    curFunc = n.get_child_at(1).get_token_at(1);
    paramNum[curFunc] = 0;
    traverse(n.get_child_at(3));//params
    traverse(n.get_child_at(6));//dcls
    //traverse(n.get_child_at(7));//statements
    //traverse(n.get_child_at(9));//expr
    counter.pop_back();
  }
  else if (rv == 4){
    counter.emplace_back(0);
    curFunc = "wain";
    traverse(n.get_child_at(3));//dcl1
    traverse(n.get_child_at(5));//dcl2
    traverse(n.get_child_at(8));//dcls
    //traverse(n.get_child_at(9));//stat
    //traverse(n.get_child_at(11));//expr
    counter.pop_back();
  }
  else if (rv == 14){//dcl type ID
    int temp = n.get_child_at(0).get_numChild();
    string t;
    if (temp == 1) t = "int";
    else t = "int*"; 
    offsetTbl[curFunc][n.get_child_at(1).get_token_at(1)] = make_pair(t, counter.back()*(-4));
    ++counter.back();
  } 
}
  //---above sets the offset tbl;-------------
  /*if (rv == 41){//new
    ++traveller["new"];  }
  else if (rv == 21){//delete
    ++traveller["delete"];  }
   else {
    int c = n.get_numChild();
    for (int k = 0; k < c; ++k){
      traverse(n.get_child_at(k));
    }
  }*/

//get offset of variable c in function fun
string CodeGenerator::get_offset(string fun, string c){
  return to_str(offsetTbl[fun][c].second);
}

//Requires:
// the value of NULL ptr is 1
//Reg usage:
// $1, $2 are parameters
// $3 is return value
// $4 is the number 4
// $5 is used when there is an binary operation
// $6, $7 are used in comparison
// $10 is .word print
// $11 is the number 1
// $12 is .word init
// $13 is .word new
// $14 is .word delete
string CodeGenerator::gen_code_help(Node & n){
  if (n.get_token_at(0) == "NUM"){
    output.emplace_back("lis $3");
    output.emplace_back(".word " + n.get_token_at(1));    
    return "int";
  }
  if (n.get_token_at(0) == "NULL"){
    output.emplace_back("add $3, $0, $11");//set the value to 1
    return "int*";
  }
  int ruleVal = the_rule2(n.get_rule());
  if (ruleVal == 0) {//start
    output.emplace_back("beq $0, $0, " + curFunc);
    gen_code_help(n.get_child_at(1));   }
  else if (ruleVal == 3) {//procedure int id 
    curFunc = n.get_child_at(1).get_token_at(1);
    int param = paramNum[curFunc];
    output.emplace_back(curFunc + ":");
    output.emplace_back("sub $29, $30, $4");
    while (param > 0) {
      --param;
      output.emplace_back("add $29, $29, $4");
    }
    output.emplace_back(";  store declarations on the stack");
    gen_code_help(n.get_child_at(6));//dcls
    output.emplace_back(";  evaluate statements and expr");
    gen_code_help(n.get_child_at(7));
    gen_code_help(n.get_child_at(9));
    output.emplace_back(";   pop the program and return");
    int temp = offsetTbl[curFunc].size();
    while (temp != 0){
      --temp;
      output.emplace_back("add $30, $30, $4");
    }
    output.emplace_back("jr $31");
  }
  else if (ruleVal == 42){
    save_regs();
    string id = n.get_child_at(0).get_token_at(1);
    output.emplace_back("lis $8");
    output.emplace_back(".word " + id);
    push_reg("$31");
    //output.emplace_back("sub $29, $30, $4");
    output.emplace_back("jalr $8");// call function 
    pop_reg("$31");
    restore_regs();   }
  else if (ruleVal == 43){
    save_regs();
    string id = n.get_child_at(0).get_token_at(1);
    output.emplace_back("lis $8");
    output.emplace_back(".word " + id);
    string lastFunc = curFunc;
    push_reg("$31");
   //output.emplace_back("sub $29, $30, $4"); 
    gen_code_help(n.get_child_at(2));//arglist; push arguments onto the stack
    output.emplace_back("jalr $8");//$30 is updated accroding to the counter in the function; 
    curFunc = lastFunc;
    pop_reg("$31");
    counter.pop_back();
    restore_regs();
  }
  else if (ruleVal == 44){//arglist expr
    gen_code_help(n.get_child_at(0));
    push_reg("$3");//update counter; push $3
  }
  else if (ruleVal == 45){//arglist expr COMMA arglist
    gen_code_help(n.get_child_at(0));
    push_reg("$3");//update counter; push $3
    gen_code_help(n.get_child_at(2));
  }
  else if (ruleVal == 4) {//main int id
    curFunc = "wain";
    output.emplace_back(curFunc + ":");
    output.emplace_back(";  initialization");
    output.emplace_back(".import print");//println
      output.emplace_back(".import init");
      output.emplace_back(".import new");
      output.emplace_back(".import delete");
      output.emplace_back("lis $12");
      output.emplace_back(".word init");
      output.emplace_back("lis $13");
      output.emplace_back(".word new");
      output.emplace_back("lis $14");
      output.emplace_back(".word delete"); 
    output.emplace_back("lis $10");
    output.emplace_back(".word print");
    output.emplace_back("lis $4");
    output.emplace_back(".word 4");
    output.emplace_back("lis $11");
    output.emplace_back(".word 1");
    output.emplace_back(";  initialize reg 29");
    output.emplace_back("sub  $29, $30, $4");
    output.emplace_back(";  store reg 1 and 2 onto the stack");
    push_reg("$1");
    push_reg("$2");
    output.emplace_back(";  store other variables onto the stack");
    gen_code_help(n.get_child_at(8));//dcls
    gen_code_help(n.get_child_at(9));//statements
    gen_code_help(n.get_child_at(11));//expr
    output.emplace_back("jr $31");   }

  
  //new and delete
  else if (ruleVal == 41){//new
    if (traveller["genInit"] == 0){//call init
      output.emplace_back(";   call init");
      func_call("$12");//call init
      ++traveller["genInit"];
    }
    output.emplace_back(";  allocation of an array");
    gen_code_help(n.get_child_at(3)); //get expr in $3
    output.emplace_back("slt $8, $3, $0");//$8 is 1 if $3 is negative
    output.emplace_back("beq $8, $11, " + to_str(skipCount));
    output.emplace_back("add $1, $3, $0"); //move value in $3 to $1
    func_call("$13");//call new
    output.emplace_back("bne $3, $0, 1");
    output.emplace_back(to_str(skipCount) + ":");
    output.emplace_back("add $3, $0, $1");//set reg 3 to NULL  
    ++skipCount;
  }
  else if (ruleVal == 21){//delete
    string c = gen_code_help(n.get_child_at(3));//$3 contains an adress to delete
    output.emplace_back("beq $3, $11, skip" + to_str(skipCount));
    output.emplace_back("add $1, $3, $0");//move val in reg 3 to reg 4
    func_call("$14");
    output.emplace_back(";    set the pointer to NULL");
    string off = get_offset(curFunc, c);
    output.emplace_back("lis $3");
    output.emplace_back(".word " + off);
    output.emplace_back("add $3, $3, $29");//reg 3 now contains an address of the ptr just deleted
    output.emplace_back("sw $11, 0($3)");//set the value of the ptr to 1
    output.emplace_back("skip" + to_str(skipCount) + ":");
    ++skipCount;
  }

  //declarations
  else if (ruleVal == 12 || ruleVal == 13){//dcls dcls dcl BECOMES NUM/NULL SEMI
    gen_code_help(n.get_child_at(0));
    gen_code_help(n.get_child_at(3));//assign the NUM or NULL to $3
    push_reg("$3");  
  }

  //simple iterations
  else if (ruleVal == 2 || ruleVal == 6 || ruleVal == 7 || ruleVal == 28 || ruleVal == 31){//procedures main; expr term; term factor
    return gen_code_help(n.get_child_at(0));   }
  else if (ruleVal == 38 || ruleVal == 48) {//factor Lparen expr Rparen
    return gen_code_help(n.get_child_at(1));   }
  else if (ruleVal == 16 || ruleVal == 1) {//stats stats stat;
    gen_code_help(n.get_child_at(0));  
    gen_code_help(n.get_child_at(1));   }
  else if (ruleVal == 8){
    gen_code_help(n.get_child_at(0));
    gen_code_help(n.get_child_at(2));   }

  //statement
  else if (ruleVal == 46){//lvalue ID
    string tempID = n.get_child_at(0).get_token_at(1);
    output.emplace_back("lis $3");
    output.emplace_back(".word " + get_offset(curFunc, tempID));
    output.emplace_back("add $3, $29, $3");   }//$3 now contains an address
  else if (ruleVal == 47){//lvalue STAR lvalue
    return gen_code_help(n.get_child_at(1));//$3 contains an address
  }
  else if (ruleVal == 17) {//statement lvalue becomes expr semi
    gen_code_help(n.get_child_at(0));//lvalue; $3 contains an address
    output.emplace_back("sw $3, -4($30)");
    output.emplace_back("sub $30, $30, $4");
    gen_code_help(n.get_child_at(2)); //put expr val into $3
    output.emplace_back("lw $5, 0($30)");
    output.emplace_back("add $30, $30, $4");
    output.emplace_back("sw $3, 0($5)");     }

  //factor
  else if (ruleVal == 35 || ruleVal == 39){//factor ID; factor AMP lvalue
    if (ruleVal == 35) {
      string tempID = n.get_child_at(0).get_token_at(1);
      string off = get_offset(curFunc, tempID);
      output.emplace_back("lw $3, " + off + "($29)");//put the value of the factor in $3 
      return tempID;
   }
    else {
//cerr<< "check for &* case" << endl;
      Node &temp = n.get_child_at(1);
      while (1) {
        Node &temp = n.get_child_at(1);
//cerr<<"temp.toke1" << temp.get_token_at(1) << endl;
        if (temp.get_numChild() == 1){
          gen_code_help(temp); //put the address of lvalue in $3
          break;  }
        if (temp.get_token_at(1) == "STAR") {
//cerr<<"temp.toke1" << temp.get_token_at(1) << endl;
          gen_code_help(temp.get_child_at(1));
          break;
        }
      }
    }
  }
  else if (ruleVal == 40) {//factor STAR factor
    gen_code_help(n.get_child_at(1));//$3 contains an address
    output.emplace_back("lw $3, 0($3)");//put the value in the address to $3
  }
  // a NUM/NULL terminal is found
  else if (ruleVal == 36 || ruleVal == 37){//factor NUM, factor NULL
    gen_code_help(n.get_child_at(0));
  }
  // the following is all the binary operands
  else if (ruleVal == 29 || ruleVal == 30 || ruleVal == 32 || ruleVal == 33 || ruleVal == 34){
    gen_code_help(n.get_child_at(0)); //expr
    output.emplace_back("sw $3, -4($30)");
    output.emplace_back("sub $30, $30, $4");
    gen_code_help(n.get_child_at(2)); //term
    output.emplace_back("lw $5, 0($30)");
    output.emplace_back("add $30, $30, $4");
    if (ruleVal == 29 || ruleVal == 30) {
//cerr<<"b1"<<endl;
      string ltype = type_of(n.get_child_at(0));
//cerr<<"b2"<<endl;
      string rtype = type_of(n.get_child_at(2));
//cerr << "ltype " << ltype << "-" << "rtype " << rtype << endl;
      if (ruleVal == 29) {
        if (ltype == "int*" && rtype == "int") {
          output.emplace_back("mult $3, $4");
          output.emplace_back("mflo $3");  }
        else if (ltype == "int" && rtype == "int*") {
          output.emplace_back("mult $5, $4");
          output.emplace_back("mflo $5");  }
        output.emplace_back("add $3, $5, $3");
      }
      else if (ruleVal == 30) {
        if (ltype == "int*" && rtype == "int"){
          output.emplace_back("mult $3, $4");
          output.emplace_back("mflo $3"); 
          output.emplace_back("sub $3, $5, $3"); }
        else if (ltype == "int*" && rtype == "int*") {
          output.emplace_back("sub $3, $5, $3");
          output.emplace_back("divu $3, $4");
          output.emplace_back("mflo $3");
        }
        else output.emplace_back("sub $3, $5, $3");
      }
    }
    else if (ruleVal == 32) {
      output.emplace_back("mult $5, $3");
      output.emplace_back("mflo $3");   }
    else if (ruleVal == 33) {
      output.emplace_back("div $5, $3");
      output.emplace_back("mflo $3");   }
    else {
      output.emplace_back("div $5, $3");
      output.emplace_back("mfhi $3");   }   }
  //the following is print
  else if (ruleVal == 20) {//PRINTLN
    gen_code_help(n.get_child_at(2));
    output.emplace_back("add $1, $3, $0");
    func_call("$10");
  }
  // the following is for control flow
  else if (ruleVal >=22 && ruleVal <= 27){
//cerr<<"b3"<<endl;
    string sign = type_of(n.get_child_at(0)) == "int" ? "slt" : "sltu";
//cerr <<"b4"<<endl;
  if (ruleVal == 24) {//test expr LT expr
    gen_code_help(n.get_child_at(0));
    output.emplace_back("sw $3, -4($30)");
    output.emplace_back("sub $30, $30, $4");
    gen_code_help(n.get_child_at(2));
    output.emplace_back("lw $5, 0($30)");
    output.emplace_back("add $30, $30, $4");
    output.emplace_back(sign + " $3, $5, $3");//if LHS < RHS, $3 is 1   
  }
  else if (ruleVal == 23) {//NE
    gen_code_help(n.get_child_at(0));
    output.emplace_back("sw $3, -4($30)");
    output.emplace_back("sub $30, $30, $4");
    gen_code_help(n.get_child_at(2));
    output.emplace_back("lw $5, 0($30)");
    output.emplace_back("add $30, $30, $4");
    output.emplace_back(sign + " $6, $5, $3");//if LHS < RHS, $6 is 1
    output.emplace_back(sign + " $7, $3, $5");//if LHS < RHS, $6 is 1
    output.emplace_back("add $3, $6, $7");
  }
  else if (ruleVal == 22) {//EQ
    gen_code_help(n.get_child_at(0));
    output.emplace_back("sw $3, -4($30)");
    output.emplace_back("sub $30, $30, $4");
    gen_code_help(n.get_child_at(2));
    output.emplace_back("lw $5, 0($30)");
    output.emplace_back("add $30, $30, $4");
    output.emplace_back(sign + " $6, $5, $3");//if LHS < RHS, $6 is 1
    output.emplace_back(sign + " $7, $3, $5");//if LHS < RHS, $6 is 1
    output.emplace_back("add $3, $6, $7");
    output.emplace_back("sub $3, $11, $3");   }   
 else if (ruleVal == 27) {//GT
    gen_code_help(n.get_child_at(0));
    output.emplace_back("sw $3, -4($30)");
    output.emplace_back("sub $30, $30, $4");
    gen_code_help(n.get_child_at(2));
    output.emplace_back("lw $5, 0($30)");
    output.emplace_back("add $30, $30, $4");
    output.emplace_back(sign + " $3, $3, $5");//if LHS > RHS, $3 is 1   
 } 
 else if (ruleVal == 25){//LE
    gen_code_help(n.get_child_at(0));
    output.emplace_back("sw $3, -4($30)");
    output.emplace_back("sub $30, $30, $4");
    gen_code_help(n.get_child_at(2));
    output.emplace_back("lw $5, 0($30)");
    output.emplace_back("add $30, $30, $4");
    output.emplace_back(sign + " $3, $3, $5");//if LHS > RHS, $3 is 1   
    output.emplace_back("sub $3, $11, $3");//if LHS <= RHS, $3 is 1
 }
 else if (ruleVal == 26){//GE
    gen_code_help(n.get_child_at(0));
    output.emplace_back("sw $3, -4($30)");
    output.emplace_back("sub $30, $30, $4");
    gen_code_help(n.get_child_at(2));
    output.emplace_back("lw $5, 0($30)");
    output.emplace_back("add $30, $30, $4");
    output.emplace_back(sign + " $3, $5, $3");//if LHS < RHS, $3 is 1   
    output.emplace_back("sub $3, $11, $3");//if LHS >= RHS, $3 is 1
 }  }
 else if (ruleVal == 19) {//While
    string tempN = to_str(loopCount);
    ++loopCount;
    output.emplace_back("loop" + tempN + ":");
    gen_code_help(n.get_child_at(2));
    output.emplace_back("beq $3, $0, done" + tempN); 
    gen_code_help(n.get_child_at(5));
    output.emplace_back("beq $0, $0, loop" + tempN);
    output.emplace_back("done" + tempN + ":");     }
  else if (ruleVal == 18) {//IF
    string tempN = to_str(ifCount);
    ++ifCount;
    gen_code_help(n.get_child_at(2));
    output.emplace_back("beq $3, $0, else" + tempN);
    gen_code_help(n.get_child_at(5));
    output.emplace_back("beq $0, $0, endif" + tempN);
    output.emplace_back("else" + tempN + ":");
    gen_code_help(n.get_child_at(9));
    output.emplace_back("endif" + tempN + ":");   }
  return "";
}


void CodeGenerator::gen_code(theTree &t) {
  //traverse through the tree first before generating code
  traveller["new"] = 0;
  traveller["delete"] = 0;
  traveller["genInit"] = 0;
//cerr << "1" << endl;
  traverse(t.get_root());
//test
for (auto k : offsetTbl){
cerr << "offset tbl" << endl;
cerr<< k.first << endl;
for (auto k1 : k.second) {
cerr << k1.first << " " << k1.second.first << " " << k1.second.second << endl;}
}
//test paramNum
cerr << "paramNum" << endl;
for (auto p : paramNum){
cerr << p.first << " " << p.second<<endl;
}
  //generate code
//cerr << "2" << endl;
  gen_code_help(t.get_root());
//cerr<< "3" << endl;
  for (auto line : output){
    cout << line << endl;
//    cerr << line << endl;
  }
}


string CodeGenerator::type_of(Node &n){
  string &token1 = n.get_token_at(1);
  int c = n.get_numChild();
  if (c == 1){
  if (token1 == "NUM") return "int";
  else if (token1 == "NULL") return "int*";
  else if (token1 == "ID") {
    return offsetTbl[curFunc][n.get_child_at(0).get_token_at(1)].first;
  }
  }
  int rv = the_rule2(n.get_rule());
  if (rv == 28 || rv == 31 || rv == 35 || rv == 46) return type_of(n.get_child_at(0));
  else if (rv == 38 ||  rv == 48) return type_of(n.get_child_at(1));
  //expr
  else if (rv == 29 || rv == 30 || rv == 32 || rv ==33 || rv == 34) {//PLUS;MINUS;STAR;SLASH;PCT
    string lhs = type_of(n.get_child_at(0));
    string rhs = type_of(n.get_child_at(2));
    if (rv == 29){//PLUS
      if (lhs == "int" && rhs == "int") return "int";
      else if (lhs == "int*" && rhs == "int") return "int*";
      else if (lhs == "int" && rhs == "int*") return "int*";}
    else if (rv == 30) {//MINUS
      if (lhs == "int" && rhs == "int") return "int";
      else if (lhs == "int*" && rhs == "int") return "int*";
      else if (lhs == "int*" && rhs == "int*") return "int";
      throw string("ERROR in substraction");   }
    else if (rv == 32 || rv == 33 || rv == 34){//MULT;DIVISION
      if (lhs == "int" && rhs == "int") return "int";  }}	  
  else if (rv == 39 || rv == 40  || rv == 47){//AMP lvalue; STAR factor
    string rhs = type_of(n.get_child_at(1));
    if (rv == 39 && rhs == "int"){//AMP
      return "int*";   }
    else if ((rv == 40 || rv == 47) && rhs == "int*"){//STAR
      return "int";   }  }
  else if (rv == 41){//NEW
    string temp = type_of(n.get_child_at(3));
    if (temp == "int"){
      return "int*";   }   }
  //procedure calls
  else if (rv == 42 || rv == 43){ return "int";}
  else {//cerr <<n.get_rule()<<endl;
    throw 1;}
}


int the_rule2(string s){
  if (s == "start BOF procedures EOF") return 0;
  if (s == "procedures procedure procedures") return 1;
  if (s == "procedures main") return 2;
  if (s == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") return 3;
  if (s == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") return 4;
  if (s == "params") return 5;
  if (s == "params paramlist") return 6;
  if (s == "paramlist dcl") return 7;
  if (s == "paramlist dcl COMMA paramlist") return 8;
  if (s == "type INT") return 9;
  if (s == "type INT STAR") return 10;
  if (s == "dcls") return 11;
  if (s == "dcls dcls dcl BECOMES NUM SEMI") return 12;
  if (s == "dcls dcls dcl BECOMES NULL SEMI") return 13;
  if (s == "dcl type ID") return 14;
  if (s == "statements") return 15;
  if (s == "statements statements statement") return 16;
  if (s == "statement lvalue BECOMES expr SEMI") return 17;
  if (s == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") return 18;
  if (s == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") return 19;
  if (s == "statement PRINTLN LPAREN expr RPAREN SEMI") return 20;
  if (s == "statement DELETE LBRACK RBRACK expr SEMI") return 21;
  if (s == "test expr EQ expr") return 22;
  if (s == "test expr NE expr") return 23;
  if (s == "test expr LT expr") return 24;
  if (s == "test expr LE expr") return 25;
  if (s == "test expr GE expr") return 26;
  if (s == "test expr GT expr") return 27;
  if (s == "expr term") return 28;
  if (s == "expr expr PLUS term") return 29;
  if (s == "expr expr MINUS term") return 30;
  if (s == "term factor") return 31;
  if (s == "term term STAR factor") return 32;
  if (s == "term term SLASH factor") return 33;
  if (s == "term term PCT factor") return 34;
  if (s == "factor ID") return 35;
  if (s == "factor NUM") return 36;
  if (s == "factor NULL") return 37;
  if (s == "factor LPAREN expr RPAREN") return 38;
  if (s == "factor AMP lvalue") return 39;
  if (s == "factor STAR factor") return 40;
  if (s == "factor NEW INT LBRACK expr RBRACK") return 41;
  if (s == "factor ID LPAREN RPAREN") return 42;
  if (s == "factor ID LPAREN arglist RPAREN") return 43;
  if (s == "arglist expr") return 44;
  if (s ==  "arglist expr COMMA arglist") return 45;
  if (s == "lvalue ID") return 46;
  if (s == "lvalue STAR factor") return 47;
  if (s == "lvalue LPAREN lvalue RPAREN") return 48;
  else throw s;
}

