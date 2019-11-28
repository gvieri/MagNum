#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "common.hpp"

#define LOAD_FACTOR 0.75

class NODE { 
    public: 
        std::string key; Value value; 
        
        NODE(std::string key, Value value) { 
            this->key = key; 
            this->value = value; 
        } 
};

class Table {
    public:        
        ~Table() { delete[] this->array; }

        Table() {
            this->array = new NODE*[this->capacity];
            
            for(int i = 0; i < this->capacity; i++) 
                this->array[i] = nullptr;
        }

        bool insert(std::string key, Value value) {
            NODE* temp = new NODE(key, value);
            int index = this->hash(key);

            while(this->array[index] != nullptr) {
                if(this->array[index]->key == key) return false;

                index++;
                index %= this->capacity;
            }

            if(this->array[index] == nullptr) this->count++;
            this->array[index] = temp;

            if(this->count + 1 > this->capacity * LOAD_FACTOR) this->fix();

            return true;
        }

        bool remove(std::string key) {
            int index = this->hash(key);

            while(this->array[index] != nullptr) {
                if(this->array[index]->key == key) {
                    this->array[index] = nullptr; this->count--; 
                    return true;
                }

                index++;
                index %= this->capacity;
            }

            return false;
        }

        Value get(std::string key) {
            int index = this->hash(key); 
            int counter = 0; 
  
            while(this->array[index] != nullptr) {    
                if(counter++ > this->capacity) return Void::VOID();          
                if(this->array[index]->key == key) return this->array[index]->value; 

                index++; 
                index %= this->capacity; 
            } 
            
            return Void::VOID(); 
        }

        bool set(std::string key, Value value) {
            int index = this->hash(key); 

            while(this->array[index] != nullptr) {    
                if(this->array[index]->key == key) {
                    this->array[index]->value = value; 
                    return true;
                }

                index++; 
                index %= this->capacity; 
            } 
            
            return false; 
        }

    private:
        NODE **array;
        int capacity = 10, count = 0;

        int hash(std::string string) {
            int ascii = 0; 
            for (int i = 0; i < string.length(); i++) ascii += string[i]; 
            return (ascii % this->capacity); 
        }

        void fix() {
            NODE **temp = new NODE*[this->count];

            for(int i = 0, k = 0; i < this->capacity; i++) 
                if(this->array[i] != nullptr) { temp[k] = this->array[i]; k++; }

            this->capacity = this->capacity * 2;
            this->array = new NODE*[this->capacity];

            for(int i = 0; i < this->count; i++) {
                NODE* populate = new NODE(temp[i]->key, temp[i]->value);
                int index = this->hash(temp[i]->key);

                while(this->array[index] != nullptr) {
                    index++;
                    index %= this->capacity;
                }

                this->array[index] = populate;
            } 
        }
};

#endif