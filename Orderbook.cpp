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

void add_order(PriceLevel* level, node* order){

    //if the current price level is empty
    if(level->head==nullptr){
        level->head = order;
        level->tail = order;
        order->left = nullptr;
        order->right = nullptr;
    }
    //if there are already orders 
    else{
        order->left = level->tail;
        order->right = nullptr;
        level->tail->right = order;
        level->tail = order;
    }
   
    level->total_quantity+=order->quantity;

}

void removeorder(PriceLevel* level, node* order){

    // the level or the order are null
    if(level==nullptr || order==nullptr ||level->head==nullptr) return;

    if(level->head->order_id==order->order_id){
        if(level->head->right==nullptr){
            level->head = nullptr;
            level->tail = nullptr;
            
        }
        else{
            level->head = level->head->right;
            level->head->left = nullptr;
            
        }
    }
    else{
        node* curr = level->head->right;
        node* prev = level->head;
        bool found = false;

        while(curr!=nullptr){
            if(curr->order_id==order->order_id){
                prev->right = curr->right;
                if(curr->right){
                    curr->right->left = prev;
                }
                else{
                    level->tail = prev;
                }
                found = true;
                break;
                

            }
            prev = curr;
            curr = curr->right;
            
        }
    }

    level->total_quantity-=order->quantity;
    order->left = nullptr;
    order->right = nullptr;
    delete order;

}



int main(){

    PriceLevel* level = new PriceLevel(108.50);
    node* order1 = new node;
    order1->order_id = 1;
    order1->quantity = 200;
    order1->price = 108.50;
    order1->is_buy = true;

    node* order2 = new node;
    order2->order_id = 2;
    order2->quantity = 300;
    order2->price = 108.50;
    order2->is_buy = false;

    add_order(level, order1);
    add_order(level, order2);
    
    auto head = level->head;
    while(head!=nullptr){
        std::cout<<head->quantity<<"\n";
        head = head->right;
    }
    delete level;
    delete order1;
    delete order2;

    return 0;
}



