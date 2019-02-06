#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <vector>
#include <iostream>
#include <map>
#include "parsetree.h"

class CodeGenerator{
    int ifCount = 0;
    int loopCount = 0;
    int skipCount = 0;
    std::string curFunc = "wain";
    std::vector<int> counter;
    std::vector<std::string> output;
    std::map<std::string, int> paramNum;
    std::map<std::string, int> traveller;//pre-traverse the tree
    std::map<std::string, std::map<std::string, std::pair<std::string, int>>> offsetTbl; //map_of<funcName, map_of<varName, pair_of<type, offset>>>
    void func_call(std::string);
    void push_reg(std::string); 
    void pop_reg(std::string);
    void save_regs();
    void restore_regs();
    void traverse(Node &);
    std::string get_offset(std::string, std::string);
    std::string gen_code_help(Node &);
    std::string type_of(Node &);
  public:
    void gen_code(theTree &);
};

#endif
