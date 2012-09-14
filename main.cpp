#include "MetaInfo.h"
#include <string>

using namespace std;
using namespace btx;

int main(int argc, char* argv[])
{
    string metafile_name("test/test.torrent");
    MetaInfo mf(metafile_name);

    mf.parse();

    return 0;
}
