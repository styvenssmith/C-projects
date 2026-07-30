#include <iostream>

template<typename T>
class dynamic_array{

    private:
        size_t sz;
        size_t cap;
        T* arr;
        
        void grow(){
            std::cout<<"We are resizing at "<<sz<<" to new size of "<<cap*2<<"\n";
            size_t new_capacity = (cap==0)? 1:cap*2;
            T* new_arr = new T[new_capacity];
            for(size_t i = 0;i<sz;i++){
                new_arr[i] = std::move(arr[i]);
            }
            delete[] arr;
            arr = new_arr;
            cap = new_capacity;
        }


    public:
        
        dynamic_array():sz(0), cap(0), arr(nullptr){}
        ~dynamic_array(){delete [] arr;}

        void push_back(const T&val){
            if(sz==cap) grow();
            arr[sz++] = val;
        }
        void pop_back(){
            if(sz>0){
                --sz;
            }
        }
        T& operator[](size_t idx) {return arr[idx];}

        size_t size() const {return sz;}
        size_t capacity() const {return cap;}


};




int main(){
    dynamic_array<int>arr;

    for(int i = 0;i<500;i++){
        arr.push_back(i*5);
    }
    std::cout<<"the value at position 30 is "<<arr[30]<<"\n";
    return 0;
}
