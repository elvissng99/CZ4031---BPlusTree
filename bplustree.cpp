#include <iostream>
#include <climits>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "bplustree.h"
#include <queue>
#include <set>
#include <chrono>

using namespace std;

// size of node = size of block = 200
// size of node = 2 +4N + 4 + 8(N+1)
// 200 = 12N +14
// N = (200-14)/12
const int N = (200 - 14) / 12;

Node::Node()
{
  key = new int[N];
  ptr = new Node *[N + 1];
  records = new unsigned char *[N];
};

BPlusTree::BPlusTree(){};

void BPlusTree::insertKey(int x, unsigned char *record)
{
  // empty tree
  if (root == NULL)
  {
    root = createNewLeafNode(x, record);
    cout << "Root created:  " << x << endl;
    this->nodes++;
    this->levels++;
    this->numKeys++;
    return;
  }
  // root exists, traverse the tree
  Node *parent = traverseToLeafNode(x)[0];
  Node *curNode = traverseToLeafNode(x)[1];
  int insertIndex = 0;
  while (x > curNode->key[insertIndex] && insertIndex < curNode->size)
  {
    insertIndex++;
  }
  // account for duplicate keys
  if (x == curNode->key[insertIndex])
  {
    Node *curBuffer = curNode->ptr[insertIndex];
    // curBuffer should never be null in the first iteration
    while (curBuffer->ptr[0] != NULL)
    {
      curBuffer = curBuffer->ptr[0];
    }

    // there is remaining space in the buffer
    if (curBuffer->size < N)
    {
      curBuffer->records[curBuffer->size++] = record;
    }
    else
    {
      // current buffer is full
      // create new buffer to store the record
      // although each buffer node can store up to N+1 buffer nodes
      // for simplicity sake
      // each buffer node only points to 1 other buffer node like a linked list
      Node *newBuffer = createNewBufferNode(x, record);
      curBuffer->ptr[0] = newBuffer;
    }
    // cout << "Inserted duplicate key" << x << "into buffer" << endl;
  }
  // sufficient space at leafnode
  else if (curNode->size < N)
  {
    this->numKeys++;
    // make space for new key
    for (int i = curNode->size; i > insertIndex; i--)
    {
      curNode->key[i] = curNode->key[i - 1];
      curNode->ptr[i] = curNode->ptr[i - 1];
    }
    curNode->key[insertIndex] = x;
    curNode->size++;
    // create buffer for the new key
    Node *newBuffer = createNewBufferNode(x, record);
    curNode->ptr[insertIndex] = newBuffer;
    cout << "Inserted " << x << endl;
  }
  // create a new leaf node if it is already at the max
  else
  {
    this->nodes++;
    this->numKeys++;
    // create new leaf node
    Node *newLeaf = new Node;

    //tempNode and buffers will store the new sequence of records of size N+1
    //this will be later used to update the curNode and newLeaf accordingly
    int tempNode[N + 1];
    Node *buffers[N + 1];

    int insert_index = 0;
    while (x > curNode->key[insert_index] && insert_index < curNode->size)
    {
      insert_index++;
    }

    for (int i = 0; i <= N; i++)
    {
      if (i < insert_index)
      {
        tempNode[i] = curNode->key[i];
        buffers[i] = curNode->ptr[i];
      }
      else if (i == insert_index)
      {
        tempNode[i] = x;
        // create buffer for the new key
        buffers[i] = createNewBufferNode(x, record);
      }
      else
      {
        tempNode[i] = curNode->key[i - 1];
        buffers[i] = curNode->ptr[i - 1];
      }
    }

    //rearrange pointers
    Node *temp = curNode->ptr[N];
    curNode->ptr[N] = newLeaf;
    newLeaf->ptr[N] = temp;

    newLeaf->IS_LEAF = true;
    newLeaf->size = (N + 1) / 2;
    curNode->size = (N + 1) - newLeaf->size;

    //update curNode and add to newLeaf 
    for (int i = 0; i < curNode->size; i++)
    {
      curNode->key[i] = tempNode[i];
      curNode->ptr[i] = buffers[i];
      if (i < newLeaf->size)
      {
        newLeaf->key[i] = tempNode[i + curNode->size];
        newLeaf->ptr[i] = buffers[i + curNode->size];
      }
    }
    // tree only has one node (root node)
    if (curNode == root)
    {
      this->levels++;
      Node *newRootNode = new Node();
      newRootNode->key[0] = newLeaf->key[0];
      newRootNode->ptr[0] = curNode;
      newRootNode->ptr[1] = newLeaf;
      newRootNode->IS_LEAF = false;
      newRootNode->size = 1;
      root = newRootNode;
    }
    else
    {
      // insert new key into parent node
      insertInternal(newLeaf->key[0], parent, newLeaf);
    }
    cout << "Inserted " << x << endl;
  }
}

