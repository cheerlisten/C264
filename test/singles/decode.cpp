#include <assert.h>

#include <gtest/gtest.h>
#include <c264/c264.h>
#include <c264/source.h>

#include <utils.hpp>

TEST(main, decode)
{
    ISource* source;
    SourceSetting sourceSetting;
    sourceSetting.url = R"(C:/Users/Administrator/Videos/In/USA.h264)";
    sourceSetting.type = E_SOURCE_ANNEXB;

    CALL(CreateNALSource(&source));
    CALL(source->Initialize(&sourceSetting));
    BufferView bufView;

    IDecoder* decoder = nullptr;
    CALL(CreateDecoder(&decoder));
    DecoderSetting decoderSetting;
    CALL(decoder->Initialize(&decoderSetting));
    const Picture* pic = 0;
    int nalParseError = 0, decGet = 0;
    while (decGet == 0 || decGet == -1 && nalParseError == 0)
    {
        decGet = decoder->GetFrame(&pic);
        ASSERT_TRUE(decGet == 0 || decGet == -1);
        if (decGet == -1)
        {
            nalParseError = source->NextNAL(&bufView);
            if (nalParseError == 0)
                CALL(decoder->SendPacket(&bufView));
            else
            {
                Log("Send null packet for flushing");
                CALL(decoder->SendPacket(0));
            }
        }
    }
}

int main(int argc, char** argv)
{
    testing::GTEST_FLAG(catch_exceptions) = 0;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}