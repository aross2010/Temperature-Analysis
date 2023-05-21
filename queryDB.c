
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define REMOVE_FIRST_CHAR(str) memmove((str), (str) + 1, strlen(str))
#define isGreater(hour1, min1, hour2, min2) ((hour1) < (hour2) || ((hour1) == (hour2) && (min1) + 15 <= (min2)))
#define getDay(date) ((date)[strlen(date) - 2])

#define ERRORMSG "\nInvalid Input. Please try again.\n"
#define PROMPT "Select your option: \n\n(1) - Single day Statistics\n(2) - Multi-day statistics\n\n"
#define NEWLINE printf("\n")

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

    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    printf("I have collected temperature data for three (3) days, Friday May 5th through Sunday May 7th.\n\nThis program will allow you to view statistics for your desired time period.");
    int timePeriodSelection;
    int select;
    do
    {

        do
        {

            timePeriodSelection = readInput(PROMPT, 1, 2);

            if (timePeriodSelection == 1)
                getSingleDayStats(conn);
            else if (timePeriodSelection == 2)
                getMultiDayStats(conn);
            else if (timePeriodSelection > 2)
                printf("%s", ERRORMSG);

        } while (timePeriodSelection != 1 && timePeriodSelection != 2);

        sleep(5);

        select = readInput("Would you like find more data?\n(1) - Yes\n(2) - No\n\n", 1, 2);

    } while (select == 1);
}

void getSingleDayStats(MYSQL *conn)
{

    int day = readInput("Please select the day you would like stats for:\n\n(1) - May 5th\n(2) - May 6th\n(3) - May 7th\n\n", 1, 3);

    char date[3];

    if (day == 1)
        strcpy(date, "05");
    else if (day == 2)
        strcpy(date, "06");
    else
        strcpy(date, "07");

    int singleDaySelection = readInput("Would you like stats for the entire day or a time interval?\n\n(1) - Entire day\n(2) - Time Period\n\n", 1, 2);

    if (singleDaySelection == 1)
        getAllDayStats(date, conn);
    else
        getTimeIntervalStats(date, conn);
}

void getAllDayStats(char *date, MYSQL *conn)
{

    char query[300];

    sprintf(query, "SELECT MAX(fahrenheit), MAX(celcius), MAX(humidity), MIN(fahrenheit), MIN(celcius), MIN(humidity), ROUND(AVG(fahrenheit), 2), ROUND(AVG(celcius), 2), ROUND(AVG(humidity), 2) FROM entries WHERE date ='2023-05-%s';", date);
    printf("Displaying Statistics For May %sth, 2023\n", REMOVE_FIRST_CHAR(date));
    fetchData(conn, query);
}

void getTimeIntervalStats(char *date, MYSQL *conn)
{

    char prompt[100];

    int startingHour;
    int startingMinute;
    char startingTime[15];
    readTimeInput("Select a starting time interval (HH:MM) in a 24-hour format:\n\n", &startingHour, &startingMinute, startingTime);

    int endingHour;
    int endingMinute;
    char endingTime[15];
    sprintf(prompt, "Select a ending time interval (HH:MM) in a 24-hour format that is past %s:\n\n", startingTime);
    readTimeInput(prompt, &endingHour, &endingMinute, endingTime);

    while (!isGreater(startingHour, startingMinute, endingHour, endingMinute))
    {
        printf("%s", ERRORMSG);
        readTimeInput(prompt, &endingHour, &endingMinute, endingTime);
    }

    char query[300];
    sprintf(query, "SELECT MAX(fahrenheit), MAX(celcius), MAX(humidity), MIN(fahrenheit), MIN(celcius), MIN(humidity), ROUND(AVG(fahrenheit), 2), ROUND(AVG(celcius), 2), ROUND(AVG(humidity), 2) FROM entries WHERE date ='2023-05-%s' AND hour >=%d AND hour <= %d;", date, startingHour, endingHour);
    printf("Displaying Statistics For May %sth, 2023 from %s to %s\n", REMOVE_FIRST_CHAR(date), startingTime, endingTime);
    fetchData(conn, query);
}