void BPlusTree::insertInternal(int x, Node *parent, Node *child)
{
  // parent is not full
  if (parent->size < N)
  {
    int insertIndex = 0;
    while (x > parent->key[insertIndex] && insertIndex < parent->size)
    {
      insertIndex++;
    }
    // make space for new key
    for (int i = parent->size; i > insertIndex; i--)
    {
      std::memcpy(parent->key + i, parent->key + i - 1, sizeof(int));
      std::memcpy(parent->ptr + i + 1, parent->ptr + i, sizeof(Node *));
    }
    parent->key[insertIndex] = x;
    parent->ptr[insertIndex + 1] = child;
    parent->size++;
    // std::cout << "Inserted key into internal node successfully" << endl;
  }
  else
  {
    // create new internal node
    this->nodes++;
    Node *newInternalNode = new Node();
    int tempKey[N + 1];
    Node *tempPtr[N + 2];
    // store original in temp
    std::memcpy(tempKey, parent->key, N * sizeof(int));
    std::memcpy(tempPtr, parent->ptr, (N + 1) * sizeof(Node *));
    tempPtr[N] = parent->ptr[N];
    int insertIndex = 0;
    while (x > tempKey[insertIndex] && insertIndex < N)
    {
      insertIndex++;
    }

    // make space for new key
    for (int i = N; i > insertIndex; i--)
    {
      std::memcpy(tempKey + i, tempKey + i - 1, sizeof(int));
    }
    tempKey[insertIndex] = x;
    for (int i = N + 1; i > insertIndex; i--)
    {
      std::memcpy(tempPtr + i, tempPtr + i - 1, sizeof(Node *));
    }
    tempPtr[insertIndex + 1] = child;
    newInternalNode->IS_LEAF = false;
    parent->size = (N + 1) / 2;
    newInternalNode->size = N - parent->size;
    // fill up the new internal node
    std::memcpy(newInternalNode->key, tempKey + parent->size + 1, newInternalNode->size * sizeof(int));
    std::memcpy(newInternalNode->ptr, tempPtr + parent->size + 1, (newInternalNode->size + 1) * sizeof(Node *));
    if (parent == root)
    {
      // create a new root node
      this->nodes++;
      this->levels++;
      Node *newRootNode = new Node();
      newRootNode->key[0] = parent->key[parent->size];
      newRootNode->ptr[0] = parent;
      newRootNode->ptr[1] = newInternalNode;
      newRootNode->IS_LEAF = false;
      newRootNode->size = 1;
      root = newRootNode;
      // std::cout << "Inserted new root node successfully" << endl;
    }
    else
    {
      insertInternal(parent->key[parent->size], findParent(root, parent), newInternalNode);
    }
  }
}

