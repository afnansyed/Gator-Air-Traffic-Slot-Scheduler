
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ostream>
#include <queue>
#include <string>

using namespace std;

int currentTime = 0;              //time value the scheduler is currently considering

class flightRequest{
public:
    int flightID;
    int airlineID;
    int submitTime;
    int priority;
    int duration;
    int runwayID;
    int startTime;          //when a flight begins using its assigned runway
    int endTime;            //when a flight finishes using its assigned runway
    int lifecycl;           //flight lifecycl: pending, schedualed, in progress, landed
    
    //initiazle contructor
    flightRequest(){
        flightID = -1;
        airlineID = -1;
        submitTime = -1;
        priority = -1;
        duration = -1;
        runwayID = -1;
        startTime = -1;
        endTime = -1;
        lifecycl = 0;
    }
};


vector<flightRequest> flights;  //all flights

//store paralle 
vector<int> runways;    //all runwayids
vector<int> runfree;    //stores time when runway is free
vector<int> rwyids;
int rwyct= 1;            //keep track of runwayids added

//nodes for max pairing heap implementation
//key
vector<int> hpri;       //priority node          
vector<int> hsub;       //submit time for node
vector<int> hid;        //flight id for node

vector<int> hpar;       //parent index
vector<int> hleft;      //stores left child of parent
vector<int> hnext;      //stores next sibling of left child
int hroot = -1;           //root node
int hsz = 0;              //index wher node is empty
vector<int> idtoh;      //stores heap index of flight ids

ofstream outFile;       //


//compares node a and node b and checks if node a should be pushed up
bool goup(int a, int b){
    
    int pa = hpri[a];   //priority of a 
    int sa = hsub[a];   //submit time of a
    int ida = hid[a];   //id of a

    int pb = hpri[b];   //prioty of b
    int sb = hsub[b];   //submit tim of b
    int idb = hid[b];   //id of b
    
    
    if(pa > pb){        //if priority of a is greater return true
        return true;
    }
    else if(pa < pb){

        return false;
    }
    else{
        if(sa < sb){    //if submit time of a is smaller go up 
            return true;
        }
        else if(sa > sb){

            return false;

        }
        else{
            if(ida < idb){   //if id of a is smaller go up

                return true;
            }
            else{

                return false;
            }



        }
    }

}


//combines two heaps 
int combine(int a, int b){


    if(a == -1){    //if a is empty return b as the combined node

        return b;
    }

    if(b == -1){
        return a;
    }
    
    int final = -1; //new root
    int n = -1;     //becolme child
    
    bool go = goup(a, b);   //compare to see if root node of a has higher priority 

    if(go == true){ //a has higher priority, it become new root and b become a child
        final = a;  
        n = b;
    }
    else{
        final = b;
        n = a;

    }
    
    int old = hleft[final];
    hleft[final] = n;   //root of b become the left child of a
    hpar[n] = final;    
    hnext[n] = old;     //original lef tchild next child of root b
    
    if(old != -1){      //old child no parent 
        hpar[old] =-1;

    }
    
    return final; //return new root
}

//merge all heaps after root delte
int mergeall(int first){

    if(first == -1){    //check empty
        return -1;

    }
    
    vector<int> childrn;
    int c = first;

    //add all in one vector
    while(c != -1){
        childrn.push_back(c);
        int n =hnext[c];
        hnext[c] = -1;
        c = n;
    }
    
    if(childrn.size() == 0){    //no children
        return -1;
    }

    if(childrn.size() == 1){    //one
        return childrn[0];  //becomes root
    }
    
    vector<int> pass1;  
    //first pass, loop through the children
    for(int i= 0; i < childrn.size(); i++){
        //make adjacent pairs
        if(i+1 < childrn.size()){
            int m = combine(childrn[i] ,childrn[i+1]);
            pass1.push_back(m);

            i++;


        }
        else{
            pass1.push_back(childrn[i]);

        }
    }
    
    int res = pass1[pass1.size()-1];
    //second pass, loop through the pairs saved frim first pass
    for(int i = pass1.size()-2; i>=0; i--){
        //combine the adjacent pairs but from last to firsf
        res = combine(pass1[i], res);
    
    
    }
    
    return res;

}

