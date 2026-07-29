#include <iostream>
/*

Custom Static vector, stack allocated no heap


*/
template<typename T, int N>
class static_vector{
    
    private:
        T data[N]{};
        int current_size = 0;
    public:
        static_vector() = default;
        
        bool push_back(const T& val){
            if(current_size>=N) return false;
            data[current_size++] = val;
            return true;
        }
        
        int size(){
            return current_size;
        }
        int capacity(){return N;}
        
        bool full(){return current_size>=N;};
        
        bool empty(){ return current_size==0;}
        
        void pop_back(){
            if(!empty()) current_size--;
        }
        
        T& operator[](const int val){
            return data[val];
        }
        
        T& front(){ return data[0];}
        const T& front() const {return data[0];}
        
        T& back(){return data[current_size-1];}
        const T& back() const {return data[current_size-1];}
        
        T* begin() { return data;}
        T* end() {return data+current_size;}
        
        const T* begin() const {return data;}
        const T* end() const {return data+current_size;}
        
        
        
};

int main(){
    

 return 0;   
    
}