void BPlusTree::deleteKey(int x)
{
  // if root is null there is nothing to delete
  if (root == NULL)
    return;
  Node *curNode = root;
  Node *parent;
  // keep track of the 3 indexes of the parent node to traverse
  // the relevant 3 leafnodes
  int leftPtrIndex, rightPtrIndex;
  int index = -1;
  while (curNode->IS_LEAF == false)
  {

    for (int i = 0; i < curNode->size; i++)
    {
      // keeps track of the parent node of the leaf nodes when the while loop finishes
      parent = curNode;
      // find the first key where x< curNode->key[i]
      if (x < curNode->key[i])
      {
        // traverse down
        curNode = curNode->ptr[i];
        index = i;
        break;
      }
    }
    // traverse the last pointer since no x >= curNode->key[i] in previous for loop
    if (index == -1)
    {
      index = curNode->size;
      curNode = curNode->ptr[curNode->size];
    }
    if (curNode->IS_LEAF)
    {
      leftPtrIndex = index - 1;
      rightPtrIndex = index + 1;
    }
    index = -1;
  }
  for (int i = 0; i < curNode->size; i++)
  {
    // found key
    if (curNode->key[i] == x)
    {
      for (int j = i; j < curNode->size - 1; j++)
      {
        curNode->key[j] = curNode->key[j + 1];
        curNode->ptr[j] = curNode->ptr[+1];
      }
      curNode->size--;
      curNode->key[curNode->size] = 0;
      curNode->ptr[curNode->size] = NULL;

      // if leafnode has less than minimum size and is not the root node
      // then we have to update parent node
      if (curNode->size < (N + 1) / 2 && curNode != root)
      {
        // check left leafnode to take a key
        int chosenIndex = -1;
        if (leftPtrIndex >= 0 && parent->ptr[leftPtrIndex]->size > (N + 1) / 2)
          chosenIndex = leftPtrIndex;
        else if (rightPtrIndex <= parent->size && parent->ptr[rightPtrIndex]->size > (N + 1) / 2)
          chosenIndex = rightPtrIndex;

        // there is a left or right node with excess keys
        if (chosenIndex == leftPtrIndex)
        {
          // make space for new key for curNode
          for (int i = curNode->size; i > 0; i--)
          {
            curNode->key[i] = curNode->key[i - 1];
            curNode->ptr[i] = curNode->ptr[i - 1];
          }
          curNode->size++;
          // add the taken key to curNode
          curNode->key[0] = parent->ptr[leftPtrIndex]->key[parent->ptr[leftPtrIndex]->size - 1];
          curNode->ptr[0] = parent->ptr[leftPtrIndex]->ptr[parent->ptr[leftPtrIndex]->size - 1];
          // remove the taken key from left node
          parent->ptr[leftPtrIndex]->ptr[parent->ptr[leftPtrIndex]->size - 1] = NULL;
          parent->ptr[leftPtrIndex]->key[parent->ptr[leftPtrIndex]->size - 1] = 0;
          parent->ptr[leftPtrIndex]->size--;
          // update the parent key to new curNode->key[0]
          parent->key[leftPtrIndex] = curNode->key[0];
        }
        else if (chosenIndex == rightPtrIndex)
        {
          // add the taken key to curNode
          curNode->ptr[curNode->size] = parent->ptr[rightPtrIndex]->ptr[0];
          curNode->key[curNode->size] = parent->ptr[rightPtrIndex]->key[0];
          curNode->size++;
          parent->ptr[rightPtrIndex]->size--;
          // remove the taken key from right Node
          for (int i = 0; i < parent->ptr[rightPtrIndex]->size; i++)
          {
            parent->ptr[rightPtrIndex]->key[i] = parent->ptr[rightPtrIndex]->key[i + 1];
            parent->ptr[rightPtrIndex]->ptr[i] = parent->ptr[rightPtrIndex]->ptr[i + 1];
          }
          parent->ptr[rightPtrIndex]->key[parent->ptr[rightPtrIndex]->size] = 0;
          parent->ptr[rightPtrIndex]->ptr[parent->ptr[rightPtrIndex]->size] = NULL;
          parent->key[rightPtrIndex - 1] = parent->ptr[rightPtrIndex]->key[0];
        }
        else
        {
          // merge can definitely happen since both left and right have minimum nodes
          if (leftPtrIndex >= 0)
            chosenIndex = leftPtrIndex;
          else if (rightPtrIndex <= parent->size)
            chosenIndex = rightPtrIndex;
          if (chosenIndex == -1)
            cout << "This should not happen" << endl;
          // merge with left
          if (chosenIndex == leftPtrIndex)
          {
            for (int i = parent->ptr[leftPtrIndex]->size, j = 0; j < curNode->size; i++, j++)
            {
              parent->ptr[leftPtrIndex]->key[i] = curNode->key[j];
              parent->ptr[leftPtrIndex]->ptr[i] = curNode->ptr[j];
            }
            parent->ptr[leftPtrIndex]->size += curNode->size;
            parent->ptr[leftPtrIndex]->ptr[N] = curNode->ptr[N];
            deleteInternal(parent->key[leftPtrIndex], parent, curNode);
          }
          // merge with right
          else
          {
            for (int i = curNode->size, j = 0; j < parent->ptr[rightPtrIndex]->size; i++, j++)
            {
              curNode->key[i] = parent->ptr[rightPtrIndex]->key[j];
              curNode->ptr[i] = parent->ptr[rightPtrIndex]->ptr[j];
            }
            curNode->size += parent->ptr[rightPtrIndex]->size;
            curNode->ptr[N] = parent->ptr[rightPtrIndex]->ptr[N];
            // cout << "Merging two leaf nodes\n";
            curNode = parent->ptr[rightPtrIndex];
            deleteInternal(parent->key[rightPtrIndex - 1], parent, parent->ptr[rightPtrIndex]);
          }
          deallocate(curNode);
          this->nodes--;
        }
      }
      // cout << "Successfully deleted key" << endl;
      return;
    }
  }
  cout << "Key was not found" << endl;
}