//remove root from heap
int pop(){

    if(hroot ==-1){ //if empty

        return -1;
    }
    
    int maxf = hid[hroot];
    idtoh[maxf] = -1;
    
    int child = hleft[hroot];

    // remove their parent
    if(child != -1){

        hpar[child] = -1;
    }
    
    //combine all childs
    hroot = mergeall(child);


    return maxf;

}

//add a flight
void push(int f, int p, int s){

    int idx = hsz;  //index in heap that is empty

    hsz++;
    
    //assign all values
    hid[idx] = f;
    hpri[idx] = p;
    hsub[idx] = s;
    hpar[idx] = -1;
    hleft[idx] = -1;
    hnext[idx] = -1;
    
    idtoh[f]= idx;

    //combin 
    hroot = combine(hroot, idx);
}




//change flight priority
void incr(int f, int newp){

    int idx = idtoh[f];
    if(idx == -1){
        return;

    }


    //update priority
    int oldp = hpri[idx];
    hpri[idx]= newp;
    
    if(newp <= oldp){

        return;
    }
    if(idx ==hroot){
        return;
    }
    
    int par = hpar[idx];
    if(par == -1){  //if it is root
        return;
    }
    
    //remove node and combine with root

    if(hleft[par] == idx){      //if leftchild
        hleft[par]= hnext[idx];

    }
    else{   //if other sibing
        int prev = hleft[par];
        bool fnd = false;

        while(fnd == false){
            if(prev == -1){
                break;
            }
            //find the left sibling of this node 
            if(hnext[prev] == idx){
                fnd = true;
                hnext[prev] = hnext[idx];   //assign the left node's right node the right node of this node
            }
            else{
                prev = hnext[prev];
            }
        }
    }
    
    hpar[idx] = -1;
    hnext[idx] = -1;
    
    hroot = combine(hroot, idx);    //combine node witg root
}

//remove node
void remove(int f){
    int idx = idtoh[f];


    if(idx == -1){  //if empty

        return;

    }
    
    if(idx == hroot){   //if root nide
        pop();

        return;
    }
    
    //set priority high
    hpri[idx] = 10000;
    int par =  hpar[idx];

    //remove node node
    if(par != -1){

        if(hleft[par] == idx){  //if left node
            hleft[par] = hnext[idx];

        }

        else{   //if other sibling
            int prev = hleft[par];

            while(prev != -1){
                 //find the left sibling of this node
                if(hnext[prev] ==idx){
                    hnext[prev] = hnext[idx]; //assign the left node's right node the right node of this node

                    break;
                }

                prev = hnext[prev];
            }
        }
    }
    
    hpar[idx] = -1;
    hnext[idx] = -1;
    
    hroot = combine(hroot, idx); //combine root and this node
    
    pop();  //remove root
}

//reset all values in heap
void clearh(){ 
    hroot = -1;
    hsz = 0;
    for(int i= 0; i <1000; i++){
        hpar[i] = -1;
        hleft[i] = -1;
        hnext[i] = -1;
        idtoh[i] = -1;
    
    }
}

//settle flight completions

