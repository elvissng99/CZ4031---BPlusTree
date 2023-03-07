#include <iostream>
#include <cstring>
#include <vector>
#include <stdio.h>
#include <limits.h>
#include <cmath>
using namespace std;
#ifndef STORAGE_H
#define STORAGE_H
class Storage{
public:
    //100MB
    int storage_size = 100*1024*1024;
    unsigned char *basePtr = (unsigned char*) malloc(storage_size);
    int used_storage_size = 0;
    //200B
    static const int block_size = 200;
    //10byte for tconst
    static const int tconst_size = 10;
    //1byte for rating
    static const int rating_size = 1;
    //4bytes for votes
    static const int votes_size = 4;
    static const int record_size = tconst_size + rating_size + votes_size;
    static const int max_records_per_block = block_size/record_size;
    static const int excess_block_size = block_size%record_size;
    int numBlocks = 1;
    int curBlockSize = 0;
    int numRecords = 0;
    Storage(){
    }

    ~Storage(){
        free(this -> basePtr);
    }

    void test();
    
    //store data and complete experiment 1
    void store_data(string file);

    //useful for experiment 3,4,5
    unsigned int retrieve_record_votes(unsigned char* curPtr);

    //used to count how many blocks are accessed
    unsigned int retrieve_block_id(unsigned char* curPtr);

    //useful for experiment 5
    void delete_record(unsigned char* curPtr);

    //convert integer to bytes
    unsigned char* convertIntToBytes(unsigned int n);
    unsigned int convertBytesToInt(unsigned char * bytes);

    //display record, useful for debugging
    void display_record(unsigned char * curPtr);
    void display_all_records();
    void experiment1();
    void experiment3(int x);
    void experiment4(int x, int y);
    void experiment5(int x);
};

struct Record{
    float rating;
    int numVotes;
    string tconst;
};
#endif