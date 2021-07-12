/*
 * clientPost.c: A very, very primitive HTTP client for sensor
 * 
 * To run, prepare config-cp.txt and try: 
 *      ./clientPost
 *
 * Sends one HTTP request to the specified HTTP server.
 * Get the HTTP response.
 */

#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <stdint.h>
#include "stems.h"
#define MAXTIMINGS 83
#define DHTPIN 7

int dht11_dat[5] = {0, 0, 0, 0, 0};
int read_dht11_dat(float *value);

int read_dht11_dat(float *value)
{
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    uint8_t flag = HIGH;
    uint8_t state = 0;
    float f;
    memset(dht11_dat, 0, sizeof(int) * 5);
    pinMode(DHTPIN, OUTPUT);
    digitalWrite(DHTPIN, LOW);
    delay(18);
    digitalWrite(DHTPIN, HIGH);
    delayMicroseconds(30);
    pinMode(DHTPIN, INPUT);
    for (i = 0; i < MAXTIMINGS; i++)
    {
        counter = 0;
        while (digitalRead(DHTPIN) == laststate)
        {
            counter++;
            delayMicroseconds(1);
            if (counter == 200)
                break;
        } // while
        laststate = digitalRead(DHTPIN);
        if (counter == 200)
            break;
        if ((i >= 4) && (i % 2 == 0))
        {
            dht11_dat[j / 8] <<= 1;
            if (counter > 40)
                dht11_dat[j / 8] |= 1;
            j++;
        } // if
    }     // for
    if ((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] +
                                        dht11_dat[2] + dht11_dat[3]) &
                                       0xff)))
        return 0;
    else
    {
        printf("Data get failed\n");
        return -1;
    }
}

void clientSend(int fd, char *filename, char *body)
{
    char buf[MAXLINE];
    char hostname[MAXLINE];

    Gethostname(hostname, MAXLINE);

    /* Form and send the HTTP request */
    sprintf(buf, "POST %s HTTP/1.1\n", filename);
    sprintf(buf, "%sHost: %s\n", buf, hostname);
    sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
    sprintf(buf, "%sContent-Length: %d\n\r\n", buf, strlen(body));
    sprintf(buf, "%s%s\n", buf, body);
    Rio_writen(fd, buf, strlen(buf));
}

/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
    rio_t rio;
    char buf[MAXBUF];
    int length = 0;
    int n;

    Rio_readinitb(&rio, fd);

    /* Read and display the HTTP Header */
    n = Rio_readlineb(&rio, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0))
    {
        /* If you want to look for certain HTTP tags... */
        if (sscanf(buf, "Content-Length: %d ", &length) == 1)
            printf("Length = %d\n", length);
        printf("Header: %s", buf);
        n = Rio_readlineb(&rio, buf, MAXBUF);
    }

    /* Read and display the HTTP Body */
    n = Rio_readlineb(&rio, buf, MAXBUF);
    while (n > 0)
    {
        printf("%s", buf);
        n = Rio_readlineb(&rio, buf, MAXBUF);
    }
}

/* currently, there is no loop. I will add loop later */
void userTask(char *myname, char *hostname, int port, char *filename, time_t time, float value) // float -> time_t change type
{
    int clientfd;
    char msg[MAXLINE];

    sprintf(msg, "name=%s&time=%ld&value=%f", myname, time, value);
    clientfd = Open_clientfd(hostname, port);
    clientSend(clientfd, filename, msg);
    clientPrint(clientfd);
    Close(clientfd);
}

void getargs_cp(char *myname, char *hostname, int *port, char *filename, float *period, float *value)
{
    FILE *fp;

    fp = fopen("config-pi.txt", "r");
    if (fp == NULL)
        unix_error("config-pi.txt file does not open.");

    //   fscanf(fp, "%s", myname);
    fscanf(fp, "%s", hostname);
    fscanf(fp, "%d", port);
    fscanf(fp, "%s", filename);
    fscanf(fp, "%f", period);
    //   fscanf(fp, "%f", value);
    fclose(fp);
}

int main(void)
{
    char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE], temp[100];
    int port;
    float period, value;
    time_t seconds; // time_t type value

    getargs_cp(myname, hostname, &port, filename, &period, &value);

    if (wiringPiSetup() == -1)
        exit(1);
    while (1)
    {
        if (read_dht11_dat(&value) == 0)
        {
            time(&seconds); // use time func
            sprintf(myname, "temperaturePI");
            sprintf(temp, "%d.%d", dht11_dat[0], dht11_dat[1]);
            value = atof(temp);
            userTask(myname, hostname, port, filename, seconds, value);
            delay((unsigned int)(period * 1000 / 2));

            sprintf(myname, "humidityPI");
            sprintf(temp, "%d.%d", dht11_dat[2], dht11_dat[3]);
            value = atof(temp);
            userTask(myname, hostname, port, filename, seconds, value);
            delay((unsigned int)(period * 1000 / 2));
        }
        else
        {
            delay(10);
        }
    }
    return (0);
}