void settle(){
    vector<int> land;
    vector<int> feta;
    
    //check if end time is less than current time for all flights
    for(int i=0;i< flights.size(); i++){
        if(flights[i].lifecycl!=3 && flights[i].endTime !=-1 && flights[i].endTime <=currentTime){
            land.push_back(i);  //flights that land
            feta.push_back(flights[i].endTime);
        }
    }
    
    // sorted and make fligths with smaller end time first
    for(int i=0; i<land.size(); i++){
        int mn = i; //current end time

        for(int j=i+1; j < land.size();j++){ //compare current to next
            if(feta[j]< feta[mn]){  //if next is less than current, switch
                mn = j;
            }
            else if(feta[j] == feta[mn]){

                if(flights[land[j]].flightID < flights[land[mn]].flightID){
                    mn = j;

            }
            }
        }
        
        if(mn != i){    //curent has new value
            int tmp = land[i];
            land[i] = land[mn];
            land[mn] = tmp;
            
            int tmp2 = feta[i];
            feta[i] = feta[mn];
            feta[mn] = tmp2;
        }
    }
    
    //print earlierst
    for(int i=0; i<land.size(); i++){

        outFile << "Flight " << flights[land[i]].flightID<<" has landed at time "<<feta[i] << endl;

        flights[land[i]].lifecycl = 3;
    }
}

//check if flight needs to be moved to in progress
void lifenext(){

    for(int i=0; i<flights.size(); i++){

        if(flights[i].lifecycl== 1 && flights[i].startTime <= currentTime){ //checdualed + startime is less

            flights[i].lifecycl = 2;    //in progres
        }
    }

}

void resechdule(bool print){

    vector<int> unsat;  //not started
    vector<int> oldeta; //old end times
    
    //loop throguh flights to check flights not started yet
    for(int i=0; i < flights.size(); i++){

        if(flights[i].lifecycl != 3){

            if(flights[i].lifecycl == 0 || (flights[i].lifecycl == 1 && flights[i].startTime > currentTime)){

                //save 
                unsat.push_back(i);
                oldeta.push_back(flights[i].endTime);
            }
        }
    }
    
    //if flights not started
    if(unsat.size() > 0){

        clearh();   //clean heap
        
        //loop through unsatidlifed flights
        for(int i  =0; i < unsat.size(); i++){

            int idx = unsat[i];
            flights[idx].runwayID = -1;
            flights[idx].startTime = -1;
            flights[idx].endTime = -1;
            flights[idx].lifecycl = 0;

            //push flights on heap
            push(flights[idx].flightID, flights[idx].priority, flights[idx].submitTime);
        }
        
        //clear runway time
        runfree.clear();

        //loop through flights currenty on runway and assign the coresponding time
        for(int i=0; i <runways.size(); i++){

            runfree.push_back(currentTime);
        }
        
        //loop through flights current in progress
        for(int i=0; i<flights.size(); i++){

            if(flights[i].lifecycl == 2){   //if flights in progress

                for(int j=0; j < runways.size(); j++){

                    if(runways[j] == flights[i].runwayID){  //if flight on runway

                        runfree[j] = flights[i].endTime;  //chane the runway free time

                    }
                }
            }
        }
        
        bool done = false;
        //schedualing, greedy
        while(done == false){

            if(hroot == -1){    
                done = true;    
            }
            else{
                int fid = pop();    //remove root
                int fidx = -1; 
                //loop throuhg flights to erase flight id
                for(int i=0; i<flights.size(); i++){

                    if(flights[i].flightID == fid){

                        fidx = i;
                    }
                }
                
                int minr = 0; //available runway
                //loop to check available runway
                for(int i=1; i<runways.size(); i++){

                    if(runfree[i] < runfree[minr]){ //compare 
                        minr = i;
                    }
                    else if(runfree[i] == runfree[minr]){ //if same time

                        if(runways[i] < runways[minr]){ //smaller id

                            minr = i;
                        }
                    }
                }
                
                flights[fidx].runwayID = runways[minr]; //asign flight to runway
                
                //find startTime = max (currentTime, runway.nextFreeTime)
                if(currentTime > runfree[minr]){ 

                    flights[fidx].startTime = currentTime;
                }
                else{
                    flights[fidx].startTime = runfree[minr];
                }
                
                //ETA = startTime + duration
                flights[fidx].endTime = flights[fidx].startTime + flights[fidx].duration;
                flights[fidx].lifecycl = 1; //scedyaled
                
                runfree[minr] = flights[fidx].endTime;
            }
        }
        
        if(print == true){  //if need to print
            vector<int> chg;
            
            //loop and compare old end time with new end time
            for(int i=0; i < unsat.size(); i++){

                int idx = unsat[i];
                if(flights[idx].endTime!= oldeta[i]){
                    chg.push_back(idx);
                }
            }
            
            //sort new end times by flight ids
            if(chg.size() > 0){
                for(int i=0; i<chg.size(); i++){

                    for(int j=i+1; j<chg.size(); j++){

                        if(flights[chg[i]].flightID > flights[chg[j]].flightID){

                            int tmp = chg[i];
                            chg[i] = chg[j];
                            chg[j] = tmp;
                        }
                    }
                }
                
                //loop trhough to print
                outFile << "Updated ETAs: [";
                for(int i=0; i<chg.size(); i++){
                    if(i>0){
                        outFile << ", ";
                    }
                    outFile << flights[chg[i]].flightID << ": " << flights[chg[i]].endTime;
                }
                outFile << "]" << endl;
            }
        }
    }
}