void BPlusTree::deleteInternal(int x, Node *curNode, Node *child)
{
  // there is only 2 pointers
  // deleting the child means the other remaining child becomes the rootnode
  if (curNode == root && curNode->size == 1)
  {
    for (int i = 0; i < 2; i++)
    {
      if (curNode->ptr[i] == child)
      {
        root = curNode->ptr[!i];
        deallocate(curNode);
        cout << "Changed root node\n";
        return;
      }
    }
  }
  int index;
  for (int i = 0; i < curNode->size; i++)
  {
    if (curNode->key[i] == x)
    {
      index = i;
      break;
    }
  }
  for (int i = index; i < curNode->size - 1; i++)
  {
    curNode->key[i] = curNode->key[i + 1];
    curNode->ptr[i + 1] = curNode->ptr[i + 2];
  }
  curNode->key[curNode->size - 1] = 0;
  curNode->ptr[curNode->size] = NULL;
  curNode->size--;
  if (curNode->size < N / 2 && curNode != root)
  {
    Node *parent = findParent(root, curNode);
    int leftPtrIndex, rightPtrIndex;
    for (int i = 0; i < parent->size + 1; i++)
    {
      if (parent->ptr[i] == curNode)
      {
        index = i;
        leftPtrIndex = index - 1;
        rightPtrIndex = index + 1;
        break;
      }
    }
    int chosenIndex = -1;
    if (leftPtrIndex >= 0 && parent->ptr[leftPtrIndex]->size > N / 2)
      chosenIndex = leftPtrIndex;
    else if (rightPtrIndex <= parent->size && parent->ptr[rightPtrIndex]->size > N / 2)
      chosenIndex = rightPtrIndex;

    if (chosenIndex == leftPtrIndex)
    {
      curNode->ptr[curNode->size + 1] = curNode->ptr[curNode->size];
      for (int i = curNode->size; i > 0; i--)
      {
        curNode->key[i] = curNode->key[i - 1];
        curNode->ptr[i] = curNode->ptr[i - 1];
      }
      curNode->key[0] = parent->key[leftPtrIndex];
      curNode->ptr[0] = parent->ptr[leftPtrIndex]->ptr[parent->ptr[leftPtrIndex]->size];
      curNode->size++;
      parent->ptr[leftPtrIndex]->size--;
      parent->key[leftPtrIndex] = curNode->key[0];
    }
    else if (chosenIndex == rightPtrIndex)
    {
      curNode->key[curNode->size] = parent->key[index];
      parent->key[index] = parent->ptr[rightPtrIndex]->key[0];
      for (int i = 0; i < parent->ptr[rightPtrIndex]->size - 1; i++)
      {
        parent->ptr[rightPtrIndex]->key[i] = parent->ptr[rightPtrIndex]->key[i + 1];
      }
      curNode->ptr[curNode->size + 1] = parent->ptr[rightPtrIndex]->ptr[0];
      for (int i = 0; i < parent->ptr[rightPtrIndex]->size; ++i)
      {
        parent->ptr[rightPtrIndex]->ptr[i] = parent->ptr[rightPtrIndex]->ptr[i + 1];
      }
      curNode->size++;
      parent->ptr[rightPtrIndex]->size--;
    }
    else
    {
      // merge can definitely happen since both left and right have minimum nodes
      if (leftPtrIndex >= 0)
        chosenIndex = leftPtrIndex;
      else if (rightPtrIndex <= parent->size)
        chosenIndex = rightPtrIndex;
      if (chosenIndex == -1)
        cout << "This should not happen" << endl;
      // merge with left
      if (chosenIndex == leftPtrIndex)
      {
        // update left node key
        parent->ptr[leftPtrIndex]->key[parent->ptr[leftPtrIndex]->size] = parent->key[leftPtrIndex];
        for (int i = parent->ptr[leftPtrIndex]->size + 1, j = 0; j < curNode->size; j++)
        {
          parent->ptr[leftPtrIndex]->key[i] = curNode->key[j];
        }
        for (int i = parent->ptr[leftPtrIndex]->size + 1, j = 0; j < curNode->size + 1; j++)
        {
          parent->ptr[leftPtrIndex]->ptr[i] = curNode->ptr[j];
          curNode->ptr[j] = NULL;
        }
        parent->ptr[leftPtrIndex]->size += curNode->size + 1;
        curNode->size = 0;
        deleteInternal(parent->key[leftPtrIndex], parent, curNode);
        deallocate(curNode);
      }
      else
      {
        curNode->key[curNode->size] = parent->key[rightPtrIndex - 1];
        for (int i = curNode->size + 1, j = 0; j < parent->ptr[rightPtrIndex]->size; j++)
        {
          curNode->key[i] = parent->ptr[rightPtrIndex]->key[j];
        }
        for (int i = curNode->size + 1, j = 0; j < parent->ptr[rightPtrIndex]->size + 1; j++)
        {
          curNode->ptr[i] = parent->ptr[rightPtrIndex]->ptr[j];
          parent->ptr[rightPtrIndex]->ptr[j] = NULL;
        }
        curNode->size += parent->ptr[rightPtrIndex]->size + 1;
        parent->ptr[rightPtrIndex]->size = 0;
        deleteInternal(parent->key[rightPtrIndex - 1], parent, parent->ptr[rightPtrIndex]);
        deallocate(parent->ptr[rightPtrIndex]);
      }
    }
  }
}

