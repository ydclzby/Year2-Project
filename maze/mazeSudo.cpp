#include "vector"
#include "string"
#include "cmath"
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

int turnLeft(int angle);
int turnRight(int angle);
int turn180(); // return current orientation
void rotate();
void moveForward();
void moveStop();
bool scanFront();
double rightDistance; // ultrasonic
double leftDistance; // ultrasonic
const double LRTHRESHOLD = 40;
int setOrientation(int angle);
bool scanRight();
bool scanLeft();
std::vector<int> turnUntilBeaconInMiddle(); // return angle and color R = 0, Y = 1, B = 2 to turn to get to beacon
int trueCount(std::vector<bool> vec);
int orientation = NORTH; // 0 = north, 1 = east, 2 = south, 3 = west
const int BEACONDISTANCE = 5;
const int POSITIONDIVIATION = 5;

bool scanLeft(){
    return leftDistance > LRTHRESHOLD;
}

bool scanRight(){
    return rightDistance > LRTHRESHOLD;
}

struct position{
    int x;
    int y;
};
struct node{
    position pos;
    std::vector<bool> open;
};

std::vector<node> visitedNodes;

position computePosition(){
    position pos;
    orientation = setOrientation(90);
    int turnForRed;
    int turnForBlue;
    for(int i = 0; i < 3; i ++){
        std::vector<int> turn = turnUntilBeaconInMiddle();
        if(turn[1] == 0){
            turnForRed = turn[0];
        }else if(turn[1] == 1){
            int turnForYellow = turn[0];
        }else if(turn[1] == 2){
            turnForBlue = turn[0];
        }
    }
    int turnForBlue = 360 - turnForBlue - turnForRed;
    pos.x = 3*BEACONDISTANCE / (tan(turnForBlue) + tan(turnForRed));

}

int checkNodeVisited(position pos){
    for(int i = 0; i < visitedNodes.size(); i++){
        if(visitedNodes[i].pos.x - pos.x < POSITIONDIVIATION && visitedNodes[i].pos.y - pos.y < POSITIONDIVIATION){
            return i;
        }
    }
    return -1;
}

// prereq: only have 2 true in vector
// return which direction to turn

std::string computeDirection(std::vector<bool> open){ 
    if(open[NORTH] && orientation != SOUTH){
        switch(orientation){
            case 0:
                return "forward";
                break;
            case 1:
                return "left";
                break;
            case 3:
                return "right";
                break;  
            default:
                return "forward";
                break;
        }
    }
    else if(open[EAST] && orientation != WEST){
        switch(orientation){
            case 0:
                return "right";
                break;
            case 1:
                return "forward";
                break;
            case 2:
                return "left";
                break;  
            default:
                return "forward";
                break;
        }
    }
    else if(open[SOUTH] && orientation != NORTH){
        switch(orientation){
            case 1:
                return "right";
                break;
            case 2:
                return "forward";
                break;
            case 3:
                return "left";
                break;  
            default:
                return "forward";
                break;
        }
    }
    else if(open[3] && orientation != 1){
        switch(orientation){
            case 0:
                return "left";
                break;
            case 2:
                return "right";
                break;
            case 3:
                return "forward";
                break;  
            default:
                return "forward";
                break;
        }
    }
    return "error";
}


std::vector<bool> scanOpen(){ // north, east, south, west
    std::vector<bool> open = {false, false, false, false};
    if(orientation == NORTH){
        open[0] = scanFront();
        open[1] = scanRight();
        open[SOUTH] = true;
        open[3] = scanLeft();
    }
    else if(orientation == EAST){
        open[0] = scanLeft();
        open[1] = scanFront();
        open[2] = scanRight();
        open[WEST] = true;
    }
    else if(orientation == SOUTH){
        open[NORTH] = true;
        open[1] = scanLeft();
        open[2] = scanFront();
        open[3] = scanRight();
    }
    else if(orientation == WEST){
        open[0] = scanRight();
        open[EAST] = true;
        open[2] = scanLeft();
        open[3] = scanFront();
    }
    return open;
};

void moveCTRL(int route){

}

void loop(){
    std::vector<bool> open = scanOpen();
    int route = trueCount(open) - 1;
    if(route == 0){ // determine if falsely flagged sloped wall as deadend
        // check if 4 corners are open
        // NE, SE, SW, NW
        std::vector<bool> fourCornerOpen = {false , false, false, false}; 
        orientation = turnLeft(45);
        fourCornerOpen[0] = scanFront();
        fourCornerOpen[1] = scanRight();
        fourCornerOpen[3] = scanLeft();
        orientation = turnRight(90);
        fourCornerOpen[2] = scanRight();
        int notADeadEnd = trueCount(fourCornerOpen);
        if(notADeadEnd == 0){ // it's infact a deadend
            turnRight(135);
            moveForward();
        }else{ 
            moveForward();
        }
    }else if(route == 1){
        if("forward" == computeDirection(open)){
            moveForward();
        }else if("left" == computeDirection(open)){
            orientation = turnLeft(45);
            moveForward();
        }else if("right" == computeDirection(open)){
            orientation = turnRight(45);
            moveForward();
        }
    }else if(route > 1){
        std::vector<bool> currentOpen = scanOpen();
        int currentOrientation = orientation;
        position pos = computePosition();
        int nodeIndex = checkNodeVisited(pos);
        if(nodeIndex == -1){
            visitedNodes.push_back({pos, currentOpen});
        }else{
            currentOpen = visitedNodes[nodeIndex].open;
            // go to the opening and make that opening in vector to false
            // priority is EAST NORTH WEST SOUTH

        }
        
    }
}