//update when current time changes
void timechang(int newt){

    currentTime = newt; //update
    settle();           //complete flights
    lifenext();         //in progress
    resechdule(true);   //rebuild and print end times

}

//Start the system with a given number of runways
void Initialize(int runwayCount){

    if(runwayCount <= 0){
        outFile << "Invalid input. Please provide a valid number of runways." << endl;
    }
    else{
        for(int i=0; i<runwayCount; i++){
            runways.push_back(rwyct);   //Create runways with ids
            runfree.push_back(0);       //run way time
            rwyct++;
        }
        
        //print confirmation
        outFile << runwayCount << " Runways are now available" << endl;
    }
}

//Add a new flight request to the system at a given submitTime
void SubmitFlight(int flightID, int airlineID, int submitTime, int priority, int duration){

    int fid = flightID;
    int aid = airlineID;
    int sub = submitTime;
    int p = priority;
    int d = duration;

    timechang(sub);     //Advance time to submitTime
    
    bool dup = false;
    //check duplicate
    for(int i=0; i<flights.size(); i++){

        if(flights[i].flightID == fid && flights[i].lifecycl != 3){

            dup=true;
        }
    }
    
    if(dup == true){
        outFile << "Duplicate FlightID" << endl;
    }
    else{
        vector<int> oldeta;
        //loop to save current end times of flights
        for(int i=0; i<flights.size(); i++){
            oldeta.push_back(flights[i].endTime);
        }
        
        //new flight
        flightRequest newf;
        newf.flightID = fid;
        newf.airlineID = aid;
        newf.submitTime = sub;
        newf.priority = p;
        newf.duration = d;
        newf.lifecycl = 0;
        flights.push_back(newf);
        
        //resechduall all unsatified flights using greedy
        resechdule(false);
        
        //Print the new flight's assigned end times
        for(int i=0; i<flights.size(); i++){

            if(flights[i].flightID == fid){
                outFile << "Flight " << fid << " scheduled - ETA: " << flights[i].endTime << endl;
            }
        }
        
        vector<int> chg;
        //loop through to check if endtimes changes for other flights
        for(int i=0; i<flights.size()-1; i++){

            if(flights[i].lifecycl!=3 && flights[i].endTime!=oldeta[i]){
                chg.push_back(i);   //save changed
            }
        }
        
        //if end times changed
        if(chg.size() > 0){

            //loop through and update the new end times by comeparing the changed
            for(int i=0; i<chg.size(); i++){
                int mn = i;

                for(int j=i+1; j < chg.size(); j++){

                    if(flights[chg[j]].flightID < flights[chg[mn]].flightID){
                        mn=j;
                    }
                }
                
                if(mn != i){
                    int tmp = chg[i];
                    chg[i] = chg[mn];
                    chg[mn] = tmp;
                }
            }
            
            //print updated entimes by looping through
            outFile << "Updated ETAs: [";

            for(int i=0; i<chg.size(); i++){
                if(i>0){
                    outFile << ", ";
                } 
                outFile << flights[chg[i]].flightID << ": " << flights[chg[i]].endTime;
            }
            outFile << "]" << endl;
        }
    }
}

