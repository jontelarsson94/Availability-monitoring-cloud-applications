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
using namespace std;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}


int checkSeconds = 60;
float unAvailabilityTime = 0.0;
float timeNoConnection = 0.0;
char controlSiteOne[] = "https://www.google.se";
char controlSiteTwo[] = "https://basecamp.com/";
char URLToTest[] = "http://snowfire.se/";

int checkAvailability()
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
            //cout << "Google code 200" << endl;
            
                
                curl_easy_setopt(curl, CURLOPT_URL, URLToTest);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //This means our url that being tested is working
                    //cout << "TestUrl code 200" << endl;
                    /* always cleanup */
                    curl_easy_cleanup(curl);
                    return 1;
                } else {
                    //This means, we have internet but our website is down
                    //cout << "Should return 0" << endl;
                    /* always cleanup */
                    curl_easy_cleanup(curl);
                    return 0;
                }
            
        } else {
            curl_easy_setopt(curl, CURLOPT_URL, controlSiteTwo);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            res = curl_easy_perform (curl);
            int http_code = 0;
            curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
            
            if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                //If we can go to basecamp we can know that the internet connection is up and running
                //This means google was either down or we had no connection
                //cout << "Google not 200" << endl;
                curl_easy_setopt(curl, CURLOPT_URL, URLToTest);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //If our url is up and running, it means that google was down but we have internet connection
                    //cout << "TestUrl code 200" << endl;
                    /* always cleanup */
                    curl_easy_cleanup(curl);
                    return 1;
                } else {
                    //If we can't reach either google or our own website, we can safely say that our internet connection was down
                    //cout << "Should return 2" << endl;
                    /* always cleanup */
                    curl_easy_cleanup(curl);
                    return 0;
                }
            } else {
                //This means google was either down or we had no connection
                //cout << "Google not 200" << endl;
                curl_easy_setopt(curl, CURLOPT_URL, URLToTest);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                res = curl_easy_perform (curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                
                if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK){
                    //If our url is up and running, it means that google was down but we have internet connection
                    //cout << "TestUrl code 200" << endl;
                    /* always cleanup */
                    curl_easy_cleanup(curl);
                    return 1;
                } else {
                    //If we can't reach either google or our own website, we can safely say that our internet connection was down
                    //cout << "Should return 2" << endl;
                    /* always cleanup */
                    curl_easy_cleanup(curl);
                    return 2;
                }
            }
        }
    }
    //cout << "Should return 2" << endl;
    return 2;
}

int main(void)
{
    struct timeval  tv1, tv2;
    
    while (true) {
        gettimeofday(&tv1, NULL);
        int success = checkAvailability();
        if(success == 0) {
            //If success = 0, it means that our website was down and taking times on how long it is down
            gettimeofday(&tv2, NULL);
            unAvailabilityTime += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
            gettimeofday(&tv1, NULL);
            //cout << "Current time: " << unAvailabilityTime << endl;
            while (checkAvailability() == 0) {
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
            while (checkAvailability() == 2) {
                gettimeofday(&tv2, NULL);
                timeNoConnection += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
                gettimeofday(&tv1, NULL);
                //cout << "Current downtime: " << unAvailabilityTime << endl;
            }
        }
        cout << "Time unavailable: " << unAvailabilityTime << endl;
        cout << "Time no connection: " << timeNoConnection << endl;
        std::this_thread::sleep_for(std::chrono::seconds(checkSeconds));
    }
    
    return 0;
}
