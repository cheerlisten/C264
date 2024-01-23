#include <stdio.h>
#include <string>
#include <vector>

#include <c264/source.h>
#include <log.h>
#include <annexBReader.h>

const static uint32_t BUFFER_CHUNCK_BASE_SIZE = 4096 * 2;

#ifdef CONFIG_SOURCE

extern "C" API int CreateNALSource(ISource** ppSource)
{
    if (!ppSource)
        return -1;
    *ppSource = new AnnexReader;
    return 0;
}
extern "C" API int DestroryNALSource(ISource** ppSource)
{
    if (!ppSource)
        return -1;
    delete (AnnexReader*)ppSource;
    return 0;
}
#endif