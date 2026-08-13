#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>
#include <random>
#include <array>

std::default_random_engine rng(std::random_device{}());





/*
buy

*/
enum class OrderSide{
    BUY, SELL
    //0, 1
    };

enum class OrderType{
    //0, 1, 2, 3, 4
    LIMIT, IOC, FOK, MARKET, STOP_MARKET, STOP_LIMIT
};

struct PriceLevel;

struct GeneratedOrder
{
    int id;
    int quantity;
    double price;
    /*
    double trigger_price;
    double limit_price;
    */
    bool is_buy;
    OrderType type;

    GeneratedOrder(
        int id_,
        int qty_,
        double price_,
        bool buy_,
        OrderType type_)
        :
        id(id_),
        quantity(qty_),
        price(price_),
        is_buy(buy_),
        type(type_)
    {}
};

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
    //vector to keep track of the stop orders
    std::vector<GeneratedOrder> stop_orders;
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

    //matching limit orders
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
        int available = 0;

        if(is_buy){
            auto it = asks.begin();
            while(it!=asks.end() && it->first<=p){
                available+=it->second->total_quantity;
                it++;
            }
            if(available>=qty){
                match_orders(qty, p,is_buy);
            }
        }
        else{
            auto it = bids.begin();
            while(it!=bids.end() && it->first>=p){
                available+=it->second->total_quantity;
                it++;
            }
            if(available>=qty){
                match_orders(qty, p,is_buy);
            }
        }
    }

    void add_order(GeneratedOrder &order){

 
        //Immediate or cancel
        if(order.type==OrderType::IOC){
            //IOC
            immediate_cancel(order.quantity ,order.price, order.is_buy);
        }
        else if (order.type==OrderType::FOK){
            //FOK
            fill_kill(order.quantity, order.price, order.is_buy);
        }
        else if(order.type==OrderType::LIMIT){
            //regular limit order
            int qty = match_orders(order.quantity, order.price, order.is_buy);
            if(qty>0){
                add_order(order.id, order.quantity, order.price, order.is_buy);
            }
        }
        else if(order.type==OrderType::STOP_MARKET){
            stop_orders.push_back(order);
        }
        else if (order.type==OrderType::MARKET){
            //market order
            market_order(order.quantity, order.is_buy);

        }
        else if(order.type==OrderType::STOP_LIMIT){
            stop_orders.push_back(order);
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
        //std::cout<<"Added order"<<" "<<((is_buy)?"buy":"sell")<<" id:"<<id<<" "<<"there are "<<qty<<" shares at "<<p<<"\n";

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
        //std::cout<<"Erased order"<<" "<<id<<"\n";
        order_map.erase(it);
        delete order;
    }

    void print_book()
{
    std::cout << "\n========== ORDER BOOK ==========\n";

    std::cout << "\nASKS\n";

    for (const auto& [price, level] : asks)
    {
        std::cout << price
                  << " | total: "
                  << level->total_quantity
                  << " | orders: ";

        Order* temp = level->head;

        while (temp != nullptr)
        {
            std::cout << "[ID " << temp->order_id
                      << ", QTY " << temp->quantity
                      << "] ";

            temp = temp->right;
        }

        std::cout << "\n";
    }

    std::cout << "\nBIDS\n";

    for (const auto& [price, level] : bids)
    {
        std::cout << price
                  << " | total: "
                  << level->total_quantity
                  << " | orders: ";

        Order* temp = level->head;

        while (temp != nullptr)
        {
            std::cout << "[ID " << temp->order_id
                      << ", QTY " << temp->quantity
                      << "] ";

            temp = temp->right;
        }

        std::cout << "\n";
    }

    std::cout << "================================\n";
}


    double random_price(double &price, std::default_random_engine& rng){
        
        double tick = 0.25;

        std::normal_distribution<double>d(0.0, 1);
        
        int tick_change = static_cast<int>(std::round(d(rng)));
        price+=tick_change*tick;
        price = std::max(price, tick);
        return price;
        
    }

    //determining the different orders we need to generate
    OrderType random_order_type(std::default_random_engine& rng){
       
        std::uniform_int_distribution<int>d(0,99);
        
        int x = d(rng);

        if(x<65) return OrderType::LIMIT;
        if(x<70) return OrderType::MARKET;
        if(x<80) return OrderType::IOC;
        if(x<90) return OrderType::FOK;
        if(x<95) return OrderType::STOP_LIMIT;

        return OrderType::STOP_MARKET;
    }

    //how much do you want to buy
    int random_quantity(std::default_random_engine& rng){
        
        std::uniform_int_distribution<int>d(100, 1000);
        return d(rng);
        
    }

    //are you a buyer or a seller
    bool random_side(std::default_random_engine & rng){
        
        std::bernoulli_distribution d(0.5);
        return d(rng);
    }

    //stop market orders
    void check_stop_orders(double last_trade_price){

        for(auto it = stop_orders.begin(); it!=stop_orders.end();){
            bool triggered = 
            (it->is_buy && last_trade_price>=it->price)||(!it->is_buy && last_trade_price<=it->price);

            if(triggered && it->type==OrderType::STOP_MARKET){
                market_order(it->quantity, it->is_buy);
                it = stop_orders.erase(it);
            }
            else if(triggered && it->type==OrderType::STOP_LIMIT){
                int qty = match_orders(it->quantity, it->price, it->is_buy);
                if(qty>0){
                    add_order(it->id, it->quantity, it->price, it->is_buy);
                }
                it = stop_orders.erase(it);
            }
            else{
                ++it;
            }

        }
    }

    double mid_price(){
        if(!asks.empty() && !bids.empty()){
            return (asks.begin()->first+bids.begin()->first)/2.0;
        }
        return 0.0;
    }

    double spread() {
        if (!asks.empty() && !bids.empty()) {
            return asks.begin()->first - bids.begin()->first;
        }
        return 0.0;
}   

    std::pair<double, double> vwap(){

        if(asks.empty() || bids.empty()) return {0.0, 0.0};

        double ask_vwap = 0;
        double bid_vwap = 0;
        int count = 0;
        int ask_quantity = 0;
        int bids_quantity = 0;
        for(auto it = asks.begin();count<5&& it!=asks.end() ;){
            ask_vwap+=(it->first*it->second->total_quantity);
            count++;
            ask_quantity+=it->second->total_quantity;
            it++;
        }
        
        count = 0;
        for(auto it = bids.begin();count<5 && it!=bids.end() ;){
            bid_vwap+=(it->first*it->second->total_quantity);
            count++;
            bids_quantity+=it->second->total_quantity;
            it++;
        }
        
        return {ask_vwap/ask_quantity,bid_vwap/bids_quantity };
    }

};


int main()
{
    LimitOrderBook book;

    double current_price = 100;

    std::vector<GeneratedOrder>orders;
    orders.reserve(10000000);

    for(int i = 0;i<10000000;i++){
        GeneratedOrder order(i, book.random_quantity(rng), book.random_price(current_price,rng), book.random_side(rng), book.random_order_type(rng));
        orders.emplace_back(order);
    }

    auto start = std::chrono::steady_clock::now();

    for(auto& order_:orders){
        book.add_order(order_);
    }

    auto end = std::chrono::steady_clock::now();

    std::cout << std::chrono::duration<double, std::micro>(end - start).count()
          << " microseconds\n";


    

   
}