//Remove a flight that has not started yet
void CancelFlight(int flightID, int currentTime){

    int fid = flightID;
    int t = currentTime;
    timechang(t);   //Advance time to currentTime
    
    int idx = -1;
    //loop through to find flightid
    for(int i=0; i<flights.size(); i++){

        if(flights[i].flightID == fid && flights[i].lifecycl!= 3){
            idx = i;
        }
    }
    
    if(idx == -1){  //not found
        outFile << "Flight " << fid << " does not exist" << endl;
    }
    else{

        if(flights[idx].lifecycl == 2){ //check lifecycle to see if in prigress
            outFile << "Cannot cancel. Flight " << fid << " has already departed" << endl;
        }
        else{
            remove(fid);    //remove flight
            flights[idx].lifecycl= 3;   
            
            outFile << "Flight " << fid << " has been canceled" << endl;
            
            resechdule(true);   //resechdual
        }
    }
}

//Change a flight's priority before it starts 
void Reprioritize(int flightID, int currentTime, int newPriority){

    int fid = flightID;
    int t = currentTime;
    int newp = newPriority;
    timechang(t);   //Advance time to currentTime
    
    int idx = -1;
    //loop thrugh flights to look up flight id
    for(int i=0; i<flights.size(); i++){

        if(flights[i].flightID == fid && flights[i].lifecycl!= 3){
            idx = i;
        }
    }
    
    if(idx == -1){  //not found
        outFile << "Flight " << fid << " not found" << endl;
    }
    else{

        if(flights[idx].lifecycl == 2){ //lifecycle is in progress
            outFile << "Cannot reprioritize. Flight " << fid << " has already departed" << endl;
        }
        else{

            //update priority
            int oldp = flights[idx].priority;
            flights[idx].priority = newp;
            
            if(newp > oldp){       
                incr(fid, newp);    //if priority increased, increase key
            }
            else{
                remove(fid);    //if decreased, remove
                push(fid, newp, flights[idx].submitTime);   //insert flight
            }
            
            outFile << "Priority of Flight "<< fid << " has been updated to " << newp << endl;
            
            resechdule(true);   //resecdule and print changed end times
        }
    }
}

//Increase capacity by add count new runwy 
void AddRunways(int count, int currentTime){

    int cnt = count;
    int t = currentTime;
    timechang(t);   //Advance time to currentTime
    
    if(cnt <= 0){   //count not valid num
        outFile << "Invalid input. Please provide a valid number of runways." << endl;
    }
    else{
        //loop through number of runways
        for(int i=0; i<cnt; i++){
            runways.push_back(rwyct);       //Create count new runways with ids
            runfree.push_back(currentTime); //next free runway is current time
            rwyct++;   
        }
        
        //print
        outFile << "Additional " << cnt << " Runways are now available" << endl;
        
        resechdule(true);   //resecdual and print end times changes
    }
}


