#ifndef PARSETREE_H
#define PARSETREE_H

#include <memory>
#include <vector>
#include<map>

class Node{
  std::string rule;
  int numChild;
  std::vector<std::string> tokens;
  std::vector <std::unique_ptr<Node>> children;
public:
  Node(std::string rule);
  void node_print();
  void addChild();
  std::string get_rule();
  int get_numChild();
  std::string &get_token_at(int);
  Node &get_child_at(int);
  void node_sym_checker(std::map<std::string, std::map<std::string, std::pair<std::string, int>>> &, std::string, int);
};

class theTree{
  std::unique_ptr<Node> root;
  std::map<std::string, std::map<std::string, std::pair<std::string, int>>> symtbl;
   
public:
  theTree();
  void print_symtbl();
  void tree_print();
  Node &get_root();
  void sym_checker();
};

#endif  
