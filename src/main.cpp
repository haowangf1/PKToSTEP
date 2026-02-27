#include "pk_to_xchg_converter.hpp"
#include <cstdio>
#include <cstdlib>

static void InitParasolid()
{
    // TODO: register frustrum functions and call PK_SESSION_start
}

static void StopParasolid()
{
    // TODO: call PK_SESSION_stop
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.x_t>\n", argv[0]);
        return 1;
    }

    InitParasolid();

    // TODO: load .x_t file via PK_PART_receive to get PK_BODY_t array

    PKToXchgConverter converter;
    converter.SetLogCallback([](const std::string& msg) {
        fprintf(stderr, "[PKToXchg] %s\n", msg.c_str());
    });

    // TODO: iterate over received bodies and call converter.Convert(body)
    // Xchg_BodyPtr xchg_body = converter.Convert(pk_body);

    StopParasolid();
    return 0;
}
