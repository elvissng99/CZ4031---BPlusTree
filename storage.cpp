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
#include <chrono>

using namespace std;

bool compareByNumVotes(const Record& a, const Record& b) {
    return a.numVotes < b.numVotes;
}

void Storage::test(){
    cout << "Testing" << endl;
}

void Storage::store_data(string file) {
    
    ifstream movieData(file);
    string line;

    string tconst;
    string rating;
    string numVotes;
    float rating_float;
    vector<Record> records;
    unsigned char record[record_size];
    unsigned char *curPtr = basePtr;
    this->numBlocks = 1;
    map<unsigned char *, unsigned int> map;
    int no_of_records_in_block = 0;

    // skip first row
    getline(movieData, line);
    // read line by line
    while (getline(movieData, line)) {
        Record record_obj;
        istringstream ss(line);
        // read each element by delimiter
        getline(ss, tconst, '\t');
        getline(ss, rating, '\t');
        getline(ss, numVotes, '\t');

        stringstream(rating) >> rating_float;
        record_obj.tconst = tconst;
        // rating is now an integer that is  out of 100 instead of out of 10
        //since 100 < 256, it can be stored in 1byte
        record_obj.rating = rating_float*10;
        record_obj.numVotes = stoi(numVotes);
        records.push_back(record_obj);
    }
    sort(records.begin(), records.end(), compareByNumVotes);
    for(Record record_obj : records){
        // check if block has space, if not move on to the next block
        if (no_of_records_in_block == max_records_per_block) {
            no_of_records_in_block = 0;
            curPtr += this->excess_block_size;
            this->numBlocks++;
        }

        //reset record to store new incoming record
        memset(record, '\0', record_size);
        int index = 0;
        //store tconst in record
        for (unsigned char c : record_obj.tconst) {
            record[index] = c;
            index++;
        }
        //store rating in record
        record[tconst_size] = record_obj.rating;
        index = tconst_size + rating_size;
        //store numVotes in record
        for (int i = 0; i < sizeof(int); i++) {
            unsigned char *p = convertIntToBytes(record_obj.numVotes);
            record[index + i] = *(p + i);
        }
        //store record into storage
        for (int i = 0; i < record_size; i++) {
            curPtr[i] = record[i];
        }
        used_storage_size += record_size;
        curPtr += record_size;
        no_of_records_in_block ++;
        numRecords++;
    }
}


unsigned int Storage::retrieve_record_votes(unsigned char* curPtr){
    return convertBytesToInt(curPtr+ tconst_size + rating_size);
}

unsigned int Storage::retrieve_block_id(unsigned char* curPtr){
    return (int)(curPtr - basePtr)/block_size +1;
}

void Storage::delete_record(unsigned char* curPtr){
    for(int i = 0; i < record_size; i++){
        *curPtr = '\0';
        curPtr++; 
    }
}

unsigned char * Storage::convertIntToBytes(unsigned int n){
    unsigned char* bytes = (unsigned char*) malloc(sizeof(int));
    //rightshift 3bytes to convert first byte of integer
    bytes[0] = (n >> 24) & 0xFF;
    //rightshift 2bytes to convert second byte of integer
    bytes[1] = (n >> 16) & 0xFF;
    //rightshift 1byte to convert third byte of integer
    bytes[2] = (n >> 8) & 0xFF;
    //rightshift 0byte to convert fourth byte of integer
    bytes[3] = n & 0xFF;
    return bytes;
}

unsigned int Storage::convertBytesToInt(unsigned char * bytes){
    //left shifting to form back the 4 byte integer
    return bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3] << 0;
}


void Storage::display_record(unsigned char* curPtr){
    if(*curPtr == '\0') cout << "Record does not exist" << endl;
    else{
        for(int i = 0; i< tconst_size;i++){
            if(*curPtr != '\0') cout << *curPtr;
            curPtr++;
        }
        cout <<"\t";
        cout << (float)*curPtr/10 << "\t";
        curPtr++;
        cout << convertBytesToInt(curPtr) << endl;
        curPtr+=4;
    }
}

