#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <float.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <limits>
#include <fstream>
#include <locale>
using namespace std;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}


float unAvailabilityTime = 0.0;
float timeNoConnection = 0.0;
float numberOfSecs = 0.0;
float intervalSecs = 10.0;
char controlSiteOne[] = "https://www.google.se";
char controlSiteTwo[] = "https://basecamp.com/";
double slowestResponseTime = 0.0;
int counter = 0;


/*
 
 Returns 0 if website is down
 Returns 1 if website is up
 Returns 2 if we have no internet connection
 
 
 */

/*
 
 Args to send from terminal is:
 argv[1] = The url to send requests to
 argv[2] = The filename to save the response times to
 argv[3] = The filename to save the availability to
 argv[4] = How many seconds to wait between each request
 
 */

int checkAvailability(char *URLToTest, double *resp_time)
{
    CURL *curl;
    CURLcode res;
    
    curl = curl_easy_init();
    
    if(curl) {
        
        curl_easy_setopt(curl, CURLOPT_URL, controlSiteOne);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
        res = curl_easy_perform (curl);
        int http_code = 0;
        double response_time;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
            //If we can go to google.com we can know that the internet connection is up and running
            
                
                curl_easy_setopt(curl, CURLOPT_URL, URLToTest);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                curl_easy_getinfo (curl, CURLINFO_TOTAL_TIME, &response_time);
                //Trying the response time
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //This means our url that being tested is working
                    *resp_time = response_time;
                    if(response_time > slowestResponseTime) {
                        slowestResponseTime = response_time;
                    }
                    cout << "Current slowest response time: " << slowestResponseTime << endl;
                    curl_easy_cleanup(curl);
                    return 1;
                } else {
                    //This means, we have internet but our website is down
                    curl_easy_cleanup(curl);
                    return 0;
                }
            
        } else {
            //This means, google was down or we had no internet connection
            curl_easy_setopt(curl, CURLOPT_URL, controlSiteTwo);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
            res = curl_easy_perform (curl);
            int http_code = 0;
            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
            
            if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                //If we couldn't go to google but we can go to basecamp we can know that the internet connection is up and running
                curl_easy_setopt(curl, CURLOPT_URL, URLToTest);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                curl_easy_getinfo (curl, CURLINFO_TOTAL_TIME, &response_time);
                
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //If our url is up and running, it means that google was down but we have internet connection and our site wasn't down
                    *resp_time = response_time;
                    if(response_time > slowestResponseTime) {
                        slowestResponseTime = response_time;
                    }
                    cout << "Current slowest response time: " << slowestResponseTime << endl;
                    curl_easy_cleanup(curl);
                    return 1;
                } else {
                    //If we couldnt reach google or our website, but we could reach basecamp, we can safely say that our website is down
                    curl_easy_cleanup(curl);
                    return 0;
                }
            } else {
                //This means google was down and basecamp was down. Which is almost impossible, so lets try one more connection
                curl_easy_setopt(curl, CURLOPT_URL, URLToTest);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                curl_easy_getinfo (curl, CURLINFO_TOTAL_TIME, &response_time);
                
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //If our url is up and running, it means that both google and basecamp was down, but our website was up.
                    *resp_time = response_time;
                    if(response_time > slowestResponseTime) {
                        slowestResponseTime = response_time;
                    }
                    cout << "Current slowest response time: " << slowestResponseTime << endl;
                    curl_easy_cleanup(curl);
                    return 1;
                } else {
                    //If we can't reach google, basecamp or our own website, we can safely say that our internet connection is lost. If we are wrong, we have at least proven that absolutely anything is possible.
                    curl_easy_cleanup(curl);
                    return 2;
                }
            }
        }
    }
    //cout << "Should return 2" << endl;
    return 2;
}

int main( int argc, char *argv[] )
{
    cout.imbue(std::locale(""));
    std::ofstream outfile;
    //Uncomment if we want commas instead of dots
    //outfile.imbue(std::locale(""));
    struct timeval  tv1, tv2;
    struct timeval  td1, td2;
    cout << "arg: " << argv[1] << endl;
    char *URLToTest = argv[1];
    //char newURL[] = "snowfire.se";
    char *fileNameResponse = argv[2];
    char *fileNameAvailability = argv[3];
    double response_time;
    long checkSeconds = strtol(argv[4], NULL, 10);
    
    
    while (true) {
        gettimeofday(&td1, NULL);
        gettimeofday(&tv1, NULL);
        int success = checkAvailability(URLToTest, &response_time);
        if(success == 0) {
            //If success = 0, it means that our website was down and taking times on how long it is down
            int i = 0;
            double timeNotConnected = 0.0;
            gettimeofday(&tv2, NULL);
            timeNotConnected += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
            gettimeofday(&tv1, NULL);
            i++;
            cout << i << endl;
            //cout << "Current time: " << unAvailabilityTime << endl;
            while (checkAvailability(URLToTest, &response_time) == 0) {
                cout << i << endl;
                gettimeofday(&tv2, NULL);
                timeNotConnected += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
                gettimeofday(&tv1, NULL);
                i++;
                //cout << "Current downtime: " << unAvailabilityTime << endl;
                if (i >= 2) {
                    unAvailabilityTime += timeNotConnected;
                }
            }
            
            //cout << "Time unavailable: " << unAvailabilityTime << endl;
        } else if (success == 2) {
            //If success = 0, it means that our website was down and taking times on how long it is down
            gettimeofday(&tv2, NULL);
            timeNoConnection += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
            gettimeofday(&tv1, NULL);
            //cout << "Current time: " << unAvailabilityTime << endl;
            while (checkAvailability(URLToTest, &response_time) == 2) {
                gettimeofday(&tv2, NULL);
                timeNoConnection += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
                gettimeofday(&tv1, NULL);
                
                //cout << "Current downtime: " << unAvailabilityTime << endl;
            }
        }
        counter++;
        cout << "Time unavailable: " << unAvailabilityTime << endl;
        cout << "Time no connection: " << timeNoConnection << endl;
        cout << "Response time" << response_time << endl;
        std::this_thread::sleep_for(std::chrono::seconds(checkSeconds));
        gettimeofday(&td2, NULL);
        numberOfSecs += (double) (td2.tv_usec - td1.tv_usec) / 1000000 + (double) (td2.tv_sec - td1.tv_sec);
        // current date/time based on current system
        time_t now = time(0);
    
        // convert now to string form
        char* dt = ctime(&now);
        outfile.open(fileNameResponse, std::ios_base::app);
        
        if(success == 1) {
            outfile << response_time << ", ";
        }
        
        if(counter % 86400/checkSeconds == 0) {
            outfile << "\n\n\n\n";
        }
        
        outfile.close();
        
        outfile.open(fileNameAvailability, std::ios_base::app);
        
        
        outfile << "\t" << unAvailabilityTime << "\t" << timeNoConnection << "\t" << dt << "\n";
        
        
        outfile.close();
        unAvailabilityTime = 0.0;
        timeNoConnection = 0.0;
        numberOfSecs = 0.0;
    }
    
    return 0;
}
