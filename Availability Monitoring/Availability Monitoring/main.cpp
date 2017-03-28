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
using namespace std;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}


int checkSeconds = 5;
float unAvailabilityTime = 0.0;
float timeNoConnection = 0.0;
float numberOfSecs = 0.0;
float intervalSecs = 43200.0;
char controlSiteOne[] = "https://www.google.se";
char controlSiteTwo[] = "https://basecamp.com/";


/*
 
 Returns 0 if website is down
 Returns 1 if website is up
 Returns 2 if we have no internet connection
 
 
 */

int checkAvailability(char *URLToTest)
{
    CURL *curl;
    CURLcode res;
    
    curl = curl_easy_init();
    
    if(curl) {
        
        curl_easy_setopt(curl, CURLOPT_URL, controlSiteOne);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        res = curl_easy_perform (curl);
        int http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
            //If we can go to google.com we can know that the internet connection is up and running
            
                
                curl_easy_setopt(curl, CURLOPT_URL, URLToTest);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //This means our url that being tested is working
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
            res = curl_easy_perform (curl);
            int http_code = 0;
            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
            
            if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                //If we couldn't go to google but we can go to basecamp we can know that the internet connection is up and running
                curl_easy_setopt(curl, CURLOPT_URL, URLToTest);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //If our url is up and running, it means that google was down but we have internet connection and our site wasn't down
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
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //If our url is up and running, it means that both google and basecamp was down, but our website was up.
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
    std::ofstream outfile;
    struct timeval  tv1, tv2;
    struct timeval  td1, td2;
    cout << "arg: " << argv[1] << endl;
    char *URLToTest = argv[1];
    
    while (true) {
        gettimeofday(&td1, NULL);
        gettimeofday(&tv1, NULL);
        int success = checkAvailability(URLToTest);
        if(success == 0) {
            //If success = 0, it means that our website was down and taking times on how long it is down
            gettimeofday(&tv2, NULL);
            unAvailabilityTime += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
            gettimeofday(&tv1, NULL);
            //cout << "Current time: " << unAvailabilityTime << endl;
            while (checkAvailability(URLToTest) == 0) {
                gettimeofday(&tv2, NULL);
                unAvailabilityTime += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
                gettimeofday(&tv1, NULL);
                //cout << "Current downtime: " << unAvailabilityTime << endl;
            }
            
            //cout << "Time unavailable: " << unAvailabilityTime << endl;
        } else if (success == 2) {
            //If success = 0, it means that our website was down and taking times on how long it is down
            gettimeofday(&tv2, NULL);
            timeNoConnection += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
            gettimeofday(&tv1, NULL);
            //cout << "Current time: " << unAvailabilityTime << endl;
            while (checkAvailability(URLToTest) == 2) {
                gettimeofday(&tv2, NULL);
                timeNoConnection += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
                gettimeofday(&tv1, NULL);
                //cout << "Current downtime: " << unAvailabilityTime << endl;
            }
        }
        cout << "Time unavailable: " << unAvailabilityTime << endl;
        cout << "Time no connection: " << timeNoConnection << endl;
        std::this_thread::sleep_for(std::chrono::seconds(checkSeconds));
        gettimeofday(&td2, NULL);
        numberOfSecs += (double) (td2.tv_usec - td1.tv_usec) / 1000000 + (double) (td2.tv_sec - td1.tv_sec);
        if(numberOfSecs > intervalSecs) {
            // current date/time based on current system
            time_t now = time(0);
            
            // convert now to string form
            char* dt = ctime(&now);
            outfile.open("test.txt", std::ios_base::app);
            outfile << unAvailabilityTime << "\t" << timeNoConnection << "\t" << dt << "\n";
            outfile.close();
            unAvailabilityTime = 0.0;
            timeNoConnection = 0.0;
            numberOfSecs = 0.0;
        }
    }
    
    return 0;
}
