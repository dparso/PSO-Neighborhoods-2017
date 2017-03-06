#include "PSO.h"

using namespace std;

int MAX_VEL_RAND_VALUE[3] = {2, 4, 4};
int MIN_VEL_RAND_VALUE = -2;
double MAX_POS_RAND_VALUE[3] = {30.0, 32.0, 5.12};
double MIN_POS_RAND_VALUE[3] = {15.0, 16.0, 2.56};
double PHI_1 = 2.05;
double PHI_2 = 2.05;

Particle::Particle(int dimension, int function) {
    pBestValue = INT_MAX;

	for (int i = 0; i < dimension; i++) {
		// generate random value within specified range for each function
		double velRandom = (rand() % (MAX_VEL_RAND_VALUE[function] - MIN_VEL_RAND_VALUE + 1)) + MIN_VEL_RAND_VALUE;
		
		double total = MAX_POS_RAND_VALUE[function] - MIN_POS_RAND_VALUE[function];
		double ratio = ((double) rand())/RAND_MAX;
		double posRandom = total * ratio + MIN_POS_RAND_VALUE[function];
		
		velocity.push_back(velRandom);
		pBest.push_back(posRandom);
		position.push_back(posRandom);
		
	}
}

Particle::~Particle() {
	
}



PSO::PSO(int neighborhood, int swarmSize, int iterations, int function, int dimension) {
	this->neighborhood = neighborhood;
	this->swarmSize = swarmSize;
	this->iterations = iterations;
	this->function = function;
	this->dimension = dimension;
	constrict = 0.7298;
    gBestValue = INT_MAX;
	
	for(int i = 0; i < dimension; i++) {
		// fill gBest with random viable values
		double total = MAX_POS_RAND_VALUE[function] - MIN_POS_RAND_VALUE[function];
		double ratio = ((double) rand())/RAND_MAX;
		double posRandom = total * ratio + MIN_POS_RAND_VALUE[function];
		gBest.push_back(posRandom);
	}
    
    for(int i = 0; i < swarmSize; i++) {
        vector<double> position;
        for(int j = 0; j < dimension; j++) {
            position.clear();
            double total = MAX_POS_RAND_VALUE[function] - MIN_POS_RAND_VALUE[function];
            double ratio = ((double) rand())/RAND_MAX;
            double posRandom = total * ratio + MIN_POS_RAND_VALUE[function];
            position.push_back(posRandom);
            
            
        }
        nBestPos.push_back(position);
        nBestList.push_back(INT_MAX);
    }
    
}

PSO::~PSO() {
	
}

void PSO::updateVelocity(int index) {
	Particle p = swarm[index];
	// iterate through dimensions, updating respective velocities
	for(int i = 0; i < dimension; i++) {
		double pAttract = ((double) rand()/RAND_MAX) * PHI_1 * (swarm[index].pBest[i] - swarm[index].position[i]);
		//cout << swarm[index].pBest[i] << endl;
		double nAttract = ((double) rand()/RAND_MAX) * PHI_2 * (nBestPos[index][i] - swarm[index].position[i]);
		double velChange = pAttract + nAttract;
		swarm[index].velocity[i] += velChange;
		swarm[index].velocity[i] *= constrict;
		
	}
}

void PSO::updatePosition(int index) {
	// iterate through dimensions, updating respective positions based on velocity
	for(int i = 0; i < dimension; i++) {
		swarm[index].position[i] += swarm[index].velocity[i];
	}
	
}

/****************************/
/* Functions */

// Rosenbrock function evaluation for particle x
double PSO::rosenbrock(Particle x) {
	double val = 0;
	
	for (int i = 0; i < dimension - 1; i++)
		val += 100 * pow((x.position[i + 1] - pow(x.position[i], 2)), 2) + pow((x.position[i] - 1),2);
	
	return val;
}

// Ackley function evaluation for particle x
double PSO::ackley(Particle x) {
	double val = 0;
	//values need to be changed to reflect our choices
	double a = 20.0;
	double b = 0.2;
	double c = 2*M_PI;
	
	double squaredSum = 0;
	for (int i = 0; i < dimension; i++)
		squaredSum += pow(x.position[i],2);
	
	double cosSum = 0;
	for (int i = 0; i < dimension; i++)
		cosSum += cos(c*x.position[i]);
	
	//go through three terms of ackley function
	val += (-a) * exp(-b * sqrt((1 / dimension) * squaredSum));
	val -= exp((1 / dimension) * cosSum);
	val += a + exp(1);
	
	return val;
}

