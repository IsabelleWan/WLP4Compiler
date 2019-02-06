#include <iostream>
#include <memory>
#include <vector>
#include<map>
#include"parsetree.h"
using namespace std;

//determining terminals;
int is_terminal(string s);

string terminals[] = {
  "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID", "IF",
  "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM", "PCT", "PLUS",
  "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI", "SLASH", "STAR", "WAIN",
  "WHILE", "AMP", "LBRACK", "RBRACK", "NEW", "DELETE", "NULL"
};

int is_terminal(string s) {
  for (auto k : terminals) {
   if (s.compare(k) == 0){
    return 1;
   }
  }
  return 0;
}
//end of terminal helper

//Node constructor 
Node::Node(string rule): rule{rule}, numChild{0} {
    int slen = rule.length();
    int beg = 0;
    for (int i = 0; i < slen; ++i) {
      if (i != 0 && rule[i] == ' ' && rule[i - 1] != ' ') {
        tokens.push_back(rule.substr(beg, i - beg));
        ++numChild;
//cout << "pushed " << rule.substr(beg, i - beg) << " ";
      } else if (i != 0  && rule[i] != ' ' && rule[i - 1] == ' ') {
        beg = i;
      }
    }
    if (slen != beg) {
      tokens.push_back(rule.substr(beg, slen - beg));
//cout << "pushed " << rule.substr(beg, slen - beg)<<endl;
    }

    if (is_terminal(tokens.front())) numChild = 0;
}

void Node::addChild(){
  for (int i = 0; i < numChild; ++i){
    string r;
    if (!getline(cin, r)) throw string("ERROR while creating children");
    children.push_back(std::make_unique<Node>(r));
    if (children.back()-> numChild > 0){
      children.back()-> addChild();
    }
  }
}

void Node::node_print(){
  for (int i = 0; i < numChild; ++i){
    children[i]->node_print();
  }
  std::cout << rule << ' ' << numChild << std::endl;
}

//Gettor for Node
string Node::get_rule(){
  return rule;
}

int Node::get_numChild(){
  return numChild;
}

string &Node::get_token_at(int i) {
  return tokens[i];
}

Node &Node::get_child_at(int i){
  return *children[i];
}

//theTree constructor
theTree::theTree(){
  string r;
  if (!getline(cin, r)) throw string("ERROR while creating root");
  root = make_unique<Node>(r);
  root->addChild();
}

void theTree::print_symtbl(){
   int i = symtbl.size();
   for (auto const &p1 : symtbl) {
     --i; 
     cerr << p1.first;
     for (auto const &p2 : p1.second) {
       if (p2.second.second) {
         cerr << " " << p2.second.first;
       }
     }
     cerr << endl;
     for (auto const &p2 : p1.second) {
       cerr << p2.first << " " << p2.second.first << endl;
     }
     if ( i != 0) {
       cerr << endl;
     }
   }
}

void theTree::tree_print(){
  root->node_print();
}

Node &theTree::get_root(){
  return *root;
}

void Node:: node_sym_checker(map<string, map<string, pair<string, int>>> &symtbl, string name, int isparam){
  //check procedure
  if (tokens.front().compare("procedure") == 0) {
    string scope = children[1]->tokens[1];
    if (scope.compare("wain") == 0 || symtbl.find(scope) != symtbl.end()) {
      throw string("ERROR: redefine procedure");
    } else {//valid
      symtbl[scope];
      children[3]->node_sym_checker(symtbl, scope, 1); //param
      for (int i = 6; i < numChild; ++i) {//non-param
        children[i]->node_sym_checker(symtbl, scope, 0);
      } 
    }
  } else if (tokens.front().compare("main") == 0) {
    string scope = "wain";
    if (symtbl.find(scope) != symtbl.end()) {
      throw string("ERROR: redefine main");
    } else {
      children[3]->node_sym_checker(symtbl, scope, 1);
      children[5]->node_sym_checker(symtbl, scope, 1);
      for (int i = 8; i < numChild; ++i) {
        children[i]->node_sym_checker(symtbl, scope, 0);
      }
    }
  } else if (tokens.front().compare("dcl")== 0) {//a declaration 
    auto const &c1 = children[0]; //c1 is non-terminal type
    auto const &c2 = children[1]; // c2 is terminal ID
    string type;
    int tlen = c1->tokens.size(); //tlen is how many children c1 has
    for (int i = 0; i < tlen -1; ++i) {
      type += c1->children[i]->tokens[1];
    }
    auto id = c2->tokens[1]; // id is the name of the terminal ID
    if (symtbl[name].find(id) == symtbl[name].end()){//the id is not defined yet
      symtbl[name][id] = make_pair(type, isparam);
    } else {
      throw string("ERROR: repeated declaration");
    }
  } else if (tokens.front() == "factor") {
    if (numChild != 1 && tokens[1].compare("ID") == 0) {//the factor is a function call
      string idname = children[0]->tokens[1];
      if (symtbl[name].find(idname) != symtbl[name].end()){//we do not allow function call if the func name is also a variable
        throw string("ERROR: invalid function call-it is also a variable");
      }
      if (symtbl.find(idname) == symtbl.end()) {//no such function 
        throw string("ERROR: invalid function call-no such func");
      }
      if (numChild == 4) {//the function has parameters
        children[2]->node_sym_checker(symtbl, name, 0);
      }
    } else {
      for (int i = 0; i < numChild; ++i) {
        children[i]->node_sym_checker(symtbl, name, 0);
      }
    }
  } else if (tokens.front() == "ID") {//this ID could not be a function call or a declaration of a function
    if (symtbl[name].find(tokens[1]) == symtbl[name].end()) {
      throw string("ERROR: use of undeclared id");
    }
  } else {
    for (int i = 0; i < numChild; ++i) {
      children[i] -> node_sym_checker(symtbl, name, isparam);
    }
  }
//  print_symtbl(symtbl);
}

void theTree::sym_checker(){
  root->node_sym_checker(symtbl, "", 0);
}
