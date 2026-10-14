#include "../browser_detection.hpp"
#include <iostream>
int main(int argc,char** argv) {
    if (argc>1 && std::string(argv[1])=="running") {
        auto result=browser::discover_running_process();
        if(result.empty())return 4;
        std::cout<<result<<"\n";return 0;
    }
    if (argc>1 && std::string(argv[1])=="path") {
        if (argc<3)return 2;
        auto result=browser::find_in_path(argv[2]);
        if(result.empty())return 3;
        std::cout<<result<<"\n";return 0;
    }
    auto result=browser::discover();
    if(result.empty())return 3;
    std::cout<<result<<"\n";
    return 0;
}