void Storage::display_all_records(){
    unsigned char *curPtr = basePtr;
    int curRecord = 0;
    int block_no = 1;
    while(curRecord < numRecords){
        if (curRecord%max_records_per_block == 0) cout << "Block: " << block_no << endl;
        display_record(curPtr);
        curRecord++;
        curPtr +=record_size;
        if (curRecord%max_records_per_block == 0) {
            curPtr += excess_block_size;
            block_no++;
            cout << endl;
        }
    }
}

void Storage::experiment1(){
    cout<< "Experiment 1" << endl;
    cout<< "Number of Records: " << numRecords << endl;
    cout<< "Record Size: " << record_size << "bytes" <<endl;
    cout<< "Number of Records in a Block: " << max_records_per_block << endl;
    cout<< "Number of Blocks: " << numBlocks << endl;
}

// brute force linear scan search operation
void Storage::experiment3(int x){
    unsigned char *curPtr = basePtr;
    int block_no = 1;
    int curRecord = 0; 
    int reached = 0;
    
    // Start timer
    auto start = chrono::high_resolution_clock::now();

    // parse through if not x yet
    while (retrieve_record_votes(curPtr) <= x){
        // retrieve record if x
        if (retrieve_record_votes(curPtr) == x){    
            // if (curRecord%max_records_per_block == 0 || reached == 0) cout << "Block: " << block_no << endl;
            // display_record(curPtr); 
            curRecord++;
            reached++;
            curPtr += record_size;
        }
        // not x, go to next record
        else{
            curPtr += record_size;
            curRecord++;
        }
        // reached end of block, next block
        if (curRecord%max_records_per_block == 0) {
            curPtr += excess_block_size;
            block_no++;
        }
    }
    // Stop timer
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << "Number of data blocks accessed by linear scan: " << block_no << endl;
    cout << "Runtime of linear scan: " << duration.count() << " microseconds" << endl;
}

// brute force linear scan ranged search operation
void Storage::experiment4(int x, int y){
    unsigned char *curPtr = basePtr;
    int block_no = 1;
    int curRecord = 0; 
    int reached = 0;
    
    // Start timer
    auto start = chrono::high_resolution_clock::now();

    // parse through if not end of range yet
    while (retrieve_record_votes(curPtr) <= y){
        // retrieve record ranging from x to y 
        if (retrieve_record_votes(curPtr) >= x && retrieve_record_votes(curPtr) <= y){    
            //if (curRecord%max_records_per_block == 0 || reached == 0) cout << "Block: " << block_no << endl;
            //display_record(curPtr); 
            curRecord++;
            reached++;
            curPtr += record_size;
        }
        // not in range, go to next record
        else{
            curPtr += record_size;
            curRecord++;
        }
        // reached end of block, next block
        if (curRecord%max_records_per_block == 0) {
            curPtr += excess_block_size;
            block_no++;
        }
    }
    // Stop timer
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << "Number of data blocks accessed by linear scan: " << block_no << endl;
    cout << "Runtime of linear scan: " << duration.count() << " microseconds" << endl;
}

void Storage::experiment5(int x){
    unsigned char *curPtr = basePtr;
    int block_no = 1;
    int curRecord = 0; 

    // Start timer
    auto start = chrono::high_resolution_clock::now();

    // parse through if not x yet
    while (retrieve_record_votes(curPtr) <= x){
        // reach records with numVotes == x and delete
        if (retrieve_record_votes(curPtr) == x){    
            // if (curRecord%max_records_per_block == 0 || reached == 0) cout << "Block: " << block_no << endl;
            // display_record(curPtr); 
            delete_record(curPtr);
            curRecord++;
            curPtr += record_size;
        }
        // not in range, go to next record
        else{
            curPtr += record_size;
            curRecord++;
        }
        // reached end of block, next block
        if (curRecord%max_records_per_block == 0) {
            curPtr += excess_block_size;
            block_no++;
        }
    }

    // Stop timer
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    cout << "Number of data blocks accessed by linear scan: " << block_no << endl;
    cout << "Runtime of linear scan for deletion: " << duration.count() << " microseconds" << endl;
}