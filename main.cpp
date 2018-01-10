#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <queue>
#include <stdio.h>
struct Order{

    double currentTime;
    double aroundTime;

    int stage = 0;
    double arrivalTime;
    double orderTime;
    double brewTime;
    double cost;

    int cashierNo;
    int baristaNo;

    Order(double _arrivalTime, double _orderTime, double _brewTime, double _cost):
            arrivalTime(_arrivalTime), orderTime(_orderTime), brewTime(_brewTime), cost(_cost) {
        currentTime = arrivalTime;
    }



};

struct Cashier{

    bool isBusy = false;
    double busyTime = 0;

};

struct Barista{

    bool isBusy = false;
    double busyTime = 0;

};
// Custom Comparator for Priority Queue - Timeline
struct TimeComparator{

    bool operator()(const Order& order, const Order& _order){

        return order.currentTime > _order.currentTime;
    }

};
// Custom Comparator for Priority Queue - Printing File
struct ArrivalTimeComparator{

    bool operator()(const Order& order, const Order& _order){

        return order.arrivalTime > _order.arrivalTime;
    }

};
// Custom Comparator for Priority Queue - Barista
struct CostComparator{

    bool operator()(const Order& order, const Order& _order){

        return order.cost < _order.cost;
    }

};

template <class Container>
void _split(const std::string& str, Container& cont)
{
    std::istringstream iss(str);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              std::back_inserter(cont));
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cout << "Run the code with the following command: ./project1 [input_file] [output_file]" << std::endl;
        return 1;
    }

    //std::cout << "input file: " << argv[1] << std::endl;
    //std::cout << "output file: " << argv[2] << std::endl;

    std::ifstream infile(argv[1]);
    std::string line;

    std::vector<Order> orders;

    int noOrders;
    int noCashiers;
    int noBaristas;
    // Read from file
    if (infile.is_open())
    {
        getline (infile,line);
        //std::cout << "line: " << line << std::endl;

        noCashiers = stoi(line);
        noBaristas = noCashiers / 3;

        getline (infile,line);
        //std::cout << "line: " << line << std::endl;

        noOrders = stoi(line);

        for (int i = 0; i < noOrders; i++)
        {
            getline(infile, line);

            //std::cout << "line: " << line << std::endl;

            std::vector<std::string> readings;

            _split(line,readings);

            Order newOrder(stod(readings[0]),stod(readings[1]),stod(readings[2]),stod(readings[3]));

            orders.push_back(newOrder);
        }

        infile.close();
    }

    else
    {
        std::cout << "Unable to open file";
    }
    //A vector to keep information about cashiers
    std::vector<Cashier> cashiers;

    for(int i = 0; i < noCashiers; i++){
        Cashier newCashier;
        cashiers.push_back(newCashier);
    }
    //A vector to keep information about baristas
    std::vector<Barista> baristas;

    for(int i = 0; i < noBaristas; i++){
        Barista newBarista;
        baristas.push_back(newBarista);
    }

    // Priority Queue for Timeline
    std::priority_queue<Order, std::vector<Order>, TimeComparator> timeline;
    // Priority Queue for Printing File
    std::priority_queue<Order, std::vector<Order>, ArrivalTimeComparator> exitingOrders;


    for(auto it = orders.begin(); it != orders.end(); it++){
        timeline.push(*it);
    }
    // Cashier Queue for custormers
    std::queue<Order> cashierQueue;
    // Barista Priority Queue for customers
    std::priority_queue<Order, std::vector<Order>, CostComparator> baristaQueue;

    // Maximum Size of Cashier and Barista Queues
    int maxCQ = 0;
    int maxBQ = 0;

    // System Time to keep track events
    double mainTime = 0;
    // Loop Through until there is no element in timeline
    while(!timeline.empty()){
        //Pop up the event from the timeline
        Order order = timeline.top();
        timeline.pop();
        // There are 6 stages:
        /*
         * 0: Order has just arrived
         * 1: Order is waiting for cashiers
         * 2: Order is in cashier
         * 3: Order is waiting for baristas
         * 4: Order is in barista
         * 5: Order has just left
         *
         * */
        switch(order.stage){
            case 0:
                //Order has just came look for an unbusy cashier by looking in the increasing order in terms of cashiers id
                for(int i = 0; i < cashiers.size(); i++){
                    //If there is an unbusy cashier
                    if(!cashiers[i].isBusy){
                        order.cashierNo = i; // Updater orders's cashier no
                        order.stage = 2; // Update order's stage
                        order.currentTime += order.orderTime; // Update order exiting time
                        cashiers[i].isBusy = true; // Update cashier as busy
                        cashiers[i].busyTime += order.orderTime; // Update cashier's busy time
                        timeline.push(order); // Update timeline
                        break;
                    }

                    if( cashiers.size() - i == 1){
                        order.stage = 1;
                        cashierQueue.push(order);

                        if(cashierQueue.size() > maxCQ){ maxCQ = cashierQueue.size(); }

                    }
                }

                break;
            case 2:
                // Looking for unbusy baristas
                for(int i = 0; i < baristas.size(); i++){
                    // If there is an unbusy baristas
                    if(!baristas[i].isBusy){
                        order.stage = 4; // Update order's stage
                        order.currentTime += order.brewTime; // Update order exiting time
                        cashiers[order.cashierNo].isBusy = false; // Update cashier as unbusy
                        order.baristaNo = i; // Update order's barista number
                        baristas[i].isBusy = true; // Update barista as busy
                        baristas[i].busyTime += order.brewTime; // Update barista's working time
                        timeline.push(order); // Add the event to the timeline
                        //If there is a waiting order do the followings
                        if(!cashierQueue.empty()){
                            Order waitingOrder = cashierQueue.front();
                            cashierQueue.pop();

                            waitingOrder.cashierNo = order.cashierNo;
                            waitingOrder.stage = 2;
                            waitingOrder.currentTime = order.currentTime - order.brewTime + waitingOrder.orderTime;
                            cashiers[waitingOrder.cashierNo].isBusy = true;
                            cashiers[waitingOrder.cashierNo].busyTime += waitingOrder.orderTime;
                            timeline.push(waitingOrder);

                        }

                        break;
                    }

                    if( baristas.size() - i == 1){
                        order.stage = 3;
                        cashiers[order.cashierNo].isBusy = false;
                        baristaQueue.push(order);

                        if(baristaQueue.size() > maxBQ){ maxBQ = baristaQueue.size(); }
                        //If there is a waiting order do the followings
                        if(!cashierQueue.empty()){
                            Order waitingOrder = cashierQueue.front();
                            cashierQueue.pop();

                            waitingOrder.cashierNo = order.cashierNo;
                            waitingOrder.stage = 2;
                            waitingOrder.currentTime = order.currentTime + waitingOrder.orderTime;
                            cashiers[waitingOrder.cashierNo].isBusy = true;
                            cashiers[waitingOrder.cashierNo].busyTime += waitingOrder.orderTime;
                            timeline.push(waitingOrder);

                        }
                    }

                }

                break;
            case 4:

                order.stage = 5; // Update order's stage as 5
                baristas[order.baristaNo].isBusy = false; // Update barista as unbusy
                order.aroundTime = order.currentTime - order.arrivalTime; // Calculate turnaround time
                exitingOrders.push(order); // Add to an another queue for printing purposes

                mainTime = order.currentTime;
                // If there is an waiting order do the followings
                if(!baristaQueue.empty()){
                    Order waitingOrder = baristaQueue.top();
                    baristaQueue.pop();

                    waitingOrder.baristaNo = order.baristaNo;
                    waitingOrder.stage = 4;
                    waitingOrder.currentTime = order.currentTime + waitingOrder.brewTime;
                    baristas[waitingOrder.baristaNo].isBusy = true;
                    baristas[waitingOrder.baristaNo].busyTime += waitingOrder.brewTime;
                    timeline.push(waitingOrder);

                }

                break;
        }

    }

    //------------------------------------------------------------------------------------------

    //Writing file Open it
    FILE * pFile = fopen(argv[2], "w");

    //------------------------------------------------------------------------------------------

    //std::cout << std::endl << mainTime << std::endl << maxCQ << std::endl << maxBQ << std::endl;

    fprintf(pFile, "%.2lf\n", mainTime);
    fprintf(pFile, "%d\n", maxCQ);
    fprintf(pFile, "%d\n", maxBQ);

    for(int i = 0 ; i < cashiers.size(); i++){
        //printf("%.2lf", cashiers[i].busyTime / mainTime);
        fprintf(pFile, "%.2lf\n", cashiers[i].busyTime / mainTime);
        //std::cout << std::endl;
        //std::cout << cashiers[i].busyTime / mainTime << std::endl;
    }

    for(int i = 0 ; i < baristas.size(); i++){
        //printf("%.2lf", baristas[i].busyTime / mainTime);
        fprintf(pFile, "%.2lf\n", baristas[i].busyTime / mainTime);
        //std::cout << std::endl;
        //std::cout << baristas[i].busyTime / mainTime << std::endl;
    }

    while(!exitingOrders.empty()){
        fprintf(pFile, "%.2lf\n", exitingOrders.top().aroundTime);
        //std::cout << exitingOrders.top().aroundTime << std::endl;
        exitingOrders.pop();
    }

    fprintf(pFile, "\n");

    //Writing finished for the first part

    //-------------------------------------------------------------------------------------------------------

    for(auto it = orders.begin(); it != orders.end(); it++){
        timeline.push(*it);
    }

    //Second part starts - NO comment will be added the same code just with a small difference

    cashiers.clear();

    for(int i = 0; i < noCashiers; i++){
        Cashier newCashier;
        cashiers.push_back(newCashier);
    }

    baristas.clear();

    std::vector<std::priority_queue<Order, std::vector<Order>, CostComparator>> baristaQueues;

    for(int i = 0; i < noBaristas; i++){
        std::priority_queue<Order, std::vector<Order>, CostComparator> newBaristaQueue;
        baristaQueues.push_back(newBaristaQueue);
    }

    for(int i = 0; i < noBaristas; i++){
        Barista newBarista;
        baristas.push_back(newBarista);
    }

    maxCQ = 0;

    int maxBQs[noBaristas];

    for(int i = 0; i < noBaristas; i++){ maxBQs[i] = 0; }

    mainTime = 0;

    //--------------------------------------------------------------------------------------------------------

    while(!timeline.empty()){

        Order order = timeline.top();
        timeline.pop();

        switch(order.stage){
            case 0:

                for(int i = 0; i < cashiers.size(); i++){
                    if(!cashiers[i].isBusy){
                        order.cashierNo = i;
                        order.stage = 2;
                        order.currentTime += order.orderTime;
                        cashiers[i].isBusy = true;
                        cashiers[i].busyTime += order.orderTime;
                        timeline.push(order);
                        break;
                    }

                    if( cashiers.size() - i == 1){
                        order.stage = 1;
                        cashierQueue.push(order);

                        if(cashierQueue.size() > maxCQ){ maxCQ = cashierQueue.size(); }

                    }
                }

                break;
            case 2:

                //for(int i = 0; i < baristas.size(); i++){


                    if(!baristas[order.cashierNo / 3].isBusy){
                        order.stage = 4;
                        order.currentTime += order.brewTime;
                        cashiers[order.cashierNo].isBusy = false;
                        order.baristaNo = order.cashierNo / 3;
                        baristas[order.cashierNo / 3].isBusy = true;
                        baristas[order.cashierNo / 3].busyTime += order.brewTime;
                        timeline.push(order);

                        if(!cashierQueue.empty()){
                            Order waitingOrder = cashierQueue.front();
                            cashierQueue.pop();

                            waitingOrder.cashierNo = order.cashierNo;
                            waitingOrder.stage = 2;
                            waitingOrder.currentTime = order.currentTime - order.brewTime + waitingOrder.orderTime;
                            cashiers[waitingOrder.cashierNo].isBusy = true;
                            cashiers[waitingOrder.cashierNo].busyTime += waitingOrder.orderTime;
                            timeline.push(waitingOrder);

                        }

                        //break;
                    }else

                    /*if( baristas.size() - i == 1)*/{
                        order.stage = 3;
                        cashiers[order.cashierNo].isBusy = false;
                        baristaQueues[order.cashierNo / 3].push(order);
                        //baristaQueue.push(order);

                        if(baristaQueues[order.cashierNo / 3].size() > maxBQs[order.cashierNo / 3]){
                            maxBQs[order.cashierNo / 3] = baristaQueues[order.cashierNo / 3].size();
                        }
                        //if(baristaQueue.size() > maxBQ){ maxBQ = baristaQueue.size(); }

                        if(!cashierQueue.empty()){
                            Order waitingOrder = cashierQueue.front();
                            cashierQueue.pop();

                            waitingOrder.cashierNo = order.cashierNo;
                            waitingOrder.stage = 2;
                            waitingOrder.currentTime = order.currentTime + waitingOrder.orderTime;
                            cashiers[waitingOrder.cashierNo].isBusy = true;
                            cashiers[waitingOrder.cashierNo].busyTime += waitingOrder.orderTime;
                            timeline.push(waitingOrder);

                        }
                    }

                //}

                break;
            case 4:

                order.stage = 5;
                baristas[order.baristaNo].isBusy = false;
                order.aroundTime = order.currentTime - order.arrivalTime;
                exitingOrders.push(order);

                mainTime = order.currentTime;

                //if(!baristaQueue.empty()){
                if(!baristaQueues[order.cashierNo / 3].empty()){

                    Order waitingOrder = baristaQueues[order.cashierNo / 3].top();
                    //Order waitingOrder = baristaQueue.top();

                    baristaQueues[order.cashierNo / 3].pop();
                    //baristaQueue.pop();

                    waitingOrder.baristaNo = order.baristaNo;
                    waitingOrder.stage = 4;
                    waitingOrder.currentTime = order.currentTime + waitingOrder.brewTime;
                    baristas[waitingOrder.baristaNo].isBusy = true;
                    baristas[waitingOrder.baristaNo].busyTime += waitingOrder.brewTime;
                    timeline.push(waitingOrder);

                }

                break;
        }

    }

    //std::cout << std:: endl << mainTime << std::endl << maxCQ << std::endl/* << maxBQ << std::endl << std::endl*/;
    ;

    fprintf(pFile, "%.2lf\n", mainTime);
    fprintf(pFile, "%d\n", maxCQ);

    for(int i = 0; i < sizeof(maxBQs) / sizeof(maxBQs[0]);i++){
        fprintf(pFile, "%d\n", maxBQs[i]);
        //std::cout << maxBQs[i] << std::endl;
    }

    for(int i = 0 ; i < cashiers.size(); i++){
        fprintf(pFile, "%.2lf\n", cashiers[i].busyTime / mainTime);
        //printf("%.2lf", cashiers[i].busyTime / mainTime);
        //std::cout << std::endl;
        //std::cout << cashiers[i].busyTime / mainTime << std::endl;
    }

    for(int i = 0 ; i < baristas.size(); i++){
        fprintf(pFile, "%.2lf\n", baristas[i].busyTime / mainTime);
        //printf("%.2lf", baristas[i].busyTime / mainTime);
        //std::cout << std::endl;
        //std::cout << baristas[i].busyTime / mainTime << std::endl;
    }

    while(!exitingOrders.empty()){
        fprintf(pFile, "%.2lf\n", exitingOrders.top().aroundTime);
        //std::cout << exitingOrders.top().aroundTime << std::endl;
        exitingOrders.pop();
    }


    fclose(pFile);

    return 0;
}