// Rastrigin function evaluation for particle x
double PSO::rastrigin(Particle x) {
	double val = 10*dimension;
	
	for (int i = 0; i < dimension; i++)
		val += (pow(x.position[i],2) - 10*cos(2*M_PI*x.position[i]));
	
	return val;
}

/****************************/
/* Neighborhoods */

// Global neighborhood initialization
void PSO::global() {
    // add every particle to a single neighborhood
    for(int i = 0; i < swarmSize; i++) {
        vector<int> x;
        for (int j = 0; j < swarmSize; j++)
            x.push_back(j);
        neighborhoods.push_back(x);
    }
}

// Ring neighborhood initialization
void PSO::ring() {
	for (int i = 0; i < swarmSize; i++) {
		vector<int> x;
		neighborhoods.push_back(x);
		int before;
		int after;
		if (i == 0) {
            // wrap to last element if first element
			before = swarmSize - 1;
			after = i + 1;
		} else if (i + 1 == swarmSize) {
            // wrap to first element if last element
			before = i - 1;
			after = 0;
		} else {
            // neighbors are before and after
			before = i - 1;
			after = i + 1;
		}
        
		neighborhoods[i].push_back(before);
		neighborhoods[i].push_back(i);
		neighborhoods[i].push_back(after);
	}
}

int findNextSquare(int i) {
    // returns closest square number > i
    for(int j = i; j < INT_MAX; j++) {
        double root = sqrt(j);
        // if "floor" of root = root, j is square
        if(((int) root) == root) {
            return j;
        }
    }
    
    return -1;
}

// von Neumann neighborhood initialization
void PSO::vonNeumann() {
    // CURRENTLY DANGEROUS FOR NON-SQUARE #'S
    
	// particles as parts of a grid: neighborhood is adjacent particles (up, down, left, right)
    
    int rowSize = sqrt(findNextSquare(swarmSize));
    cout << "Using rowSize " << rowSize << endl;
    
    for(int index = 0; index < swarmSize; index++) {
        vector<int> tempNeighborhood;
        
        // note: (x, y) = (row, col) (in conceptual grid)
        int row = floor(index / rowSize);
        int col = index - (row * rowSize);
        
//        cout << index << " has coordinates (" << row << ", " << col << ")" << endl;
        
        // determine coordinates of neighbors
        int neighborUpRow;
        int neighborUpCol;
        int neighborDownRow;
        int neighborDownCol;
        int neighborLeftRow;
        int neighborLeftCol;
        int neighborRightRow;
        int neighborRightCol;
        
        if(row == 0) {
            // wrap up
            neighborUpRow = rowSize - 1;
            neighborDownRow = row + 1;
        } else if(row == rowSize - 1) {
            // wrap down
            neighborUpRow = row - 1;
            neighborDownRow = 0;
        } else {
            neighborUpRow = row - 1;
            neighborDownRow = row + 1;
        }
        
        if(col == 0) {
            // wrap left
            neighborLeftCol = rowSize - 1;
            neighborRightCol = col + 1;
        } else if(col == rowSize - 1) {
            // wrap right
            neighborLeftCol = col - 1;
            neighborRightCol = 0;
        } else {
            neighborLeftCol = col - 1;
            neighborRightCol = col + 1;
        }
        
        // true regardless of particle position
        neighborLeftRow = row;
        neighborRightRow = row;
        neighborUpCol = col;
        neighborDownCol = col;
        
        // determine indices in swarm of neighbors
        int neighborUp = rowSize * neighborUpRow + neighborUpCol;
        int neighborDown = rowSize * neighborDownRow + neighborDownCol;
        int neighborLeft = rowSize * neighborLeftRow + neighborLeftCol;
        int neighborRight = rowSize * neighborRightRow + neighborLeftCol;
        
        // add each neighbor to neighborhood
        tempNeighborhood.push_back(index);
        tempNeighborhood.push_back(neighborUp);
        tempNeighborhood.push_back(neighborDown);
        tempNeighborhood.push_back(neighborLeft);
        tempNeighborhood.push_back(neighborRight);
        
        neighborhoods.push_back(tempNeighborhood);
    }
    
    // DEAL WITH NON-SQUARE NUMBERS?
    
}

