cc_library(
    name="cnetpp",
    srcs=[
        "src/cnetpp/base/*.cc",
        "src/cnetpp/concurrency/*.cc",
        "src/cnetpp/http/*.cc",
        "src/cnetpp/tcp/*.cc",
    ],
    incs=["src"],
    export_incs=["src"],
    deps=[
        "#pthread"
    ],
    extra_cppflags=["-Wextra -Wno-unused-local-typedefs -std=c++14"]
)

cc_binary(
    name="cnetpp_server_test",
    srcs=[
        "examples/server.cc",
    ],
    incs=[
        "src"
    ],
    deps=[
        "#pthread",
        ":cnetpp",
    ],
    extra_cppflags=["-Wextra -Wno-unused-local-typedefs -std=c++14"]
)

cc_binary(
    name="cnetpp_client_test",
    srcs=[
        "examples/client.cc",
    ],
    incs=[
        "src",
    ],
    deps=[
        "#pthread",
        ":cnetpp",
    ],
    extra_cppflags=["-Wextra -Wno-unused-local-typedefs -std=c++14"]
)
