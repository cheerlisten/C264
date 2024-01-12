#include <assert.h>

#include <gtest/gtest.h>
#include <c264/c264.h>
#include <c264/source.h>

#include <utils.hpp>
#define Log(...)
TEST(Source, NAL_Parser)
{
    ISource* source;
    SourceSetting sourceSetting;
    sourceSetting.url = CMAKE_SOURCE_DIR "/../C264-Resource/Res/USA20.h264";
    sourceSetting.url = R"(C:\Users\Administrator\Videos\In\USA.h264)" ;
    sourceSetting.type = E_SOURCE_ANNEXB;

    CALL(CreateNALSource(&source));
    CALL(source->Initialize(&sourceSetting));

    BufferView bufView;
    int error = 0;
    int nalu_count = 0;
    int frame_count = 0;
    while (error == 0)
    {
        error = source->NextNAL(&bufView);

        int x = 0;
        while (bufView.buf[x++] != 0x01)
            ;
        switch (error)
        {
        case 0:
        {
            int type = bufView.buf[x] & 0x1f;
            nalu_count++;
            frame_count += type <= 5;
            Log("meet nalu:%4d f:%d type=%2d(%3x)   size=%5d    pos=%d",
                nalu_count,
                frame_count,
                type,
                uint32_t(bufView.buf[x]),
                bufView.size,
                bufView.source_pos);
            break;
        }
        case -1:
            Log("ISource::NextNAL: reach end");
            break;
        default:
            Log("ISource::NextNAL: unexpected ret val=%d", error);
            ASSERT_TRUE(error == -1 || error == 0);
        }
    }
}

int main(int argc, char** argv)
{
    testing::GTEST_FLAG(catch_exceptions) = 0;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}