void getMultiDayStats(MYSQL *conn)
{

    int interval = readInput("Please select your interval:\n\n(1) - May 5th to May 6th\n(2) - May 5th to May 7th\n(3) - May 6th to May 7th\n\n", 1, 3);

    char startDate[20];
    char endDate[20];

    if (interval == 1)
    {
        strcpy(startDate, "'2023-05-05'");
        strcpy(endDate, "'2023-05-06'");
    }
    else if (interval == 2)
    {
        strcpy(startDate, "'2023-05-05'");
        strcpy(endDate, "'2023-05-07'");
    }
    else
    {
        strcpy(startDate, "'2023-05-06'");
        strcpy(endDate, "'2023-05-07'");
    }

    int statsInterval = readInput("Would you like to select the full days' statistics or a specific time interval?\n\n(1) - Entire days\n(2) - Time interval\n\n", 1, 2);

    if (statsInterval == 1)
        getMultiEntireDayStats(conn, startDate, endDate);
    else
    {
        getMultiTimeIntervalStats(conn, startDate, endDate);
    }
}

void getMultiEntireDayStats(MYSQL *conn, char *startDate, char *endDate)
{

    char query[300];
    sprintf(query, "SELECT MAX(fahrenheit), MAX(celcius), MAX(humidity), MIN(fahrenheit), MIN(celcius), MIN(humidity), ROUND(AVG(fahrenheit), 2), ROUND(AVG(celcius), 2), ROUND(AVG(humidity), 2) FROM entries WHERE date >= %s AND date <= %s;", startDate, endDate);
    printf("Displaying Statistics from May %cth, 2023 to May %cth, 2023\n", getDay(startDate), getDay(endDate));
    fetchData(conn, query);
}

void getMultiTimeIntervalStats(MYSQL *conn, char *startDate, char *endDate)
{

    char prompt[100];
    sprintf(prompt, "Select a starting time interval (HH:MM) in a 24-hour format for May %cth:\n\n", getDay(startDate));
    int startingHour;
    int startingMinute;
    char startingTime[10];

    readTimeInput(prompt, &startingHour, &startingMinute, startingTime);

    sprintf(prompt, "Select a ending time interval (HH:MM) in a 24-hour format for May %cth:\n\n", getDay(endDate));
    int endingHour;
    int endingMinute;
    char endingTime[10];

    readTimeInput(prompt, &endingHour, &endingMinute, endingTime);

    char query[400];
    sprintf(query, "SELECT MAX(fahrenheit), MAX(celcius), MAX(humidity), MIN(fahrenheit), MIN(celcius), MIN(humidity), ROUND(AVG(fahrenheit), 2), ROUND(AVG(celcius), 2), ROUND(AVG(humidity), 2) FROM entries WHERE (date = %s AND (hour > %d OR (hour = %d AND minute >= %d))) OR (date > %s AND date < %s) OR (date = %s AND (hour < %d OR (hour = %d AND minute <= %d)));", startDate, startingHour, startingHour, startingMinute, startDate, endDate, endDate, endingHour, endingHour, endingMinute);
    printf("Displaying Statistics from May %cth, 2023 %s to May %cth, 2023 %s\n", getDay(startDate), startingTime, getDay(endDate), endingTime);
    fetchData(conn, query);
}

void fetchData(MYSQL *conn, char *query)
{

    MYSQL_ROW data;

    mysql_query(conn, query);
    MYSQL_RES *result = mysql_store_result(conn);
    data = mysql_fetch_row(result);

    printf("\nMaximum Values:\nFahrenheit: %s, Celcius: %s, Humidity: %s%%\n\nMinimum Values:\nFahrenheit: %s, Celcius: %s, Humidity: %s%%\n\nAverage Values:\nFahrenheit: %s, Celcius: %s, Humidity: %s%%\n\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
}

int isValidTime(const char *str, int *hour, int *minute)
{
    if (strlen(str) != 5)
        return 0;

    *hour = atoi(str);
    *minute = atoi(str + 3);

    if (*hour >= 0 && *hour < 24 && *minute >= 0 && *minute <= 59 && str[2] == ':')
        return 1;

    return 0;
}

int readInput(const char *prompt, int n1, int n2)
{
    int value;
    bool validInput = false;

    do
    {
        printf("%s", prompt);
        scanf("%d", &value);

        // Check for valid input
        if (getchar() != '\n' || !(n1 <= value && n2 >= value))
            printf("%s", ERRORMSG);
        else
        {
            validInput = true;
            NEWLINE;
        }

    } while (!validInput);

    return value;
}

void readTimeInput(const char *prompt, int *hour, int *minute, char *time)
{

    bool validInput = false;

    do
    {
        printf("%s", prompt);
        scanf("%s", time);

        if (isValidTime(time, hour, minute))
        {
            validInput = true;
            NEWLINE;
        }
        else
        {
            printf("%s", ERRORMSG);
        }

    } while (!validInput);

    return time;
}
