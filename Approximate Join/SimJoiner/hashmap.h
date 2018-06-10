#ifndef HASH_MAP_H_
#define HASH_MAP_H_

#include <stdio.h>

template<class Key, class Value>
class HashNode
{
public:
    Key    _key;
    Value  _value;
    HashNode *next;

    HashNode(Key key, Value value) {
        _key = key;
        _value = value;
        next = NULL;
    }
    ~HashNode() {}
    HashNode& operator=(const HashNode& node) {
        _key = node._key;
        _value = node._key;
        next = node.next;
        return *this;
    }
};

template <class Key, class Value, class HashFunc, class EqualKey>
class HashMap
{
public:
    Value ValueNULL;
    
public:
    HashMap(int size);
    ~HashMap();
    bool insert(const Key& key, const Value& value);
    bool del(const Key& key);
    Value& find(const Key& key);
    Value& operator [](const Key& key);
    Value* findp(const Key& key);

private:
    HashFunc hash;
    EqualKey equal;
    HashNode<Key, Value> **table;
    unsigned int _size;
    
};

template <class Key, class Value, class HashFunc, class EqualKey>
HashMap<Key, Value, HashFunc, EqualKey>::HashMap(int size) : _size(size)
{
    hash = HashFunc();
    equal = EqualKey();
    table = new HashNode<Key, Value>*[_size];
    for (unsigned i = 0; i < _size; i++)
        table[i] = NULL;
}

template <class Key, class Value, class HashFunc, class EqualKey>
HashMap<Key, Value, HashFunc, EqualKey>::~HashMap()
{
    for (unsigned i = 0; i < _size; i++)
    {
        HashNode<Key, Value> *currentNode = table[i];
        while (currentNode)
        {
            HashNode<Key, Value> *temp = currentNode;
            currentNode = currentNode->next;
            delete temp;
        }
    }
    delete table;
}

template <class Key, class Value, class HashFunc, class EqualKey>
bool HashMap<Key, Value, HashFunc, EqualKey>::insert(const Key& key, const Value& value)
{
    int index = hash(key)%_size;
    int last_index = -1;
    HashNode<Key, Value> * node = new HashNode<Key, Value>(key,value);
    if(node == NULL) {printf("Apply failed.\n"); exit(0);}
    while(table[index] != NULL) {
        last_index = index;
        index = (index + 1) % _size;
    }
    if(last_index != -1) table[last_index]->next = table[index];
    node->next = NULL;
    table[index] = node;
    return true;
}


template <class Key, class Value, class HashFunc, class EqualKey>
bool HashMap<Key, Value, HashFunc, EqualKey>::del(const Key& key)
{
    unsigned index = hash(key) % _size;
    HashNode<Key, Value> * node = table[index];
    HashNode<Key, Value> * prev = NULL;
    while (node)
    {
        if (node->_key == key)
        {
            if (prev == NULL)
            {
                table[index] = node->next;
            }
            else
            {
                prev->next = node->next;
            }
            delete node;
            return true;
        }
        prev = node;
        node = node->next;
    }
    return false;
}

template <class Key, class Value, class HashFunc, class EqualKey>
Value& HashMap<Key, Value, HashFunc, EqualKey>::find(const Key& key)
{
    unsigned index = hash(key) % _size;
    if (table[index] == NULL)
        return ValueNULL;
    else
    {
        HashNode<Key, Value> * node = table[index];
        while (node)
        {
            if (node->_key == key)
                return node->_value;
            node = node->next;
        }
    }
}

template <class Key, class Value, class HashFunc, class EqualKey>
Value* HashMap<Key, Value, HashFunc, EqualKey>::findp(const Key& key)
{
    unsigned index = hash(key) % _size;
    
    if (table[index] == NULL) {
        //printf("F: %s\n", key.c_str());
        return &ValueNULL;
    } else {
        HashNode<Key, Value> * node = table[index];
        //printf("T: %s\n", key.c_str());
        while (node)
        {
            //printf("Node: %s\n", node->_key.c_str());
            if (node->_key == key)
                return &node->_value;
            node = node->next;
        }
        return &ValueNULL;
    }
}

template <class Key, class Value, class HashFunc, class EqualKey>
Value& HashMap<Key, Value, HashFunc, EqualKey>::operator [](const Key& key)
{
    return find(key);
}

class HashFunc
{
public:
    int operator()(const string & key)
    {
        const char *s = key.c_str();
	    const unsigned int seed = 13131313; unsigned int value = 0;
	    while (*s) value = value * seed + (*s++);
	    return value;
    }
};


class EqualKey
{
public:
    bool operator()(const string & A, const string & B)
    {
        if (A.compare(B) == 0)
            return true;
        else
            return false;
    }
};

#endif