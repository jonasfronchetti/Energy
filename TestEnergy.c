#include "Energy.h"

int main(){
	int a = 0;
	
	Energy *energy0 = calloc(sizeof(Energy), 1);
	Energy *energy1 = calloc(sizeof(Energy), 1);
	//Energy *energy2 = calloc(sizeof(Energy), 1);
	
	initEnergy(energy0, 1);
	initEnergy(energy1, 1);
	//initEnergy(energy2, 1);

	setSensor(energy0, "ACa", "A", "", "/dev/ttyACM0");
	setSensor(energy1, "ACb", "B", "", "/dev/ttyACM0");
	//setSensor(energy5, "AC ", "C", "", "/dev/ttyACM0");

	setTimer(energy0, 10);
	setTimer(energy1, 10);
	//setTimer(energy2, 10);


	startEnergy(energy0);
	startEnergy(energy1);
	//startEnergy(energy2);

	while(a < 60){
		//system("clear");
		printEnergy(energy0);
		printEnergy(energy1);
		//printEnergy(energy2);
		a ++;
		sleep(10);
	}
	//system("clear");
	printf("stop \n");
	stopEnergy(energy0);
	stopEnergy(energy1);
	//stopEnergy(energy2);
	sleep(6);
	
    printEnergy(energy0);
	printEnergy(energy1);
	//printEnergy(energy2);
}
