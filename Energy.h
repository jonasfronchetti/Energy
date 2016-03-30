#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>

typedef struct Energy Energy;

struct Energy{
	char* name;
	char* tensionSensor;
	char* currentSensor;
	char* initiate;
	float tension;
	float current;
	float instantPower;
	float power;
	int sleep;
	int time;
	int loop;
	int type;
	int port;
	pthread_t thread;
};


int initEnergy(Energy *energy, int type);
int setSensor(Energy *energy, char *name, char *tension, char *current, char* initiate);
void setTimer(Energy *energy, int sleep);
int startEnergy(Energy *energy);
int stopEnergy(Energy *energy);
void printEnergy(Energy *energy);
float getEnergy(Energy *energy, int interval);
float getCurrent(Energy *energy);
float getTension(Energy *energy);

int confPort(char *url);
void readSensorSerial(Energy *energy);
void startSensor(char *url);
float readSensor(char *url);
void *start(void *tmp);



int initEnergy(Energy *energy, int type){
	//energy = calloc(sizeof(Energy), 1);
	energy->type = type;
	energy->loop = 1;
	energy->sleep = 5;
	return 0;
}

int setSensor(Energy *energy, char *name, char *tension, char *current, char* initiate){
	energy->name = name;
	energy->tensionSensor = tension;
	energy->currentSensor = current;
	energy->initiate = initiate;
	switch (energy->type){
		case 0://sensor interno
			startSensor(energy->initiate);
		break;
		case 1://porta serial
			energy->port = confPort(energy->initiate);
		break;
	}
	return 0;
}

void setTimer(Energy *energy, int sleep){
	energy->sleep = sleep;
}

int startEnergy(Energy *energy){
	int ret = pthread_create(&(energy->thread), NULL, start, energy);
	if (ret != 0) {
		ret = 1;
	}
	return ret;
}

int stopEnergy(Energy *energy){
	energy->loop = 0;
	if (energy->type == 1){
		close(energy->port);
	}
	return 0;
}

float getEnergy(Energy *energy, int interval){
	if (interval > 0){
		energy->power = 0;
		pthread_create(&(energy->thread), NULL, start, energy);
		sleep(interval);
		stopEnergy(energy);
	}
	return energy->power;
}

void printEnergy(Energy *energy){
	printf("%s=%f V; ", energy->name, energy->tension);
	printf("%s=%f A; ", energy->name, energy->current);
	printf("%s=%f W; ", energy->name, energy->instantPower);
	printf("%s Consumo=%f J ", energy->name, energy->power);
	printf("%f KWh\n", (energy->power /(1000*60*60)) );
}

float getCurrent(Energy *energy){
	return energy->current;
}

float getTension(Energy *energy){
	return energy->tension;
}

void *start(void *tmp){
	Energy *energy = (Energy *) tmp;
	char *tempo;
	while (energy->loop){
		switch (energy->type){
			case 0://sensor interno
				if (energy->tensionSensor != ""){
					energy->tension = readSensor(energy->tensionSensor);
				}
				if (energy->currentSensor != ""){
					energy->current = readSensor(energy->currentSensor);
				}
				if ((energy->tension > 0) && (energy->current > 0)){
					energy->instantPower = energy->tension * energy->current;
				}
			break;
			case 1://portaserial
				readSensorSerial(energy);
				energy->instantPower = energy->tension * energy->current;
			break;
		}
		sleep(energy->sleep);
		energy->power += energy->instantPower * energy->sleep;
		energy->time += energy->sleep;
	}
	pthread_exit(NULL);
}

void startSensor(char *url){	
	FILE *arq;
 
	arq = fopen(url, "w");

	if(arq != NULL){
		fwrite("1", sizeof(char), 1, arq);
		fclose(arq);
	}
}

float readSensor(char *url){	
	FILE *arq;
	float back; 
	arq = fopen(url, "r+");

	if(arq != NULL){
		fscanf(arq,"%f\n", &back);
		fclose(arq);
	}
	return back;
}

void readSensorSerial(Energy *energy){	
	char myStr[20];
	float f;

	if (energy->port == -1){
		printf("ErroReadserial \n");//system("error");
	} else {
		write(energy->port, energy->tensionSensor, 1);
		read(energy->port, myStr, 20);
		f = atof((char*) strtok(myStr, "#"));
		energy->current = f;

		f = atof((char*) strtok(NULL, "#"));
		energy->tension = f;
	}//close(port);
}

int confPort(char *url){
	int fd = 0; //arquivo da porta 
	struct termios iniConfig, myConfig;// porta serial

	fd = open(url, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1){
		printf("Erro ao abrir porta");
		close(fd);
		return fd;
	}
	else
		//ioctl(fd, F_SETFL, FNDELAY);//para nao esperar na porta seria 
		fcntl(fd, F_SETFL, 0);

	//configura porta
	tcgetattr(fd, &iniConfig);
	tcgetattr(fd, &myConfig); 
	
	//seta velocidade porta
	cfsetispeed(&myConfig, B9600);
	cfsetospeed(&myConfig, B9600);
	
	//checar paridade
	myConfig.c_cflag &= ~PARENB;
	myConfig.c_cflag &= ~CSTOPB;
	myConfig.c_cflag &= ~CSIZE;
	myConfig.c_cflag |= CS8;//8 bits dados
	
	myConfig.c_cflag &= ~HUPCL;
	myConfig.c_cflag &= ~CRTSCTS;			
	
	myConfig.c_lflag = ICANON;
	
	tcsetattr(fd, TCSANOW, &myConfig);
	sleep(2);
	return (fd);
}