//experiment 3 and 4 and re-formatted search functions
void BPlusTree::search(int x)
{
  if (root != NULL)
  {
    Node *curNode = traverseToLeafNode(x)[1];
    for (int i = 0; i < curNode->size; i++)
    {
      if (curNode->key[i] == x)
      {
        cout << "Found" << endl;

        //cross check count of a particular key
        int count = 0;
        Node *buffer = curNode->ptr[i];
        while (true)
        {
          count += buffer->size;
          if (buffer->size == N)
            buffer = buffer->ptr[0];
          else
            break;
        }
        cout << count << endl;
        return;
      }
    }
  }
  cout << "Not found\n";
}
void BPlusTree::experiment2()
{
  cout << "Experiment 2" << endl;
  // cout << "Number of Unique Key Values: " << this->numKeys << endl;
  cout << "Parameter N: " << N << endl;
  cout << "Number of Nodes: " << this->nodes << endl;
  cout << "Number of Levels: " << this->levels << endl;
  cout << "Keys of Root Node:" << endl;
  if (root != NULL)
  {
    for (int i = 0; i < root->size; i++)
    {
      cout << "Key " << i << ": " << root->key[i] << endl;
    }
  }
}
void BPlusTree::experiment3(int x, Storage *storage)
{
  cout << "Experiment 3" << endl;
  if (root == NULL)
  {
    cout << "Empty" << endl;
    return;
  }
  // Start timer
  auto start = chrono::high_resolution_clock::now();
  // it is at least the number of levels
  int nodes_accessed = this->levels;
  set<int> blocks_accessed;
  Node *curNode = traverseToLeafNode(x)[1];
  for (int i = 0; i < curNode->size; i++)
  {
    if (curNode->key[i] == x)
    {
      // cout << "Found\n";
      int count = 0;
      Node *buffer = curNode->ptr[i];
      float avg_avg_rating = 0;
      while (true)
      {
        count += buffer->size;
        for (int j = 0; j < buffer->size; j++)
        {
          blocks_accessed.insert(storage->retrieve_block_id(buffer->records[j]));
          avg_avg_rating += (float)*(buffer->records[j] + storage->tconst_size) / 10;
        }
        if (buffer->size == N)
          buffer = buffer->ptr[0];
        else
          break;
      }
      avg_avg_rating /= count;
      // Stop timer
      auto stop = chrono::high_resolution_clock::now();
      auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);

      cout << "Index nodes accessed: " << nodes_accessed << endl;
      cout << "Number of data blocks accessed: " << blocks_accessed.size() << endl;
      cout << "Average of averageRatings: " << avg_avg_rating << endl;
      cout << "Runtime of retrieval process: " << duration.count() << "microseconds" << endl;
      storage->experiment3(x);
      return;
    }
  }
  cout << "Not found\n";
}

