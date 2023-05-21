

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <mysql/mysql.h>
#define MAXTIMINGS 85
#define DHTPIN 7
#define FIVE_MIN 300000

int dht11_dat[5] = {0, 0, 0, 0, 0};
double *read_dht11_dat()
{
    int8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    float f;
    dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
    pinMode(DHTPIN, OUTPUT);
    digitalWrite(DHTPIN, LOW);
    delay(18);
    digitalWrite(DHTPIN, HIGH);
    delayMicroseconds(40);
    pinMode(DHTPIN, INPUT);

    for (i = 0; i < MAXTIMINGS; i++)
    {
        counter = 0;
        while (digitalRead(DHTPIN) == laststate)
        {
            counter++;
            delayMicroseconds(1);
            if (counter == 255)
            {
                break;
            }
        }

        laststate = digitalRead(DHTPIN);
        if (counter == 255)
            break;
        if ((i >= 4) && (i % 2 == 0))
        {
            dht11_dat[j / 8] <<= 1;
            if (counter > 16)
                dht11_dat[j / 8] |= 1;
            j++;
        }
    }
    static double returnValues[3];
    if ((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF)))
    {
        double celciusValue = (double)dht11_dat[2] + ((double)dht11_dat[3] / 10.0);
        double humidityValue = (double)dht11_dat[0] + ((double)dht11_dat[1] / 10.0);
        f = celciusValue * 9. / 5. + 32;
        if (f == 32.0)
        {
            returnValues[0] = 0.0;
            returnValues[1] = 0.0;
            returnValues[2] = 0.0;
            return returnValues;
        }
        else
        {
            returnValues[0] = humidityValue;
            returnValues[1] = celciusValue;
            returnValues[2] = f;
            return returnValues;
        }
    }
    else
    {
        returnValues[0] = 0.0;
        returnValues[1] = 0.0;
        returnValues[2] = 0.0;
        return returnValues;
    }
}

int main(void)
{

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "aross";
    char *password = "llll";
    char *database = "weatherdb";
    conn = mysql_init(NULL);

    / if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (wiringPiSetup() == -1)
        exit(1);

    while (1)
    {
        // Retreive data from sensor
        double *values = read_dht11_dat(); // [0] = humidity || [1] = celcius || [2] = fahrenheit

        if (values[0] != 0 && values[1] != 0 && values[2] != 0)
        {
            // Get date and time
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            char date_string[11]; // date
            strftime(date_string, sizeof(date_string), "%Y-%m-%d", tm);
            int current_hour = tm->tm_hour;
            int current_minute = tm->tm_min;

            printf("MAIN VALUES: Hum %.2f Celc %.2f Fahrenheit %.2f Date: %s Hour: %d Minute: %d\n", values[0], values[1], values[2], date_string, current_hour, current_minute);

            char query[100];
            sprintf(query, "insert into entries values ('%s', %d, %d, %.2f, %.2f, %.2f)", date_string, current_hour, current_minute, values[0], values[1], values[2]);

            // query DB to add values
            if (mysql_query(conn, query))
                fprintf(stderr, "%s\n", mysql_error(conn));

            delay(FIVE_MIN);
        }
    }

    mysql_free_result(res);
    mysql_close(conn);
}
