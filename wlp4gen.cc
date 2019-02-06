#include<iostream>
//#include"typechecker.h"
#include"codegen.h"
#include"parsetree.h"
using namespace std;

int main(int argc, const char *argv[]){
  auto tree = make_unique<theTree>();
  auto thecode = make_unique<CodeGenerator>();
  try{tree->sym_checker();}
  catch (string &e) {
    cerr << e << endl;
    return 1;
  }
  catch(...) {cerr << "b1"<< endl;}
  try{
  thecode->gen_code(*tree);}
  catch(string &e) {cerr << e << endl;}
  //tree->tree_print();
  //tree->print_symtbl();
}
