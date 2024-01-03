#include <c264/c264.h>
#include <c264/source.h>
#include <assert.h>
int main()
{
    ISource * source;
    SourceSetting sourceSetting;
    sourceSetting.url = R"(C:/Users/Administrator/Videos/In/USA.h264)";
    sourceSetting.type = E_SOURCE_ANNEXB;

    CreateNALSource(&source);

    IDecoder * decoder=nullptr;
    CreateDecoder(&decoder);
    decoder->Initialize(0);

    return 0;
}