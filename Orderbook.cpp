#include <iostream>

struct node{
    int order_id; //this is the ID for the order, each order will have one
    int quantity; //how many shares does this person want to buy
    double price; //the price they want to buy at
    bool is_buy; //what kind of order is this, buy order or sell order
    node* left; //order before you, closest to the head
    node* right; //order after you, closest to the tail
    
    node():order_id(0),quantity(0), price(0.0),is_buy(false),left(nullptr),right(nullptr){}
};

struct PriceLevel{

    double price; //current price level
    int total_quantity; //the total amount of shares from the orders at the price level
    node* head; // references the top of the order book: eldest orders
    node* tail; //references the bottom of the order book: most recent order

    PriceLevel(double p):price(p), total_quantity(0), head(nullptr), tail(nullptr){}

};

void add_order(PriceLevel* level, node* new_order){

    new_order->left = level->tail;
    new_order->right = nullptr;

    if(level->head==nullptr){
        level->head = new_order;
        level->tail = new_order;
    }
    else{
        level->tail->right = new_order;
        level->tail = new_order;
    }

    level->total_quantity+=new_order->quantity;
}





int main(){

    
    

    return 0;
}



