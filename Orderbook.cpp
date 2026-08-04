#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>


struct PriceLevel;

struct Order{
    int order_id; //this is the ID for the order, each order will have one
    int quantity; //how many shares does this person want to buy
    double price; //the price they want to buy at
    bool is_buy; //what kind of order is this, buy order or sell order
    Order* left; //order before you, closest to the head
    Order* right; //order after you, closest to the tail

    PriceLevel* level; //pointer back to the parent level
    
    Order():order_id(0),quantity(0), price(0.0),is_buy(false),left(nullptr),right(nullptr), level(nullptr){}
};

struct PriceLevel{

    double price; //current price level
    int total_quantity; //the total amount of shares from the orders at the price level
    Order* head; // references the top of the order book: eldest orders
    Order* tail; //references the bottom of the order book: most recent order

    PriceLevel(const double p):price(p), total_quantity(0), head(nullptr), tail(nullptr){}

};

class LimitOrderBook{
    private:
    //bids: highest price first
    std::map<double, PriceLevel*, std::greater<double>>bids;
    //asks
    std::map<double, PriceLevel*, std::less<double>>asks;

    //O(1) lookup for cancellations
    std::unordered_map<int, Order*>order_map;

    public:
    void cleanup_empty_level(PriceLevel* level, bool is_buy){
        if(!level) return;
        if(level->total_quantity==0 && level->head==nullptr){
            if(is_buy){
                bids.erase(level->price);
            }
            else asks.erase(level->price);
            delete level;
        }
    }

    void add_order(int id, int qty, double p, bool is_buy){

    //create a new order
        Order* order = new Order();
        order->order_id = id;
        order->quantity = qty;
        order->price = p;
        order->is_buy = is_buy;
        
        
        if(is_buy){
            if(bids.find(p)==bids.end()){
                bids[p] = new PriceLevel(p);
            }

            PriceLevel* level = bids[p];
            order->level = level;
        }
        else{
            if(asks.find(p)==asks.end()){
                asks[p] = new PriceLevel(p);
            }
            PriceLevel* level = asks[p];
            order->level = level;
        }

        PriceLevel* level = order->level;
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
        order_map[id] = order;
        std::cout<<"Added order"<<" "<<((is_buy)?"buy":"sell")<<" id:"<<id<<" "<<"there are "<<qty<<" shares at "<<p<<"\n";

}
    void cancel_order(int id){

        //find the order
        auto it = order_map.find(id);

        if(it==order_map.end()){
            std::cerr<<"Order not found"<<" "<<id<<"\n";
            return;
        }

        //find the order
        Order* order = it->second;
        //find the level
        PriceLevel* level = order->level;

        if(order->left){
            order->left->right = order->right;
        }
        else{
            level->head = order->right;
        }

        if(order->right){
            order->right->left = order->left;
        }
        else{
            level->tail = order->left;
        }

        level->total_quantity-=order->quantity;
        std::cout<<"Erased order"<<" "<<id<<"\n";
        order_map.erase(it);
        delete order;
    }


};

void removeorder(PriceLevel* level, Order* order){

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
        Order* curr = level->head->right;
        Order* prev = level->head;
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
    

}



#include <random>

int main()
{
    LimitOrderBook lob;

    std::mt19937 rng(42);

    std::uniform_int_distribution<int> qty_dist(1, 1000);
    std::uniform_real_distribution<double> price_dist(95.0, 105.0);
    std::bernoulli_distribution side_dist(0.5);

    for (int id = 1; id <= 50000000; ++id)
    {
        lob.add_order(
            id,
            qty_dist(rng),
            price_dist(rng),
            side_dist(rng)
        );
    }

    std::uniform_int_distribution<int> cancel_dist(1, 1000);

for (int i = 0; i < 300; ++i)
{
    int id = cancel_dist(rng);

    lob.cancel_order(id);
}
}
