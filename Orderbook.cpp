#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>

/*
buy

*/

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

enum class OrderSide{
    BUY, SELL
    //0, 1
    };

enum class OrderType{
    //0, 1, 2
    REGULAR, IOC, FOK
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


    //matching market orders
    /*
        1. type = are you a buyer or a seller
    */
    void match_orders(int qty,double price, bool is_buy){
        //match all orders 
        /*
            type = 0 = buy 
        */

        if(is_buy==1){
            auto it = asks.begin();
            
            //start with the most aggressive sellers
            while(it!=asks.end() && it->first<=price && qty>0){
                
                auto level = it->second;
                //qty is greater than amount at level
                    if(qty>=level->total_quantity){
                        //update the quantity, we remove the entire price level
                        qty-=level->total_quantity;
                        delete level; //frees the memory
                        it = asks.erase(it); //returns the next iterator
                        
                    }
                        //qty is less than amount at level
                        //partial fill
                    else{
                        
                        //the oldest order at that level
                        Order* temp = level->head;
                        Order* next_order = nullptr;
                        while(temp!=nullptr && qty>0){
                            //
                            next_order = temp->right;

                            if(qty<temp->quantity){
                                temp->quantity-=qty;
                                level->total_quantity-=qty;
                                qty = 0;
                            }
                            //we remove this order because it was completely filled
                            // Precondition:
                            // qty < level->total_quantity
                            //
                            // Therefore the FIFO loop must terminate because qty becomes 0,
                            // not because temp becomes nullptr.
                            else{
                                qty-=temp->quantity;
                                level->total_quantity-=temp->quantity;
                                level->head = next_order;
                                if(next_order!=nullptr){
                                    next_order->left = nullptr;
                                }
                                else{
                                    level->tail = nullptr;
                                }
                                
                                delete temp;

                            }
                            temp = next_order;
                        }
                        
                    }
                
            }


        }
        else{

        }
    }

    //matching orders in the background (limit orders)
    void match_orders(){

    }
    //Fill as much of my order as you can and cancel what you cant
    //start 
    void immediate_cancel(int qty, double p, bool is_buy){
        

    }

    //
    void fill_kill(int qty, double p, bool is_buy){
        if(is_buy){

        }
        else{

        }
    }

    void add_order(int id, int qty, double p, bool is_buy, OrderType type){
        int order_val = static_cast<int>(type);
        //Immediate or cancel
        if(order_val==1){
            immediate_cancel(qty,p, is_buy);
        }
        else{
            fill_kill(qty, p, is_buy);
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



#include <random>

int main()
{
    LimitOrderBook lob;
std::mt19937 rng(42);

// Order quantities
std::uniform_int_distribution<int> qty_dist(1, 1000);

// Price random walk
double current_price = 100.0;
std::normal_distribution<double> step_dist(0.0, 0.15);  // Mean 0, std dev 15 cents
double drift = 0.0005;  // Tiny upward bias

// Side distribution (buy/sell)
std::bernoulli_distribution side_dist(0.5);

for (int id = 1; id <= 50000000; ++id)
{
    // Random step with drift
    double step = step_dist(rng) + drift;
    current_price += step;
    
    // Keep price in a realistic range
    if (current_price < 0.01) current_price = 0.01;
    if (current_price > 1000.0) current_price = 1000.0;
    
    // Round to nearest cent (optional, for realistic prices)
    current_price = std::round(current_price * 100.0) / 100.0;
    
    lob.add_order(
        id,
        qty_dist(rng),
        current_price,
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
