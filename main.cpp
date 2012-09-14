#include <iostream>

using namespace std;

int test(int& a)
{
    a = 20;
}

int main(int argc, char* argv[])
{
    int s = 10;

    test(s);

    cout << s << endl;
}
