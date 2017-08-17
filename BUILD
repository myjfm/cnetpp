cc_library(
    name="cnetpp",
    srcs=[
        "src/base/*.cc",
        "src/concurrency/*.cc",
        "src/http/*.cc",
        "src/tcp/*.cc",
    ],
    incs=["src"],
    export_incs=["src"],
    deps=[
        "#pthread"
    ],
    extra_cppflags=["-Wextra -Wno-unused-local-typedefs -std=c++14"]
)
