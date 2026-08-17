#include <cpr/cpr.h>
 

#include <iostream>

int main() {
    cpr::Response r = cpr::Get(cpr::Url{"https://httpbin.org"});

    std::cout << "Response body: " << r.text << std::endl;


    return 0;
}