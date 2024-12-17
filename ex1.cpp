#include <map>
#include <iostream>

int main (int argc, char *argv[]) {
    std::map<int, char> m {{1,'a'},{2,'b'}};
    auto ch = m.find(2);
    std::cout << ch->second;
    return 0;
}
