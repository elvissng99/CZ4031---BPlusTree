#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <set>
#include <stdio.h>
#include <limits.h>
#include <map>
#include <cmath>
#include <sstream>
#include "storage.h"
#include <fstream>
#include "bplustree.h"

int main(){
    Storage* storage = new Storage();
    std::cout << "Storing data into storage" << endl;
    storage->store_data("data.tsv");
    BPlusTree bplustree;
    std::cout << "Creating B+ Tree" << endl;
    bplustree.createTreeFromStorage(storage);
    std::cout <<"B+ Tree successfully created" << endl;
    std::cout << "Start Experiments" << std::endl;
    storage->experiment1();
    std::cout << std::endl;
    bplustree.experiment2();
    std::cout << std::endl;
    bplustree.experiment3(500,storage);
    std::cout << std::endl;
    bplustree.experiment4(30000,40000, storage);
    std::cout << std::endl;
    bplustree.experiment5(1000, storage);
    std::cout << "End of Experiments" << std::endl;
}