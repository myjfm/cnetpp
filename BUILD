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