void BPlusTree::experiment4(int x, int y, Storage *storage)
{
  cout << "Experiment 4" << endl;
  if (root == NULL)
  {
    cout << "Empty";
    return;
  }
  int nodes_accessed = this->levels;
  set<int> blocks_accessed;
  int count = 0;
  float avg_avg_rating = 0;

  // Start timer
  auto start = chrono::high_resolution_clock::now();
  Node *curNode = traverseToLeafNode(x)[1];
  Node *temp;
  int index;
  for (int i = 0; i < curNode->size; i++)
  {
    if (curNode->key[i] >= x)
    {
      temp = curNode;
      index = i;
      break;
    }
  }
  while (temp->key[index] <= y)
  {
    // cout << "Found" << temp->key[index] << endl;
    Node *buffer = temp->ptr[index];
    while (true)
    {
      count += buffer->size;
      for (int j = 0; j < buffer->size; j++)
      {
        blocks_accessed.insert(storage->retrieve_block_id(buffer->records[j]));
        avg_avg_rating += (float)*(buffer->records[j] + storage->tconst_size) / 10;
      }
      if (buffer->size == N)
        buffer = buffer->ptr[0];
      else
        break;
    }

    index++;

    // jump to next leaf node
    if (index == temp->size)
    {
      temp = temp->ptr[N];
      index = 0;
      if (temp->key[0] <= y)
        nodes_accessed++;
    }
  }

  avg_avg_rating /= count;
  // Stop timer
  auto stop = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);

  cout << "Index nodes accessed: " << nodes_accessed << endl;
  cout << "Number of data blocks accessed: " << blocks_accessed.size() << endl;
  cout << "Average of averageRatings: " << avg_avg_rating << endl;
  cout << "Runtime of retrieval process: " << duration.count() << "microseconds" << endl;
  storage->experiment4(x, y);
}

void BPlusTree::experiment5(int x, Storage *storage)
{
  cout << "Experiment 5" << endl;
  if (root == NULL)
  {
    cout << "Empty" << endl;
    return;
  }
  // Start timer
  auto start = chrono::high_resolution_clock::now();
  deleteKey(x);

  // Stop timer
  auto stop = chrono::high_resolution_clock::now();
  auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);

  cout << "Number of Nodes: " << this->nodes << endl;
  cout << "Number of Levels: " << this->levels << endl;
  cout << "Keys of Root Node:" << endl;
  if (root != NULL)
  {
    for (int i = 0; i < root->size; i++)
    {
      cout << "Key " << i << ": " << root->key[i] << endl;
    }
  }
  cout << "Runtime of retrieval process: " << duration.count() << "microseconds" << endl;
  storage->experiment5(x);
}

void BPlusTree::createTreeFromStorage(Storage *storage)
{
  // iterate through records
  // each record in storage takes up 15bytes
  // the basePtr points to the first byte in the first record
  // as such, the first 10bytes represents tconst of the first record
  // the 11th byte represents the rating
  // the 12th - 15th byte represents numVotes
  // the 16th byte represents the first byte in the second record
  // so on n so forth
  unsigned char *curPtr = storage->basePtr;
  int curRecord = 0;
  int block_no = 1;
  while (curRecord < storage->numRecords)
  {
    this->insertKey(storage->convertBytesToInt(curPtr + storage->tconst_size + storage->rating_size), curPtr);

    // uncomment this section of code to print every record in terminal
    // please uncomment line 575-577 below if you comment out this section of code
    // section start//
    //  if (curRecord%storage->max_records_per_block == 0) std::cout << "Block: " << block_no << std::endl;
    //  if(*curPtr == '\0') std::cout << "Record does not exist" << std::endl;
    //  else{
    //    for(int i = 0; i< storage->tconst_size;i++){
    //      if(*curPtr != '\0') std::cout << *curPtr;
    //      curPtr++;
    //    }
    //    std::cout <<"\t";
    //    std::cout << (float)*curPtr/10 << "\t";
    //    curPtr++;
    //    std::cout << storage->convertBytesToInt(curPtr) << std::endl;
    //    curPtr+=4;
    //  }
    // section end

    // keeps track of number of records iterated
    // this is also used to track which block we are in
    curRecord++;

    // uncomment this line of code if you comment out the previous code section
    curPtr += storage->record_size;
    //  storage->display_record(curPtr);
    // jumps to the first byte of the next record

    // if curRecord%store->max_records_per_block is zero
    // it means that the current block is full
    if (curRecord % storage->max_records_per_block == 0)
    {
      // points to the first byte of the first record in the next block
      curPtr += storage->excess_block_size;
      block_no++;
      // std::cout << std::endl;
    }
  }
}

