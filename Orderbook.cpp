#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>
#include <random>

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
    //0, 1, 2, 3
    LIMIT, IOC, FOK, MARKET
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
    int match_orders(int qty,double price, bool is_buy){
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
                        Order* temp = level->head;
                        while(temp){
                            Order* next = temp->right;
                            delete temp;
                            temp = next;
                        }
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
                                level->head = next_order;
                                delete temp;
                                temp = next_order;
                        
                            }
                            
                        }   
                    }   
            }
        }
        else{
            auto it = bids.begin();

            while(it!=bids.end() && it->first>=price && qty>0){

                
                auto level = it->second;

                //we ingest the entire level
                if(qty>=level->total_quantity){
                    qty-=level->total_quantity;
                    Order* temp = level->head;
                    while(temp){
                        Order* next = temp->right;
                        delete temp;
                        temp = next;
                    }
                    delete level;
                    it = bids.erase(it);
                }
                else{
                    Order* temp = level->head;
                    Order* next_order = nullptr;

                    while(temp!=nullptr && qty>0){

                        next_order = temp->right;

                        if(qty<temp->quantity){
                            temp->quantity-=qty;
                            level->total_quantity-=qty;
                            qty = 0;
                        }
                        else{
                            
                            level->total_quantity-=temp->quantity;
                            qty-=temp->quantity;

                            
                            if(next_order){
                                next_order->left = nullptr;
                            }
                            else{
                                level->tail = nullptr;
                            }
                            level->head = next_order;
                            delete temp;
                            temp = next_order;

                            
                        }
                        
                    }
                }

            }

        }
        return qty;
    }



    //matching orders in the background (limit orders)
    void market_order(int qty, bool is_buy){
        //check the ask map
        if(is_buy){
            auto it = asks.begin();
            
            while(it!=asks.end() && qty>0){
                auto level = it->second;
                //delete the entire level
                if(qty>=level->total_quantity){
                    qty-=level->total_quantity;

                    auto temp = level->head;
                    while(temp!=nullptr){
                        Order*next = temp->right;
                        delete temp;
                        temp = next;
                    }
                    delete level;
                    it = asks.erase(it);
                }
                else{

                    Order* temp = level->head;
                    Order* next_order = nullptr;
                    while(temp!=nullptr && qty>0){

                        next_order = temp->right;

                        if(qty<temp->quantity){
                            temp->quantity-=qty;
                            level->total_quantity-=qty;
                            qty = 0;
                            

                        }
                        else{
                            
                            level->total_quantity-=temp->quantity;
                            qty-=temp->quantity;

                            if(next_order==nullptr){
                                level->tail = nullptr;
                            }
                            else{
                                next_order->left = nullptr;
                            }
                            level->head = next_order;
                            delete temp;
                            temp = next_order;
                            
                        }
                        

                    }
                    
                }
            }
        }
        //check the bid map
        else{

            auto it = bids.begin();

            if(it==bids.end()) return;

            while(it!=bids.end() && qty>0){

                auto level = it->second;
                
                if(level->total_quantity<=qty){
                    Order* temp = level->head;

                    while(temp!=nullptr){
                        Order* t = temp->right;
                        delete temp;
                        temp = t;
                    }
                    delete level;
                    it = bids.erase(it);
                }
                else{
                    Order* temp = level->head;
                    Order* next_order = nullptr;

                    while(temp!=nullptr && qty>0){
                        next_order = temp->right;
                        
                        if(qty<=temp->quantity){
                            temp->quantity-=qty;
                            level->total_quantity-=qty;
                            qty = 0;

                        }
                        else{

                            qty-=temp->quantity;
                            level->total_quantity-=temp->quantity;

                            if(next_order==nullptr){
                                level->tail = nullptr;
                            }
                            else{
                                next_order->left = nullptr;
                            }
                            level->head = next_order;
                            delete temp;
                            temp = next_order;
                        }
                        
                        
                    }
                }
            }
        }
    }


    //Fill as much of my order as you can and cancel what you cant
    //start 
    void immediate_cancel(int qty, double p, bool is_buy){
        match_orders(qty, p, is_buy);

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
            //IOC
            immediate_cancel(qty,p, is_buy);
        }
        else if (order_val==2){
            //FOK
            fill_kill(qty, p, is_buy);
        }
        else if(order_val==0){
            //regular limit order
            qty = match_orders(qty, p, is_buy);
            if(qty>0){
                add_order(id, qty, p, is_buy);
            }
        }
        else{
            //market order
            market_order(qty, is_buy);

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


int main()
{

}