//Temporarily block unsatisfied flights id is in range low,high from being scheduled
void GroundHold(int airlineLow, int airlineHigh, int currentTime){

    int low = airlineLow;
    int high = airlineHigh;
    int t = currentTime;
    timechang(t);   //Advance time to currentTime 
    
    if(high < low){
        outFile << "Invalid input. Please provide a valid airline range." << endl;
    }
    else{
        vector<int> torm;
        
        //loop through flights and save flights with id in range low, high and that are in not in progress
        for(int i=0; i<flights.size(); i++){

            if(flights[i].airlineID>=low && flights[i].airlineID<=high){

                if(flights[i].lifecycl== 0 || (flights[i].lifecycl==1 && flights[i].startTime>currentTime)){
                    torm.push_back(i);
                }
            }
        }
        
        //loop through the saved flights
        for(int i=0; i<torm.size(); i++){

            int idx = torm[i];

            remove(flights[idx].flightID);  //remove these flights
            flights[idx].lifecycl = 3;      //landed
        }
        
        outFile << "Flights of the airlines in the range ["<<low<<", "<<high<<"] have been grounded" << endl;
        
        resechdule(true);   //resechdula and print changed entimes
    }
}

//Print all flights still in the system 
void PrintActive(){
    vector<int> act;
    //loop through to save all flight not landed
    for(int i=0; i<flights.size(); i++){

        if(flights[i].lifecycl != 3){
            act.push_back(i);
        }
    }
    
    if(act.size() == 0){
        outFile << "No active flights" << endl;
    }
    else{

        //loop through active flights and sort by flight id
        for(int i=0; i<act.size(); i++){
            int mn = i;

            for(int j=i+1; j<act.size(); j++){

                if(flights[act[j]].flightID < flights[act[mn]].flightID){
                    mn = j;
                }
            }
            
            if(mn != i){
                int tmp = act[i];
                act[i] = act[mn];
                act[mn] = tmp;
            }
        }
        
        //loop through sorted flights and print
        for(int i=0; i<act.size(); i++){

            int idx = act[i];
            outFile << "[flight" << flights[idx].flightID<<", airline" << flights[idx].airlineID << ", runway"<<flights[idx].runwayID << ", start" << flights[idx].startTime << ", ETA" << flights[idx].endTime << "]" << endl;
        }
    }
}

//Print unsatisfied flights with endtimes between t1,t2
void PrintSchedule(int t1, int t2){

    vector<int> schd;
    
    //loop through flights and save flights secdualed and have not started yer
    for(int i=0; i<flights.size(); i++){

        if(flights[i].lifecycl==1 && flights[i].startTime>currentTime){

            if(flights[i].endTime >= t1 && flights[i].endTime <= t2){   //endtimes betwene this interval
                schd.push_back(i);
            }
        }
    }
    
    if(schd.size() == 0){

        outFile << "There are no flights in that time period" << endl;
    }
    else{

        //loop through flights  
        for(int i=0; i<schd.size(); i++){

            for(int j=i+1; j<schd.size(); j++){
                //order by endtime
                if(flights[schd[i]].endTime > flights[schd[j]].endTime){

                    int tmp = schd[i];
                    schd[i] = schd[j];
                    schd[j] = tmp;
                }
                else if(flights[schd[i]].endTime == flights[schd[j]].endTime){
                    //order by flight id
                    if(flights[schd[i]].flightID > flights[schd[j]].flightID){
                        int tmp = schd[i];
                        schd[i] = schd[j];
                        schd[j] = tmp;
                    }
                }
            }
        }
        
        //print flight id
        for(int i=0; i<schd.size(); i++){
            outFile << "[" << flights[schd[i]].flightID << "]" << endl;
        }
    }
}

//Advance the system clock to t
void Tick(int t){
    timechang(t);
}