// Random neighborhood initialization
void PSO::initializeRandomNeighborhood() {
	int k = 5;
	
	for (int i = 0; i < swarmSize; i++) {
		vector<int> x;
		neighborhoods.push_back(x);
		for (int j = 0; j < k; j++) {
			// get non-duplicate index
			int randIndex = getNewRandIndex(i);
			// add the index that is not a duplicate
			neighborhoods[i].push_back(randIndex);
		}
	}
}

// Random neighborhood update function
void PSO::updateRandomNeighborhood() {
	int k = 5;
	double minProb = 0.2;
	
	for (int i = 0; i < swarmSize; i++) {
		double probability = (double) rand() / RAND_MAX;
		if (probability < minProb) {
			// clear the vector of its old neighborhood
			neighborhoods[i].clear();
			for (int j = 0; j < k; j++) {
				// get non-duplicate index
				int randIndex = getNewRandIndex(i);
				// add the index that isn't a duplicate to the neighborhood
				neighborhoods[i].push_back(randIndex);
			}
		}
	}
}

int PSO::getNewRandIndex(int i) {
	int randIndex = rand() % swarmSize;
	
	// check to make sure that this index isn't a duplicate
	bool indexAlreadySelected = false;
	while (!indexAlreadySelected) {
		indexAlreadySelected = true;
		for (int n = 0; n < neighborhoods[i].size(); n++) {
			if (randIndex == neighborhoods[i][n]) {
				// if you've added the index already
				// break, get a new number, and repeat
				indexAlreadySelected = false;
				break;
			}
		}
		if (indexAlreadySelected == false) {
			randIndex = rand() % swarmSize;
		}
	}
	
	return randIndex;
}

// Neighborhood initialization controller
void PSO::initializeNeighborhoods() {
	if (neighborhood == RANDOM_NEIGHBORHOOD_INT) {
		initializeRandomNeighborhood();
	} else if (neighborhood == RING_NEIGHBORHOOD_INT) {
		ring();
	} else if (neighborhood == VON_NEUMANN_NEIGHBORHOOD_INT) {
		vonNeumann();
	} else {
		global();
	}
}

void PSO::updateNeighborhoodBest() {
    // iterate through neighborhoods, updating bests
	for (int i = 0; i < swarmSize; i++) {
		for (int j = 0; j < neighborhoods[i].size(); j++) {
			int index = neighborhoods[i][j];
			if (swarm[index].pBestValue < nBestList[i]) {
                // if particle's value better than neighborhood best, replace
                nBestList[i] = swarm[index].pBestValue;
//                nBestPos[i]
			}
		}
	}
}

// Evaluation controller
void PSO::eval() {
	for (int i = 0; i < swarmSize; i++) {
		double pVal;
		
		Particle x = swarm[i];
		
		if (function == ROSENBROCK_FUNCTION_INT)
			pVal = rosenbrock(x);
		else if (function == ACKLEY_FUNCTION_INT)
			pVal = ackley(x);
		else
			pVal = rastrigin(x);
		
		if (pVal < x.pBestValue) {
			swarm[i].pBest = x.position;
			swarm[i].pBestValue = pVal;
		}
        
        if(pVal < gBestValue) {
            gBestValue = pVal;
            gBest = x.pBest;
        }
	}
	
	updateNeighborhoodBest();
}

void PSO::initializeSwarm() {
	// create swarm of 'swarmSize' particles
	for (int i = 0; i < swarmSize; i++) {
		Particle newParticle = Particle(dimension, function);
		swarm.push_back(newParticle);
	}
}

void PSO::solvePSO() {
    
	srand(time(NULL));
	
	initializeSwarm();
	initializeNeighborhoods();
	
	int iterRemaining = iterations;

	while(iterRemaining >= 0) {
        cout << "gBest = " << gBestValue << endl;

		// iterate through particles, updating velocity & position
		for(int i = 0; i < swarmSize; i++) {
            
			updateVelocity(i);
			updatePosition(i);
			// evaluate at new position

		}
        
        eval();
        if (neighborhood == RANDOM_NEIGHBORHOOD_INT) {
            updateRandomNeighborhood();
        }

        
		iterRemaining--;

	}
	
}
