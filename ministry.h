#ifndef MINISTRY_H
#define MINISTRY_H

#include <iostream>
#include <string>
#include <vector>

class MinistryException : public std::exception {
    std::string message;
public:
    MinistryException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class IntNode {
public:
    int data;
    IntNode* next;
    IntNode(int val, IntNode* nxt = nullptr) : data(val), next(nxt) {}
};

class IntList {
    IntNode* head;
public:
    IntList();
    ~IntList();
    void add(int value);
    void clear();
    int get(int index) const;
    int size() const;
    bool isEmpty() const;
};

class Official {
public:
    int id;
    int bribe;
    int bossId;
    IntList subordinates;
    Official(int i, int b, int boss);
    ~Official() = default;
};

class Ministry {
    int N;
    Official** officials;
    long long* minCostCache;
    int* bestChild;
    bool dataLoaded;

    void clearMemory();
    void buildSubordinationLists();
    long long computeMinCost(int id);
    void restorePath(int id, std::vector<int>& path) const;
    void printTree(int id, int level) const;
    bool isCycleExists() const;
    void validateStructure() const;
    int readIntFromConsole(const std::string& prompt, int minVal, int maxVal);
    void validateAndStore(int n, int* bribes, int* bosses);

public:
    Ministry();
    ~Ministry();
    void inputFromKeyboard();
    void loadFromFile();
    void generateRandom();
    void showDataAndSolution();
    bool isDataLoaded() const { return dataLoaded; }
};

#endif