int main(int argc, char* argv[]){

    string nameFile = argv[1];
    ifstream inFile(nameFile);  
    
    string outn = "";
    //loop through filename to save output file name
    for(int i =0; i<nameFile.length(); i++){
        //save till the period
        if(nameFile[i]=='.'){       
            i = nameFile.length();
        }
        else{
            outn+=nameFile[i];  
        }
    }
    outn+="_output_file.txt";
    
    outFile.open(outn);
    
    //initialize heap
    hpri.resize(1000);
    hsub.resize(1000);
    hid.resize(1000);
    hpar.resize(1000);
    hleft.resize(1000);
    hnext.resize(1000);
    idtoh.resize(1000);
    
    for(int i=0; i<1000; i++){
        hpar[i] = -1;
        hleft[i] = -1;
        hnext[i] = -1;
        idtoh[i] = -1;
    }
    
    //parse through file
    if(inFile.is_open()){
    string line;
    bool done = false;
    
    //read entire line
    while(getline(inFile, line)){

        if(done == false){
            
            //if not empty, remove return char
            while(line.length()>0 && (line[line.length()-1] == '\r' || line[line.length()-1] == '\n')){
                line = line.substr(0, line.length()-1);
            }
            
            
            string cmd="";
            int op=-1;
            
            //
            for(int i=0; i<line.length(); i++){
                if(line[i]=='('){
                    op=i;
                    break;  
                }
                else{
                    cmd+=line[i];   //save command till the open parentesis
                }
            }
            
            vector<int> values;
            
            if(op != -1){
                string prms="";
                for(int i=op+1; i<line.length(); i++){
                    if(line[i]!=')' && line[i]!='\r' && line[i]!='\n'){  //filter \r and \n
                        prms+=line[i];
                    }
                }
                
                //check if string has numbers
                bool hasNum = false;
                for(int i=0; i<prms.length(); i++){
                    if(prms[i] != ' ' && prms[i] != '\t'){
                        hasNum = true;
                        break;
                    }
                }
                
                //if string has numbers then parse through to save the numbers in the vector using the comma as an indicator for one number
                if(hasNum){
                    string cur="";
                    for(int i=0; i<prms.length(); i++){
                        if(prms[i] == ','){
                            if(cur.length() > 0){
                                string num = "";
                                for(int j=0; j<cur.length(); j++){
                                    if(cur[j]!=' ' && cur[j]!='\t' && cur[j]!='\r' && cur[j]!='\n'){
                                        num+=cur[j];
                                    }
                                }
                                if(num.length() > 0){
                                    values.push_back(stoi(num));
                                }
                            }
                            cur="";
                        }
                        else{
                            cur+=prms[i];
                        }
                    }
                    
                   //for the last number
                    if(cur.length() > 0){
                        string num="";
                        for(int j=0; j<cur.length(); j++){
                            if(cur[j]!=' ' && cur[j]!='\t' && cur[j]!='\r' && cur[j]!='\n'){
                                num+=cur[j];
                            }
                        }
                        if(num.length()>0){
                            values.push_back(stoi(num));
                        }
                    }
                }
            }
            
           
            if(cmd == "Initialize"){
                if(values.size() >= 1){
                    Initialize(values[0]);
                }
            }
            else if(cmd == "SubmitFlight"){
                if(values.size() >= 5){
                    SubmitFlight(values[0],values[1],values[2],values[3],values[4]);
                }
            }
            else if(cmd == "CancelFlight"){
                if(values.size() >= 2){
                    CancelFlight(values[0],values[1]);
                }
            }
            else if(cmd == "Reprioritize"){
                if(values.size() >= 3){
                    Reprioritize(values[0],values[1],values[2]);
                }
            }
            else if(cmd == "AddRunways"){
                if(values.size() >= 2){
                    AddRunways(values[0],values[1]);
                }
            }
            else if(cmd == "GroundHold"){
                if(values.size() >= 3){
                    GroundHold(values[0],values[1],values[2]);
                }
            }
            else if(cmd == "PrintActive"){
                PrintActive();
            }
            else if(cmd == "PrintSchedule"){
                if(values.size() >= 2){
                    PrintSchedule(values[0],values[1]);
                }
            }
            else if(cmd == "Tick"){
                if(values.size() >= 1){
                    Tick(values[0]);
                }
            }
            else if(cmd == "Quit"){
                outFile << "Program Terminated!!";  
                done = true;
            }
        }
    }
}
    
    inFile.close();
    outFile.close();
    
    return 0;
}