void BPlusTree::displayRecords(Storage *storage)
{
  // level order traversal
  if (!root)
  {
    return;
  }
  Node *cur = root;
  while (!cur->IS_LEAF)
  {
    cur = cur->ptr[0];
  }
  while (cur != NULL)
  {
    for (int j = 0; j < cur->size; j++)
    {
      Node *buffer = cur->ptr[j];
      while (true)
      {
        for (int i = 0; i < buffer->size; i++)
        {
          storage->display_record(buffer->records[i]);
        }
        if (buffer->size == N)
          buffer = buffer->ptr[0];
        else
          break;
      }
    }
    cur = cur->ptr[N];
  }
}

void BPlusTree::display()
{
  // level order traversal
  if (root == NULL) return;
  queue<Node *> q;
  q.push(root);
  int currentLevelNodes = 1;
  int nextLevelNodes = 0;

  while (!q.empty())
  {
    Node *node = q.front();
    q.pop();
    currentLevelNodes--;

    for (int i = 0; i < node->size; i++)
    {
      cout << node->key[i] << " ";
    }
    cout << ",";
    if (!node->IS_LEAF)
    {
      for (int i = 0; i < node->size + 1; i++)
      {
        q.push(node->ptr[i]);
        nextLevelNodes++;
      }
    }

    if (currentLevelNodes == 0)
    {
      cout << endl; // print a new line for every layer
      currentLevelNodes = nextLevelNodes;
      nextLevelNodes = 0;
    }
  }
}

// helper functions
void BPlusTree::deallocate(Node *node)
{
  delete[] node->key;
  delete[] node->ptr;
  delete node;
}

Node *BPlusTree::createNewLeafNode(int x, unsigned char *record)
{
  Node *node = new Node;
  node->key[0] = x;
  // node->records[0] = record;
  node->IS_LEAF = true;
  node->size = 1;

  node->ptr[0] = createNewBufferNode(x, record);
  node->ptr[N] = NULL;
  return node;
}

Node *BPlusTree::createNewBufferNode(int x, unsigned char *record)
{
  Node *node = new Node;
  node->records[0] = record;
  node->size = 1;
  node->key[0] = x;
  node->ptr[0] = NULL;
  return node;
}

// returns [Node* parentOfLeafNode, Node *leafNode]
Node **BPlusTree::traverseToLeafNode(int x)
{
  Node **parent_child = new Node *[2];
  parent_child[1] = root;
  // To travel to the leaf node
  while (parent_child[1]->IS_LEAF == false)
  {
    parent_child[0] = parent_child[1];
    for (int i = 0; i < parent_child[1]->size; i++)
    {
      if (x < parent_child[1]->key[i])
      {
        parent_child[1] = parent_child[1]->ptr[i];
        break;
      }
    }
    if (parent_child[0] == parent_child[1])
      parent_child[1] = parent_child[1]->ptr[parent_child[1]->size];
  }
  return parent_child;
}

Node *BPlusTree::findParent(Node *curNode, Node *child)
{
  // ignore first and second level as child is at least second level
  // so the parent must at least be third level
  if (curNode->IS_LEAF || (curNode->ptr[0]->IS_LEAF))
    return NULL;
  for (int i = 0; i < curNode->size + 1; i++)
  {
    // found direct parent
    if (curNode->ptr[i] == child)
      return curNode;
    // check child of curNode to be parent of child
    else
    {
      Node *parent = findParent(curNode->ptr[i], child);
      if (parent != NULL)
        return parent;
    }
  }
  return NULL;
}