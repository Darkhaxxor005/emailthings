#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <curl/curl.h>
#include <boost/asio.hpp>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void process_domain(std::string domain, std::ofstream& outfile, int request_number) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ("http://" + domain).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            //print dead 
        } else {
            std::regex email_pattern(R"([A-Za-z0-9%&*+?^_{|}~-]+(?:\.[A-Za-z0-9!#$%&*+?^_{|}~-]+)*@(?:[A-Za-z0-9](?:[a-z0-9-]*[A-Za-z0-9])?\.)+(?:[A-Za-z]{2}|com|org|net|edu|gov|mil|biz|info|mobi|name|aero|asia|jobs|museum)\b)");
            auto words_begin = std::sregex_iterator(readBuffer.begin(), readBuffer.end(), email_pattern);
            auto words_end = std::sregex_iterator();

            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::smatch match = *i;
                std::string match_str = match.str();
                std::cout << "Request " << request_number << ": " << match_str << std::endl;
                outfile << match_str << std::endl;
            }
        }
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}

int main() {
    std::ifstream infile("list.txt");
    std::ofstream outfile("Founded_emails.txt");

    std::vector<std::string> domains;
    std::string line;
    while (std::getline(infile, line)) {
        domains.push_back(line);
    }

    int num_threads;
    std::cout << "2 core cpu max 50 | 4 Core 100 | 8 core 500 | 16 core 1000 \n Never try to be smart \n Enter the number of threads: ";
    std::cin >> num_threads;

    boost::asio::thread_pool pool(num_threads);

    int request_number = 0;
    for (const auto& domain : domains) {
        request_number++;
        int current_request_number = request_number;
        boost::asio::post(pool, [&, current_request_number]() { process_domain(domain, outfile, current_request_number); });
    }

    pool.join();

    return 0;
}

